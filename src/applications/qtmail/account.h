/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
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



#ifndef Account_H
#define Account_H

#include <qobject.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qlist.h>
#include <qfile.h>
#include <qtextstream.h>

#include <qtopiaglobal.h>

#include <stdlib.h>

#include "folder.h"

class MailAccount;
class QSettings;
class QTimer;

enum SyncSetting
{
    Sync_AllMessages    = 0x0001,
    Sync_OnlyNew        = 0x0002,
    Sync_OnlyHeaders    = 0x0004
};

typedef uint FolderSyncSetting;

class QTOPIAMAIL_EXPORT Mailbox : public Folder
{
public:
    Mailbox(MailAccount *account, QString _flags, QString _delimiter, QString _name);
    Mailbox(MailAccount *account);
    ~Mailbox();

    void setServerUid(QStringList list);
    QStringList getServerUid() const;
    QStringList getNewUid(QStringList list) const;

    void saveSettings(QSettings *config);
    void readSettings(QSettings *config);

    bool containsMsg(QString _uid) const;
    void changeName(QString n, bool userChange);
    bool userCreated() const { return byUser; };
    void setUserCreated(bool user) {byUser = user; };
    void setDeleted(bool del) { deleted = del; };
    bool isDeleted() const { return deleted; };

    void deleteMsg(QString id);
    void msgDeleted(QString id);
    QStringList msgToDelete() const { return delList; };
    QString nameChanged() const { return oldName; };

    void setName(QString _name) { this->_name = _name; };
    QString pathName() const { return _name; };
    QString path() const;
    QString baseName() const;

    void setDelimiter(QString del) { delimiter = del; };
    QString getDelimiter() const { return delimiter; };

    void setExists(int _exists) { exists = _exists; };
    int getExists() const { return exists; };

    void setLocalCopy(bool b) { _localCopy = b; };
    bool localCopy() const { return _localCopy; };
    void setFolderSync(FolderSyncSetting sync) { _syncSetting = sync; };
    FolderSyncSetting folderSync() const { return _syncSetting; };

    void setUid(QString _uid) { uid = _uid; };
    QString getUid() const { return uid; };

    //folder stuff
    QString name() const { return baseName(); };
    QString fullName() const;
    QString displayName() const {return _displayName;};
    bool matchesEmail(const QMailMessage& message) const;
    MailAccount* account() const { return _account; };
private:
    QString decodeModUTF7(QString in);
    QString decodeModBase64(QString in);
private:
    int exists, recent, unseen;
    bool noInferiors, noSelect, marked, unMarked;
    bool newName, byUser, deleted;

    bool _localCopy;
    FolderSyncSetting _syncSetting;

    QString _name, oldName, uid, delimiter;
    QStringList serverUidList, delList;

    MailAccount *_account;
    Search *search;
    QString _displayName;
};

class QTOPIAMAIL_EXPORT MailAccount : public Folder
{
    Q_OBJECT

public:
    MailAccount();
    ~MailAccount();

    /* General */
    enum AccountType {
        POP = 0,
        IMAP = 1,
        Synchronized = 2,
        SMS = 3,
        MMS = 4,
        System = 5
    };

    enum AuthType {
    Auth_NONE = 0,
#ifndef QT_NO_OPENSSL
    Auth_LOGIN = 1,
    Auth_PLAIN = 2
#endif
    };

    enum EncryptType {
    Encrypt_NONE = 0,
#ifndef QT_NO_OPENSSL
    Encrypt_SSL = 1,
    Encrypt_TLS = 2
#endif
    };

    QString accountName() const;
    void setAccountName(QString str);
    QString userName() const;
    void setUserName(QString str);

