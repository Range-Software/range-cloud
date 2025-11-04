#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include <QObject>
#include <QMap>
#include <QFile>
#include <QQueue>
#include <QUuid>
#include <QMutex>

#include <rbl_job.h>

#include "file_index.h"
#include "file_manager_settings.h"
#include "file_manager_statistics.h"
#include "file_manager_task.h"
#include "file_object.h"
#include "user_manager.h"

class FileManager : public RJob
{
    Q_OBJECT

    private:

        //! File manager settings.
        FileManagerSettings settings;
        //! File manager satistics.
        FileManagerStatistics statistics;

        //! User manager service.
        const UserManager *userManager;

        //! Flag signaling to stop service.
        bool stopFlag;
        //! File store path.
        QString storePath;
        //! Index file.
        QString indexFileName;
        //! Index map.
        FileIndex fileIndex;

        QQueue<FileManagerTask> tasks;

        QMutex syncMutex;
        QMutex serviceMutex;

        //! Total file size in store.
        qint64 totalSize;

    public:

        //! Constructor.
        explicit FileManager(const FileManagerSettings &fileManagerSettings, const UserManager *userManager);

        //! Run server.
        virtual int perform() override final;

        //! Request stop service.
        void stop();

        //! Request list files.
        QUuid requestListFiles(const RUserInfo &executor, FileObject *object);

        //! Request file information.
        QUuid requestFileInfo(const RUserInfo &executor, FileObject *object);

        //! Request store file.
        QUuid requestStoreFile(const RUserInfo &executor, FileObject *object);

        //! Request replace file.
        QUuid requestReplaceFile(const RUserInfo &executor, FileObject *object);

        //! Request update file.
        QUuid requestUpdateFile(const RUserInfo &executor, FileObject *object);

        //! Request update file access mode.
        QUuid requestUpdateFileAccessOwner(const RUserInfo &executor, FileObject *object);

        //! Request update file access mode.
        QUuid requestUpdateFileAccessMode(const RUserInfo &executor, FileObject *object);

        //! Request update file version.
        QUuid requestUpdateFileVersion(const RUserInfo &executor, FileObject *object);

        //! Request update file tags.
        QUuid requestUpdateFileTags(const RUserInfo &executor, FileObject *object);

        //! Request retrieve file.
        QUuid requestRetrieveFile(const RUserInfo &executor, FileObject *object);

        //! Request remove file.
        QUuid requestRemoveFile(const RUserInfo &executor, FileObject *object);

        //! Get statistics output in Json form.
        QJsonObject getStatisticsJson() const;

        //! Return const pointer to user manager.
        const UserManager *getUserManager() const;

        //! Return const reference to file index.
        const FileIndex &getFileIndex() const;

    private:

        //! Initialize the store.
        void initialize();

        //! Enqueue task.
        QUuid enqueueTask(const FileManagerTask &task);

        //! Build abolute path to file in stroe.
        QString findFilePath(const RFileInfo &fileInfo) const;

        //! List files.
        RError::Type listFiles(const RUserInfo &executor, QByteArray &output) const;

        //! Detailed file information.
        RError::Type fileInfo(const RUserInfo &executor, const QUuid &id, QByteArray &output) const;

        //! Store file.
        RError::Type storeFile(const RUserInfo &executor, const FileObject &object, QByteArray &output);

        //! Replace file.
        RError::Type replaceFile(const RUserInfo &executor, const FileObject &object, QByteArray &output);

        //! Update file.
        RError::Type updateFile(const RUserInfo &executor, const FileObject &object, QByteArray &output);

        //! Update file ownership.
        RError::Type updateFileAccessOwner(const RUserInfo &executor, const FileObject &object, QByteArray &output);

        //! Update file access mode.
        RError::Type updateFileAccessMode(const RUserInfo &executor, const FileObject &object, QByteArray &output);

        //! Update file version.
        RError::Type updateFileVersion(const RUserInfo &executor, const FileObject &object, QByteArray &output);

        //! Update file tags.
        RError::Type updateFileTags(const RUserInfo &executor, const FileObject &object, QByteArray &output);

        //! Retrieve file.
        RError::Type retrieveFile(const RUserInfo &executor, FileObject &object, QByteArray &output);

        //! Remove file.
        RError::Type removeFile(const RUserInfo &executor, const QUuid &id, QByteArray &output);

    signals:

        //! Service is ready.
        void ready();

        //! Request completed.
        void requestCompleted(const QUuid &requestId, const FileObject *object);

};

#endif // FILE_MANAGER_H
