#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>

#include <rbl_error.h>
#include <rbl_logger.h>

#include "process.h"
#include "process_manager.h"

ProcessManager::ProcessManager(const ProcessManagerSettings &settings, QObject *parent)
    : QObject{parent}
    , settings{settings}
{
    this->statistics.setName(this->settings.getName());

    if (QFile::exists(this->settings.getProcessesFileName()))
    {
        this->readFile();
    }
    else
    {
        RAccessRights accessRights;

        RAccessOwner accessOwner;
        accessOwner.setUser(RUserInfo::rootUser);
        accessOwner.setGroup(RUserInfo::rootGroup);
        accessRights.setOwner(accessOwner);

        RAccessMode accessMode;
        accessMode.setUserModeMask(RAccessMode::Execute);
        accessMode.setGroupModeMask(RAccessMode::Execute);
        accessMode.setOtherModeMask(RAccessMode::None);
        accessRights.setMode(accessMode);

        RCloudProcessInfo helloWorldProcessInfo;
        helloWorldProcessInfo.setName("hello-world");
        helloWorldProcessInfo.setExecutable("<processes>/helo_world.sh");
        helloWorldProcessInfo.setArguments(QList<QString>{"--parameter1=<value1>", "--parameter2=<value2>", "--switch"});
        helloWorldProcessInfo.setAccessRights(accessRights);
        this->processes.append(helloWorldProcessInfo);

        RCloudProcessInfo processCsrProcessInfo;
        processCsrProcessInfo.setName("process-csr");
        processCsrProcessInfo.setExecutable("<processes>/process_csr.sh");
        processCsrProcessInfo.setArguments(QList<QString>{"--csr-base64=<csr-content-base64>"});
        processCsrProcessInfo.setAccessRights(accessRights);
        this->processes.append(processCsrProcessInfo);

        RCloudProcessInfo processReportProcessInfo;
        processReportProcessInfo.setName("process-report");
        processReportProcessInfo.setExecutable("<processes>/process_report.sh");
        processReportProcessInfo.setArguments(QList<QString>{"--report-base64=<report-content-base64>"});
        processReportProcessInfo.setAccessRights(accessRights);
        this->processes.append(processReportProcessInfo);

        this->writeFile();
    }
}

bool ProcessManager::containsProcess(const QString &name) const
{
    foreach (const RCloudProcessInfo &p, this->processes)
    {
        if (p.getName() == name)
        {
            return true;
        }
    }
    return false;
}

RCloudProcessInfo ProcessManager::findProcess(const QString &name) const
{
    foreach (const RCloudProcessInfo &p, this->processes)
    {
        if (p.getName() == name)
        {
            return p;
        }
    }
    return RCloudProcessInfo();
}

RCloudProcessInfo ProcessManager::updateProcessAccessRights(const QString &name, const RAccessRights &accessRights)
{
    if (!accessRights.isValid())
    {
        throw RError(RError::Type::InvalidInput,R_ERROR_REF,"Invalid access rights \"%s\".",accessRights.toString().toUtf8().constData());
    }
    for (qsizetype i=0;i<this->processes.size();i++)
    {
        if (this->processes[i].getName() == name)
        {
            RLogger::info("[%s] Updating process \"%s\" access-rights: \"%s\".\n",
                          this->settings.getName().toUtf8().constData(),
                          name.toUtf8().constData(),
                          accessRights.toString().toUtf8().constData());
            this->processes[i].setAccessRights(accessRights);
            return this->processes[i];
        }
    }
    throw RError(RError::Type::InvalidInput,R_ERROR_REF,"Action name \"%s\" does not exist.",name.toUtf8().constData());
}

const QList<RCloudProcessInfo> &ProcessManager::getProcesses() const
{
    return this->processes;
}

void ProcessManager::readFile()
{
    RLogger::info("[%s] Reading processes file \"%s\".\n",
                  this->settings.getName().toUtf8().constData(),
                  this->settings.getProcessesFileName().toUtf8().constData());

    QFile inFile(this->settings.getProcessesFileName());

    if (!inFile.exists())
    {
        throw RError(RError::Type::InvalidFileName,R_ERROR_REF,"Processes file \"%s\" does not exist.",settings.getProcessesFileName().toUtf8().constData());
    }

    if(!inFile.open(QIODevice::ReadOnly))
    {
        throw RError(RError::Type::OpenFile,R_ERROR_REF,
                     "Failed to open users file \"%s\" for reading. %s.",
                     inFile.fileName().toUtf8().constData(),
                     inFile.errorString().toUtf8().constData());
    }

    QByteArray byteArray = inFile.readAll();
    RLogger::info("[%s] Successfuly read \"%ld\" bytes from \"%s\".\n",
                  this->settings.getName().toUtf8().constData(),
                  byteArray.size(),
                  inFile.fileName().toUtf8().constData());

    this->fromJson(QJsonDocument::fromJson(byteArray).object());

    inFile.close();
}

