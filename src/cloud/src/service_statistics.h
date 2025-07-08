#ifndef SERVICE_STATISTICS_H
#define SERVICE_STATISTICS_H

#include <QString>
#include <QMap>
#include <QJsonObject>

#include <rbl_rvector.h>

class ServiceStatistics
{

    protected:

        //! Internal initialization function.
        void _init(const ServiceStatistics *pServiceStatistics = nullptr);

    protected:

        //! Identity name.
        QString name;
        //! Statistics data-counters.
        QMap<QString,qsizetype> dataCounter;
        //! Statistics data-set map.
        QMap<QString,RRVector> dataSetMap;

    public:

        //! Constructor.
        ServiceStatistics();

        //! Copy constructor.
        ServiceStatistics(const ServiceStatistics &serviceStatistics);

        //! Destructor.
        ~ServiceStatistics() { }

        //! Assignment operator.
        ServiceStatistics &operator =(const ServiceStatistics &serviceStatistics);

        //! Get const reference to name.
        const QString &getName() const;

        //! Set new name.
        void setName(const QString &name);

        //! Clear data sets.
        void clear();

        //! Record counter (+counter) for given data-counter key.
        void recordCounter(const QString &key, qsizetype counter);

        //! Record value for given data-set key.
        void recordValue(const QString &key, double value);

        //! Set values for given data key.
        void setValues(const QString &key, const RRVector &values);

        //! To Json.
        QJsonObject toJson() const;

};

#endif // SERVICE_STATISTICS_H
