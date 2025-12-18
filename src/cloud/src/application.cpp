#include <csignal>
#include <locale.h>

#include <QDir>
#include <QTimer>
#include <QSettings>
#include <QStandardPaths>
#include <QLibraryInfo>

#include <rbl_arguments_parser.h>
#include <rbl_error.h>
#include <rbl_job_manager.h>
#include <rbl_logger.h>
#include <rbl_utils.h>

#include <rcl_cloud_session_manager.h>

#include "application.h"
#include "configuration.h"
#include "unix_signal_handler.h"

const QString Application::cloudDirectoryKey = "cloud-directory";
const QString Application::rangeCaDirectoryKey = "range-ca-directory";
const QString Application::logDebugKey = "log-debug";
const QString Application::logTraceKey = "log-trace";
const QString Application::publicHttpPortKey = "public-http-port";
const QString Application::privateHttpPortKey = "private-http-port";
const QString Application::publicKeyKey = "public-key";
const QString Application::privateKeyKey = "private-key";
const QString Application::privateKeyPasswordKey = "private-key-password";
const QString Application::caPublicKeyKey = "ca-public-key";
const QString Application::fileStoreKey = "file-store-path";
const QString Application::fileStoreMaxSizeKey = "file-store-max-size";
const QString Application::fileStoreMaxFileSizeKey = "file-store-max-file-size";
const QString Application::printSettingsKey = "print-settings";
const QString Application::storeSettingsKey = "store-settings";

Application::Application(int &argc, char **argv) :
    QCoreApplication(argc,argv),
    stopTimeout(1000),
    publicHttpServer(nullptr),
    privateHttpServer(nullptr),
    fileManager(nullptr),
    mailer(nullptr),
    actionHandler(nullptr),
    nStartedServices(0)
{
    R_LOG_TRACE_IN;

    this->publicHttpServiceIsReady = false;
    this->privateHttpServiceIsReady = false;
    this->fileServiceIsReady = false;
    this->mailerServiceIsReady = false;

    // Needed for printf functions cloud to work correctly.
    setlocale(LC_ALL,"C");

    QCoreApplication::setOrganizationName(RVendor::familyName());
    QCoreApplication::setOrganizationDomain(RVendor::wwwDomain());
    QCoreApplication::setApplicationName(RVendor::shortName());

    qRegisterMetaType<RCloudAction>();

    QObject::connect(this,&Application::started,this,&Application::onStarted);
    QObject::connect(this,&Application::stopped,this,&Application::onStopped);
    QTimer::singleShot(0, this, SIGNAL(started()));
}

void Application::serviceStarted()
{
    R_LOG_TRACE_IN;
    this->nStartedServices++;
    RLogger::info("[Application] Number of services running = %u.\n",this->nStartedServices);
    R_LOG_TRACE_OUT;
}

void Application::serviceStopped(bool failed)
{
    R_LOG_TRACE_IN;
    this->nStartedServices--;
    RLogger::info("[Application] Number of services still running = %u.\n",this->nStartedServices);
    if (failed)
    {
        R_LOG_TRACE_MESSAGE("Application::exit(1)");
        this->exit(1);
    }
    else
    {
        if (this->nStartedServices == 0)
        {
            QTimer::singleShot(this->stopTimeout, this, SIGNAL(stopped()));
        }
    }
    R_LOG_TRACE_OUT;
}

bool Application::createDirectories(const Configuration &configuration)
{
    R_LOG_TRACE_IN;
    RLogger::info("[Application] Create directory structure\n");
    RLogger::indent();
    bool success = (Application::createDirectory(configuration.getLogDirectoryPath()) &&
                    Application::createDirectory(configuration.getConfigurationDirectoryPath()) &&
                    Application::createDirectory(configuration.getCertificateDirectoryPath()) &&
                    Application::createDirectory(configuration.getServerCertificateDirectoryPath()) &&
                    Application::createDirectory(configuration.getCaCertificateDirectoryPath()) &&
                    Application::createDirectory(configuration.getVariableDirectoryPath()) &&
                    Application::createDirectory(configuration.getProcessesDirectoryPath()) &&
                    Application::createDirectory(configuration.getReportsDirectoryPath()));
    if (!success)
    {
        RLogger::error("[Application] Failed to create directory structure\n");
    }
    RLogger::unindent();
    R_LOG_TRACE_RETURN(success);
}

