#include "file_manager_statistics.h"

const QString FileManagerStatistics::Type::FileSizeStore = "file-size-store";
const QString FileManagerStatistics::Type::FileSizeUpdate = "file-size-update";
const QString FileManagerStatistics::Type::FileSizeRetrieve = "file-size-retrieve";
const QString FileManagerStatistics::Type::FileSizeRemove = "file-size-remove";

void FileManagerStatistics::_init(const FileManagerStatistics *pFileManagerStatistics)
{
    if (pFileManagerStatistics)
    {
    }
}

FileManagerStatistics::FileManagerStatistics()
{
    this->_init();
    this->name = "FileService";
}

FileManagerStatistics::FileManagerStatistics(const FileManagerStatistics &fileManagerStatistics)
    : ServiceStatistics(fileManagerStatistics)
{
    this->_init(&fileManagerStatistics);
}

FileManagerStatistics &FileManagerStatistics::operator =(const FileManagerStatistics &fileManagerStatistics)
{
    this->_init(&fileManagerStatistics);
    return (*this);
}
