#ifndef MAILER_H
#define MAILER_H

#include <QObject>
#include <QQueue>

#include <rbl_job.h>

#include "mailer_settings.h"
#include "service_statistics.h"

class Mailer : public RJob
{
    Q_OBJECT

    public:

        struct Mail
        {
            QString toAddress;
            QString subject;
            QString body;
        };

    protected:

        //! Settings.
        MailerSettings settings;
        //! Service satistics.
        ServiceStatistics statistics;

        //! Flag signaling to stop service.
        bool stopFlag;

        //! Email queue.
        QQueue<Mail> emails;

        QMutex syncMutex;
        QMutex serviceMutex;

    public:

        //! Constructor.
        explicit Mailer(const MailerSettings &settings);

        //! Run server.
        virtual int perform() override final;

        //! Request stop service.
        void stop();

        //! Get statistics output in Json form.
        QJsonObject getStatisticsJson() const;

    private:

        //! Send email.
        bool sendMail(const Mail &mail);

    public slots:

        //! Submit email.
        void submitMail(const QString &toAddress, const QString &subject, const QString &body);

    signals:

        //! Service is ready.
        void ready();

};

#endif // MAILER_H
