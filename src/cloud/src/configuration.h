#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <QJsonObject>

class Configuration
{

    protected:

        static const QString cloudDirectoryBase;
        static const QString rangeCaDirectoryBase;
        static const QString storeDirectoryBase;
        static const QString configurationDirectoryBase;
        static const QString certificateDirectoryBase;
        static const QString serverCertificateDirectoryBase;
        static const QString caCertificateDirectoryBase;
        static const QString logDirectoryBase;
        static const QString variableDirectoryBase;
        static const QString processesDirectoryBase;
        static const QString reportsDirectoryBase;

        static const QString configurationFileBase;
        static const QString actionsFileBase;
        static const QString processesFileBase;
        static const QString usersFileBase;
        static const QString logFileBase;

    protected:

        //! Internal initialization function.
        void _init(const Configuration *pConfiguration = nullptr);

    protected:

        QString cloudDirectory;
        QString rangeCaDirectory;

        uint publicHttpPort;
        uint privateHttpPort;

        quint32 rateLimitPerSecond;

        QString publicKey;
        QString privateKey;
        QString privateKeyPassword;
        QString caPublicKey;

        QString fileStore;
        qint64 fileStoreMaxSize;
        qint64 fileStoreMaxFileSize;

        qint64 maxReportLength;
        qint64 maxCommentLength;

        QString senderEmailAddress;

    public:

        //! Constructor.
        Configuration(const QString &cloudDirectory);

        //! Copy constructor.
        Configuration(const Configuration &configuration);

        //! Destructor.
        ~Configuration() { }

        //! Assignment operator.
        Configuration &operator =(const Configuration &configuration);

        const QString &getRangeCaDirectory() const;
        void setRangeCaDirectory(const QString &rangeCaDirectory);

        uint getPublicHttpPort() const;
        void setPublicHttpPort(uint publicHttpPort);

        uint getPrivateHttpPort() const;
        void setPrivateHttpPort(uint privateHttpPort);

        quint32 getRateLimitPerSecond() const;
        void setRateLimitPerSecond(quint32 rateLimitPerSecond);

        const QString &getPublicKey() const;
        void setPublicKey(const QString &publicKey);

        const QString &getPrivateKey() const;
        void setPrivateKey(const QString &privateKey);

        const QString &getPrivateKeyPassword() const;
        void setPrivateKeyPassword(const QString &privateKeyPassword);

        const QString &getCaPublicKey() const;
        void setCaPublicKey(const QString &caPublicKey);

        const QString &getFileStore() const;
        void setFileStore(const QString &fileStore);

        qint64 getFileStoreMaxSize() const;
        void setFileStoreMaxSize(qint64 fileStoreMaxSize);

        qint64 getFileStoreMaxFileSize() const;
        void setFileStoreMaxFileSize(qint64 fileStoreMaxFileSize);

        qint64 getMaxReportLength() const;
        void setMaxReportLength(qint64 maxReportLength);

        qint64 getMaxCommentLength() const;
        void setMaxCommentLength(qint64 maxCommentLength);

        const QString &getSenderEmailAddress() const;
        void setSenderEmailAddress(const QString &senderEmailAddress);

        //! Get log directory path.
        QString getLogDirectoryPath() const;

        //! Get configuration directory path.
        QString getConfigurationDirectoryPath() const;

        //! Get certificate directory path.
        QString getCertificateDirectoryPath() const;

        //! Get server certificate directory path.
        QString getServerCertificateDirectoryPath() const;

        //! Get CA certificate directory path.
        QString getCaCertificateDirectoryPath() const;

        //! Get variable directory path.
        QString getVariableDirectoryPath() const;

        //! Get processes scripts directory path.
        QString getProcessesDirectoryPath() const;

        //! Get reports directory path.
        QString getReportsDirectoryPath() const;

        //! Get users file path.
        QString getUsersFilePath() const;

        //! Get actions file path.
        QString getActionsFilePath() const;

        //! Get processes file path.
        QString getProcessesFilePath() const;

        //! Get log file path.
        QString getLogFilePath() const;

        //! Print configuration to standard output.
        void print() const;

        //! Writes any unsaved changes to permanent storage.
        void sync() const;

    private:

        //! Create configuration object from Json.
        void fromJson(const QJsonObject &json);

        //! Create Json from configuration object.
        QJsonObject toJson() const;

        //! Read from file.
        void readFile(const QString fileName);

        //! Write to file.
        void writeFile(const QString fileName) const;

        //! Build path.
        static QString buildPath(const QString &direcotryPath, const QString &basename);

        //! Build configuration directory path.
        static QString buildConfigurationDirectoryPath(const QString &cloudDirectoryPath);

        //! Build log directory path.
        static QString buildLogDirectoryPath(const QString &cloudDirectoryPath);

        //! Build variable directory path.
        static QString buildVariableDirectoryPath(const QString &cloudDirectoryPath);

        //! Build processes scripts directory path.
        static QString buildProcessesDirectoryPath(const QString &cloudDirectoryPath);

        //! Build reports directory path.
        static QString buildReportsDirectoryPath(const QString &cloudDirectoryPath);

        //! Build certificate directory path.
        static QString buildCertificateDirectoryPath(const QString &cloudDirectoryPath);

        //! Build server certificate directory path.
        static QString buildServerCertificateDirectoryPath(const QString &cloudDirectoryPath);

        //! Build CA certificate directory path.
        static QString buildCaCertificateDirectoryPath(const QString &cloudDirectoryPath);

        //! Build configuration file name.
        static QString buildConfigurationFilePath(const QString &cloudDirectoryPath);

        //! Build users file path.
        static QString buildUsersFilePath(const QString &cloudDirectoryPath);

        //! Build actions file path.
        static QString buildActionsFilePath(const QString &cloudDirectoryPath);

        //! Build processes file path.
        static QString buildProcessesFilePath(const QString &cloudDirectoryPath);

        //! Build log file path.
        static QString buildLogFilePath(const QString &cloudDirectoryPath);

    public:

        //! Get default cloud directory path.
        static QString getDefaultCloudDirectoryPath();

        //! Get default Range CA directory path.
        static QString getDefaultRangeCaDirectoryPath();

        //! Get default file store path.
        static QString getDefaultFileStorePath(const QString &cloudDirectoryPath = Configuration::getDefaultCloudDirectoryPath());

        //! Get default maximum file store size.
        static qint64 getDefaultFileStoreMaxSize();

        //! Get default maximum file size in file store.
        static qint64 getDefaultFileStoreMaxFileSize();

        //! Get maximum report length.
        static qint64 getDefaultMaxReportLength();

        //! Get maximum comment length.
        static qint64 getDefaultMaxCommentLength();

        //! Get default public HTTP port.
        static uint getDefaultPublicHttpPort();

        //! Get default private HTTP port.
        static uint getDefaultPrivateHttpPort();

        //! Get default maximum number of incoming requests per second per IP accepted by the server.
        static quint32 getDefaultRateLimitPerSecond();

        //! Get default private key file path.
        static QString getDefaultPrivateKeyPath(const QString &cloudDirectoryPath = Configuration::getDefaultCloudDirectoryPath());

        //! Get default public key file path.
        static QString getDefaultPublicKeyPath(const QString &cloudDirectoryPath = Configuration::getDefaultCloudDirectoryPath());

        //! Get default CA public key file path.
        static QString getDefaultCaPublicKeyPath(const QString &cloudDirectoryPath = Configuration::getDefaultCloudDirectoryPath());

        //! Get default sender email address.
        static QString getDefaultSenderEmailAddress();

};

#endif // CONFIGURATION_H
