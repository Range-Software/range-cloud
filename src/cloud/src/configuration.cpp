#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QJsonDocument>

#include <rbl_error.h>
#include <rbl_logger.h>
#include <rbl_utils.h>

#include <rcl_cloud_session_manager.h>
#include <rcl_http_server_settings.h>
#include <rcl_report_record.h>

#include "configuration.h"

const QString Configuration::cloudDirectoryBase = "range-cloud";
const QString Configuration::rangeCaDirectoryBase = "range-ca";
const QString Configuration::storeDirectoryBase = "store";
const QString Configuration::configurationDirectoryBase = "etc";
const QString Configuration::certificateDirectoryBase = "cert";
const QString Configuration::serverCertificateDirectoryBase = "server";
const QString Configuration::caCertificateDirectoryBase = "ca";
const QString Configuration::logDirectoryBase = "log";
const QString Configuration::variableDirectoryBase = "var";
const QString Configuration::processesDirectoryBase = "processes";
const QString Configuration::reportsDirectoryBase = "reports";

const QString Configuration::configurationFileBase = "configuration.json";
const QString Configuration::actionsFileBase = "actions.json";
const QString Configuration::processesFileBase = "processes.json";
const QString Configuration::usersFileBase = "users.json";
const QString Configuration::logFileBase = QString("%1.log").arg(RVendor::shortName());

void Configuration::_init(const Configuration *pConfiguration)
{
    if (pConfiguration)
    {
        this->cloudDirectory = pConfiguration->cloudDirectory;
        this->rangeCaDirectory = pConfiguration->rangeCaDirectory;
        this->publicHttpPort = pConfiguration->publicHttpPort;
        this->privateHttpPort = pConfiguration->privateHttpPort;
        this->rateLimitPerSecond = pConfiguration->rateLimitPerSecond;
        this->publicKey = pConfiguration->publicKey;
        this->privateKey = pConfiguration->privateKey;
        this->privateKeyPassword = pConfiguration->privateKeyPassword;
        this->caPublicKey = pConfiguration->caPublicKey;
        this->fileStore = pConfiguration->fileStore;
        this->fileStoreMaxSize = pConfiguration->fileStoreMaxSize;
        this->fileStoreMaxFileSize = pConfiguration->fileStoreMaxFileSize;
        this->maxReportLength = pConfiguration->maxReportLength;
        this->maxCommentLength = pConfiguration->maxCommentLength;
        this->senderEmailAddress = pConfiguration->senderEmailAddress;
    }
}

Configuration::Configuration(const QString &cloudDirectory)
    : cloudDirectory{cloudDirectory}
    , rangeCaDirectory{Configuration::getDefaultRangeCaDirectoryPath()}
    , publicHttpPort{Configuration::getDefaultPublicHttpPort()}
    , privateHttpPort{Configuration::getDefaultPrivateHttpPort()}
    , rateLimitPerSecond{Configuration::getDefaultRateLimitPerSecond()}
    , publicKey{Configuration::getDefaultPublicKeyPath(this->cloudDirectory)}
    , privateKey{Configuration::getDefaultPrivateKeyPath(this->cloudDirectory)}
    , privateKeyPassword{QString()}
    , caPublicKey{Configuration::getDefaultCaPublicKeyPath(this->cloudDirectory)}
    , fileStore{Configuration::getDefaultFileStorePath(this->cloudDirectory)}
    , fileStoreMaxSize{Configuration::getDefaultFileStoreMaxSize()}
    , fileStoreMaxFileSize{Configuration::getDefaultFileStoreMaxFileSize()}
    , maxReportLength{Configuration::getDefaultMaxReportLength()}
    , maxCommentLength{Configuration::getDefaultMaxCommentLength()}
    , senderEmailAddress{Configuration::getDefaultSenderEmailAddress()}
{
    this->_init();
    QString configurationFile = Configuration::buildConfigurationFilePath(this->cloudDirectory);
    if (!QFileInfo::exists(configurationFile))
    {
        RLogger::warning("[Configuration] Requested configuration file \"%s\" does not exist.\n",configurationFile.toUtf8().constData());
    }
    else
    {
        this->readFile(configurationFile);
    }
}

