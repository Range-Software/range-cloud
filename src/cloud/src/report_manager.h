#ifndef REPORT_MANAGER_H
#define REPORT_MANAGER_H

#include <QJsonObject>
#include <QObject>
#include <QString>

#include <rcl_report_record.h>

#include "report_manager_settings.h"
#include "service_statistics.h"

class ReportManager : public QObject
{

    Q_OBJECT

    protected:

        ReportManagerSettings settings;
        //! Service satistics.
        ServiceStatistics statistics;

        uint64_t nReportsSubmitted;

    public:

        //! Constructor.
        explicit ReportManager(const ReportManagerSettings &settings, QObject *parent);

        //! Submit new report.
        QUuid submitReport(const QString &from, const RReportRecord &reportRecord);

        //! Get statistics output in Json form.
        QJsonObject getStatisticsJson() const;

};

#endif // REPORT_MANAGER_H
