#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <rcl_file_tools.h>
#include <rbl_logger.h>
#include <rbl_error.h>

#include "file_manager.h"

FileManager::FileManager(const FileManagerSettings &fileManagerSettings,
                         const UserManager *userManager)
    : settings{fileManagerSettings}
    , userManager{userManager}
    , stopFlag{false}
    , totalSize{0}
{
    R_LOG_TRACE_IN;
    this->setBlocking(false);
    this->setParallel(true);

    this->initialize();
    R_LOG_TRACE_OUT;
}

int FileManager::perform()
{
    R_LOG_TRACE_IN;
    try
    {
        this->serviceMutex.lock();
        emit this->ready();

        bool safeStopFlag = false;
        while (!safeStopFlag)
        {
            RLogger::trace("[%s] Loop\n",this->settings.getName().toUtf8().constData());
            this->syncMutex.lock();

            if (!this->tasks.isEmpty())
            {
                RLogger::trace("[%s] Processing task\n",this->settings.getName().toUtf8().constData());

                bool writeIndex = false;
                FileManagerTask task = this->tasks.dequeue();
                RError::Type resultErrorType = RError::None;
                QByteArray result;

                if (task.getAction() == FileManagerTask::Action::ListFiles)
                {
                    resultErrorType = this->listFiles(task.getExecutor(),result);
                    writeIndex = false;
                }
                else if (task.getAction() == FileManagerTask::Action::FileInfo)
                {
                    resultErrorType = this->fileInfo(task.getExecutor(),task.getObject()->getInfo().getId(),result);
                    writeIndex = false;
                }
                else if (task.getAction() == FileManagerTask::Action::StoreFile)
                {
                    resultErrorType = this->storeFile(task.getExecutor(),*task.getObject(),result);
                    writeIndex = true;
                }
                else if (task.getAction() == FileManagerTask::Action::UpdateFile)
                {
                    resultErrorType = this->updateFile(task.getExecutor(),*task.getObject(),result);
                    writeIndex = true;
                }
                else if (task.getAction() == FileManagerTask::Action::UpdateFileAccessOwner)
                {
                    resultErrorType = this->updateFileAccessOwner(task.getExecutor(),*task.getObject(),result);
                    writeIndex = true;
                }
                else if (task.getAction() == FileManagerTask::Action::UpdateFileAccessMode)
                {
                    resultErrorType = this->updateFileAccessMode(task.getExecutor(),*task.getObject(),result);
                    writeIndex = true;
                }
                else if (task.getAction() == FileManagerTask::Action::UpdateFileVersion)
                {
                    resultErrorType = this->updateFileVersion(task.getExecutor(),*task.getObject(),result);
                    writeIndex = true;
                }
                else if (task.getAction() == FileManagerTask::Action::UpdateFileTags)
                {
                    resultErrorType = this->updateFileTags(task.getExecutor(),*task.getObject(),result);
                    writeIndex = true;
                }
                else if (task.getAction() == FileManagerTask::Action::RetrieveFile)
                {
                    resultErrorType = this->retrieveFile(task.getExecutor(),*task.getObject(),result);
                    writeIndex = false;
                }
                else if (task.getAction() == FileManagerTask::Action::RemoveFile)
                {
                    resultErrorType = this->removeFile(task.getExecutor(),task.getObject()->getInfo().getId(),result);
                    writeIndex = true;
                }
                else
                {
                    RLogger::error("[%s] Unknown task \"%d\"\n",
                                   this->settings.getName().toUtf8().constData(),
                                   task.getAction());
                    resultErrorType = RError::Unknown;
                }

                task.getObject()->getContent().clear();
                task.getObject()->getContent().push_back(result);
                task.getObject()->setErrorType(resultErrorType);

                if (writeIndex)
                {
                    try
                    {
                        RLogger::info("[%s] Writing index file \"%s\".\n",
                                      this->settings.getName().toUtf8().constData(),
                                      this->indexFileName.toUtf8().constData());
                        this->fileIndex.writeToFile(this->indexFileName);
                    }
                    catch (const RError &error)
                    {
                        R_LOG_TRACE_OUT;
                        RLogger::error("[%s] Failed to write index file \"%s\". %s\n",
                                       this->settings.getName().toUtf8().constData(),
                                       this->indexFileName.toUtf8().constData(),
                                       error.getMessage().toUtf8().constData());
                    }
                }

                emit this->requestCompleted(task.getId(),task.getObject());
            }

            safeStopFlag = this->stopFlag;

            this->syncMutex.unlock();

            QThread::msleep(10);
        }
        this->syncMutex.lock();
        this->stopFlag = false;
        this->syncMutex.unlock();
        this->serviceMutex.unlock();
    }
    catch (const std::exception &e)
    {
        RLogger::error("%s\n",e.what());
    }
    catch (const RError &e)
    {
        RLogger::error("%s\n",e.getMessage().toUtf8().constData());
    }

    R_LOG_TRACE_RETURN(0);
}

