#include "user_manager_settings.h"

void UserManagerSettings::_init(const UserManagerSettings *pUserManagerSettings)
{
    this->ServiceSettings::_init(pUserManagerSettings);
    if (pUserManagerSettings)
    {
        this->fileName = pUserManagerSettings->fileName;
    }
}

UserManagerSettings::UserManagerSettings()
{
    this->_init();
    this->name = "UserService";
}

UserManagerSettings::UserManagerSettings(const UserManagerSettings &userManagerSettings)
    : ServiceSettings(userManagerSettings)
{
    this->_init(&userManagerSettings);
}

UserManagerSettings &UserManagerSettings::operator =(const UserManagerSettings &userManagerSettings)
{
    this->_init(&userManagerSettings);
    return (*this);
}

const QString &UserManagerSettings::getFileName() const
{
    return this->fileName;
}

void UserManagerSettings::setFileName(const QString &fileName)
{
    this->fileName = fileName;
}