void ProcessManager::writeFile() const
{
    RLogger::info("[%s] Writing processes file \"%s\".\n",
                  this->settings.getName().toUtf8().constData(),
                  this->settings.getProcessesFileName().toUtf8().constData());

    QFile outFile(this->settings.getProcessesFileName());

    if(!outFile.open(QIODevice::WriteOnly))
    {
        throw RError(RError::Type::OpenFile,R_ERROR_REF,
                     "Failed to open processes file \"%s\" for writing. %s.",
                     outFile.fileName().toUtf8().constData(),
                     outFile.errorString().toUtf8().constData());
    }

    qint64 bytesOut = outFile.write(QJsonDocument(this->toJson()).toJson());

    RLogger::info("[%s] Successfuly wrote \"%ld\" bytes to \"%s\".\n",
                  this->settings.getName().toUtf8().constData(),
                  bytesOut,
                  outFile.fileName().toUtf8().constData());

    outFile.close();
}

bool ProcessManager::authorizeUser(const RUserInfo &userInfo, const QString &name) const
{
    return this->findProcess(name).getAccessRights().isUserAuthorized(userInfo,RAccessMode::Mode::Execute);
}

QUuid ProcessManager::submitProcess(const RCloudProcessRequest &processRequest)
{
    RLogger::debug("[%s] Submitting process \"%s\".\n",
                   this->settings.getName().toUtf8().constData(),
                   QJsonDocument(processRequest.toJson()).toJson(QJsonDocument::Compact).constData());

    RCloudProcessInfo processInfo(this->findProcess(processRequest.getName()));
    processInfo.getExecutable().replace("<processes>",this->settings.getProcessesDirectory());

    QStringList arguments(processInfo.getArguments());
    for (auto i = processRequest.getArgumentValues().cbegin(), iend = processRequest.getArgumentValues().cend(); i != iend; ++i)
    {
        for (auto &argument : arguments)
        {
            argument = argument.replace(QString("<%1>").arg(i.key()),i.value());
        }
    }
    processInfo.setArguments(arguments);

    QDir workingDirectory(this->settings.getWorkingDirectory());
    QString processWorkindDirectoryPath(workingDirectory.filePath(processInfo.getName()));
    if (!workingDirectory.exists(processInfo.getName()))
    {
        if (!workingDirectory.mkdir(processInfo.getName()))
        {
            RLogger::error("[%s] Failed to create directory \"%s\".\n",processWorkindDirectoryPath.toUtf8().constData());
            throw RError(RError::Application,R_ERROR_REF,"Internal application error");
        }
    }
    QFileInfo processWorkingDirectory(processWorkindDirectoryPath);
    if (!processWorkingDirectory.isDir())
    {
        RLogger::error("[%s] Not a valid directory \"%s\".\n",processWorkindDirectoryPath.toUtf8().constData());
        throw RError(RError::Application,R_ERROR_REF,"Internal application error");
    }

    QProcessEnvironment processEnvironment = QProcessEnvironment::systemEnvironment();
    processEnvironment.insert("CLOUD_PROCESS_WORK_DIR", processWorkindDirectoryPath);
    processEnvironment.insert("CLOUD_PROCESS_RANGE_CA_DIR", this->settings.getRangeCaDirectory());
    processEnvironment.insert("CLOUD_PROCESS_EXECUTOR", processRequest.getExecutor().getName() + ":" + processRequest.getExecutor().getGroupNames().join(','));
    processEnvironment.insert("CLOUD_PROCESS_OWNER", processInfo.getAccessRights().getOwner().getUser() + ":" + processInfo.getAccessRights().getOwner().getGroup());
    processEnvironment.insert("CLOUD_PROCESS_LOG_FILE", QDir(this->settings.getLogDirectory()).filePath(QString("%1-%2.log").arg(processRequest.getName(),processRequest.getExecutor().getName())));

    RLogger::debug("[%s] Starting process \"%s\".\n",
                   this->settings.getName().toUtf8().constData(),
                   processInfo.buildCommand().toUtf8().constData());

    QSharedPointer<Process> process = QSharedPointer<Process>(new Process(processInfo,processRequest,this),&QObject::deleteLater);
    process->setWorkingDirectory(processWorkindDirectoryPath);
    process->setProcessEnvironment(processEnvironment);

    QObject::connect(process.get(),&Process::processErrorOccurred,this,&ProcessManager::onProcessErrorOccurred);
    QObject::connect(process.get(),&Process::processFinished,this,&ProcessManager::onProcessFinished);
    QObject::connect(process.get(),&Process::processStarted,this,&ProcessManager::onProcessStarted);
    QObject::connect(process.get(),&Process::processStateChanged,this,&ProcessManager::onProcessStateChanged);

    this->runningProcesses.insert(process->getId(),process);

    process->start();

    return process->getId();
}