void FileManager::stop()
{
    R_LOG_TRACE_IN;
    RLogger::info("[%s] Signal service to stop.\n",
                  this->settings.getName().toUtf8().constData());
    this->syncMutex.lock();
    this->stopFlag = true;
    this->syncMutex.unlock();

    while (!this->serviceMutex.tryLock())
    {
        QThread::msleep(10);
    }
    this->serviceMutex.unlock();
    RLogger::info("[%s] Service has been stopped.\n",
                  this->settings.getName().toUtf8().constData());
    R_LOG_TRACE_OUT;
}

QUuid FileManager::requestListFiles(const RUserInfo &executor, FileObject *object)
{
    return this->enqueueTask(FileManagerTask(executor,FileManagerTask::ListFiles,object));
}

QUuid FileManager::requestFileInfo(const RUserInfo &executor, FileObject *object)
{
    return this->enqueueTask(FileManagerTask(executor,FileManagerTask::FileInfo,object));
}

QUuid FileManager::requestStoreFile(const RUserInfo &executor, FileObject *object)
{
    return this->enqueueTask(FileManagerTask(executor,FileManagerTask::StoreFile,object));
}

QUuid FileManager::requestUpdateFile(const RUserInfo &executor, FileObject *object)
{
    return this->enqueueTask(FileManagerTask(executor,FileManagerTask::UpdateFile,object));
}

QUuid FileManager::requestUpdateFileAccessOwner(const RUserInfo &executor, FileObject *object)
{
    return this->enqueueTask(FileManagerTask(executor,FileManagerTask::UpdateFileAccessOwner,object));
}

QUuid FileManager::requestUpdateFileAccessMode(const RUserInfo &executor, FileObject *object)
{
    return this->enqueueTask(FileManagerTask(executor,FileManagerTask::UpdateFileAccessMode,object));
}

QUuid FileManager::requestUpdateFileVersion(const RUserInfo &executor, FileObject *object)
{
    return this->enqueueTask(FileManagerTask(executor,FileManagerTask::UpdateFileVersion,object));
}

QUuid FileManager::requestUpdateFileTags(const RUserInfo &executor, FileObject *object)
{
    return this->enqueueTask(FileManagerTask(executor,FileManagerTask::UpdateFileTags,object));
}

QUuid FileManager::requestRetrieveFile(const RUserInfo &executor, FileObject *object)
{
    return this->enqueueTask(FileManagerTask(executor,FileManagerTask::RetrieveFile,object));
}

QUuid FileManager::requestRemoveFile(const RUserInfo &executor, FileObject *object)
{
    return this->enqueueTask(FileManagerTask(executor,FileManagerTask::RemoveFile,object));
}

QJsonObject FileManager::getStatisticsJson() const
{
    RLogger::debug("[%s] Producting statistics\n",this->settings.getName().toUtf8().constData());
    QJsonObject jObject = this->statistics.toJson();
    jObject["index"] = this->fileIndex.getStatisticsJson();
    return jObject;
}

const UserManager *FileManager::getUserManager() const
{
    return this->userManager;
}

const FileIndex &FileManager::getFileIndex() const
{
    return this->fileIndex;
}

