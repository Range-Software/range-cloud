#include "file_manager_settings.h"

void FileManagerSettings::_init(const FileManagerSettings *pFileManagerSettings)
{
    this->ServiceSettings::_init(pFileManagerSettings);
    if (pFileManagerSettings)
    {
        this->fileStore = pFileManagerSettings->fileStore;
        this->maxStoreSize = pFileManagerSettings->maxStoreSize;
        this->maxFileSize = pFileManagerSettings->maxFileSize;
    }
}

FileManagerSettings::FileManagerSettings()
    : maxStoreSize(-1)
    , maxFileSize(-1)
{
    this->_init();
    this->name = "FileService";
}

FileManagerSettings::FileManagerSettings(const FileManagerSettings &fileManagerSettings)
    : ServiceSettings(fileManagerSettings)
{
    this->_init(&fileManagerSettings);
}

FileManagerSettings &FileManagerSettings::operator =(const FileManagerSettings &fileManagerSettings)
{
    this->_init(&fileManagerSettings);
    return (*this);
}

const QString &FileManagerSettings::getFileStore() const
{
    return this->fileStore;
}

void FileManagerSettings::setFileStore(const QString &fileStore)
{
    this->fileStore = fileStore;
}

qint64 FileManagerSettings::getMaxStoreSize() const
{
    return this->maxStoreSize;
}

void FileManagerSettings::setMaxStoreSize(qint64 maxStoreSize)
{
    this->maxStoreSize = maxStoreSize;
}

qint64 FileManagerSettings::getMaxFileSize() const
{
    return this->maxFileSize;
}

void FileManagerSettings::setMaxFileSize(qint64 maxFileSize)
{
    this->maxFileSize = maxFileSize;
}
