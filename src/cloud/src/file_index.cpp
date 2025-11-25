#include <QFile>
#include <QTextStream>

#include <rbl_error.h>
#include <rbl_logger.h>
#include <rbl_statistics.h>

#include "file_index.h"

void FileIndex::_init(const FileIndex *pFileIndex)
{
    if (pFileIndex)
    {
        this->index = pFileIndex->index;
    }
}

FileIndex::FileIndex()
{
    this->_init();
}

FileIndex::FileIndex(const FileIndex &fileIndex)
{
    this->_init(&fileIndex);
}

FileIndex::~FileIndex()
{

}

FileIndex &FileIndex::operator =(const FileIndex &fileIndex)
{
    this->_init(&fileIndex);
    return (*this);
}

void FileIndex::readFromFile(const QString &fileName)
{
    QFile indexFile(fileName);
    if (!indexFile.exists())
    {
        return;
    }

    if(!indexFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        throw RError(RError::Type::OpenFile,R_ERROR_REF,
                     "Failed to open index file \"%s\" for reading. %s.",
                     indexFile.fileName().toUtf8().constData(),
                     indexFile.errorString().toUtf8().constData());
    }

    QTextStream in(&indexFile);

    while(!in.atEnd())
    {
        RFileInfo info = RFileInfo::fromString(in.readLine());
        this->index.insert(info.getId(),info);
    }

    indexFile.close();
}

void FileIndex::writeToFile(const QString &fileName) const
{
    QFile indexFile(fileName);
    if(!indexFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        throw RError(RError::Type::OpenFile,R_ERROR_REF,
                     "Failed to open index file \"%s\" for writing. %s.",
                     indexFile.fileName().toUtf8().constData(),
                     indexFile.errorString().toUtf8().constData());
    }
    QTextStream out(&indexFile);

    for (auto iter = this->index.cbegin(); iter != this->index.cend(); ++iter)
    {
        out << this->index.value(iter.key()).toString() << "\n";
    }

    indexFile.close();
}

void FileIndex::registerObject(const RFileInfo &fileInfo)
{
    this->index.insert(fileInfo.getId(),fileInfo);
}

RFileInfo FileIndex::unregisterObject(const QUuid &id)
{
    return this->index.take(id);
}

bool FileIndex::objectExists(QUuid objectId) const
{
    return this->index.contains(objectId);
}

RFileInfo FileIndex::getObjectInfo(const QUuid &id) const
{
    return this->index[id];
}

qsizetype FileIndex::getSize() const
{
    return this->index.size();
}

qint64 FileIndex::findStoreSize(const QString &user) const
{
    qint64 totalSize = 0;

    for (auto it = this->index.cbegin(); it != this->index.cend(); ++it)
    {
        if (user.isEmpty() || user == it.value().getAccessRights().getOwner().getUser())
        {
            totalSize += it.value().getSize();
        }
    }

    return totalSize;
}

qint64 FileIndex::findStoreCount(const QString &user) const
{
    qint64 totalCount = 0;

    for (auto it = this->index.cbegin(); it != this->index.cend(); ++it)
    {
        if (user.isEmpty() || user == it.value().getAccessRights().getOwner().getUser())
        {
            totalCount++;
        }
    }

    return totalCount;
}

QJsonObject FileIndex::getStatisticsJson() const
{
    RLogger::debug("[%s] Producting statistics\n",QString("FileIndex").toUtf8().constData());
    QJsonObject jObject;

    RRVector fileSize(this->index.count());

    uint i=0;
    for (auto it = this->index.cbegin(); it != this->index.cend(); ++it)
    {
        fileSize[i++] = it.value().getSize();
    }

    jObject["files"] = RStatistics(fileSize).toJson();
    jObject["bytes"] = this->findStoreSize();
    jObject["size"] = this->getSize();

    return jObject;
}