void FileManager::initialize()
{
    R_LOG_TRACE_IN;
    RLogger::info("[%s] Store path: \"%s\"\n",
                  this->settings.getName().toUtf8().constData(),
                  this->settings.getFileStore().toUtf8().constData());

    QDir storeDir(this->settings.getFileStore());

    this->storePath = storeDir.absolutePath();
    this->indexFileName = storeDir.absoluteFilePath("index.txt");

    if (!storeDir.exists() && !storeDir.mkpath(this->settings.getFileStore()))
    {
        RLogger::error("[%s] Failed to create path \"%s\".\n",
                       this->settings.getName().toUtf8().constData(),
                       this->settings.getFileStore().toUtf8().constData());
    }

    try
    {
        RLogger::info("[%s] Reading index file \"%s\".\n",
                      this->settings.getName().toUtf8().constData(),
                      this->indexFileName.toUtf8().constData());
        this->fileIndex.readFromFile(this->indexFileName);
        this->totalSize = this->fileIndex.findStoreSize();
    }
    catch (const RError &error)
    {
        RLogger::error("[%s] Failed to read index file \"%s\". %s\n",
                       this->settings.getName().toUtf8().constData(),
                       this->indexFileName.toUtf8().constData(),
                       error.getMessage().toUtf8().constData());
    }
    R_LOG_TRACE_OUT;
}

QUuid FileManager::enqueueTask(const FileManagerTask &task)
{
    R_LOG_TRACE_IN;
    RLogger::debug("[%s] request-id=\"%s\", action=\"%s\", executor=\"%s\", object-id=\"%s\"\n",
                   this->settings.getName().toUtf8().constData(),
                   task.getId().toString(QUuid::WithoutBraces).toUtf8().constData(),
                   FileManagerTask::actionToString(task.getAction()).toUtf8().constData(),
                   task.getExecutor().getName().toUtf8().constData(),
                   task.getObject()->getInfo().getId().toString(QUuid::WithoutBraces).toUtf8().constData());
    this->syncMutex.lock();
    this->tasks.enqueue(task);
    this->syncMutex.unlock();
    R_LOG_TRACE_RETURN(task.getId());
}

QString FileManager::findFilePath(const RFileInfo &fileInfo) const
{
    QDir storeDir(this->storePath);
    return storeDir.absoluteFilePath(fileInfo.getId().toString(QUuid::WithoutBraces));
}

RError::Type FileManager::listFiles(const RUserInfo &executor, QByteArray &output) const
{
    R_LOG_TRACE_IN;

    RLogger::debug("[%s] listFiles: executor=\"%s\".\n",
                   this->settings.getName().toUtf8().constData(),
                   executor.getName().toUtf8().constData());

    QList<RFileInfo> files = this->fileIndex.listUserObjects(
        [=](const RFileInfo &fileInfo)
        {
            return UserManager::authorizeUserAccess(executor,fileInfo.getAccessRights(),RAccessMode::Read);
        }
    );

    QJsonObject filesJson;

    QJsonArray filesArray;
    for (const RFileInfo &fileInfo : std::as_const(files))
    {
        filesArray.append(fileInfo.toJson());
    }
    filesJson["files"] = filesArray;
    output = QJsonDocument(filesJson).toJson();

    R_LOG_TRACE_RETURN(RError::None);
}

RError::Type FileManager::fileInfo(const RUserInfo &executor, const QUuid &id, QByteArray &output) const
{
    R_LOG_TRACE_IN;

    RLogger::debug("[%s] fileInfo: executor=\"%s\".\n",
                   this->settings.getName().toUtf8().constData(),
                   executor.getName().toUtf8().constData());

    if (!this->fileIndex.objectExists(id))
    {
        output = QString("File object \"%1\" does not exist").arg(id.toString(QUuid::WithoutBraces)).toUtf8();
        RLogger::error("[%s] %s.\n",
                       this->settings.getName().toUtf8().constData(),
                       output.constData());
        R_LOG_TRACE_RETURN(RError::InvalidInput);
    }

    RFileInfo fileInfo(this->fileIndex.getObjectInfo(id));

    if (!UserManager::authorizeUserAccess(executor,fileInfo.getAccessRights(),RAccessMode::Read))
    {
        output = QString("User \"%1\" is not authorized to retrieve file id=\"%2\"").arg(executor.getName(),id.toString(QUuid::WithoutBraces)).toUtf8();
        RLogger::error("[%s] %s.\n",
                       this->settings.getName().toUtf8().constData(),
                       output.constData());
        R_LOG_TRACE_RETURN(RError::Unauthorized);
    }

    output = QJsonDocument(fileInfo.toJson()).toJson();

    R_LOG_TRACE_RETURN(RError::None);
}