bool Application::createDirectory(const QString &directoryPath)
{
    R_LOG_TRACE_IN;
    RLogger::info("[Application] Creating directory \"%s\"\n", directoryPath.toUtf8().constData());
    if (!QDir(directoryPath).mkpath("."))
    {
        RLogger::error("[Application] Failed to create directory \"%s\"\n", directoryPath.toUtf8().constData());
        R_LOG_TRACE_RETURN(false);
    }
    R_LOG_TRACE_RETURN(true);
}

void Application::printReadiness() const
{
    if (this->publicHttpServiceIsReady &&
        this->privateHttpServiceIsReady &&
        this->fileServiceIsReady &&
        this->mailerServiceIsReady)
    {
        RLogger::info("[Application] All services are ready.\n");
    }
}

void Application::onStarted()
{
    RLogger::getInstance().setPrintTimeEnabled(true);

    try {
        // Process command line arguments.
        QList<RArgumentOption> validOptions;

        validOptions.append(RArgumentOption(Application::cloudDirectoryKey,RArgumentOption::Path,Configuration::getDefaultCloudDirectoryPath(),"Path to cloud data direcory.",RArgumentOption::File,false));
        validOptions.append(RArgumentOption(Application::rangeCaDirectoryKey,RArgumentOption::Path,QString(),"Path to Range CA direcory.",RArgumentOption::File,false));

        validOptions.append(RArgumentOption(Application::logDebugKey,RArgumentOption::Switch,QVariant(),"Switch on debug log level",RArgumentOption::Logger,false));
        validOptions.append(RArgumentOption(Application::logTraceKey,RArgumentOption::Switch,QVariant(),"Switch on trace log level",RArgumentOption::Logger,false));

        validOptions.append(RArgumentOption(Application::publicHttpPortKey,RArgumentOption::Integer,Configuration::getDefaultPublicHttpPort(),"Public HTTP Server port",RArgumentOption::Optional,false));
        validOptions.append(RArgumentOption(Application::privateHttpPortKey,RArgumentOption::Integer,Configuration::getDefaultPrivateHttpPort(),"Private HTTP Server port",RArgumentOption::Optional,false));

        validOptions.append(RArgumentOption(Application::privateKeyKey,RArgumentOption::Path,QString(),"Host private key in PEM format",RArgumentOption::Optional,false));
        validOptions.append(RArgumentOption(Application::privateKeyPasswordKey,RArgumentOption::String,QString(),"Password to host private key",RArgumentOption::Optional,false));
        validOptions.append(RArgumentOption(Application::publicKeyKey,RArgumentOption::Path,QString(),"Host public key in PEM format",RArgumentOption::Optional,false));
        validOptions.append(RArgumentOption(Application::caPublicKeyKey,RArgumentOption::Path,QString(),"Client or CA public key in PEM format",RArgumentOption::Optional,false));

        validOptions.append(RArgumentOption(Application::fileStoreKey,RArgumentOption::Path,QString(),"Path to file store direcory.",RArgumentOption::File,false));
        validOptions.append(RArgumentOption(Application::fileStoreMaxSizeKey,RArgumentOption::Path,Configuration::getDefaultFileStoreMaxSize(),"Maximum file store size.",RArgumentOption::Optional,false));
        validOptions.append(RArgumentOption(Application::fileStoreMaxFileSizeKey,RArgumentOption::Path,Configuration::getDefaultFileStoreMaxFileSize(),"Maximum file size in file store.",RArgumentOption::Optional,false));

        validOptions.append(RArgumentOption(Application::printSettingsKey,RArgumentOption::Switch,QVariant(),"Print settings and exit",RArgumentOption::Optional,false));
        validOptions.append(RArgumentOption(Application::storeSettingsKey,RArgumentOption::Switch,QVariant(),"Store settings and exit",RArgumentOption::Optional,false));

        RArgumentsParser argumentsParser(Application::arguments(),validOptions,false);

        if (argumentsParser.isSet(Application::logDebugKey))
        {
            RLogger::getInstance().setLevel(R_LOG_LEVEL_DEBUG);
        }
        if (argumentsParser.isSet(Application::logTraceKey))
        {
            RLogger::getInstance().setLevel(R_LOG_LEVEL_TRACE);
        }

        if (argumentsParser.isSet("help"))
        {
            argumentsParser.printHelp();
            this->exit(0);
            R_LOG_TRACE_OUT;
            return;
        }

        if (argumentsParser.isSet("version"))
        {
            argumentsParser.printVersion();
            this->exit(0);
            R_LOG_TRACE_OUT;
            return;
        }

        QString cloudDirectory = Configuration::getDefaultCloudDirectoryPath();

        if (argumentsParser.isSet(Application::cloudDirectoryKey))
        {
            cloudDirectory = argumentsParser.getValue(Application::cloudDirectoryKey).toString();
        }

        Configuration configuration(cloudDirectory);

        if (argumentsParser.isSet(Application::rangeCaDirectoryKey))
        {
            configuration.setRangeCaDirectory(argumentsParser.getValue(Application::rangeCaDirectoryKey).toString());
        }
        if (argumentsParser.isSet(Application::publicHttpPortKey))
        {
            configuration.setPublicHttpPort(argumentsParser.getValue(Application::publicHttpPortKey).toUInt());
        }
        if (argumentsParser.isSet(Application::privateHttpPortKey))
        {
            configuration.setPrivateHttpPort(argumentsParser.getValue(Application::privateHttpPortKey).toUInt());
        }
        if (argumentsParser.isSet(Application::publicKeyKey))
        {
            configuration.setPublicKey(argumentsParser.getValue(Application::publicKeyKey).toString());
        }
        if (argumentsParser.isSet(Application::privateKeyKey))
        {
            configuration.setPrivateKey(argumentsParser.getValue(Application::privateKeyKey).toString());
        }
        if (argumentsParser.isSet(Application::privateKeyPasswordKey))
        {
            configuration.setPrivateKeyPassword(argumentsParser.getValue(Application::privateKeyPasswordKey).toString());
        }
        if (argumentsParser.isSet(Application::caPublicKeyKey))
        {
            configuration.setCaPublicKey(argumentsParser.getValue(Application::caPublicKeyKey).toString());
        }
        if (argumentsParser.isSet(Application::fileStoreKey))
        {
            configuration.setFileStore(argumentsParser.getValue(Application::fileStoreKey).toString());
        }
        if (argumentsParser.isSet(Application::fileStoreMaxSizeKey))
        {
            configuration.setFileStoreMaxSize(argumentsParser.getValue(Application::fileStoreMaxSizeKey).toLongLong());
        }
        if (argumentsParser.isSet(Application::fileStoreMaxFileSizeKey))
        {
            configuration.setFileStoreMaxFileSize(argumentsParser.getValue(Application::fileStoreMaxFileSizeKey).toLongLong());
        }

        if (argumentsParser.isSet(Application::printSettingsKey))
        {
            RLogger::info("[Application] Current application settings\n");
            configuration.print();
            this->exit(0);
            R_LOG_TRACE_OUT;
            return;
        }

        this->createDirectories(configuration);

        configuration.sync();

        if (argumentsParser.isSet(Application::storeSettingsKey))
        {
            this->exit(0);
            R_LOG_TRACE_OUT;
            return;
        }

        RLogger::getInstance().setFile(configuration.getLogFilePath());

        RLogger::info("[Application] Qt version: %s\n", QLibraryInfo::version().toString().toUtf8().constData());
        RLogger::info("[Application] Qt plugins paths: %s\n", QLibraryInfo::paths(QLibraryInfo::PluginsPath).join(",").toUtf8().constData());
        RLogger::info("[Application] Qt libraries paths: %s\n", QLibraryInfo::paths(QLibraryInfo::LibrariesPath).join(",").toUtf8().constData());
        RLogger::info("[Application] Qt library executables paths: %s\n", QLibraryInfo::paths(QLibraryInfo::LibraryExecutablesPath).join(",").toUtf8().constData());
        RLogger::info("[Application] Qt core application library paths: %s\n", QCoreApplication::libraryPaths().join(",").toUtf8().constData());
        RLogger::info("[Application] Qt ssl support: %s\n",QSslSocket::supportsSsl()?"true":"false");
        RLogger::info("[Application] Qt ssl active backend: %s\n", QSslSocket::activeBackend().toUtf8().constData());
        RLogger::info("[Application] Qt ssl available backends: %s\n",QSslSocket::availableBackends().join(",").toUtf8().constData());

        RLogger::info("[Application] Starting services\n");
        RLogger::info("[Application] Ideal thread count: %d\n",QThread::idealThreadCount());

        // User manager service
        UserManagerSettings userManagerSettings;
        userManagerSettings.setFileName(configuration.getUsersFilePath());

        this->userManager = new UserManager(userManagerSettings,this);

        // Action manager service
        ActionManagerSettings actionManagerSettings;
        actionManagerSettings.setFileName(configuration.getActionsFilePath());

        this->actionManager = new ActionManager(actionManagerSettings,this);

        // Process manager service
        ProcessManagerSettings processManagerSettings;
        processManagerSettings.setLogDirectory(configuration.getLogDirectoryPath());
        processManagerSettings.setWorkingDirectory(configuration.getVariableDirectoryPath());
        processManagerSettings.setProcessesDirectory(configuration.getProcessesDirectoryPath());
        processManagerSettings.setProcessesFileName(configuration.getProcessesFilePath());
        processManagerSettings.setRangeCaDirectory(configuration.getRangeCaDirectory());

        this->processManager = new ProcessManager(processManagerSettings,this);

        // File manager service
        FileManagerSettings fileManagerSettings;
        fileManagerSettings.setFileStore(configuration.getFileStore());
        fileManagerSettings.setMaxStoreSize(configuration.getFileStoreMaxSize());
        fileManagerSettings.setMaxFileSize(configuration.getFileStoreMaxFileSize());

        this->fileManager = new FileManager(fileManagerSettings,this->userManager);
        QObject::connect(this->fileManager, &FileManager::ready, this, &Application::fileServiceReady);
        QObject::connect(this->fileManager, &FileManager::started, this, &Application::fileServiceStarted);
        QObject::connect(this->fileManager, &FileManager::finished, this, &Application::fileServiceFinished);
        QObject::connect(this->fileManager, &FileManager::failed, this, &Application::fileServiceFailed);

        // Report manager service
        ReportManagerSettings reportManagerSettings;
        reportManagerSettings.setReportDirectory(configuration.getReportsDirectoryPath());
        reportManagerSettings.setMaxReportLength(configuration.getMaxReportLength());
        reportManagerSettings.setMaxCommentLength(configuration.getMaxCommentLength());

        this->reportManager = new ReportManager(reportManagerSettings,this);

        // Mailer service.
        MailerSettings mailerSettings;
        mailerSettings.setFromAddress(configuration.getSenderEmailAddress());

        this->mailer = new Mailer(mailerSettings);
        QObject::connect(this->mailer, &Mailer::ready, this, &Application::mailerServiceReady);
        QObject::connect(this->mailer, &Mailer::started, this, &Application::mailerServiceStarted);
        QObject::connect(this->mailer, &Mailer::finished, this, &Application::mailerServiceFinished);
        QObject::connect(this->mailer, &Mailer::failed, this, &Application::mailerServiceFailed);

        // Public HTTP service
        RHttpServerSettings publicHttpServerSettings;
        publicHttpServerSettings.setPort(configuration.getPublicHttpPort());
        publicHttpServerSettings.setTlsKeyStore(RTlsKeyStore(configuration.getPublicKey(),
                                                             configuration.getPrivateKey(),
                                                             configuration.getPrivateKeyPassword()));
        publicHttpServerSettings.setTlsTrustStore(RTlsTrustStore(configuration.getCaPublicKey()));
        publicHttpServerSettings.setRateLimitPerSecond(configuration.getRateLimitPerSecond());

        this->publicHttpServer = new RHttpServer(RHttpServer::Public,publicHttpServerSettings);
        this->publicHttpServer->setAuthTokenValidator(this->userManager->getAuthTokenValidator());

        QObject::connect(this->publicHttpServer, &RHttpServer::ready, this, &Application::publicHttpServiceReady);
        QObject::connect(this->publicHttpServer, &RHttpServer::started, this, &Application::publicHttpServiceStarted);
        QObject::connect(this->publicHttpServer, &RHttpServer::finished, this, &Application::publicHttpServiceFinished);
        QObject::connect(this->publicHttpServer, &RHttpServer::failed, this, &Application::publicHttpServiceFailed);
        QObject::connect(this->publicHttpServer, &RHttpServer::requestAvailable, this, &Application::networkMessageAvailable);

        this->publicHttpServer->start();

        // Private HTTP service
        RHttpServerSettings privateHttpServerSettings;
        privateHttpServerSettings.setPort(configuration.getPrivateHttpPort());
        privateHttpServerSettings.setTlsKeyStore(RTlsKeyStore(configuration.getPublicKey(),
                                                              configuration.getPrivateKey(),
                                                              configuration.getPrivateKeyPassword()));
        privateHttpServerSettings.setTlsTrustStore(RTlsTrustStore(configuration.getCaPublicKey()));
        privateHttpServerSettings.setRateLimitPerSecond(configuration.getRateLimitPerSecond());

        this->privateHttpServer = new RHttpServer(RHttpServer::Private,privateHttpServerSettings);

        QObject::connect(this->privateHttpServer, &RHttpServer::ready, this, &Application::privateHttpServiceReady);
        QObject::connect(this->privateHttpServer, &RHttpServer::started, this, &Application::privateHttpServiceStarted);
        QObject::connect(this->privateHttpServer, &RHttpServer::finished, this, &Application::privateHttpServiceFinished);
        QObject::connect(this->privateHttpServer, &RHttpServer::failed, this, &Application::privateHttpServiceFailed);
        QObject::connect(this->privateHttpServer, &RHttpServer::requestAvailable, this, &Application::networkMessageAvailable);

        this->privateHttpServer->start();

        // Action handler
        this->actionHandler = new ActionHandler(this->userManager,
                                                this->actionManager,
                                                this->processManager,
                                                this->fileManager,
                                                this->reportManager,
                                                this->mailer,
                                                this);
        QObject::connect(this->actionHandler, &ActionHandler::resolved, this, &Application::actionResolved);

        RJobManager::getInstance().submit(this->fileManager);
        RJobManager::getInstance().submit(this->mailer);

        QObject::connect(new UnixSignalHandler(SIGTERM, this), &UnixSignalHandler::raised, this, &Application::shutdown);
        QObject::connect(new UnixSignalHandler(SIGINT, this), &UnixSignalHandler::raised, this, &Application::shutdown);
    }
    catch (const RError &error)
    {
        RLogger::error("[Application] Failed to start. %s\n",error.getMessage().toUtf8().constData());
        this->exit(1);
    }
    catch (const std::exception &e)
    {
        RLogger::error("[Application] Failed to start. %s\n", e.what());
        this->exit(1);
    }
    catch (...)
    {
        RLogger::error("[Application] Failed to start.\n");
        this->exit(1);
    }

    RLogger::info("[Application] Started\n");
    R_LOG_TRACE_OUT;
}

