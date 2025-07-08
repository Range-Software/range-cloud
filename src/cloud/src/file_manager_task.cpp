#include "file_manager_task.h"

void FileManagerTask::_init(const FileManagerTask *pFileManagerTask)
{
    if (pFileManagerTask)
    {
        this->id = pFileManagerTask->id;
        this->executor = pFileManagerTask->executor;
        this->action = pFileManagerTask->action;
        this->object = pFileManagerTask->object;
    }
}

FileManagerTask::FileManagerTask(const RUserInfo &executor, Action action, FileObject *object) :
    id(QUuid::createUuid()),
    executor(executor),
    action(action),
    object(object)
{
    this->_init();
}

FileManagerTask::FileManagerTask(const FileManagerTask &fileManagerTask)
{
    this->_init(&fileManagerTask);
}

FileManagerTask::~FileManagerTask()
{

}

FileManagerTask &FileManagerTask::operator =(const FileManagerTask &fileManagerTask)
{
    this->_init(&fileManagerTask);
    return (*this);
}

const QUuid &FileManagerTask::getId() const
{
    return this->id;
}

const RUserInfo &FileManagerTask::getExecutor() const
{
    return this->executor;
}

FileManagerTask::Action FileManagerTask::getAction() const
{
    return this->action;
}

const FileObject *FileManagerTask::getObject() const
{
    return this->object;
}

FileObject *FileManagerTask::getObject()
{
    return this->object;
}

QString FileManagerTask::actionToString(const Action &action)
{
    switch (action)
    {
        case NoAction:
            return QString("No action");
        case ListFiles:
            return QString("List files");
        case FileInfo:
            return QString("File information");
        case StoreFile:
            return QString("Store file");
        case UpdateFile:
            return QString("Update file");
        case UpdateFileAccessOwner:
            return QString("Update file access owner");
        case UpdateFileAccessMode:
            return QString("Update file access mode");
        case UpdateFileVersion:
            return QString("Update file version");
        case UpdateFileTags:
            return QString("Update file tags");
        case RetrieveFile:
            return QString("Retrieve file");
        case RemoveFile:
            return QString("Remove file");
        default:
            return QString("Unknown");
    }
}