RError::Type FileManager::storeFile(const RUserInfo &executor, const FileObject &object, QByteArray &output)
{
    R_LOG_TRACE_IN;
    RLogger::debug("[%s] storeFile: executor=\"%s\", storePath=\"%s\".\n",
                   this->settings.getName().toUtf8().constData(),
                   executor.getName().toUtf8().constData(),
                   this->storePath.toUtf8().constData());

    if (!UserManager::authorizeUserAccess(executor,object.getInfo().getAccessRights(),RAccessMode::Write))
    {
        output = QString("User \"%1\" is not authorized to store file id=\"%2\"").arg(executor.getName(),object.getInfo().getId().toString(QUuid::WithoutBraces)).toUtf8();
        RLogger::error("[%s] %s.\n",
                       this->settings.getName().toUtf8().constData(),
                       output.constData());
        R_LOG_TRACE_RETURN(RError::Unauthorized);
    }

    if (this->settings.getMaxFileSize() > 0)
    {
        if (object.getContent().size() > this->settings.getMaxFileSize())
        {
            output = QString("Invalid file size \"%1 bytes\" (max: \"%2 bytes\")").arg(object.getContent().size()).arg(this->settings.getMaxFileSize()).toUtf8();
            RLogger::error("[%s] %s.\n",
                           this->settings.getName().toUtf8().constData(),
                           output.constData());
            R_LOG_TRACE_RETURN(RError::InvalidInput);
        }
    }

    if (this->settings.getMaxStoreSize() > 0)
    {
        if (object.getContent().size() + this->totalSize > this->settings.getMaxStoreSize())
        {
            output = QString("Invalid file size \"%1 bytes\". File store is full.").arg(object.getContent().size()).toUtf8();
            RLogger::error("[%s] %s.\n",
                           this->settings.getName().toUtf8().constData(),
                           output.constData());
            R_LOG_TRACE_RETURN(RError::InvalidInput);
        }
    }

    if (!RFileInfo::isPathValid(object.getInfo().getPath()))
    {
        output = QString("Invalid path \"%1\"").arg(object.getInfo().getPath()).toUtf8();
        RLogger::error("[%s] %s.\n",
                       this->settings.getName().toUtf8().constData(),
                       output.constData());
        R_LOG_TRACE_RETURN(RError::InvalidInput);
    }

    RFileInfo fileInfo(object.getInfo());

    if (!RFileTools::writeBinaryFile(this->findFilePath(fileInfo),object.getContent()))
    {
        output = QString("Failed to write file id=\"%1\"").arg(fileInfo.getId().toString(QUuid::WithoutBraces)).toUtf8();
        RLogger::error("[%s] %s.\n",
                       this->settings.getName().toUtf8().constData(),
                       output.constData());
        R_LOG_TRACE_RETURN(RError::WriteFile);
    }

    fileInfo.setSize(QFileInfo(this->findFilePath(fileInfo)).size());
    fileInfo.setMd5Checksum(RFileInfo::findMd5Checksum(this->findFilePath(fileInfo)));

    this->fileIndex.registerObject(fileInfo);

    this->totalSize += fileInfo.getSize();
    this->statistics.recordValue(FileManagerStatistics::Type::FileSizeStore,double(fileInfo.getSize()));

    output = QJsonDocument(fileInfo.toJson()).toJson();

    R_LOG_TRACE_RETURN(RError::None);
}