void Application::onStopped()
{
    R_LOG_TRACE_MESSAGE("Application::quit()");
    this->quit();
}

void Application::publicHttpServiceReady()
{
    R_LOG_TRACE_IN;
    RLogger::info("[Application] Public HTTP service is ready.\n");
    this->publicHttpServiceIsReady = true;
    R_LOG_TRACE_OUT;
}

void Application::publicHttpServiceStarted()
{
    R_LOG_TRACE_IN;
    RLogger::info("[Application] Public HTTP service has started.\n");
    this->serviceStarted();
    this->printReadiness();
    R_LOG_TRACE_OUT;
}

void Application::publicHttpServiceFinished()
{
    R_LOG_TRACE_IN;
    RLogger::info("[Application] Public HTTP service has finished.\n");
    this->publicHttpServiceIsReady = false;
    this->serviceStopped(false);
    R_LOG_TRACE_OUT;
}

void Application::publicHttpServiceFailed()
{
    R_LOG_TRACE_IN;
    RLogger::info("[Application] Public HTTP service has failed.\n");
    this->serviceStopped(true);
    R_LOG_TRACE_OUT;
}

void Application::privateHttpServiceReady()
{
    R_LOG_TRACE_IN;
    RLogger::info("[Application] Private HTTP service is ready.\n");
    this->privateHttpServiceIsReady = true;
    this->printReadiness();
    R_LOG_TRACE_OUT;
}