Configuration::Configuration(const Configuration &configuration)
{
    this->_init(&configuration);
}

Configuration &Configuration::operator =(const Configuration &configuration)
{
    this->_init(&configuration);
    return (*this);
}

const QString &Configuration::getRangeCaDirectory() const
{
    return this->rangeCaDirectory;
}

void Configuration::setRangeCaDirectory(const QString &rangeCaDirectory)
{
    this->rangeCaDirectory = rangeCaDirectory;
}

uint Configuration::getPublicHttpPort() const
{
    return this->publicHttpPort;
}

void Configuration::setPublicHttpPort(uint publicHttpPort)
{
    this->publicHttpPort = publicHttpPort;
}

uint Configuration::getPrivateHttpPort() const
{
    return this->privateHttpPort;
}

void Configuration::setPrivateHttpPort(uint privateHttpPort)
{
    this->privateHttpPort = privateHttpPort;
}

quint32 Configuration::getRateLimitPerSecond() const
{
    return this->rateLimitPerSecond;
}

void Configuration::setRateLimitPerSecond(quint32 rateLimitPerSecond)
{
    this->rateLimitPerSecond = rateLimitPerSecond;
}

const QString &Configuration::getPublicKey() const
{
    return this->publicKey;
}

void Configuration::setPublicKey(const QString &publicKey)
{
    this->publicKey = publicKey;
}

const QString &Configuration::getPrivateKey() const
{
    return this->privateKey;
}

void Configuration::setPrivateKey(const QString &privateKey)
{
    this->privateKey = privateKey;
}

const QString &Configuration::getPrivateKeyPassword() const
{
    return this->privateKeyPassword;
}

void Configuration::setPrivateKeyPassword(const QString &privateKeyPassword)
{
    this->privateKeyPassword = privateKeyPassword;
}

const QString &Configuration::getCaPublicKey() const
{
    return this->caPublicKey;
}

void Configuration::setCaPublicKey(const QString &caPublicKey)
{
    this->caPublicKey = caPublicKey;
}

const QString &Configuration::getFileStore() const
{
    return this->fileStore;
}

void Configuration::setFileStore(const QString &fileStore)
{
    this->fileStore = fileStore;
}

qint64 Configuration::getFileStoreMaxSize() const
{
    return this->fileStoreMaxSize;
}

void Configuration::setFileStoreMaxSize(qint64 fileStoreMaxSize)
{
    this->fileStoreMaxSize = fileStoreMaxSize;
}

qint64 Configuration::getFileStoreMaxFileSize() const
{
    return this->fileStoreMaxFileSize;
}

void Configuration::setFileStoreMaxFileSize(qint64 fileStoreMaxFileSize)
{
    this->fileStoreMaxFileSize = fileStoreMaxFileSize;
}

qint64 Configuration::getMaxReportLength() const
{
    return this->maxReportLength;
}

void Configuration::setMaxReportLength(qint64 maxReportLength)
{
    this->maxReportLength = maxReportLength;
}

qint64 Configuration::getMaxCommentLength() const
{
    return this->maxCommentLength;
}

void Configuration::setMaxCommentLength(qint64 maxCommentLength)
{
    this->maxCommentLength = maxCommentLength;
}

const QString &Configuration::getSenderEmailAddress() const
{
    return this->senderEmailAddress;
}

void Configuration::setSenderEmailAddress(const QString &senderEmailAddress)
{
    this->senderEmailAddress = senderEmailAddress;
}

QString Configuration::getLogDirectoryPath() const
{
    return Configuration::buildLogDirectoryPath(this->cloudDirectory);
}

QString Configuration::getConfigurationDirectoryPath() const
{
    return Configuration::buildConfigurationDirectoryPath(this->cloudDirectory);
}

QString Configuration::getCertificateDirectoryPath() const
{
    return Configuration::buildCertificateDirectoryPath(this->cloudDirectory);
}