RError::Type FileManager::updateFile(const RUserInfo &executor, const FileObject &object, QByteArray &output)
{
    R_LOG_TRACE_IN;
    RLogger::debug("[%s] updateFile: executor=\"%s\", storePath=\"%s\".\n",
                   this->settings.getName().toUtf8().constData(),
                   executor.getName().toUtf8().constData(),
                   this->storePath.toUtf8().constData());

    if (!this->fileIndex.objectExists(object.getInfo().getId()))
    {
        output = QString("File object \"%1\" does not exist").arg(object.getInfo().getId().toString(QUuid::WithoutBraces)).toUtf8();
        RLogger::error("[%s] %s.\n",
                       this->settings.getName().toUtf8().constData(),
                       output.constData());
        R_LOG_TRACE_RETURN(RError::InvalidInput);
    }

    RFileInfo fileInfo(this->fileIndex.getObjectInfo(object.getInfo().getId()));

    if (!UserManager::authorizeUserAccess(executor,fileInfo.getAccessRights(),RAccessMode::Write))
    {
        output = QString("User \"%1\" is not authorized to update file id=\"%2\"").arg(executor.getName(),fileInfo.getId().toString(QUuid::WithoutBraces)).toUtf8();
        RLogger::error("[%s] %s.\n",
                       this->settings.getName().toUtf8().constData(),
                       output.constData());
        R_LOG_TRACE_RETURN(RError::Unauthorized);
    }

    if (!RFileInfo::isPathValid(object.getInfo().getPath()))
    {
        output = QString("Invalid path \"%1\"").arg(object.getInfo().getPath()).toUtf8();
        RLogger::error("[%s] %s.\n",
                       this->settings.getName().toUtf8().constData(),
                       output.constData());
        R_LOG_TRACE_RETURN(RError::InvalidInput);
    }

    fileInfo.setPath(object.getInfo().getPath());
    fileInfo.setUpdateDateTime(QDateTime::currentSecsSinceEpoch());

    if (!RFileTools::writeBinaryFile(this->findFilePath(fileInfo),object.getContent()))
    {
        output = QString("Failed to write file id=\"%1\"").arg(fileInfo.getId().toString(QUuid::WithoutBraces)).toUtf8();
        RLogger::error("[%s] %s.\n",
                       this->settings.getName().toUtf8().constData(),
                       output.constData());
        R_LOG_TRACE_RETURN(RError::WriteFile);
    }

    qint64 oldSize = fileInfo.getSize();
    fileInfo.setSize(QFileInfo(this->findFilePath(fileInfo)).size());
    fileInfo.setMd5Checksum(RFileInfo::findMd5Checksum(this->findFilePath(fileInfo)));

    this->fileIndex.registerObject(fileInfo);

    this->totalSize += fileInfo.getSize() - oldSize;
    this->statistics.recordValue(FileManagerStatistics::Type::FileSizeUpdate,double(fileInfo.getSize()));

    output = QJsonDocument(fileInfo.toJson()).toJson();

    R_LOG_TRACE_RETURN(RError::None);
}

