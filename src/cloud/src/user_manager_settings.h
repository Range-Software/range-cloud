#ifndef USER_MANAGER_SETTINGS_H
#define USER_MANAGER_SETTINGS_H

#include <QString>

#include "service_settings.h"

class UserManagerSettings : public ServiceSettings
{

    protected:

        //! Internal initialization function.
        void _init(const UserManagerSettings *pUserManagerSettings = nullptr);

    protected:

        //! Path to file store.
        QString fileName;

    public:

        //! Constructor.
        UserManagerSettings();

        //! Copy constructor.
        UserManagerSettings(const UserManagerSettings &userManagerSettings);

        //! Destructor.
        ~UserManagerSettings() {}

        //! Assignment operator.
        UserManagerSettings &operator =(const UserManagerSettings &userManagerSettings);

        //! Get const reference to file store.
        const QString &getFileName() const;

        //! Set new file store.
        void setFileName(const QString &fileName);

};

#endif // USER_MANAGER_SETTINGS_H