void ProcessManager::finalizeProcess(const QUuid &id)
{
    RLogger::debug("[%s] Finalize process id = \"%s\".\n",
                   this->settings.getName().toUtf8().constData(),
                   id.toString(QUuid::WithoutBraces).toUtf8().constData());
    this->finishedProcesses.remove(id);
}

QJsonObject ProcessManager::getStatisticsJson() const
{
    RLogger::debug("[%s] Producting statistics\n",this->settings.getName().toUtf8().constData());
    ServiceStatistics snapshotStatistics(this->statistics);
    snapshotStatistics.recordCounter("size",this->processes.size());
    return snapshotStatistics.toJson();
}

void ProcessManager::fromJson(const QJsonObject &json)
{
    if (const QJsonValue &v = json["processes"]; v.isArray())
    {
        const QJsonArray &usersArray = v.toArray();
        this->processes.clear();
        this->processes.reserve(usersArray.size());
        for (const QJsonValue &user : usersArray)
        {
            this->processes.append(RCloudProcessInfo::fromJson(user.toObject()));
        }
    }
}

QJsonObject ProcessManager::toJson() const
{
    QJsonObject json;

    // Users
    QJsonArray processesArray;
    for (const RCloudProcessInfo &processInfo : this->processes)
    {
        processesArray.append(processInfo.toJson());
    }
    json["processes"] = processesArray;

    return json;
}

void ProcessManager::onProcessErrorOccurred(QUuid id, QProcess::ProcessError error)
{
    this->statistics.recordCounter(this->runningProcesses.value(id)->getProcessInfo().getName() + "Errored",1);
    RLogger::error("[%s] Error occured id = \"%s\" with error \"%s\".\n",
                   this->settings.getName().toUtf8().constData(),
                   id.toString(QUuid::WithoutBraces).toUtf8().constData(),
                   Process::processErrorToString(error).toUtf8().constData());

    this->runningProcesses.value(id);
    this->finishedProcesses.insert(id,this->runningProcesses.value(id));
    this->runningProcesses.remove(id);

    emit this->processCompleted(id,this->finishedProcesses.value(id)->getProcessResult());
}

void ProcessManager::onProcessFinished(QUuid id, int exitCode, QProcess::ExitStatus exitStatus)
{
    this->statistics.recordCounter(this->runningProcesses.value(id)->getProcessInfo().getName() + (exitStatus == QProcess::NormalExit ? "Finished" : "Crashed"),1);
    RLogger::info("[%s] Finished id = \"%s\" with exit code \"%d\" and exit status \"%s\".\n",
                  this->settings.getName().toUtf8().constData(),
                  id.toString(QUuid::WithoutBraces).toUtf8().constData(),
                  exitCode,
                  exitStatus == QProcess::NormalExit ? "Normal exit" : "Crash exit");

    this->runningProcesses.value(id);
    this->finishedProcesses.insert(id,this->runningProcesses.value(id));
    this->runningProcesses.remove(id);

    emit this->processCompleted(id,this->finishedProcesses.value(id)->getProcessResult());
}

void ProcessManager::onProcessStarted(QUuid id)
{
    this->statistics.recordCounter(this->runningProcesses.value(id)->getProcessInfo().getName() + "Started",1);
    RLogger::info("[%s] Started id = \"%s\" command = \"%s\".\n",
                  this->settings.getName().toUtf8().constData(),
                  id.toString(QUuid::WithoutBraces).toUtf8().constData(),
                  this->runningProcesses.value(id)->getProcessInfo().buildCommand().toUtf8().constData());
}

void ProcessManager::onProcessStateChanged(QUuid id, QProcess::ProcessState newState)
{
    RLogger::info("[%s] State changed id = \"%s\" state \"%s\".\n",
                  this->settings.getName().toUtf8().constData(),
                  id.toString(QUuid::WithoutBraces).toUtf8().constData(),
                  Process::processStateToString(newState).toUtf8().constData());
}
