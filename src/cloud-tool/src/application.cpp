#include <locale.h>
#include <QTimer>
#include <QJsonDocument>

#include <rbl_arguments_parser.h>
#include <rbl_error.h>
#include <rbl_job_manager.h>
#include <rbl_logger.h>

#include <rcl_cloud_action.h>
#include <rcl_report_record.h>
#include <rcl_cloud_session_manager.h>
#include <rcl_cloud_tool_action.h>

#include "application.h"
#include "main_task.h"

Application::Application(int &argc, char **argv)
    : QCoreApplication(argc,argv)
    , httpClient(nullptr)
    , nStartedServices(0)
{
    // Needed for printf functions family to work correctly.
    setlocale(LC_ALL,"C");

    qRegisterMetaType<RCloudAction>();

    QObject::connect(this,&Application::started,this,&Application::onStarted);
    QTimer::singleShot(0, this, SIGNAL(started()));
}

void Application::onStarted()
{
    try {
        // Process command line arguments.
        QList<RArgumentOption> validOptions;

        validOptions.append(RArgumentOption("log-file",RArgumentOption::Path,QVariant(),"Log file name",RArgumentOption::Logger,false));
        validOptions.append(RArgumentOption("log-debug",RArgumentOption::Switch,QVariant(),"Switch on debug log level",RArgumentOption::Logger,false));
        validOptions.append(RArgumentOption("log-trace",RArgumentOption::Switch,QVariant(),"Switch on trace log level",RArgumentOption::Logger,false));

        validOptions.append(RArgumentOption("address",RArgumentOption::String,QVariant("127.0.0.1"),"Server address",RArgumentOption::Optional,false));
        validOptions.append(RArgumentOption("http-port",RArgumentOption::Integer,QVariant(RCloudSessionManager::DefaultCloudSession::privatePort),"Server https port",RArgumentOption::Optional,false));

        validOptions.append(RArgumentOption("proxy-host",RArgumentOption::String,QVariant(),"Proxy host",RArgumentOption::Optional,false));
        validOptions.append(RArgumentOption("proxy-port",RArgumentOption::Integer,QVariant(80),"Proxy port",RArgumentOption::Optional,false));
        validOptions.append(RArgumentOption("proxy-user",RArgumentOption::String,QVariant(),"Proxy user",RArgumentOption::Optional,false));
        validOptions.append(RArgumentOption("proxy-password",RArgumentOption::String,QVariant(),"Proxy password",RArgumentOption::Optional,false));

        validOptions.append(RArgumentOption("private-key",RArgumentOption::String,QVariant(),"Client private key in PEM format",RArgumentOption::Optional,false));
        validOptions.append(RArgumentOption("private-key-password",RArgumentOption::String,QVariant(),"Password to client private key",RArgumentOption::Optional,false));
        validOptions.append(RArgumentOption("public-key",RArgumentOption::String,QVariant(),"Client public key in PEM format",RArgumentOption::Optional,false));
        validOptions.append(RArgumentOption("host-key",RArgumentOption::String,QVariant(),"Host or CA public key in PEM format",RArgumentOption::Optional,false));

        QMap<QString,QString> actionMap = RCloudAction::getActionMap();
        for (auto iter = actionMap.cbegin(); iter != actionMap.cend(); ++iter)
        {
            validOptions.append(RArgumentOption(iter.key(),RArgumentOption::None,QVariant(),iter.value(),RArgumentOption::Action,false));
        }

        validOptions.append(RArgumentOption(RCloudAction::Resource::Path::key,RArgumentOption::Path,QVariant(),RCloudAction::Resource::Path::description,RArgumentOption::File,false));
        validOptions.append(RArgumentOption(RCloudAction::Resource::Name::key,RArgumentOption::Path,QVariant(),RCloudAction::Resource::Name::description,RArgumentOption::File,false));
        validOptions.append(RArgumentOption(RCloudAction::Resource::Id::key,RArgumentOption::String,QVariant(),RCloudAction::Resource::Id::description,RArgumentOption::File,false));

        validOptions.append(RArgumentOption(RCloudAction::Auth::User::key,RArgumentOption::Path,QVariant(),RCloudAction::Auth::User::description,RArgumentOption::Optional,false));
        validOptions.append(RArgumentOption(RCloudAction::Auth::Token::key,RArgumentOption::Path,QVariant(),RCloudAction::Auth::Token::description,RArgumentOption::Optional,false));

        validOptions.append(RArgumentOption("json-content",RArgumentOption::String,QVariant(),"Message content in Json format",RArgumentOption::File,false));

        validOptions.append(RArgumentOption("access-mode-mask-options",RArgumentOption::Switch,QVariant(),"Print access mode mask values",RArgumentOption::Help,false));
        validOptions.append(RArgumentOption("print-actions",RArgumentOption::Switch,QVariant(),"Print url addresses",RArgumentOption::Help,false));

        RArgumentsParser argumentsParser(Application::arguments(),validOptions,false);

        if (argumentsParser.isSet("help"))
        {
            argumentsParser.printHelp();
            this->exit(0);
            R_LOG_TRACE_OUT;
            return;
        }
        if (argumentsParser.isSet("print-actions"))
        {
            QMap<QString,QString> actionMap = RCloudAction::getActionMap();
            for (auto iter = actionMap.cbegin(); iter != actionMap.cend(); ++iter)
            {
                RLogger::info("%-30s - %s\n",iter.key().toUtf8().constData(), iter.value().toUtf8().constData());
            }
            this->exit(0);
            R_LOG_TRACE_OUT;
            return;
        }
        if (argumentsParser.isSet("access-mode-mask-options"))
        {
            RLogger::info("Read    | X |   |   | X | X |   | X |\n");
            RLogger::info("Write   |   | X |   | X |   | X | X |\n");
            RLogger::info("Execute |   |   | X |   | X | X | X |\n");
            RLogger::info("--------+---+---+---+---+---+---+---+\n");
            RLogger::info("        | %1d | %1d | %1d | %1d | %1d | %1d | %1d |\n",
                          RAccessMode::R,
                          RAccessMode::W,
                          RAccessMode::X,
                          RAccessMode::RW,
                          RAccessMode::RX,
                          RAccessMode::WX,
                          RAccessMode::RWX);

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

        if (argumentsParser.isSet("log-debug"))
        {
            RLogger::getInstance().setLevel(R_LOG_LEVEL_DEBUG);
        }
        if (argumentsParser.isSet("log-trace"))
        {
            RLogger::getInstance().setLevel(R_LOG_LEVEL_TRACE);
        }
        if (argumentsParser.isSet("log-file"))
        {
            RLogger::getInstance().setFile(argumentsParser.getValue("log-file").toString());
        }

        QString address = argumentsParser.getValue("address").toString();
        uint httpPort = argumentsParser.getValue("http-port").toUInt();

        QString proxyHost = argumentsParser.getValue("proxy-host").toString();
        qint16 proxyPort = argumentsParser.getValue("proxy-port").toUInt();
        QString proxyUser = argumentsParser.getValue("proxy-user").toString();
        QString proxyPassword = argumentsParser.getValue("proxy-password").toString();

        QString clientPrivateKey = argumentsParser.getValue("private-key").toString();
        QString clientPublicKey = argumentsParser.getValue("public-key").toString();
        QString clientPassword = argumentsParser.getValue("private-key-password").toString();
        QString caPublicKey = argumentsParser.getValue("host-key").toString();

        // HTTP Client
        RHttpClientSettings httpClientSettings;
        httpClientSettings.setUrl(RHttpClient::buildUrl(address,httpPort));
        if (!proxyHost.isEmpty())
        {
            RHttpProxySettings httpProxySettings;
            httpProxySettings.setType(RHttpProxySettings::ManualProxy);
            httpProxySettings.setHost(proxyHost);
            httpProxySettings.setPort(proxyPort);
            httpProxySettings.setUser(proxyUser);
            httpProxySettings.setPassword(proxyPassword);
            httpClientSettings.setProxySettings(httpProxySettings);
        }
        if (!clientPrivateKey.isEmpty())
        {
            httpClientSettings.setTlsKeyStore(RTlsKeyStore(clientPublicKey,clientPrivateKey,clientPassword));
        }
        if (!caPublicKey.isEmpty())
        {
            httpClientSettings.setTlsTrustStore(RTlsTrustStore(caPublicKey));
        }

        this->httpClient = new RHttpClient(clientPrivateKey.isEmpty() ? RHttpClient::Public : RHttpClient::Private, httpClientSettings);

        MainTask *mainTask = new MainTask(this);
        QTimer::singleShot(0, mainTask, SLOT(run()));

        QString authUser = argumentsParser.getValue(RCloudAction::Auth::User::key).toString();
        QString authToken = argumentsParser.getValue(RCloudAction::Auth::Token::key).toString();

        if (argumentsParser.isSet(RCloudAction::Action::Test::key))
        {
            this->toolInput.addAction(RCloudToolAction::requestTest(this->httpClient, "Test request", authUser, authToken));
        }

        if (argumentsParser.isSet(RCloudAction::Action::ListFiles::key))
        {
            this->toolInput.addAction(RCloudToolAction::requestListFiles(this->httpClient, authUser, authToken));
        }

        if (argumentsParser.isSet(RCloudAction::Action::FileInfo::key))
        {
            QUuid id = argumentsParser.getValue(RCloudAction::Resource::Id::key).toUuid();
            this->toolInput.addAction(RCloudToolAction::requestFileInfo(this->httpClient, id, authUser, authToken));
        }

        if (argumentsParser.isSet(RCloudAction::Action::FileUpload::key))
        {
            QString path = argumentsParser.getValue(RCloudAction::Resource::Path::key).toString();
            QString name = argumentsParser.getValue(RCloudAction::Resource::Name::key).toString();
            this->toolInput.addAction(RCloudToolAction::requestFileUpload(this->httpClient, path, name, authUser, authToken));
        }

        if (argumentsParser.isSet(RCloudAction::Action::FileUpdate::key))
        {
            QString path = argumentsParser.getValue(RCloudAction::Resource::Path::key).toString();
            QString name = argumentsParser.getValue(RCloudAction::Resource::Name::key).toString();
            QUuid id = argumentsParser.getValue(RCloudAction::Resource::Id::key).toUuid();
            this->toolInput.addAction(RCloudToolAction::requestFileUpdate(this->httpClient, path, name, id, authUser, authToken));
        }

        if (argumentsParser.isSet(RCloudAction::Action::FileUpdateAccessOwner::key))
        {
            QUuid id = argumentsParser.getValue(RCloudAction::Resource::Id::key).toUuid();
            RAccessOwner accessOwner(RAccessOwner::fromJson(QJsonDocument::fromJson(argumentsParser.getValue("json-content").toString().toUtf8()).object()));
            this->toolInput.addAction(RCloudToolAction::requestFileUpdateAccessOwner(this->httpClient, accessOwner, id, authUser, authToken));
        }

        if (argumentsParser.isSet(RCloudAction::Action::FileUpdateAccessMode::key))
        {
            QUuid id = argumentsParser.getValue(RCloudAction::Resource::Id::key).toUuid();
            RAccessMode accessMode(RAccessMode::fromJson(QJsonDocument::fromJson(argumentsParser.getValue("json-content").toString().toUtf8()).object()));
            this->toolInput.addAction(RCloudToolAction::requestFileUpdateAccessMode(this->httpClient, accessMode, id, authUser, authToken));
        }

        if (argumentsParser.isSet(RCloudAction::Action::FileUpdateVersion::key))
        {
            QUuid id = argumentsParser.getValue(RCloudAction::Resource::Id::key).toUuid();
            RVersion version(argumentsParser.getValue("json-content").toString());
            this->toolInput.addAction(RCloudToolAction::requestFileUpdateVersion(this->httpClient, version, id, authUser, authToken));
        }

        if (argumentsParser.isSet(RCloudAction::Action::FileUpdateTags::key))
        {
            QUuid id = argumentsParser.getValue(RCloudAction::Resource::Id::key).toUuid();
            QStringList tags(argumentsParser.getValue("json-content").toString().split(','));
            this->toolInput.addAction(RCloudToolAction::requestFileUpdateTags(this->httpClient, tags, id, authUser, authToken));
        }

        if (argumentsParser.isSet(RCloudAction::Action::FileDownload::key))
        {
            QString path = argumentsParser.getValue(RCloudAction::Resource::Path::key).toString();
            QUuid id = argumentsParser.getValue(RCloudAction::Resource::Id::key).toUuid();
            this->toolInput.addAction(RCloudToolAction::requestFileDownload(this->httpClient, path, id, authUser, authToken));
        }

        if (argumentsParser.isSet(RCloudAction::Action::FileRemove::key))
        {
            QUuid id = argumentsParser.getValue(RCloudAction::Resource::Id::key).toUuid();
            this->toolInput.addAction(RCloudToolAction::requestFileRemove(this->httpClient, id, authUser, authToken));
        }

        if (argumentsParser.isSet(RCloudAction::Action::Stop::key))
        {
            this->toolInput.addAction(RCloudToolAction::requestStop(this->httpClient, authUser, authToken));
        }

        if (argumentsParser.isSet(RCloudAction::Action::Statistics::key))
        {
            this->toolInput.addAction(RCloudToolAction::requestStatistics(this->httpClient, authUser, authToken));
        }

        if (argumentsParser.isSet(RCloudAction::Action::Process::key))
        {
            RCloudProcessRequest processRequest(RCloudProcessRequest::fromJson(QJsonDocument::fromJson(argumentsParser.getValue("json-content").toString().toUtf8()).object()));
            this->toolInput.addAction(RCloudToolAction::requestProcess(this->httpClient, processRequest, authUser, authToken));
        }

        if (argumentsParser.isSet(RCloudAction::Action::ListUsers::key))
        {
            this->toolInput.addAction(RCloudToolAction::requestListUsers(this->httpClient, authUser, authToken));
        }

        if (argumentsParser.isSet(RCloudAction::Action::UserInfo::key))
        {
            QString userName = argumentsParser.getValue(RCloudAction::Resource::Name::key).toString();
            this->toolInput.addAction(RCloudToolAction::requestUserInfo(this->httpClient, userName, authUser, authToken));
        }

        if (argumentsParser.isSet(RCloudAction::Action::UserAdd::key))
        {
            QString userName = argumentsParser.getValue(RCloudAction::Resource::Name::key).toString();
            this->toolInput.addAction(RCloudToolAction::requestUserAdd(this->httpClient, userName, authUser, authToken));
        }

        if (argumentsParser.isSet(RCloudAction::Action::UserUpdate::key))
        {
            QString userName = argumentsParser.getValue(RCloudAction::Resource::Name::key).toString();
            RUserInfo userInfo(RUserInfo::fromJson(QJsonDocument::fromJson(argumentsParser.getValue("json-content").toString().toUtf8()).object()));
            this->toolInput.addAction(RCloudToolAction::requestUserUpdate(this->httpClient, userName, userInfo, authUser, authToken));
        }

        if (argumentsParser.isSet(RCloudAction::Action::UserRemove::key))
        {
            QString userName = argumentsParser.getValue(RCloudAction::Resource::Name::key).toString();
            this->toolInput.addAction(RCloudToolAction::requestUserRemove(this->httpClient, userName, authUser, authToken));
        }

        if (argumentsParser.isSet(RCloudAction::Action::UserRegister::key))
        {
            QString userName = argumentsParser.getValue(RCloudAction::Resource::Name::key).toString();
            this->toolInput.addAction(RCloudToolAction::requestUserRegister(this->httpClient, userName, authUser, authToken));
        }

        if (argumentsParser.isSet(RCloudAction::Action::ListUserTokens::key))
        {
            QString userName = argumentsParser.getValue(RCloudAction::Resource::Name::key).toString();
            this->toolInput.addAction(RCloudToolAction::requestListUserTokens(this->httpClient, userName, authUser, authToken));
        }

        if (argumentsParser.isSet(RCloudAction::Action::UserTokenGenerate::key))
        {
            QString userName = argumentsParser.getValue(RCloudAction::Resource::Name::key).toString();
            this->toolInput.addAction(RCloudToolAction::requestUserTokenGenerate(this->httpClient, userName, authUser, authToken));
        }

        if (argumentsParser.isSet(RCloudAction::Action::UserTokenRemove::key))
        {
            QUuid id = argumentsParser.getValue(RCloudAction::Resource::Id::key).toUuid();
            this->toolInput.addAction(RCloudToolAction::requestUserTokenRemove(this->httpClient, id, authUser, authToken));
        }

        if (argumentsParser.isSet(RCloudAction::Action::ListGroups::key))
        {
            this->toolInput.addAction(RCloudToolAction::requestListGroups(this->httpClient, authUser, authToken));
        }

        if (argumentsParser.isSet(RCloudAction::Action::GroupInfo::key))
        {
            QString groupName = argumentsParser.getValue(RCloudAction::Resource::Name::key).toString();
            this->toolInput.addAction(RCloudToolAction::requestGroupInfo(this->httpClient, groupName, authUser, authToken));
        }

        if (argumentsParser.isSet(RCloudAction::Action::GroupAdd::key))
        {
            QString groupName = argumentsParser.getValue(RCloudAction::Resource::Name::key).toString();
            this->toolInput.addAction(RCloudToolAction::requestGroupAdd(this->httpClient, groupName, authUser, authToken));
        }

        if (argumentsParser.isSet(RCloudAction::Action::GroupRemove::key))
        {
            QString groupName = argumentsParser.getValue(RCloudAction::Resource::Name::key).toString();
            this->toolInput.addAction(RCloudToolAction::requestGroupRemove(this->httpClient, groupName, authUser, authToken));
        }

        if (argumentsParser.isSet(RCloudAction::Action::ListActions::key))
        {
            this->toolInput.addAction(RCloudToolAction::requestListActions(this->httpClient, authUser, authToken));
        }

        if (argumentsParser.isSet(RCloudAction::Action::ActionUpdateAccessOwner::key))
        {
            QString actionName = argumentsParser.getValue(RCloudAction::Resource::Name::key).toString();
            RAccessOwner accessOwner(RAccessOwner::fromJson(QJsonDocument::fromJson(argumentsParser.getValue("json-content").toString().toUtf8()).object()));
            this->toolInput.addAction(RCloudToolAction::requestActionUpdateAccessOwner(this->httpClient, actionName, accessOwner, authUser, authToken));
        }

        if (argumentsParser.isSet(RCloudAction::Action::ActionUpdateAccessMode::key))
        {
            QString actionName = argumentsParser.getValue(RCloudAction::Resource::Name::key).toString();
            RAccessMode accessMode(RAccessMode::fromJson(QJsonDocument::fromJson(argumentsParser.getValue("json-content").toString().toUtf8()).object()));
            this->toolInput.addAction(RCloudToolAction::requestActionUpdateAccessMode(this->httpClient, actionName, accessMode, authUser, authToken));
        }

        if (argumentsParser.isSet(RCloudAction::Action::ListProcesses::key))
        {
            this->toolInput.addAction(RCloudToolAction::requestListProcesses(this->httpClient, authUser, authToken));
        }

        if (argumentsParser.isSet(RCloudAction::Action::ProcessUpdateAccessOwner::key))
        {
            QString processName = argumentsParser.getValue(RCloudAction::Resource::Name::key).toString();
            RAccessOwner accessOwner(RAccessOwner::fromJson(QJsonDocument::fromJson(argumentsParser.getValue("json-content").toString().toUtf8()).object()));
            this->toolInput.addAction(RCloudToolAction::requestActionUpdateAccessOwner(this->httpClient, processName, accessOwner, authUser, authToken));
        }

        if (argumentsParser.isSet(RCloudAction::Action::ProcessUpdateAccessMode::key))
        {
            QString processName = argumentsParser.getValue(RCloudAction::Resource::Name::key).toString();
            RAccessMode accessMode(RAccessMode::fromJson(QJsonDocument::fromJson(argumentsParser.getValue("json-content").toString().toUtf8()).object()));
            this->toolInput.addAction(RCloudToolAction::requestActionUpdateAccessMode(this->httpClient, processName, accessMode, authUser, authToken));
        }

        if (argumentsParser.isSet(RCloudAction::Action::SubmitReport::key))
        {
            RReportRecord reportRecord(RReportRecord::fromJson(QJsonDocument::fromJson(argumentsParser.getValue("json-content").toString().toUtf8()).object()));
            this->toolInput.addAction(RCloudToolAction::requestSubmitReport(this->httpClient, reportRecord, authUser, authToken));
        }
    }
    catch (const RError &error)
    {
        RLogger::error("Failed to start tool. %s\n",error.getMessage().toUtf8().constData());
        this->exit(1);
    }

    R_LOG_TRACE_OUT;
}

const RToolInput &Application::getToolInput() const
{
    return this->toolInput;
}

void Application::disconnect()
{
    bool keepWaiting = true;
    while (keepWaiting)
    {
        uint nWaiting = RJobManager::getInstance().getNWaiting();
        uint nRunning = RJobManager::getInstance().getNRunning();
        keepWaiting = (nWaiting || nRunning);
        if (keepWaiting)
        {
            RLogger::info("Waiting for %u waiting and %u running jobs to finish\n",nWaiting,nRunning);
        }
    }
    this->httpClient->deleteLater();
    this->quit();
}