RError::Type FileManager::updateFileAccessOwner(const RUserInfo &executor, const FileObject &object, QByteArray &output)
{
    R_LOG_TRACE_IN;
    RLogger::debug("[%s] updateFileAccessOwner: executor=\"%s\", storePath=\"%s\".\n",
                   this->settings.getName().toUtf8().constData(),
                   executor.getName().toUtf8().constData(),
                   this->storePath.toUtf8().constData());

    if (!this->fileIndex.objectExists(object.getInfo().getId()))
    {
        output = QString("File object \"%1\" does not exist").arg(object.getInfo().getId().toString(QUuid::WithoutBraces)).toUtf8();
        RLogger::error("[%s] %s.\n",
                       this->settings.getName().toUtf8().constData(),
                       output.constData());
        R_LOG_TRACE_RETURN(RError::InvalidInput);
    }

    RFileInfo fileInfo(this->fileIndex.getObjectInfo(object.getInfo().getId()));

    if (!object.getInfo().getAccessRights().getOwner().isValid())
    {
        output = QString("Invalid access owner").toUtf8();
        RLogger::error("[%s] %s.\n",
                       this->settings.getName().toUtf8().constData(),
                       output.constData());
        R_LOG_TRACE_RETURN(RError::InvalidInput);
    }

    RAccessRights accessRights = fileInfo.getAccessRights();
    accessRights.setOwner(object.getInfo().getAccessRights().getOwner());
    fileInfo.setAccessRights(accessRights);

    if (!this->userManager->containsUser(fileInfo.getAccessRights().getOwner().getUser()))
    {
        output = QString("Invalid access owner user \"%1\"").arg(fileInfo.getAccessRights().getOwner().getUser()).toUtf8();
        RLogger::error("[%s] %s.\n",
                       this->settings.getName().toUtf8().constData(),
                       output.constData());
        R_LOG_TRACE_RETURN(RError::InvalidInput);
    }

    if (!this->userManager->containsGroup(fileInfo.getAccessRights().getOwner().getGroup()))
    {
        output = QString("Invalid access owner group \"%1\"").arg(fileInfo.getAccessRights().getOwner().getGroup()).toUtf8();
        RLogger::error("[%s] %s.\n",
                       this->settings.getName().toUtf8().constData(),
                       output.constData());
        R_LOG_TRACE_RETURN(RError::InvalidInput);
    }

    this->fileIndex.registerObject(fileInfo);

    output = QJsonDocument(fileInfo.toJson()).toJson();

    R_LOG_TRACE_RETURN(RError::None);
}

RError::Type FileManager::updateFileAccessMode(const RUserInfo &executor, const FileObject &object, QByteArray &output)
{
    R_LOG_TRACE_IN;
    RLogger::debug("[%s] updateFileAccessMode: executor=\"%s\", storePath=\"%s\".\n",
                   this->settings.getName().toUtf8().constData(),
                   executor.getName().toUtf8().constData(),
                   this->storePath.toUtf8().constData());

    if (!this->fileIndex.objectExists(object.getInfo().getId()))
    {
        output = QString("File object \"%1\" does not exist").arg(object.getInfo().getId().toString(QUuid::WithoutBraces)).toUtf8();
        RLogger::error("[%s] %s.\n",
                       this->settings.getName().toUtf8().constData(),
                       output.constData());
        R_LOG_TRACE_RETURN(RError::InvalidInput);
    }

    RFileInfo fileInfo(this->fileIndex.getObjectInfo(object.getInfo().getId()));

    if (!UserManager::authorizeUserAccess(executor,fileInfo.getAccessRights(),RAccessMode::None))
    {
        output = QString("User \"%1\" is not authorized to change access mode of file id=\"%2\"").arg(executor.getName(),object.getInfo().getId().toString(QUuid::WithoutBraces)).toUtf8();
        RLogger::error("[%s] %s.\n",
                       this->settings.getName().toUtf8().constData(),
                       output.constData());
        R_LOG_TRACE_RETURN(RError::Unauthorized);
    }

    if (!object.getInfo().getAccessRights().getMode().isValid())
    {
        output = QString("Invalid access rights").toUtf8();
        RLogger::error("[%s] %s.\n",
                       this->settings.getName().toUtf8().constData(),
                       output.constData());
        R_LOG_TRACE_RETURN(RError::InvalidInput);
    }

    RAccessRights accessRights = fileInfo.getAccessRights();
    accessRights.setMode(object.getInfo().getAccessRights().getMode());
    fileInfo.setAccessRights(accessRights);

    this->fileIndex.registerObject(fileInfo);

    output = QJsonDocument(fileInfo.toJson()).toJson();

    R_LOG_TRACE_RETURN(RError::None);
}

