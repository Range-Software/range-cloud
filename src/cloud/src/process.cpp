#include "process.h"

Process::Process(const RCloudProcessInfo processInfo, const RCloudProcessRequest &processRequest, QObject *parent)
    : QProcess(parent)
    , id(QUuid::createUuid())
    , processInfo(processInfo)
{
    this->processResult.setProcessRequest(processRequest);

    QObject::connect(this,&QProcess::errorOccurred,this,&Process::onErrorOccurred);
    QObject::connect(this,&QProcess::finished,this,&Process::onFinished);
    QObject::connect(this,&QProcess::readyReadStandardError,this,&Process::onReadyReadStandardError);
    QObject::connect(this,&QProcess::readyReadStandardOutput,this,&Process::onReadyReadStandardOutput);
    QObject::connect(this,&QProcess::started,this,&Process::onStarted);
    QObject::connect(this,&QProcess::stateChanged,this,&Process::onStateChanged);
}

const QUuid &Process::getId() const
{
    return this->id;
}

const RCloudProcessInfo &Process::getProcessInfo() const
{
    return this->processInfo;
}

const RCloudProcessResult &Process::getProcessResult() const
{
    return this->processResult;
}

QString Process::processErrorToString(const ProcessError &processError)
{
    switch (processError)
    {
        case QProcess::FailedToStart:
            return QString("The process failed to start. Either the invoked program is missing, or you may have insufficient permissions or resources to invoke the program.");
        case QProcess::Crashed:
            return QString("The process crashed some time after starting successfully.");
        case QProcess::Timedout:
            return QString("The last waitFor...() function timed out.");
        case QProcess::ReadError:
            return QString("An error occurred when attempting to read from the process.");
        case QProcess::WriteError:
            return QString("An error occurred when attempting to write to the process.");
        case QProcess::UnknownError:
        default:
            return QString("An unknown error occurred.");
    }
}

QString Process::processStateToString(const ProcessState &processState)
{
    switch (processState)
    {
    case QProcess::NotRunning:
        return QString("The process is not running.");
    case QProcess::Starting:
        return QString("The process is starting, but the program has not yet been invoked.");
    case QProcess::Running:
        return QString("The process is running and is ready for reading and writing.");
    default:
        return QString("An unknown process state.");
    }
}

void Process::start()
{
    this->QProcess::start(this->processInfo.getExecutable(),this->processInfo.getArguments());
}

void Process::onErrorOccurred(ProcessError error)
{
    this->processResult.setErrorType(RError::ChildProcess);
    if (this->processResult.getErrorBuffer().isEmpty())
    {
        this->processResult.getErrorBuffer().append("Child process failed.");
    }
    emit this->processErrorOccurred(this->id,error);
}

void Process::onFinished(int exitCode, ExitStatus exitStatus)
{
    this->processResult.setErrorType(exitCode == 0 ? RError::None : RError::ChildProcess);
    emit this->processFinished(this->id,exitCode,exitStatus);
}

void Process::onReadyReadStandardError()
{
    this->processResult.getErrorBuffer().append(this->readAllStandardError());
}

void Process::onReadyReadStandardOutput()
{
    this->processResult.getOutputBuffer().append(this->readAllStandardOutput());
}

void Process::onStarted()
{
    emit this->processStarted(this->id);
}

void Process::onStateChanged(ProcessState newState)
{
    emit this->processStateChanged(this->id,newState);
}
