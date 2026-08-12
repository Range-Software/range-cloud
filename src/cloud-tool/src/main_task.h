#ifndef MAIN_TASK_H
#define MAIN_TASK_H

#include <QObject>
#include <QThread>
#include <QUuid>

#include "application.h"

class MainTask : public QObject
{

    Q_OBJECT

    protected:

        //! Interval between AI query result polls.
        static const int aiQueryPollIntervalMs = 1000;
        //! Maximum number of AI query result polls before giving up.
        static const uint aiQueryMaxPolls = 600;

        //! Application.
        Application *application;

        //! An AI query is awaiting its result; defers application shutdown.
        bool aiQueryInFlight;
        //! Number of AI query result polls performed so far.
        uint aiQueryPollCount;

    public:

        //! Constructor.
        explicit MainTask(Application *application);

    private:

        //! Schedule a poll for the result of a pending AI query.
        void scheduleAIQueryResultPoll(const QUuid &requestId);

    protected slots:

        //! Run task.
        void run();

        //! Action has finished.
        void actionFinished(const QSharedPointer<RToolAction> &action);

        //! Action has failed.
        void actionFailed(const QSharedPointer<RToolAction> &action);

        //! Task has finished.
        void taskFinished();

        //! Task has failed.
        void taskFailed();

};

#endif // MAIN_TASK_H
