#ifndef USER_MANAGER_H
#define USER_MANAGER_H

#include <QJsonObject>
#include <QObject>
#include <QString>

#include <rcl_access_rights.h>
#include <rcl_auth_token.h>
#include <rcl_auth_token_validator.h>
#include <rcl_group_info.h>
#include <rcl_user_info.h>

#include "user_manager_settings.h"
#include "service_statistics.h"

class UserManager : public QObject
{

    Q_OBJECT

    public:

        class AuthTokenValidator : public RAuthTokenValidator
        {

        protected:

            //! Pointer to user manager.
            UserManager *pUserManager;

        public:

            //! Constructor.
            explicit AuthTokenValidator(UserManager *pUserManager = nullptr);

            //! Validate.
            bool validate(const QString &resourceName, const QString &token) final;

        };

    protected:

        UserManagerSettings settings;
        //! Service satistics.
        ServiceStatistics statistics;
        //! List of users.
        QList<RUserInfo> users;
        //! List of groups.
        QList<RGroupInfo> groups;
        //! List of authentication tokens.
        QList<RAuthToken> tokens;
        //! User authentication token validator.
        AuthTokenValidator *authTokenValidator;

    public:

        //! Constructor.
        explicit UserManager(const UserManagerSettings &settings, QObject *parent);

        //! Check if given user is in the manager.
        bool containsUser(const QString &name) const;

        //! Find user.
        RUserInfo findUser(const QString &name) const;

        //! Check if given group is in the manager.
        bool containsGroup(const QString &name) const;

        //! Find group.
        RGroupInfo findGroup(const QString &name) const;

        //! Check if given token is in the manager.
        bool containsToken(const QString &resourceName, const QString &content) const;

        //! Check if given token with specified ID is in the manager.
        bool containsToken(const QUuid &id) const;

        //! Find token.
        RAuthToken findToken(const QString &resourceName, const QString &content) const;

        //! Find token bu its ID.
        RAuthToken findToken(const QUuid &id) const;

        //! Add new user.
        //! If a user with given name already exists or name is empty an exception will be raised.
        void addUser(const RUserInfo &user);

        //! Set user.
        //! If a user with given name does not exist or name is empty an exception will be raised.
        void setUser(const QString &userName, const RUserInfo &user);

        //! Remove user.
        //! If a user with given name does not exist or name is empty an exception will be raised.
        void removeUser(const QString &name);

        //! Add new group.
        //! If a group with given name already exists or name is empty an exception will be raised.
        void addGroup(const RGroupInfo &group);

        //! Set group.
        //! If a group with given name does not exist or name is empty an exception will be raised.
        void setGroup(const RGroupInfo &group);

        //! Remove group.
        //! If a group with given name does not exist or name is empty an exception will be raised.
        void removeGroup(const QString &name);

        //! Add new token.
        //! If a token with given id already exists or id is empty an exception will be raised.
        void addToken(const RAuthToken &token);

        //! Remove token.
        //! If a group with given id does not exist or id is empty an exception will be raised.
        void removeToken(const QUuid &id);

        //! Return pointer to user authentication token validator.
        AuthTokenValidator *getAuthTokenValidator();

        //! Read from file.
        void readFile();

        //! Write to file.
        void writeFile() const;

        //! Return list of users.
        inline const QList<RUserInfo> getUsers() const { return this->users; }

        //! Return list of groups.
        inline const QList<RGroupInfo> getGroups() const { return this->groups; }

        //! Return list of tokens.
        inline const QList<RAuthToken> getTokens() const { return this->tokens; }

        //! Return list of user names.
        QList<QString> getUserNames() const;

        //! Return list of group names.
        QList<QString> getGroupNames() const;

        //! Authorize user based on access rights.
        static bool authorizeUserAccess(const RUserInfo &userInfo, const RAccessRights &accessRights, const RAccessMode::Mode &accessMode);

        //! Create a user with specified user name and default user group.
        static RUserInfo createUser(const QString &userName);

        //! Create a group with specified group name.
        static RGroupInfo createGroup(const QString &groupName);

        //! Get statistics output in Json form.
        QJsonObject getStatisticsJson() const;

    private:

        //! Create user manager object from Json.
        void fromJson(const QJsonObject &json);

        //! Create Json from user manager object.
        QJsonObject toJson() const;

    protected slots:

        //! On changed signal.
        void onChanged();

        //! Group has been removed.
        void onGroupRemoved(RGroupInfo group);

    signals:

        //! Changed signal.
        void changed();

        //! User was added.
        void userAdded(const RUserInfo &user);

        //! User was changed.
        void userChanged(const RUserInfo &user);

        //! User was removed.
        void userRemoved(RUserInfo user);

        //! Group was added.
        void groupAdded(const RGroupInfo &group);

        //! Group was changed.
        void groupChanged(const RGroupInfo &group);

        //! Group was removed.
        void groupRemoved(RGroupInfo group);

        //! Token was added.
        void tokenAdded(const RAuthToken &toket);

        //! Token was removed.
        void tokenRemoved(RAuthToken toket);

};

#endif // USER_MANAGER_H
