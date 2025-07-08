#ifndef ACTION_MANAGER_H
#define ACTION_MANAGER_H

#include <QJsonObject>
#include <QList>

#include <rcl_cloud_action_info.h>

#include "action_manager_settings.h"
#include "service_statistics.h"

class ActionManager : public QObject
{

    Q_OBJECT

    protected:

        ActionManagerSettings settings;
        //! Service satistics.
        ServiceStatistics statistics;
        //! List of users.
        QList<RCloudActionInfo> actions;

    public:

        //! Constructor
        explicit ActionManager(const ActionManagerSettings &settings, QObject *parent = nullptr);

        //! Check if given action is in the manager.
        bool containsAction(const QString &name) const;

        //! Find action.
        RCloudActionInfo findAction(const QString &name) const;

        //! Update action access rights.
        RCloudActionInfo updateActionAccessRights(const QString &name, const RAccessRights &accessRights);

        //! Return const reference to list of actions.
        const QList<RCloudActionInfo> &getActions() const;

        //! Read from file.
        void readFile();

        //! Write to file.
        void writeFile() const;

        //! Authorize if userInfo is allowed to execute action.
        bool authorizeUser(const RUserInfo &userInfo, const QString &name) const;

        //! Get statistics output in Json form.
        QJsonObject getStatisticsJson() const;

    private:

        //! Create action manager object from Json.
        void fromJson(const QJsonObject &json);

        //! Create Json from action manager object.
        QJsonObject toJson() const;

    signals:

};

#endif // ACTION_MANAGER_H