void Application::privateHttpServiceStarted()
{
    R_LOG_TRACE_IN;
    RLogger::info("[Application] Private HTTP service has started.\n");
    this->serviceStarted();
    R_LOG_TRACE_OUT;
}

void Application::privateHttpServiceFinished()
{
    R_LOG_TRACE_IN;
    RLogger::info("[Application] Private HTTP service has finished.\n");
    this->privateHttpServiceIsReady = false;
    this->serviceStopped(false);
    R_LOG_TRACE_OUT;
}

void Application::privateHttpServiceFailed()
{
    R_LOG_TRACE_IN;
    RLogger::info("[Application] Private HTTP service has failed.\n");
    this->serviceStopped(true);
    R_LOG_TRACE_OUT;
}

void Application::fileServiceReady()
{
    R_LOG_TRACE_IN;
    RLogger::info("[Application] File service is ready.\n");
    this->fileServiceIsReady = true;
    this->printReadiness();
    R_LOG_TRACE_OUT;
}

void Application::fileServiceStarted()
{
    R_LOG_TRACE_IN;
    RLogger::info("[Application] File service has started.\n");
    this->serviceStarted();
    R_LOG_TRACE_OUT;
}

void Application::fileServiceFinished()
{
    R_LOG_TRACE_IN;
    RLogger::info("[Application] File service has finished.\n");
    this->fileServiceIsReady = false;
    this->serviceStopped(false);
    R_LOG_TRACE_OUT;
}

