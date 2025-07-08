#ifndef MAILER_SETTINGS_H
#define MAILER_SETTINGS_H

#include "service_settings.h"

class MailerSettings : public ServiceSettings
{

    protected:

        //! Internal initialization function.
        void _init(const MailerSettings *pMailerSettings = nullptr);

    protected:

        //! From address.
        QString fromAddress;
        //! Send timeout in miliseconds.
        int sendTimeout;

    public:

        //! Constructor.
        MailerSettings();

        //! Copy constructor.
        MailerSettings(const MailerSettings &mailerSettings);

        //! Destructor.
        ~MailerSettings() { }

        //! Assignment operator.
        MailerSettings &operator =(const MailerSettings &mailerSettings);

        //! Get const reference to from address.
        const QString &getFromAddress() const;

        //! Set new from address.
        void setFromAddress(const QString &fromAddress);

        //! Return send timeout;
        int getSendTimeout() const;

        //! Set new send timeout.
        void setSendTimeout(int sendTimeout);

};

#endif // MAILER_SETTINGS_H