QString Configuration::getServerCertificateDirectoryPath() const
{
    return Configuration::buildServerCertificateDirectoryPath(this->cloudDirectory);
}

QString Configuration::getCaCertificateDirectoryPath() const
{
    return Configuration::buildCaCertificateDirectoryPath(this->cloudDirectory);
}

QString Configuration::getVariableDirectoryPath() const
{
    return Configuration::buildVariableDirectoryPath(this->cloudDirectory);
}

QString Configuration::getProcessesDirectoryPath() const
{
    return Configuration::buildProcessesDirectoryPath(this->cloudDirectory);
}

QString Configuration::getReportsDirectoryPath() const
{
    return Configuration::buildReportsDirectoryPath(this->cloudDirectory);
}

QString Configuration::getUsersFilePath() const
{
    return Configuration::buildUsersFilePath(this->cloudDirectory);
}

QString Configuration::getActionsFilePath() const
{
    return Configuration::buildActionsFilePath(this->cloudDirectory);
}

QString Configuration::getProcessesFilePath() const
{
    return Configuration::buildProcessesFilePath(this->cloudDirectory);
}

QString Configuration::getLogFilePath() const
{
    return Configuration::buildLogFilePath(this->cloudDirectory);
}

void Configuration::print() const
{
    RLogger::info("\n%s\n", QJsonDocument(this->toJson()).toJson().constData());
}

void Configuration::sync() const
{
    this->writeFile(Configuration::buildConfigurationFilePath(this->cloudDirectory));
}

void Configuration::fromJson(const QJsonObject &json)
{
    if (const QJsonValue &v = json["cloudDirectory"]; v.isString())
    {
        this->cloudDirectory = v.toString();
    }
    if (const QJsonValue &v = json["rangeCaDirectory"]; v.isString())
    {
        this->rangeCaDirectory = v.toString();
    }
    if (const QJsonValue &v = json["publicHttpPort"]; v.isString())
    {
        this->publicHttpPort = v.toString().toUInt();
    }
    if (const QJsonValue &v = json["privateHttpPort"]; v.isString())
    {
        this->privateHttpPort = v.toString().toUInt();
    }
    if (const QJsonValue &v = json["rateLimitPerSecond"]; v.isString())
    {
        this->rateLimitPerSecond = v.toString().toUInt();
    }
    if (const QJsonValue &v = json["publicKey"]; v.isString())
    {
        this->publicKey = v.toString();
    }
    if (const QJsonValue &v = json["privateKey"]; v.isString())
    {
        this->privateKey = v.toString();
    }
    if (const QJsonValue &v = json["privateKeyPassword"]; v.isString())
    {
        this->privateKeyPassword = v.toString();
    }
    if (const QJsonValue &v = json["caPublicKey"]; v.isString())
    {
        this->caPublicKey = v.toString();
    }
    if (const QJsonValue &v = json["fileStore"]; v.isString())
    {
        this->fileStore = v.toString();
    }
    if (const QJsonValue &v = json["fileStoreMaxSize"]; v.isString())
    {
        this->fileStoreMaxSize = v.toString().toLongLong();
    }
    if (const QJsonValue &v = json["fileStoreMaxFileSize"]; v.isString())
    {
        this->fileStoreMaxFileSize = v.toString().toLongLong();
    }
    if (const QJsonValue &v = json["maxReportLength"]; v.isString())
    {
        this->maxReportLength = v.toString().toLongLong();
    }
    if (const QJsonValue &v = json["maxCommentLength"]; v.isString())
    {
        this->maxCommentLength = v.toString().toLongLong();
    }
    if (const QJsonValue &v = json["senderEmailAddress"]; v.isString())
    {
        this->senderEmailAddress = v.toString();
    }
}

