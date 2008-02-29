/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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
#ifndef EMAILACCESS_H
#define EMAILACCESS_H

#include <qtopia/mail/qtopiamail.h>

#include <qtopiaglobal.h>
#include <qobject.h>
#include <qstring.h>

#include <qstringlist.h>
#include <qmap.h>

#include <qfile.h>
#include <qdatetime.h>

#include <quuid.h>
#include <QTimer>

class AccessPrivate;
class MailMessage;

/*
    WARNING!!!

    This class is part of the Qtopia mail library and is considered unfinished.  Any usage
    of this class or library is not advisable.

    The current library will not be source or binary compatible with subsequent releases and updates.
*/

class QTOPIAMAIL_EXPORT EmailFolderAccess : public QObject
{
    Q_OBJECT

public:
    EmailFolderAccess(QString mailbox, QObject *parent=0);
    ~EmailFolderAccess();

    enum MailboxStatus {
        Open = 0,
        Closed,
        Locked,
        Invalid,
        OldVersion,
        IndexFileDeleted
    };

    QString mailbox() const {return _mailbox; };
    MailboxStatus status() {return _status; };

    bool hasMail(const QUuid &id);
    QList<QUuid> mailUuidList() const;
    QUuid generateUuid() const;

    bool addMail(MailMessage &m);
    bool removeMail(const QUuid &id);
    bool moveMail(const QUuid& id, EmailFolderAccess& dest);
    bool copyMail(const QUuid& id, EmailFolderAccess& dest, QUuid& newid);

    bool updateFullMail(MailMessage &m);
    bool updateMailContextInfo(const MailMessage &m);

    MailMessage fullMail(const QUuid &id);
    MailMessage mailContextInfo(const QUuid &id);

    void autoCompact();
    bool recreateIndexFiles();
signals:
    void mailboxChanged();

protected slots:
    void channelMsg(const QString &, const QByteArray &);
    void sentMailBoxChanged();
    void writeDirtyIndex();
private:
    struct IndexItem
    {
        QString mailFile;
        int headerFilePos;
        int headerFileSize;
    };
    void setDirty();
    void readIndexFile();
    void readHeaderFile();
    bool appendMailString(MailMessage &m, IndexItem *newRef);
    bool saveHeader(const MailMessage &m, IndexItem &item);
    bool writeIndexFile(QString filename );
    void tryWriteIndex();

    void setContextInfo(MailMessage *m, const QByteArray &b, bool keepOld = false);
    QByteArray contextInfo(const MailMessage &m);

    bool ensureExists(bool forceNew = false);
    bool ensureValid();
    bool ensureCurrent();
    void ensureSynced();

    bool isWriteLocked(QFile &file);
    bool lockDataFile(QFile& file);
    bool unlockDataFile(QFile& file);

    void sendMsg(const QString msg);

    void compactHeaders();

    QString createUniqueFileName();
    QString randomString(int length);
    bool transferMail(const QUuid& id, EmailFolderAccess& dest, bool copy = false, QUuid* newId = 0);

private:
    QString _mailbox;
    QString curDirPath;
    QString tmpDirPath;
    QString indexfn, headerfn;
    QDateTime indexDate, headerDate;
    MailboxStatus _status;
    QTimer _sendMsgTimer;
    QTimer _dirtyIndexTimer;
    int _indexWriteSkip;

    QMap<QUuid, IndexItem> mappedMails;
    QList<QUuid> _uuidList; //list of uid's in order added

    AccessPrivate *d;
    static const int MAX_WRITESKIP = 15;
};

#endif
