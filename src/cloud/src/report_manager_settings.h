#ifndef REPORT_MANAGER_SETTINGS_H
#define REPORT_MANAGER_SETTINGS_H

#include <QString>

#include "service_settings.h"

class ReportManagerSettings : public ServiceSettings
{

    protected:

        //! Internal initialization function.
        void _init(const ReportManagerSettings *pReportManagerSettings = nullptr);

    protected:

        //! Path to file store.
        QString reportDirectory;
        //! Maximum report length.
        qint64 maxReportLength;
        //! Maximum comment length.
        qint64 maxCommentLength;

    public:

        //! Constructor.
        ReportManagerSettings();

        //! Copy constructor.
        ReportManagerSettings(const ReportManagerSettings &userManagerSettings);

        //! Destructor.
        ~ReportManagerSettings() {}

        //! Assignment operator.
        ReportManagerSettings &operator =(const ReportManagerSettings &userManagerSettings);

        //! Get const reference to report directory.
        const QString &getReportDirectory() const;

        //! Set new report directory.
        void setReportDirectory(const QString &reportDirectory);

        //! Return maximum report length.
        qint64 getMaxReportLength() const;

        //! Set maximum report length.
        void setMaxReportLength(qint64 maxReportLength);

        //! Return maximum comment length.
        qint64 getMaxCommentLength() const;

        //! Set maximum comment length.
        void setMaxCommentLength(qint64 maxCommentLength);

};

#endif // REPORT_MANAGER_SETTINGS_H