RError::Type FileManager::updateFileVersion(const RUserInfo &executor, const FileObject &object, QByteArray &output)
{
    R_LOG_TRACE_IN;
    RLogger::debug("[%s] updateFileVersion: executor=\"%s\", storePath=\"%s\".\n",
                   this->settings.getName().toUtf8().constData(),
                   executor.getName().toUtf8().constData(),
                   this->storePath.toUtf8().constData());

    if (!this->fileIndex.objectExists(object.getInfo().getId()))
    {
        output = QString("File object \"%1\" does not exist").arg(object.getInfo().getId().toString(QUuid::WithoutBraces)).toUtf8();
        RLogger::error("[%s] %s.\n",
                       this->settings.getName().toUtf8().constData(),
                       output.constData());
        R_LOG_TRACE_RETURN(RError::InvalidInput);
    }

    RFileInfo fileInfo(this->fileIndex.getObjectInfo(object.getInfo().getId()));

    if (!UserManager::authorizeUserAccess(executor,fileInfo.getAccessRights(),RAccessMode::Write))
    {
        output = QString("User \"%1\" is not authorized to change version of file id=\"%2\"").arg(executor.getName(),object.getInfo().getId().toString(QUuid::WithoutBraces)).toUtf8();
        RLogger::error("[%s] %s.\n",
                       this->settings.getName().toUtf8().constData(),
                       output.constData());
        R_LOG_TRACE_RETURN(RError::Unauthorized);
    }

    fileInfo.setVersion(object.getInfo().getVersion());

    this->fileIndex.registerObject(fileInfo);

    output = QJsonDocument(fileInfo.toJson()).toJson();

    R_LOG_TRACE_RETURN(RError::None);
}

RError::Type FileManager::updateFileTags(const RUserInfo &executor, const FileObject &object, QByteArray &output)
{
    R_LOG_TRACE_IN;
    RLogger::debug("[%s] updateFileTags: executor=\"%s\", storePath=\"%s\".\n",
                   this->settings.getName().toUtf8().constData(),
                   executor.getName().toUtf8().constData(),
                   this->storePath.toUtf8().constData());

    if (!this->fileIndex.objectExists(object.getInfo().getId()))
    {
        output = QString("File object \"%1\" does not exist").arg(object.getInfo().getId().toString(QUuid::WithoutBraces)).toUtf8();
        RLogger::error("[%s] %s.\n",
                       this->settings.getName().toUtf8().constData(),
                       output.constData());
        R_LOG_TRACE_RETURN(RError::InvalidInput);
    }

    RFileInfo fileInfo(this->fileIndex.getObjectInfo(object.getInfo().getId()));

    if (!UserManager::authorizeUserAccess(executor,fileInfo.getAccessRights(),RAccessMode::Write))
    {
        output = QString("User \"%1\" is not authorized to change tags of file id=\"%2\"").arg(executor.getName(),object.getInfo().getId().toString(QUuid::WithoutBraces)).toUtf8();
        RLogger::error("[%s] %s.\n",
                       this->settings.getName().toUtf8().constData(),
                       output.constData());
        R_LOG_TRACE_RETURN(RError::Unauthorized);
    }

    const QStringList &tags = object.getInfo().getTags();

    if (tags.size() > RFileInfo::MaxNumTags)
    {
        output = QString("Invalid number of tags \"%1\" (max=\"%2\")").arg(QString::number(tags.size()),QString::number(RFileInfo::MaxNumTags)).toUtf8();
        RLogger::error("[%s] %s.\n",
                       this->settings.getName().toUtf8().constData(),
                       output.constData());
        R_LOG_TRACE_RETURN(RError::InvalidInput);
    }

    for (const QString &tag : tags)
    {
        if (!RFileInfo::isTagValid(tag))
        {
            output = QString("Invalid tag \"%1\"").arg(tag).toUtf8();
            RLogger::error("[%s] %s.\n",
                           this->settings.getName().toUtf8().constData(),
                           output.constData());
            R_LOG_TRACE_RETURN(RError::InvalidInput);
        }
    }

    fileInfo.setTags(object.getInfo().getTags());

    this->fileIndex.registerObject(fileInfo);

    output = QJsonDocument(fileInfo.toJson()).toJson();

    R_LOG_TRACE_RETURN(RError::None);
}

