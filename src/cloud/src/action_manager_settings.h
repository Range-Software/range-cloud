#ifndef ACTION_MANAGER_SETTINGS_H
#define ACTION_MANAGER_SETTINGS_H

#include <QString>

#include "service_settings.h"

class ActionManagerSettings : public ServiceSettings
{

    protected:

        //! Internal initialization function.
        void _init(const ActionManagerSettings *pActionManagerSettings = nullptr);

    protected:

        //! Path to file store.
        QString fileName;

    public:

        //! Constructor.
        ActionManagerSettings();

        //! Copy constructor.
        ActionManagerSettings(const ActionManagerSettings &fileManagerSettings);

        //! Destructor.
        ~ActionManagerSettings() {}

        //! Assignment operator.
        ActionManagerSettings &operator =(const ActionManagerSettings &fileManagerSettings);

        //! Get const reference to file store.
        const QString &getFileName() const;

        //! Set new file store.
        void setFileName(const QString &fileName);

};

#endif // ACTION_MANAGER_SETTINGS_H
