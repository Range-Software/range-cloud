#include "mailer_settings.h"

void MailerSettings::_init(const MailerSettings *pMailerSettings)
{
    this->ServiceSettings::_init(pMailerSettings);
    if (pMailerSettings)
    {
        this->fromAddress = pMailerSettings->fromAddress;
        this->sendTimeout = pMailerSettings->sendTimeout;
    }
}

MailerSettings::MailerSettings()
    : sendTimeout{30000}
{
    this->_init();
    this->name = "MailerService";
}

MailerSettings::MailerSettings(const MailerSettings &mailerSettings)
    : ServiceSettings{mailerSettings}
{
    this->_init(&mailerSettings);
}

MailerSettings &MailerSettings::operator =(const MailerSettings &mailerSettings)
{
    this->_init(&mailerSettings);
    return (*this);
}

const QString &MailerSettings::getFromAddress() const
{
    return this->fromAddress;
}

void MailerSettings::setFromAddress(const QString &fromAddress)
{
    this->fromAddress = fromAddress;
}

int MailerSettings::getSendTimeout() const
{
    return sendTimeout;
}

void MailerSettings::setSendTimeout(int sendTimeout)
{
    this->sendTimeout = sendTimeout;
}