QJsonObject Configuration::toJson() const
{
    QJsonObject json;

    json["cloudDirectory"] = this->cloudDirectory;
    json["rangeCaDirectory"] = this->rangeCaDirectory;
    json["publicHttpPort"] = QString::number(this->publicHttpPort);
    json["privateHttpPort"] = QString::number(this->privateHttpPort);
    json["rateLimitPerSecond"] = QString::number(this->rateLimitPerSecond);
    json["publicKey"] = this->publicKey;
    json["privateKey"] = this->privateKey;
    json["privateKeyPassword"] = this->privateKeyPassword;
    json["caPublicKey"] = this->caPublicKey;
    json["fileStore"] = this->fileStore;
    json["fileStoreMaxSize"] = QString::number(this->fileStoreMaxSize);
    json["fileStoreMaxFileSize"] = QString::number(this->fileStoreMaxFileSize);
    json["maxReportLength"] = QString::number(this->maxReportLength);
    json["maxCommentLength"] = QString::number(this->maxCommentLength);
    json["senderEmailAddress"] = this->senderEmailAddress;

    return json;
}

void Configuration::readFile(const QString fileName)
{
    RLogger::info("[Configuration] Reading configuration file \"%s\".\n",fileName.toUtf8().constData());

    QFile inFile(fileName);

    if (!inFile.exists())
    {
        throw RError(RError::Type::InvalidFileName,R_ERROR_REF,"Configuration file \"%s\" does not exist.",fileName.toUtf8().constData());
    }

    if(!inFile.open(QIODevice::ReadOnly))
    {
        throw RError(RError::Type::OpenFile,R_ERROR_REF,
                     "Failed to open configuration file \"%s\" for reading. %s.",
                     inFile.fileName().toUtf8().constData(),
                     inFile.errorString().toUtf8().constData());
    }

    QByteArray byteArray = inFile.readAll();
    RLogger::info("[Configuration] Successfuly read \"%ld\" bytes from \"%s\".\n",byteArray.size(),inFile.fileName().toUtf8().constData());

    this->fromJson(QJsonDocument::fromJson(byteArray).object());

    inFile.close();
}

void Configuration::writeFile(const QString fileName) const
{
    RLogger::info("[Configuration] Writing configuration file \"%s\".\n",fileName.toUtf8().constData());

    QFile outFile(fileName);

    if(!outFile.open(QIODevice::WriteOnly))
    {
        throw RError(RError::Type::OpenFile,R_ERROR_REF,
                     "Failed to open configuration file \"%s\" for writing. %s.",
                     outFile.fileName().toUtf8().constData(),
                     outFile.errorString().toUtf8().constData());
    }

    qint64 bytesOut = outFile.write(QJsonDocument(this->toJson()).toJson());

    RLogger::info("[Configuration] Successfuly wrote \"%ld\" bytes to \"%s\".\n",bytesOut,outFile.fileName().toUtf8().constData());

    outFile.close();
}

QString Configuration::buildPath(const QString &direcotryPath, const QString &basename)
{
    return QDir(direcotryPath).absoluteFilePath(basename);
}

QString Configuration::buildConfigurationDirectoryPath(const QString &cloudDirectoryPath)
{
    return Configuration::buildPath(cloudDirectoryPath,Configuration::configurationDirectoryBase);
}

QString Configuration::buildLogDirectoryPath(const QString &cloudDirectoryPath)
{
    return Configuration::buildPath(cloudDirectoryPath,Configuration::logDirectoryBase);
}

QString Configuration::buildVariableDirectoryPath(const QString &cloudDirectoryPath)
{
    return Configuration::buildPath(cloudDirectoryPath,Configuration::variableDirectoryBase);
}

QString Configuration::buildProcessesDirectoryPath(const QString &cloudDirectoryPath)
{
    return Configuration::buildPath(cloudDirectoryPath,Configuration::processesDirectoryBase);
}

QString Configuration::buildReportsDirectoryPath(const QString &cloudDirectoryPath)
{
    return Configuration::buildPath(cloudDirectoryPath,Configuration::reportsDirectoryBase);
}

QString Configuration::buildCertificateDirectoryPath(const QString &cloudDirectoryPath)
{
    return Configuration::buildPath(cloudDirectoryPath,Configuration::certificateDirectoryBase);
}

