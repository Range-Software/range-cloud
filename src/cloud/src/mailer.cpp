#include <QProcess>

#include <rbl_error.h>
#include <rbl_logger.h>

#include "mailer.h"

Mailer::Mailer(const MailerSettings &settings)
    : settings{settings}
    , stopFlag{false}
{
    R_LOG_TRACE_IN;
    this->setBlocking(false);
    this->setParallel(true);
    this->statistics.setName(this->settings.getName());
    R_LOG_TRACE_OUT;
}

int Mailer::perform()
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

            if (!this->emails.empty())
            {
                RLogger::trace("[%s] Processing mail\n",this->settings.getName().toUtf8().constData());

                Mail email = this->emails.dequeue();

                if (this->sendMail(email))
                {
                    this->statistics.recordCounter("Sent",1);
                    RLogger::info("[%s] Email to \"%s\" with subject \"%s\" has been successfully sent.\n",
                                  this->settings.getName().toUtf8().constData(),
                                  email.toAddress.toUtf8().constData(),
                                  email.subject.toUtf8().constData());
                }
                else
                {
                    this->statistics.recordCounter("Failed",1);
                    RLogger::warning("[%s] Sending email to \"%s\" with subject \"%s\" has timed out.\n",
                                     this->settings.getName().toUtf8().constData(),
                                     email.toAddress.toUtf8().constData(),
                                     email.subject.toUtf8().constData());
                }
            }

            safeStopFlag = this->stopFlag;

            this->syncMutex.unlock();

            QThread::msleep(1000);
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

void Mailer::stop()
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

QJsonObject Mailer::getStatisticsJson() const
{
    RLogger::debug("[%s] Producting statistics\n",this->settings.getName().toUtf8().constData());
    ServiceStatistics snapshotStatistics(this->statistics);
    return snapshotStatistics.toJson();
}

bool Mailer::sendMail(const Mail &mail)
{
    R_LOG_TRACE_IN;
    QString program = "sendmail";
    QStringList arguments;
    arguments << "-t" << mail.toAddress;

    QString messageContent;
    if (!this->settings.getFromAddress().isEmpty())
    {
        messageContent += "From:" + this->settings.getFromAddress() + "\n";
    }
    messageContent += "Subject:" + mail.subject + "\n";
    messageContent += "Body:\n" + mail.body + "\n";

    QProcess sendMail;
    sendMail.start(program,arguments);
    sendMail.write(messageContent.toUtf8());
    sendMail.closeWriteChannel();

    R_LOG_TRACE_RETURN(sendMail.waitForFinished(this->settings.getSendTimeout()));
}

void Mailer::submitMail(const QString &toAddress, const QString &subject, const QString &body)
{
    R_LOG_TRACE_IN;
    Mail mail;
    mail.toAddress = toAddress;
    mail.subject = subject;
    mail.body = body;

    this->emails.enqueue(mail);
    R_LOG_TRACE_OUT;
}
