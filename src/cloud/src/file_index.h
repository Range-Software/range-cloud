#ifndef FILE_INDEX_H
#define FILE_INDEX_H

#include <QMap>
#include <QUuid>

#include <rcl_file_info.h>

class FileIndex
{

    protected:

        //! Internal initialization function.
        void _init(const FileIndex *pFileIndex = nullptr);

    protected:

        //! File info.
        QMap<QUuid,RFileInfo> index;

    public:

        //! Constructor.
        FileIndex();

        //! Copy constructor.
        FileIndex(const FileIndex &fileIndex);

        //! Destructor.
        ~FileIndex();

        //! Assignment operator.
        FileIndex &operator =(const FileIndex &fileIndex);

        //! Read index from file.
        void readFromFile(const QString &fileName);

        //! Read index to file.
        void writeToFile(const QString &fileName) const;

        //! List files for given user.
        template<typename AccessHandler> QList<RFileInfo> listUserObjects(AccessHandler &&accessHandler) const
        {
            QList<RFileInfo> fileList;

            for (auto iter = this->index.cbegin(); iter != this->index.cend(); ++iter)
            {
                const RFileInfo &fileInfo = this->index[iter.key()];
                if (accessHandler(fileInfo))
                {
                    fileList.append(fileInfo);
                }
            }

            return fileList;
        }

        //! Register file.
        void registerObject(const RFileInfo &fileInfo);

        //! Unregister file.
        RFileInfo unregisterObject(const QUuid &id);

        //! Find object.
        bool objectExists(QUuid objectId) const;

        //! Get object info.
        RFileInfo getObjectInfo(const QUuid &id) const;

        //! Return size of the index (number of entries).
        qsizetype getSize() const;

        //! Find store size.
        qint64 findStoreSize() const;

        //! Get statistics output in Json form.
        QJsonObject getStatisticsJson() const;

};

#endif // FILE_INDEX_H
