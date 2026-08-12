#ifndef AI_QUERY_INFO_H
#define AI_QUERY_INFO_H

#include <QJsonObject>
#include <QString>
#include <QStringList>

//! AI query application profile: keys identifying a client application
//! and the predefined system prompt (context and guardrails) to use for it.
class AIQueryInfo
{

    protected:

        //! Name (primary application key, e.g. "family-tree").
        QString name;
        //! Alternative application keys (e.g. "familytree").
        QStringList aliases;
        //! System prompt (context and guardrails).
        QString systemPrompt;

    private:

        //! Internal initialization function.
        void _init(const AIQueryInfo *pAIQueryInfo = nullptr);

    public:

        //! Constructor.
        AIQueryInfo();

        //! Copy constructor.
        AIQueryInfo(const AIQueryInfo &aiQueryInfo);

        //! Destructor.
        ~AIQueryInfo();

        //! Assignment operator.
        AIQueryInfo &operator =(const AIQueryInfo &aiQueryInfo);

        //! Return const reference to name.
        const QString &getName() const;

        //! Set new name.
        void setName(const QString &name);

        //! Return const reference to list of aliases.
        const QStringList &getAliases() const;

        //! Set new list of aliases.
        void setAliases(const QStringList &aliases);

        //! Return const reference to system prompt.
        const QString &getSystemPrompt() const;

        //! Set new system prompt.
        void setSystemPrompt(const QString &systemPrompt);

        //! Check whether given application key matches name or one of the aliases (case insensitive).
        bool matches(const QString &application) const;

        //! Create AI query info object from Json.
        static AIQueryInfo fromJson(const QJsonObject &json);

        //! Create Json from AI query info object.
        QJsonObject toJson() const;
};

#endif // AI_QUERY_INFO_H
