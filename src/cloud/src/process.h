#ifndef PROCESS_H
#define PROCESS_H

#include <QProcess>
#include <QUuid>

#include <rcl_cloud_process_info.h>
#include <rcl_cloud_process_request.h>
#include <rcl_cloud_process_result.h>

class Process : public QProcess
{

    Q_OBJECT

    protected:

        //! Process ID.
        QUuid id;
        //! Process info.
        RCloudProcessInfo processInfo;
        //! Process result.
        RCloudProcessResult processResult;

    public:

        //! Constructor.
        Process(const RCloudProcessInfo processInfo, const RCloudProcessRequest &processRequest, QObject *parent);

        //! Return const reference to process ID.
        const QUuid &getId() const;

        //! Return const reference to process info.
        const RCloudProcessInfo &getProcessInfo() const;

        //! Return const reference to output buffer.
        const RCloudProcessResult &getProcessResult() const;

        //! Get corresponding error string for given process error code.
        static QString processErrorToString(const QProcess::ProcessError &processError);

        //! Get corresponding state string for given process state code.
        static QString processStateToString(const QProcess::ProcessState &processState);

        //! Start process.
        void start();

    private slots:

        void onErrorOccurred(QProcess::ProcessError error);
        void onFinished(int exitCode, QProcess::ExitStatus exitStatus);
        void onReadyReadStandardError();
        void onReadyReadStandardOutput();
        void onStarted();
        void onStateChanged(QProcess::ProcessState newState);

    signals:

        void processErrorOccurred(QUuid id, QProcess::ProcessError error);
        void processFinished(QUuid id, int exitCode, QProcess::ExitStatus exitStatus = NormalExit);
        void processStarted(QUuid id);
        void processStateChanged(QUuid id, QProcess::ProcessState newState);

};

#endif // PROCESS_H
