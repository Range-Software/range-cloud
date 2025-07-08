#ifndef FILE_MANAGER_STATISTICS_H
#define FILE_MANAGER_STATISTICS_H

#include <QString>

#include "service_statistics.h"

class FileManagerStatistics : public ServiceStatistics
{

    public:

        struct Type
        {
            static const QString FileSizeStore;
            static const QString FileSizeUpdate;
            static const QString FileSizeRetrieve;
            static const QString FileSizeRemove;
        };

    protected:

        //! Internal initialization function.
        void _init(const FileManagerStatistics *pFileManagerStatistics = nullptr);

    public:

        //! Constructor.
        FileManagerStatistics();

        //! Copy constructor.
        FileManagerStatistics(const FileManagerStatistics &fileManagerStatistics);

        //! Destructor.
        ~FileManagerStatistics() {}

        //! Assignment operator.
        FileManagerStatistics &operator =(const FileManagerStatistics &fileManagerStatistics);

};

#endif // FILE_MANAGER_STATISTICS_H