void Application::fileServiceFailed()
{
    R_LOG_TRACE_IN;
    RLogger::info("[Application] File service has failed.\n");
    this->serviceStopped(true);
    R_LOG_TRACE_OUT;
}

void Application::mailerServiceReady()
{
    R_LOG_TRACE_IN;
    RLogger::info("[Application] Mailer service is ready.\n");
    this->mailerServiceIsReady = true;
    this->printReadiness();
    R_LOG_TRACE_OUT;
}

void Application::mailerServiceStarted()
{
    R_LOG_TRACE_IN;
    RLogger::info("[Application] Mailer service has started.\n");
    this->serviceStarted();
    R_LOG_TRACE_OUT;
}

void Application::mailerServiceFinished()
{
    R_LOG_TRACE_IN;
    RLogger::info("[Application] Mailer service has finished.\n");
    this->mailerServiceIsReady = false;
    this->serviceStopped(false);
    R_LOG_TRACE_OUT;
}

void Application::mailerServiceFailed()
{
    R_LOG_TRACE_IN;
    RLogger::info("[Application] Mailer service has failed.\n");
    this->serviceStopped(true);
    R_LOG_TRACE_OUT;
}

void Application::networkMessageAvailable(const RNetworkMessage &networkMessage)
{
    R_LOG_TRACE_IN;
    RCloudAction action(networkMessage.toAction());
    RLogger::debug("[Application] Registering private action ID: \"%s\" (%s)\n",
                   action.getId().toString(QUuid::WithoutBraces).toUtf8().constData(),
                   action.getAction().toUtf8().constData());
    this->actionToMessageMap.insert(action.getId(),networkMessage);
    this->actionHandler->resolveAction(action,QString("%1@%2").arg(networkMessage.getOwner(),networkMessage.getFrom()));
    R_LOG_TRACE_OUT;
}

