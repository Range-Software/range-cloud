#include "file_object.h"

void FileObject::_init(const FileObject *pFileObject)
{
    if (pFileObject)
    {
        this->info = pFileObject->info;
        this->content = pFileObject->content;
        this->errorType = pFileObject->errorType;
    }
}

FileObject::FileObject()
    : errorType(RError::None)
{
    this->_init();
}

FileObject::FileObject(const FileObject &fileObject)
{
    this->_init(&fileObject);
}

FileObject::~FileObject()
{

}

const RFileInfo &FileObject::getInfo() const
{
    return this->info;
}

RFileInfo &FileObject::getInfo()
{
    return this->info;
}

void FileObject::setInfo(const RFileInfo &info)
{
    this->info = info;
}

const QByteArray &FileObject::getContent() const
{
    return this->content;
}

QByteArray &FileObject::getContent()
{
    return this->content;
}

void FileObject::setContent(const QByteArray &content)
{
    this->content = content;
}

RError::Type FileObject::getErrorType() const
{
    return this->errorType;
}

void FileObject::setErrorType(RError::Type errorType)
{
    this->errorType = errorType;
}
