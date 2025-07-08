#ifndef ACTION_HANDLER_H
#define ACTION_HANDLER_H

#include <QObject>
#include <QMap>

#include <rcl_cloud_action.h>

#include "action_manager.h"
#include "file_manager.h"
#include "mailer.h"
#include "process_manager.h"
#include "report_manager.h"
#include "user_manager.h"

class ActionHandler : public QObject
{

    Q_OBJECT

    protected:

        //! Pointer to user manager.
        UserManager *userManager;
        //! Pointer to action manager.
        ActionManager *actionManager;
        //! Pointer to process manager.
        ProcessManager *processManager;
        //! Pointer to file manager.
        FileManager *fileManager;
        //! Pointer to process manager.
        ReportManager *reportManager;
        //! Pointer to mailer.
        Mailer *mailer;

        //! Map of file requests.
        QMap<QUuid,QUuid> fileRequests;
        //! Map of process requests.
        QMap<QUuid,QUuid> processRequests;

    public:

        //! Constructor.
        explicit ActionHandler(UserManager *userManager,
                               ActionManager *actionManager,
                               ProcessManager *processManager,
                               FileManager *fileManager,
                               ReportManager *reportManager,
                               Mailer *mailer,
                               QObject *parent = nullptr);

        //! Resolve action.
        void resolveAction(const RCloudAction &action, const QString &from);

    protected slots:

        //! File manager request is completed.
        void onFileRequestCompleted(const QUuid &requestId, const FileObject *object);
        //! Process manager request is completed.
        void onProcessRequestCompleted(const QUuid &requestId, const RCloudProcessResult &result);

    signals:

        //! Action resolved.
        void resolved(const RCloudAction &action);

};

#endif // ACTION_HANDLER_H
