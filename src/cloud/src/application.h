#ifndef APPLICATION_H
#define APPLICATION_H

#include <QCoreApplication>

#include <rcl_http_server.h>

#include "action_handler.h"
#include "action_manager.h"
#include "configuration.h"
#include "mailer.h"
#include "process_manager.h"
#include "report_manager.h"
#include "user_manager.h"

class Application : public QCoreApplication
{

    Q_OBJECT

    protected:

        static const QString cloudDirectoryKey;
        static const QString rangeCaDirectoryKey;
        static const QString logDebugKey;
        static const QString logTraceKey;
        static const QString logQtKey;
        static const QString logSslKey;
        static const QString publicHttpPortKey;
        static const QString privateHttpPortKey;
        static const QString publicKeyKey;
        static const QString privateKeyKey;
        static const QString privateKeyPasswordKey;
        static const QString caPublicKeyKey;
        static const QString fileStoreKey;
        static const QString fileStoreMaxSizeKey;
        static const QString fileStoreMaxFileSizeKey;
        static const QString printSettingsKey;
        static const QString storeSettingsKey;

    protected:

        //! Stop timeout in ms.
        uint stopTimeout;

        //! User manager service.
        UserManager *userManager;

        //! Action manager service.
        ActionManager *actionManager;

        //! Process manager service.
        ProcessManager *processManager;

        //! Report manager service.
        ReportManager *reportManager;

        //! Public HTTP server.
        RHttpServer *publicHttpServer;
        //! Public HTTP service is ready.
        bool publicHttpServiceIsReady;

        //! Private HTTP server.
        RHttpServer *privateHttpServer;
        //! Private HTTP service is ready.
        bool privateHttpServiceIsReady;

        //! File manager.
        FileManager *fileManager;
        //! File service is ready;
        bool fileServiceIsReady;

        //! Mailer.
        Mailer *mailer;
        //! File service is ready;
        bool mailerServiceIsReady;

        //! Acton handler.
        ActionHandler *actionHandler;

        //! Number of started services.
        uint nStartedServices;

        //! Map of action IDs to messages
        QMap<QUuid, RNetworkMessage> actionToMessageMap;

    public:

        //! Constructor.
        explicit Application(int &argc, char **argv);

    private:

        void serviceStarted();

        void serviceStopped(bool failed);

        //! Create application directory structure.
        static bool createDirectories(const Configuration &configuration);

        //! Create directory.
        static bool createDirectory(const QString &directoryPath);

        //! Print cloud readiness if all services are ready.
        void printReadiness() const;

    signals:

        //! Application has started.
        void started();

        //! Application has stopped.
        void stopped();

    protected slots:

        //! Catch started signal.
        void onStarted();

        //! Catch stopped signal.
        void onStopped();

        //! Public HTTP server is ready.
        void publicHttpServiceReady();

        //! Public HTTP server has finished.
        void publicHttpServiceStarted();

        //! Public HTTP server has stopped.
        void publicHttpServiceFinished();

        //! Public HTTP server has failed.
        void publicHttpServiceFailed();

        //! Private HTTP server is ready.
        void privateHttpServiceReady();

        //! Private HTTP server has finished.
        void privateHttpServiceStarted();

        //! Private HTTP server has stopped.
        void privateHttpServiceFinished();

        //! Private HTTP server has failed.
        void privateHttpServiceFailed();

        //! File manager is ready.
        void fileServiceReady();

        //! File manager has finished.
        void fileServiceStarted();

        //! File manager has finished.
        void fileServiceFinished();

        //! File manager has failed.
        void fileServiceFailed();

        //! Mailer is ready.
        void mailerServiceReady();

        //! Mailer has finished.
        void mailerServiceStarted();

        //! Mailer has finished.
        void mailerServiceFinished();

        //! Mailer has failed.
        void mailerServiceFailed();

        //! Network message is available.
        void networkMessageAvailable(const RNetworkMessage &networkMessage);

        //! Action resolved.
        void actionResolved(const RCloudAction &action);

        //! Shutdown.
        void shutdown();

};

#endif // APPLICATION_H
