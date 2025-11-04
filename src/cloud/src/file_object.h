#ifndef FILE_OBJECT_H
#define FILE_OBJECT_H

#include <QByteArray>
#include <QString>
#include <QUuid>

#include <rbl_error.h>
#include <rcl_file_info.h>

class FileObject
{

    protected:

        //! Internal initialization function.
        void _init(const FileObject *pFileObject = nullptr);

    protected:

        //! File info.
        RFileInfo info;
        //! File content.
        QByteArray content;
        //! Error type.
        RError::Type errorType;

    public:

        //! Constructor.
        FileObject();

        //! Copy constructor.
        FileObject(const FileObject &fileObject);

        //! Destructor.
        ~FileObject();

        //! Assignment operator.
        FileObject &operator =(const FileObject &fileObject);

        //! Get const reference to file information.
        const RFileInfo &getInfo() const;

        //! Get reference to file information.
        RFileInfo &getInfo();

        //! Set new file information.
        void setInfo(const RFileInfo &info);

        //! Get const reference to file content.
        const QByteArray &getContent() const;

        //! Get reference to file content.
        QByteArray &getContent();

        //! Set new file content.
        void setContent(const QByteArray &content);

        //! Get error type.
        RError::Type getErrorType() const;

        //! Set error type.
        void setErrorType(RError::Type errorType);

};

#endif // FILE_OBJECT_H
