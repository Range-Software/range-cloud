#include "action_manager_settings.h"

void ActionManagerSettings::_init(const ActionManagerSettings *pActionManagerSettings)
{
    this->ServiceSettings::_init(pActionManagerSettings);
    if (pActionManagerSettings)
    {
        this->fileName = pActionManagerSettings->fileName;
    }
}

ActionManagerSettings::ActionManagerSettings()
{
    this->_init();
    this->name = "ActionService";
}

ActionManagerSettings::ActionManagerSettings(const ActionManagerSettings &fileManagerSettings)
    : ServiceSettings(fileManagerSettings)
{
    this->_init(&fileManagerSettings);
}

ActionManagerSettings &ActionManagerSettings::operator =(const ActionManagerSettings &fileManagerSettings)
{
    this->_init(&fileManagerSettings);
    return (*this);
}

const QString &ActionManagerSettings::getFileName() const
{
    return this->fileName;
}

void ActionManagerSettings::setFileName(const QString &fileName)
{
    this->fileName = fileName;
}
