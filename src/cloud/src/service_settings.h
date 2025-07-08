#ifndef SERVICE_SETTINGS_H
#define SERVICE_SETTINGS_H

#include <QString>

class ServiceSettings
{

    protected:

        //! Internal initialization function.
        void _init(const ServiceSettings *pServiceSettings = nullptr);

    protected:

        //! Identity name.
        QString name;

    public:

        //! Constructor.
        ServiceSettings();

        //! Copy constructor.
        ServiceSettings(const ServiceSettings &serviceSettings);

        //! Destructor.
        ~ServiceSettings() { }

        //! Assignment operator.
        ServiceSettings &operator =(const ServiceSettings &serviceSettings);

        //! Get const reference to name.
        const QString &getName() const;

        //! Set new name.
        void setName(const QString &name);

};

#endif // SERVICE_SETTINGS_H
