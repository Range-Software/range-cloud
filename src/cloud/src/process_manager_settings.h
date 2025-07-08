#ifndef PROCESS_MANAGER_SETTINGS_H
#define PROCESS_MANAGER_SETTINGS_H

#include <QString>

#include "service_settings.h"

class ProcessManagerSettings : public ServiceSettings
{

    protected:

        //! Internal initialization function.
        void _init(const ProcessManagerSettings *pProcessManagerSettings = nullptr);

    protected:

        //! Path to log directory.
        QString logDirectory;
        //! Path to working directory.
        QString workingDirectory;
        //! Path to processes directory.
        QString processesDirectory;
        //! Path to file name.
        QString processesFileName;
        //! Path to Range CA directory.
        QString rangeCaDirectory;

    public:

        //! Constructor.
        ProcessManagerSettings();

        //! Copy constructor.
        ProcessManagerSettings(const ProcessManagerSettings &processManagerSettings);

        //! Destructor.
        ~ProcessManagerSettings() {}

        //! Assignment operator.
        ProcessManagerSettings &operator =(const ProcessManagerSettings &processManagerSettings);

        //! get const reference to log directory path.
        const QString &getLogDirectory() const;

        //! Set new log directory path.
        void setLogDirectory(const QString &logDirectory);

        //! get const reference to working directory path.
        const QString &getWorkingDirectory() const;

        //! Set new working directory path.
        void setWorkingDirectory(const QString &workingDirectory);

        //! get const reference to processes directory path.
        const QString &getProcessesDirectory() const;

        //! Set new processes directory path.
        void setProcessesDirectory(const QString &processesDirectory);

        //! Get const reference to processes file name.
        const QString &getProcessesFileName() const;

        //! Set new processes file name.
        void setProcessesFileName(const QString &processesFileName);

        //! get const reference to Range CA directory path.
        const QString &getRangeCaDirectory() const;

        //! Set new processes Range CA path.
        void setRangeCaDirectory(const QString &rangeCaDirectory);

};

#endif // PROCESS_MANAGER_SETTINGS_H
