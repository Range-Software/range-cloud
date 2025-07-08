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

    public:

        //! Constructor.
        explicit Application(int &argc, char **argv);

        //! Return const reference to tool input.
        const RToolInput &getToolInput() const;

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
