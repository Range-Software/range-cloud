#ifndef FILE_MANAGER_TASK_H
#define FILE_MANAGER_TASK_H

#include <QString>

#include <rcl_user_info.h>

#include "file_object.h"

class FileManagerTask
{

    protected:

        //! Internal initialization function.
        void _init(const FileManagerTask *pFileManagerTask = nullptr);

    public:

        enum Action
        {
            NoAction = 0,
            ListFiles,
            FileInfo,
            StoreFile,
            ReplaceFile,
            UpdateFile,
            UpdateFileAccessOwner,
            UpdateFileAccessMode,
            UpdateFileVersion,
            UpdateFileTags,
            RetrieveFile,
            RemoveFile,
            NTypes
        };

    protected:

        QUuid id;
        RUserInfo executor;
        Action action;
        FileObject *object;

    public:

        //! Constructor.
        explicit FileManagerTask(const RUserInfo &executor, Action action, FileObject *object);

        //! Copy constructor.
        FileManagerTask(const FileManagerTask &fileManagerTask);

        //! Destructor.
        ~FileManagerTask();

        //! Assignment operator.
        FileManagerTask &operator =(const FileManagerTask &fileManagerTask);

        //! Get action.
        const QUuid &getId() const;

        //! Get executor.
        const RUserInfo &getExecutor() const;

        //! Get action.
        Action getAction() const;

        //! Get const pointer to object.
        const FileObject *getObject() const;

        //! Get pointer to object.
        FileObject *getObject();

        static QString actionToString(const FileManagerTask::Action &action);

};

#endif // FILE_MANAGER_TASK_H