RError::Type FileManager::retrieveFile(const RUserInfo &executor, FileObject &object, QByteArray &output)
{
    R_LOG_TRACE_IN;
    RLogger::debug("[%s] retrieveFile: executor=\"%s\", storePath=\"%s\".\n",
                   this->settings.getName().toUtf8().constData(),
                   executor.getName().toUtf8().constData(),
                   this->storePath.toUtf8().constData());

    if (!this->fileIndex.objectExists(object.getInfo().getId()))
    {
        output = QString("File object \"%1\" does not exist").arg(object.getInfo().getId().toString(QUuid::WithoutBraces)).toUtf8();
        RLogger::error("[%s] %s.\n",
                       this->settings.getName().toUtf8().constData(),
                       output.constData());
        R_LOG_TRACE_RETURN(RError::InvalidInput);
    }

    object.setInfo(this->fileIndex.getObjectInfo(object.getInfo().getId()));

    if (!UserManager::authorizeUserAccess(executor,object.getInfo().getAccessRights(),RAccessMode::Read))
    {
        output = QString("User \"%1\" is not authorized to retrieve file id=\"%2\"").arg(executor.getName(),object.getInfo().getId().toString(QUuid::WithoutBraces)).toUtf8();
        RLogger::error("[%s] %s.\n",
                       this->settings.getName().toUtf8().constData(),
                       output.constData());
        R_LOG_TRACE_RETURN(RError::Unauthorized);
    }

    if (!RFileTools::readBinaryFile(this->findFilePath(object.getInfo()),output))
    {
        output = QString("Failed to read file id=\"%1\"").arg(object.getInfo().getId().toString(QUuid::WithoutBraces)).toUtf8();
        RLogger::error("[%s] %s.\n",
                       this->settings.getName().toUtf8().constData(),
                       output.constData());
        R_LOG_TRACE_RETURN(RError::ReadFile);
    }
    this->statistics.recordValue(FileManagerStatistics::Type::FileSizeRetrieve,double(object.getInfo().getSize()));

    R_LOG_TRACE_RETURN(RError::None);
}

RError::Type FileManager::removeFile(const RUserInfo &executor, const QUuid &id, QByteArray &output)
{
    R_LOG_TRACE_IN;
    RLogger::debug("[%s] removeFile: executor=\"%s\", storePath=\"%s\".\n",
                   this->settings.getName().toUtf8().constData(),
                   executor.getName().toUtf8().constData(),
                   this->storePath.toUtf8().constData());

    if (!this->fileIndex.objectExists(id))
    {
        output = QString("File object \"%1\" does not exist").arg(id.toString(QUuid::WithoutBraces)).toUtf8();
        RLogger::error("[%s] %s.\n",
                       this->settings.getName().toUtf8().constData(),
                       output.constData());
        R_LOG_TRACE_RETURN(RError::InvalidInput);
    }

    RFileInfo fileInfo = this->fileIndex.unregisterObject(id);

    if (!UserManager::authorizeUserAccess(executor,fileInfo.getAccessRights(),RAccessMode::Write))
    {
        output = QString("User \"%1\" is not authorized to remove file id=\"%2\"").arg(executor.getName(),fileInfo.getId().toString(QUuid::WithoutBraces)).toUtf8();
        RLogger::error("[%s] %s.\n",
                       this->settings.getName().toUtf8().constData(),
                       output.constData());
        R_LOG_TRACE_RETURN(RError::Unauthorized);
    }

    QDir storeDir(this->storePath);
    if (!storeDir.remove(fileInfo.getId().toString(QUuid::WithoutBraces)))
    {
        output = QString("Failed to remove file id=\"%1\"").arg(fileInfo.getId().toString(QUuid::WithoutBraces)).toUtf8();
        RLogger::error("[%s] %s.\n",
                       this->settings.getName().toUtf8().constData(),
                       output.constData());
        R_LOG_TRACE_RETURN(RError::WriteFile);
    }

    this->totalSize -= fileInfo.getSize();
    this->statistics.recordValue(FileManagerStatistics::Type::FileSizeRemove,double(fileInfo.getSize()));

    output = QJsonDocument(fileInfo.toJson()).toJson();
    R_LOG_TRACE_RETURN(RError::None);
}
