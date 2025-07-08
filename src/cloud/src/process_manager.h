#ifndef PROCESS_MANAGER_H
#define PROCESS_MANAGER_H

#include <QJsonObject>
#include <QList>
#include <QObject>
#include <QProcess>

#include <rcl_cloud_process_request.h>

#include "process.h"
#include "process_manager_settings.h"
#include "service_statistics.h"

class ProcessManager : public QObject
{

    Q_OBJECT

    protected:

        //! Settings.
        ProcessManagerSettings settings;
        //! Service satistics.
        ServiceStatistics statistics;
        //! List of users.
        QList<RCloudProcessInfo> processes;
        //! List of running processes.
        QMap<QUuid,QSharedPointer<Process>> runningProcesses;
        //! List of finished processes.
        QMap<QUuid,QSharedPointer<Process>> finishedProcesses;

    public:

        //! Constructor
        explicit ProcessManager(const ProcessManagerSettings &settings, QObject *parent = nullptr);

        //! Check if given process is in the manager.
        bool containsProcess(const QString &name) const;

        //! Find process.
        RCloudProcessInfo findProcess(const QString &name) const;

        //! Update process access rights.
        RCloudProcessInfo updateProcessAccessRights(const QString &name, const RAccessRights &accessRights);

        //! Return const reference to list of processes.
        const QList<RCloudProcessInfo> &getProcesses() const;

        //! Read from file.
        void readFile();

        //! Write to file.
        void writeFile() const;

        //! Authorize if userInfo is allowed to execute process.
        bool authorizeUser(const RUserInfo &userInfo, const QString &name) const;

        //! Submit process.
        QUuid submitProcess(const RCloudProcessRequest &processRequest);

        //! Finalize process (remove from finished list)
        void finalizeProcess(const QUuid &id);

        //! Get statistics output in Json form.
        QJsonObject getStatisticsJson() const;

    private:

        //! Create process manager object from Json.
        void fromJson(const QJsonObject &json);

        //! Create Json from process manager object.
        QJsonObject toJson() const;

    private slots:

        void onProcessErrorOccurred(QUuid id, QProcess::ProcessError error);
        void onProcessFinished(QUuid id, int exitCode, QProcess::ExitStatus exitStatus);
        void onProcessStarted(QUuid id);
        void onProcessStateChanged(QUuid id, QProcess::ProcessState newState);

    signals:

        //! Process completed.
        void processCompleted(const QUuid &processtId, const RCloudProcessResult &processResult);

};

#endif // PROCESS_MANAGER_H
