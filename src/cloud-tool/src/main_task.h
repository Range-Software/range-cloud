#ifndef MAIN_TASK_H
#define MAIN_TASK_H

#include <QObject>
#include <QThread>

#include "application.h"

class MainTask : public QObject
{

    Q_OBJECT

    protected:

        //! Application.
        Application *application;

    public:

        //! Constructor.
        explicit MainTask(Application *application);

    protected slots:

        //! Run task.
        void run();

        //! Task has finished.
        void actionFinished(const QSharedPointer<RToolAction> &action);

        //! Task has failed.
        void actionFailed(const QSharedPointer<RToolAction> &action);

        //! Task has finished.
        void taskFinished();

        //! Task has failed.
        void taskFailed();

};

#endif // MAIN_TASK_H
