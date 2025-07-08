#ifndef FILE_MANAGER_SETTINGS_H
#define FILE_MANAGER_SETTINGS_H

#include <QString>

#include "service_settings.h"

class FileManagerSettings : public ServiceSettings
{

    protected:

        //! Internal initialization function.
        void _init(const FileManagerSettings *pFileManagerSettings = nullptr);

    protected:

        //! Path to file store.
        QString fileStore;
        //! Max store size.
        qint64 maxStoreSize;
        //! Max file size.
        qint64 maxFileSize;

    public:

        //! Constructor.
        FileManagerSettings();

        //! Copy constructor.
        FileManagerSettings(const FileManagerSettings &fileManagerSettings);

        //! Destructor.
        ~FileManagerSettings() {}

        //! Assignment operator.
        FileManagerSettings &operator =(const FileManagerSettings &fileManagerSettings);

        //! Get const reference to file store.
        const QString &getFileStore() const;

        //! Set new file store.
        void setFileStore(const QString &fileStore);

        //! Return maximum store size.
        qint64 getMaxStoreSize() const;

        //! Set maximum store size.
        void setMaxStoreSize(qint64 maxStoreSize);

        //! Return maximum file size.
        qint64 getMaxFileSize() const;

        //! Set maximum file size.
        void setMaxFileSize(qint64 maxFileSize);

};

#endif // FILE_MANAGER_SETTINGS_H