void Application::actionResolved(const RCloudAction &action)
{
    R_LOG_TRACE_IN;
    RLogger::debug("[Application] Resolving action ID: \"%s\" error type: \"%d - %s\".\n",
                   action.getId().toString(QUuid::WithoutBraces).toUtf8().constData(),
                   action.getErrorType(),
                   RError::getTypeMessage(action.getErrorType()).toUtf8().constData());
    if (!this->actionToMessageMap.contains(action.getId()))
    {
        RLogger::error("[Application] Unknown action ID: \"%s\"\n",action.getId().toString(QUuid::WithoutBraces).toUtf8().constData());
        R_LOG_TRACE_OUT;
        return;
    }

    RHttpMessage message = this->actionToMessageMap.value(action.getId()).toReply(action);
    if (this->publicHttpServer->containsServerHandlerId(message.getHandlerId()))
    {
        RLogger::debug("[Application] Send public response.\n");
        this->publicHttpServer->sendMessageReply(message);
    }
    else if (this->privateHttpServer->containsServerHandlerId(message.getHandlerId()))
    {
        RLogger::debug("[Application] Send private response.\n");
        this->privateHttpServer->sendMessageReply(message);
    }
    else
    {
        RLogger::error("[Application] Unknown handler for action ID: \"%s\"\n",action.getId().toString(QUuid::WithoutBraces).toUtf8().constData());
        R_LOG_TRACE_OUT;
        return;
    }
    RLogger::debug("[Application] Remove message from action map.\n");
    this->actionToMessageMap.remove(action.getId());

    // Postprocess stop action.
    if (action.getAction() == RCloudAction::Action::Stop::key)
    {
        this->shutdown();
    }
    R_LOG_TRACE_OUT;
}

void Application::shutdown()
{
    R_LOG_TRACE_IN;
    this->fileManager->stop();
    this->mailer->stop();
    this->publicHttpServer->stop();
    this->privateHttpServer->stop();

    RLogger::info("[Application] Waiting for all waiting jobs to finish.\n");
    while (RJobManager::getInstance().getNWaiting() > 0)
    {
        QThread::msleep(10);
    }
    RLogger::info("[Application] Waiting for all running jobs to finish.\n");
    while (RJobManager::getInstance().getNRunning() > 0)
    {
        QThread::msleep(10);
    }

    this->userManager->writeFile();
    this->actionManager->writeFile();
    this->processManager->writeFile();
    R_LOG_TRACE_OUT;
}
