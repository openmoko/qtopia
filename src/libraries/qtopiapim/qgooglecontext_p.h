/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef APPOINTMENT_GCALIO_PRIVATE_H
#define APPOINTMENT_GCALIO_PRIVATE_H

#include <QList>
#include <QDateTime>
#include <QUrl>
#include <QHttp>

#include <qtopia/pim/qappointment.h>
#include <qtopiasql.h>
#include <qtsslsocket.h>

#include "qappointmentsqlio_p.h"

class QXmlSimpleReader;
class QXmlInputSource;
class GoogleCalHandler;
class QtopiaHttp;

class QGoogleCalendarFetcher;
// not a public class, but needs to be used by datebook.
// later need to make a plugin or some other optional context.
class QTOPIAPIM_EXPORT QGoogleCalendarContext : public QAppointmentContext
{
    Q_OBJECT
public:
    QGoogleCalendarContext(QObject *parent, QObject *access);

    QIcon icon() const;
    QString description() const;
    QString title() const;
    QString title(const QPimSource &) const;

    bool editable() const;

    // do available and set, same as outer model?
    // do as contexts later.
    void setVisibleSources(const QSet<QPimSource> &);
    QSet<QPimSource> visibleSources() const;
    QSet<QPimSource> sources() const;
    QUuid id() const;

    bool exists(const QUniqueId &) const;
    bool exists(const QUniqueId &, const QPimSource &) const;
    QPimSource source(const QUniqueId &) const;

    bool updateAppointment(const QAppointment &);
    bool removeAppointment(const QUniqueId &);
    QUniqueId addAppointment(const QAppointment &, const QPimSource &);

    bool removeOccurrence(const QUniqueId &original, const QDate &);
    QUniqueId replaceOccurrence(const QUniqueId &original, const QOccurrence &);
    QUniqueId replaceRemaining(const QUniqueId &original, const QAppointment &);

    // Google functions
    //void addUser(const QString &user);
    //QStringList users() const;

    void addAccount(const QString &user, const QString &password);
    void removeAccount(const QString &);

    QStringList accounts() const;

    enum FeedType {
        FullPrivate,
        FullPublic,
        FreeBusyPublic
    };

    QString password(const QString &email) const;
    FeedType feedType(const QString &email) const;
    QString name(const QString &email) const;

    void setPassword(const QString &email, const QString &password);
    void setFeedType(const QString &email, FeedType);

    void syncAccount(const QString &account);
    void syncAllAccounts();

    bool syncing() const;
    void syncProgress(int &amount, int &total) const;

    // need error reporting....
    // enum + string

    enum Status {
        NotStarted,
        InProgress,
        Completed,
        BadAuthentication,
        NotVerified,
        TermsNotAgreed,
        CaptchaRequired,
        AccountDeleted,
        AccountDisabled,
        ServiceUnavailable,
        CertificateError,
        ParseError,
        UnknownError
    };

    static QString statusMessage(Status status);
signals:
    void syncProgressChanged(int, int);
    void syncStatusChanged(const QString &account, Status);
    void finishedSyncing();

private slots:
    void updateFetchingProgress(int, int);
    void updateFetchingState(Status);
    void syncAccountList();
private:
    QList<QGoogleCalendarFetcher *> mFetchers;
    void saveAccount(const QString &);

    struct Account {
        Account() : type(FullPrivate) {}
        FeedType type;
        QString password;
        QString name;
    };

    QMap<QString, Account> mAccounts;

    QAppointmentSqlIO *mAccess;
};

/*
   Split off as separate class to enable multiple concurrent downloads.

   the database itself already handles locking db for the adds, this gives us a measure of
    protection in that it will be more difficult to confuse multiple http downloads
    of different accounts if they are done with different QHttp objects

    Later, turn into QThread so can also avoid halting while adding items to DB
*/
class QGoogleCalendarFetcher : public QObject // : public QThread
{
    Q_OBJECT
public:
    QGoogleCalendarFetcher(const QPimSource & sqlContext, const QString &email, const QString &password, const QString &url, QObject *parent = 0);

    QGoogleCalendarContext::Status status() const;
    QString statusMessage() const;

    int lastValue() const;
    int lastMaximum() const;

    // async call.
    void fetch();

    QString account() const;
    QString name() const;

    void fetchProgress(int &, int &) const;
signals:
    void fetchProgressChanged(int, int);
    void completed(QGoogleCalendarContext::Status);

    void certificateError(const QMap<QString, QString> &, const QString &);

private slots:
    void parseRemaining();
    void abortParsing();
    void parsePartial(const QHttpResponseHeader &);
    void httpFetchProgress(int, int);

    void sendCertificateError(QtSslSocket::VerifyResult result, bool hostNameWrong, const QString &str);
private:
    void fetchAuthentication();
    void fetchAppointments();

    enum State
    {
        IdleState,
        AuthenticationState,
        AppointmentDownloadState,
        ErrorState,
        CompletedState
    };

    /* shared */
    int lastProgress;
    int lastTotal;
    State mState;
    QGoogleCalendarContext::Status mStatus;
    QUrl mUrl;
    QPimSource mContext;
    QString mAccount;
    QString mPassword;
    QString mAuth;

    QDateTime syncTime;
    QDateTime lastSyncTime;

    /* worker side */
    QXmlSimpleReader *xmlReader;
    QXmlInputSource *mSource;
    GoogleCalHandler *mHandler;

    QtopiaHttp *mDownloader;
    QtSslSocket *mSslSocket;

    QAppointmentSqlIO *mAccess;
};

#endif
