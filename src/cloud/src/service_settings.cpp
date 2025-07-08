#include "service_settings.h"

void ServiceSettings::_init(const ServiceSettings *pServiceSettings)
{
    if (pServiceSettings)
    {
        this->name = pServiceSettings->name;
    }
}

ServiceSettings::ServiceSettings()
{
    this->_init();
}

ServiceSettings::ServiceSettings(const ServiceSettings &serviceSettings)
{
    this->_init(&serviceSettings);
}

ServiceSettings &ServiceSettings::operator =(const ServiceSettings &serviceSettings)
{
    this->_init(&serviceSettings);
    return (*this);
}

const QString &ServiceSettings::getName() const
{
    return this->name;
}

void ServiceSettings::setName(const QString &name)
{
    this->name = name;
}
