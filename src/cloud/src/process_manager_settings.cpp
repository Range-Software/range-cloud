#include "process_manager_settings.h"

void ProcessManagerSettings::_init(const ProcessManagerSettings *pProcessManagerSettings)
{
    this->ServiceSettings::_init(pProcessManagerSettings);
    if (pProcessManagerSettings)
    {
        this->logDirectory = pProcessManagerSettings->logDirectory;
        this->workingDirectory = pProcessManagerSettings->workingDirectory;
        this->processesDirectory = pProcessManagerSettings->processesDirectory;
        this->processesFileName = pProcessManagerSettings->processesFileName;
        this->rangeCaDirectory = pProcessManagerSettings->rangeCaDirectory;
    }
}

ProcessManagerSettings::ProcessManagerSettings()
{
    this->_init();
    this->name = "ProcessService";
}

ProcessManagerSettings::ProcessManagerSettings(const ProcessManagerSettings &processManagerSettings)
    : ServiceSettings(processManagerSettings)
{
    this->_init(&processManagerSettings);
}

ProcessManagerSettings &ProcessManagerSettings::operator =(const ProcessManagerSettings &processManagerSettings)
{
    this->_init(&processManagerSettings);
    return (*this);
}

const QString &ProcessManagerSettings::getLogDirectory() const
{
    return this->logDirectory;
}

void ProcessManagerSettings::setLogDirectory(const QString &logDirectory)
{
    this->logDirectory = logDirectory;
}

const QString &ProcessManagerSettings::getWorkingDirectory() const
{
    return this->workingDirectory;
}

void ProcessManagerSettings::setWorkingDirectory(const QString &workingDirectory)
{
    this->workingDirectory = workingDirectory;
}

const QString &ProcessManagerSettings::getProcessesDirectory() const
{
    return this->processesDirectory;
}

void ProcessManagerSettings::setProcessesDirectory(const QString &processesDirectory)
{
    this->processesDirectory = processesDirectory;
}

const QString &ProcessManagerSettings::getProcessesFileName() const
{
    return this->processesFileName;
}

void ProcessManagerSettings::setProcessesFileName(const QString &processesFileName)
{
    this->processesFileName = processesFileName;
}

const QString &ProcessManagerSettings::getRangeCaDirectory() const
{
    return this->rangeCaDirectory;
}

void ProcessManagerSettings::setRangeCaDirectory(const QString &rangeCaDirectory)
{
    this->rangeCaDirectory = rangeCaDirectory;
}