QString Configuration::buildServerCertificateDirectoryPath(const QString &cloudDirectoryPath)
{
    return Configuration::buildPath(Configuration::buildCertificateDirectoryPath(cloudDirectoryPath),Configuration::serverCertificateDirectoryBase);
}

QString Configuration::buildCaCertificateDirectoryPath(const QString &cloudDirectoryPath)
{
    return Configuration::buildPath(Configuration::buildCertificateDirectoryPath(cloudDirectoryPath),Configuration::caCertificateDirectoryBase);
}

QString Configuration::buildConfigurationFilePath(const QString &cloudDirectoryPath)
{
    return Configuration::buildPath(Configuration::buildConfigurationDirectoryPath(cloudDirectoryPath),Configuration::configurationFileBase);
}

QString Configuration::buildUsersFilePath(const QString &cloudDirectoryPath)
{
    return Configuration::buildPath(Configuration::buildConfigurationDirectoryPath(cloudDirectoryPath),Configuration::usersFileBase);
}

QString Configuration::buildActionsFilePath(const QString &cloudDirectoryPath)
{
    return Configuration::buildPath(Configuration::buildConfigurationDirectoryPath(cloudDirectoryPath),Configuration::actionsFileBase);
}

QString Configuration::buildProcessesFilePath(const QString &cloudDirectoryPath)
{
    return Configuration::buildPath(Configuration::buildConfigurationDirectoryPath(cloudDirectoryPath),Configuration::processesFileBase);
}

QString Configuration::buildLogFilePath(const QString &cloudDirectoryPath)
{
    return Configuration::buildPath(Configuration::buildLogDirectoryPath(cloudDirectoryPath),Configuration::logFileBase);
}

QString Configuration::getDefaultCloudDirectoryPath()
{
    return Configuration::buildPath(QStandardPaths::writableLocation(QStandardPaths::HomeLocation),Configuration::cloudDirectoryBase);
}

QString Configuration::getDefaultRangeCaDirectoryPath()
{
    return Configuration::buildPath(QStandardPaths::writableLocation(QStandardPaths::HomeLocation),Configuration::rangeCaDirectoryBase);
}

QString Configuration::getDefaultFileStorePath(const QString &cloudDirectoryPath)
{
    return Configuration::buildPath(cloudDirectoryPath,Configuration::storeDirectoryBase);
}

qint64 Configuration::getDefaultFileStoreMaxSize()
{
    return -1;
}

qint64 Configuration::getDefaultFileStoreMaxFileSize()
{
    return -1;
}

qint64 Configuration::getDefaultMaxReportLength()
{
    return RReportRecord::defaultMaxReportLength;
}

qint64 Configuration::getDefaultMaxCommentLength()
{
    return RReportRecord::defaultMaxCommentLength;
}

uint Configuration::getDefaultPublicHttpPort()
{
    return RCloudSessionManager::DefaultCloudSession::publicPort;
}

uint Configuration::getDefaultPrivateHttpPort()
{
    return RCloudSessionManager::DefaultCloudSession::privatePort;
}

quint32 Configuration::getDefaultRateLimitPerSecond()
{
    return RHttpServerSettings::defaultRateLimitPerSecond;
}

QString Configuration::getDefaultPrivateKeyPath(const QString &cloudDirectoryPath)
{
    return QDir(Configuration::buildServerCertificateDirectoryPath(cloudDirectoryPath)).absoluteFilePath("server.key.pem");
}

QString Configuration::getDefaultPublicKeyPath(const QString &cloudDirectoryPath)
{
    return QDir(Configuration::buildServerCertificateDirectoryPath(cloudDirectoryPath)).absoluteFilePath("server.cert.pem");
}

QString Configuration::getDefaultCaPublicKeyPath(const QString &cloudDirectoryPath)
{
    return QDir(Configuration::buildCaCertificateDirectoryPath(cloudDirectoryPath)).absoluteFilePath("ca-chain.cert.pem");
}

QString Configuration::getDefaultSenderEmailAddress()
{
    return QString();
}
