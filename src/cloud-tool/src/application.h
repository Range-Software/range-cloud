#ifndef APPLICATION_H
#define APPLICATION_H

#include <QCoreApplication>

#include <rbl_tool_input.h>

#include <rcl_http_client.h>


class Application : public QCoreApplication
{

    Q_OBJECT

    protected:

        //! HTTP client.
        RHttpClient *httpClient;
        //! Number of started services.
        uint nStartedServices;

        //! Tool input.
        RToolInput toolInput;

        //! Action output file name.
        QString outputFileName;

    public:

        //! Constructor.
        explicit Application(int &argc, char **argv);

        //! Return application instance.
        static Application *instance() noexcept;

        //! Return const reference to tool input.
        const RToolInput &getToolInput() const;

        //! Return const reference to an action output file name.
        const QString &getOutputFileName() const;

        //! Disconnect all running clients.
        void disconnect();

    signals:

        //! Application has started.
        void started();

    protected slots:

        //! Catch started signal.
        void onStarted();

};

#endif // APPLICATION_H