    /* SMTP */
    QString emailAddress() const;
    void setEmailAddress(QString str);
    void setSmtpServer(QString str);
    QString smtpServer() const;
    int smtpPort() const;
    void setSmtpPort(int i);
#ifndef QT_NO_OPENSSL
    QString smtpUsername() const;
    void setSmtpUsername(const QString& username);
    QString smtpPassword() const;
    void setSmtpPassword(const QString& password);
#endif
    AuthType smtpAuthentication() const;
#ifndef QT_NO_OPENSSL
    void setSmtpAuthentication(AuthType t);
#endif
    EncryptType smtpEncryption() const;
#ifndef QT_NO_OPENSSL
    void setSmtpEncryption(EncryptType t);
#endif
    bool useSig() const;
    void setUseSig(bool b);
    QString sig() const;
    void setSig(QString str);

    bool defaultMailServer() const;
    void setDefaultMailServer(bool b);

    /* POP/IMAP */
    QString mailUserName() const;
    void setMailUserName(QString str);
    QString mailPassword() const;
    void setMailPassword(QString str);
    QString mailServer() const;
    void setMailServer(QString str);
    int mailPort() const;
    void setMailPort(int i);
    EncryptType mailEncryption() const;
#ifndef QT_NO_OPENSSL
    void setMailEncryption(EncryptType t);
#endif
    AccountType accountType() const { return _accountType; };
    void setAccountType(AccountType at) { _accountType = at; };
    bool canCollectMail() const;

    bool deleteMail() const;
    void setDeleteMail(bool b);

    int maxMailSize() const;
    void setMaxMailSize(int i);

    int checkInterval() const;
    void setCheckInterval(int i);

    void deleteMsg(QString serverId, const QString &box);
    void msgDeleted(QString serverId, const QString &box);
    QStringList msgToDelete() const { return delList; };

    /* POP Only */
    bool synchronize() const;
    void setSynchronize(bool b);

    QStringList getUidlList() const;
    void setUidlList(QStringList in);

    /* IMAP Only */
    void setBaseFolder(QString s) { _baseFolder = s; };
    QString baseFolder() const { return _baseFolder; };

    void removeBox(Mailbox *box);
    Mailbox* getMailboxRef(QString name);
    Mailbox* getMailboxRefByMsgUid(QString _uid, const QString &box);

    /* MMS Only */
    void setNetworkConfig(QString c) { _networkCfg = c; }
    QString networkConfig() const;
    void setAutoDownload(bool autodl) { _autoDL = autodl; }
    bool autoDownload() const { return _autoDL; }

    /* General management */
    QString id() const;

    void saveSettings(QSettings *conf);
    void readSettings(QSettings *conf);
    bool hasSettings() const;

    /* Folder reimplementation */
    QString name() const { return accountName(); };
    QString fullName() const {return accountName(); };
    QString displayName() const { return accountName(); };
    bool matchesEmail(const QMailMessage& message) const;

    QList<Mailbox*> mailboxes;

    /* Message counts */
    int unreadCount() const;
    void setUnreadCount( int count );
    int count() const;
    void setCount( int count );

public slots:
    void checkIntervalTimeout();

signals:
    void intervalCheck(MailAccount*);

private:
    AccountType _accountType;
    QString _accountName;
    QString _userName;
    QString _emailAddress;
    QString _mailUserName;
    QString _mailPassword;
    QString _mailServer;
#ifndef QT_NO_OPENSSL
    EncryptType _mailEncryption;
#endif
    int _mailPort;
    QString _smtpServer;
    int _smtpPort;
    QString _baseFolder;
#ifndef QT_NO_OPENSSL
    QString _smtpUsername;
    QString _smtpPassword;
    AuthType _smtpAuthentication;
    EncryptType _smtpEncryption;
#endif

    bool _synchronize;
    bool _deleteMail;
    bool _useSig;
    bool _defaultMailServer;
    QString _sig;
    int _maxMailSize;
    int _checkInterval;
    QString _networkCfg;
    bool _autoDL;

    QStringList uidList;

    Search *search;

    QStringList delList;

    QTimer *intervalCheckTimer;
    int _unreadCount;
    int _count;
};

#endif
