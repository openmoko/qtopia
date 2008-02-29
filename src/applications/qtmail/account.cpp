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

#include "account.h"
#include "search.h"
#include <qtimer.h>
#include <qsettings.h>
#include <qtopialog.h>
#include <QApplication>
#include <QDir>

//* LIST (\flags) "/" "INBOX" response for each list
Mailbox::Mailbox(MailAccount *account, QString _flags, QString _delimiter, QString _name)
    : Folder( FolderTypeMailbox )
{
    noInferiors = ( _flags.indexOf("Noinferiors", 0, Qt::CaseInsensitive) != -1);
    noSelect = ( _flags.indexOf("Noselect", 0, Qt::CaseInsensitive) != -1);
    marked = ( _flags.indexOf("Marked", 0, Qt::CaseInsensitive) != -1); // No tr
    unMarked = ( _flags.indexOf("Unmarked", 0, Qt::CaseInsensitive) != -1); // No tr

    _localCopy = false;
    _syncSetting = Sync_AllMessages;

    delimiter = _delimiter;
    this->_name = _name;
    exists = 0;
    recent = 0;
    unseen  = 0;

    oldName = "";
    newName = false;
    byUser = false;
    deleted = false;

    _account = account;
    search = new Search();
    search->setFromFolder( _name );
    search->setFromAccount( _account->id() );

    if(_account->accountType() == MailAccount::IMAP)
        _displayName = decodeModUTF7(baseName());
    else
        _displayName = _name;
}

Mailbox::Mailbox(MailAccount *account)
    : Folder( FolderTypeMailbox )
{
    noInferiors = false;
    noSelect = false;
    marked = false;
    unMarked = false;
    _localCopy = false;
    _syncSetting = Sync_AllMessages;

    delimiter = "/";
    this->_name = "";
    exists = 0;
    recent = 0;
    unseen  = 0;
    delList.clear();

    oldName = "";
    newName = false;
    byUser = false;
    deleted = false;

    _account = account;
    search = new Search();
    search->setFromFolder( _name );
    search->setFromAccount( _account->id() );
}

Mailbox::~Mailbox()
{
    delete search;
}

QString Mailbox::baseName() const
{
    QStringList list = _name.split(delimiter);
    return list.last();
}

QString Mailbox::path() const
{
    QStringList list = _name.split(delimiter);
    list.erase( list.end() );
    QString str = list.join(delimiter);
    if ( !str.isEmpty() )
        str += delimiter;

    return str;
}

void Mailbox::changeName(QString n, bool userChange)
{
    if (userChange && !byUser) {
        if ( _name != path() + n) {
            if ( oldName.isEmpty() )
                oldName = _name;

            _name = path() + n;
        }
    } else {
        oldName = "";
        _name = path() + n;
    }

    if(_account && _account->accountType() == MailAccount::IMAP)
        _displayName = decodeModUTF7(baseName());
    else
        _displayName = _name;
}

void Mailbox::deleteMsg(QString id)
{
    //add to queue only if message still is on server, or
    //an identical copy has not been markedas to be deleted
    if ( serverUidList.contains(id) ) {
        if ( !delList.contains(id) ) {
            delList.append(id);
        }
    }
}

void Mailbox::msgDeleted(QString id)
{
    delList.removeAll(id);
}

void Mailbox::saveSettings(QSettings *config)
{
    config->remove("");
    config->setValue("name", _name );
    config->setValue("oldname", oldName );
    config->setValue("delimiter", delimiter );
    config->setValue("uid", uid );
    config->setValue("exists", exists);
    config->setValue("byuser", byUser);
    config->setValue("deleted", deleted);
    config->setValue("queuedelete", delList.join(",") );
    config->setValue("localcopy", _localCopy);
    config->setValue("syncsettings", _syncSetting);

    int pos = 1;
    for (QStringList::Iterator it = serverUidList.begin();
        it != serverUidList.end(); ++it) {

        config->setValue( QString::number(pos), *it);
        pos++;
    }   int maxMailSize();

}

void Mailbox::readSettings(QSettings *config)
{
    _name = config->value("name").toString();
    oldName = config->value("oldname").toString();
    delimiter = config->value("delimiter").toString();
    uid = config->value("uid").toString();
    exists = config->value("exists", 0).toInt();
    byUser = config->value("byuser").toBool();
    deleted = config->value("deleted").toBool();
    delList = config->value("queuedelete").toString().split( "," );
    _localCopy = config->value("localcopy", false ).toBool();
    _syncSetting = config->value("syncsettings", Sync_AllMessages).toInt();

    serverUidList.clear();
    QString str;
    for (int x = 1; x < (exists + 1); x++) {
        int theUid = config->value( QString::number(x) ).toInt();
        str = QString::number(theUid);
        if ( !str.isEmpty() )
            serverUidList.append( str );
    }

    search->setFromFolder( pathName() );

    if(_account && _account->accountType() == MailAccount::IMAP)
        _displayName = decodeModUTF7(baseName());
    else
        _displayName = _name;
}

void Mailbox::setServerUid(QStringList list)
{
    serverUidList = list;
    exists = serverUidList.count();
}

QStringList Mailbox::getServerUid() const
{
    return serverUidList;
}

bool Mailbox::containsMsg(QString _uid) const
{
    if ( (serverUidList.contains(_uid)) )
        return true;

    return false;
}

QStringList Mailbox::getNewUid(QStringList list) const
{
    QStringList newUidList;

    for ( QStringList::Iterator it = list.begin(); it != list.end(); ++it ) {
        if ( !serverUidList.contains(*it) )
            newUidList.append(*it);
    }

    return newUidList;
}

bool Mailbox::matchesEmail(const QMailMessage& message) const
{
    return search->matches(message);
}

QString Mailbox::fullName() const
{
     return ( _account->accountName() + " - " + _displayName );
}

//decode modified UTF-7 in imap

QString Mailbox::decodeModUTF7(QString in)
{
    QRegExp reg("&[^&-]*-");

    int startIndex = 0;
    int endIndex = 0;

    startIndex = in.indexOf(reg,endIndex);

    while(startIndex != -1)
    {
        endIndex = startIndex;
        while(endIndex < in.length() && in[endIndex] != '-')
            endIndex++;
        endIndex++;
        //extract the base64 string from the input string
        QString mbase64 = in.mid(startIndex,(endIndex - startIndex));
        //decode
        QString unicodeString = decodeModBase64(mbase64);
        //remove encoding
        in.remove(startIndex,(endIndex-startIndex));

        in.insert(startIndex,unicodeString);
        endIndex = startIndex + unicodeString.length();

        startIndex = in.indexOf(reg,endIndex);
    }
    return in;
}

QString Mailbox::decodeModBase64(QString in)
{
    //remove  & -

    in.remove(0,1);
    in.remove(in.length()-1,1);

    if(in.isEmpty())
        return "&";

    QByteArray buf(in.length(),static_cast<char>(0));
    QByteArray out(in.length() * 3 / 4 + 2,static_cast<char>(0));

    //chars to numeric

    QByteArray latinChars = in.toLatin1();
    for (int x = 0; x < in.length(); x++) {
        int c = latinChars[x];
        if ( c >= 'A' && c <= 'Z')
            buf[x] = c - 'A';
        if ( c >= 'a' && c <= 'z')
            buf[x] = c - 'a' + 26;
        if ( c >= '0' && c <= '9')
            buf[x] = c - '0' + 52;
        if ( c == '+')
            buf[x] = 62;
        if ( c == ',')
            buf[x] = 63;
    }

    int i = 0; //in buffer index
    int j = i; //out buffer index

    unsigned char z;
    QString result;

    while(i+1 < buf.size())
    {
        out[j] = buf[i] & (0x3F); //mask out top 2 bits
        out[j] = out[j] << 2;
        z = buf[i+1] >> 4;
        out[j] = (out[j] | z);      //first byte retrieved

        i++;
        j++;

        if(i+1 >= buf.size())
            break;

        out[j] = buf[i] & (0x0F);   //mask out top 4 bits
        out[j] = out[j] << 4;
        z = buf[i+1] >> 2;
        z &= 0x0F;
        out[j] = (out[j] | z);      //second byte retrieved

        i++;
        j++;

        if(i+1 >= buf.size())
            break;

        out[j] = buf[i] & 0x03;   //mask out top 6 bits
        out[j] = out[j] <<  6;
        z = buf[i+1];
        out[j] = out[j] | z;  //third byte retrieved

        i+=2; //next byte
        j++;
    }


    //go through the buffer and extract 16 bit unicode
    //network byte order

    for(int z = 0; z < out.count(); z+=2)
    {
        unsigned short outcode = 0x0000;
        outcode = out[z];
        outcode <<= 8;
        outcode &= 0xFF00;

        unsigned short b = 0x0000;
        b = out[z+1];
        b &= 0x00FF;
        outcode = outcode | b;
        if(outcode)
            result += QChar(outcode);
    }

    return result;
}

/*  MailAccount   */

MailAccount::MailAccount()
    : Folder( FolderTypeAccount )
{
    _mailPort = 110;
    _smtpPort = 25;
#ifndef QT_NO_OPENSSL
    _smtpAuthentication = Auth_NONE;
    _smtpEncryption = Encrypt_NONE;
    _mailEncryption = Encrypt_NONE;
#endif
    _maxMailSize = -1;
    _checkInterval = -1;

    _accountType = POP;
    _synchronize = true;
    _deleteMail = false;
    _defaultMailServer = false;
    _useSig = false;

    search = new Search();
    intervalCheckTimer = new QTimer();
    connect( intervalCheckTimer, SIGNAL(timeout()),
             this, SLOT(checkIntervalTimeout()) );

    _unreadCount = 0;
    _count = 0;
}

MailAccount::~MailAccount()
{
    delete search;
}

/* General */
QString MailAccount::accountName() const
{
    return qApp->translate( "AccountList", _accountName.toLatin1() );
}

void MailAccount::setAccountName(QString str)
{
    _accountName = str;
}

QString MailAccount::userName() const
{
    return _userName;
}

void MailAccount::setUserName(QString str)
{
    _userName = str;
}

/* SMTP */
QString MailAccount::emailAddress() const
{
    return _emailAddress;
}

void MailAccount::setEmailAddress(QString str)
{
    _emailAddress = str;
}

void MailAccount::setSmtpServer(QString str)
{
    _smtpServer = str;
}

QString MailAccount::smtpServer() const
{
    return _smtpServer;
}

int MailAccount::smtpPort() const
{
    return _smtpPort;
}

void MailAccount::setSmtpPort(int i)
{
    _smtpPort = i;
}

#ifndef QT_NO_OPENSSL

QString MailAccount::smtpUsername() const
{
    return _smtpUsername;
}

void MailAccount::setSmtpUsername(const QString& username)
{
    _smtpUsername = username;
}

QString MailAccount::smtpPassword() const
{
    return _smtpPassword;
}

void MailAccount::setSmtpPassword(const QString& password)
{
    _smtpPassword = password;
}

#endif

MailAccount::AuthType MailAccount::smtpAuthentication() const
{
#ifndef QT_NO_OPENSSL
    return _smtpAuthentication;
#else
    return Auth_NONE;
#endif
}

#ifndef QT_NO_OPENSSL

void MailAccount::setSmtpAuthentication(AuthType t)
{
    _smtpAuthentication = t;
}

#endif

MailAccount::EncryptType MailAccount::smtpEncryption() const
{
#ifndef QT_NO_OPENSSL
    return _smtpEncryption;
#else
    return Encrypt_NONE;
#endif
}

#ifndef QT_NO_OPENSSL

void MailAccount::setSmtpEncryption(EncryptType t)
{
    _smtpEncryption = t;
}

#endif

MailAccount::EncryptType MailAccount::mailEncryption() const
{
#ifndef QT_NO_OPENSSL
    return _mailEncryption;
#else
    return Encrypt_NONE;
#endif
}

#ifndef QT_NO_OPENSSL

void MailAccount::setMailEncryption(EncryptType t)
{
    _mailEncryption = t;
}

#endif

bool MailAccount::useSig() const
{
    return _useSig;
}

void MailAccount::setUseSig(bool b)
{
    _useSig = b;
}

QString MailAccount::sig() const
{
    return _sig;
}

void MailAccount::setSig(QString str)
{
    _sig = str;
}

bool MailAccount::defaultMailServer() const
{
    return _defaultMailServer;
}

void MailAccount::setDefaultMailServer(bool b)
{
    _defaultMailServer = b;
}

/* POP/IMAP */
QString MailAccount::mailUserName() const
{
    return _mailUserName;
}

void MailAccount::setMailUserName(QString str)
{
    _mailUserName = str;

    search->setFromAccount( id() );
}

QString MailAccount::mailPassword() const
{
    return _mailPassword;
}

void MailAccount::setMailPassword(QString str)
{
    _mailPassword = str;
}

QString MailAccount::mailServer() const
{
    return _mailServer;
}

void MailAccount::setMailServer(QString str)
{
    _mailServer = str;

    search->setFromAccount( id() );
}

int MailAccount::mailPort() const
{
    return _mailPort;
}

void MailAccount::setMailPort(int i)
{
    _mailPort = i;
}

bool MailAccount::canCollectMail() const
{
    if ( accountType() == MailAccount::Synchronized ||
         accountType() == MailAccount::SMS ||
         accountType() == MailAccount::MMS ||
         accountType() == MailAccount::System )
        return false;
    return true;
}

bool MailAccount::deleteMail() const
{
    return _deleteMail;
}

void MailAccount::setDeleteMail(bool b)
{
    _deleteMail = b;
}

int MailAccount::maxMailSize() const
{
    return _maxMailSize;
}

void MailAccount::setMaxMailSize(int i)
{
    _maxMailSize = i;
}

int MailAccount::checkInterval() const
{
    return _checkInterval;
}

void MailAccount::setCheckInterval(int i)
{
    _checkInterval = i;
    if ( i == -1)
        intervalCheckTimer->stop();
    else
        intervalCheckTimer->start( i * 1000 * 60 );
}

void MailAccount::deleteMsg(QString serverId, const QString &boxName)
{
    if ( _accountType == IMAP ) {
        Mailbox *box = getMailboxRefByMsgUid( serverId, boxName );
        if ( box != NULL)
            box->deleteMsg( serverId );
    } else {
        if ( !delList.contains(serverId) ) {
            delList.append(serverId);
        }
    }
}

void MailAccount::msgDeleted(QString serverId, const QString &boxName )
{
    if ( _accountType == IMAP ) {
        Mailbox *box = getMailboxRefByMsgUid( serverId, boxName );
        if ( box != NULL)
            box->msgDeleted( serverId );
    } else {
        delList.removeAll( serverId );
    }
}

/* POP Only */
bool MailAccount::synchronize() const
{
    return _synchronize;
}

void MailAccount::setSynchronize(bool b)
{
    _synchronize = b;
}

// set uidllist belonging to this account
void MailAccount::setUidlList(QStringList in)
{
    uidList = in;

    QSettings accountconf("Trolltech","qtmail_account");
    accountconf.beginGroup( id() );
    accountconf.remove("");

    accountconf.setValue("idcount", uidList.count() );

    int pos = 1;
    for (QStringList::Iterator it = uidList.begin();
        it != uidList.end(); ++it) {

        accountconf.setValue( QString::number(pos), *it);
        pos++;
    }
    accountconf.endGroup();
}

// get uidllist belonging to this account
QStringList MailAccount::getUidlList() const
{
    return uidList;
}


/* IMAP Only */
Mailbox* MailAccount::getMailboxRef(QString name)
{
    QListIterator<Mailbox*> it(mailboxes);
    while ( it.hasNext() ) {
        Mailbox *box = it.next();
        if ( box->pathName() == name )
            return box;
        if ( box->nameChanged() == name)
            return box;
    }

    return NULL;
}

Mailbox* MailAccount::getMailboxRefByMsgUid(QString uid, const QString &boxName )
{
    qLog(Messaging) << "searching for mail with uid" << QString(*uid.data());

    QListIterator<Mailbox*> it(mailboxes);
    while ( it.hasNext() ) {
        Mailbox *box = it.next();
        if ( box->pathName() == boxName && box->containsMsg(uid) ) {
            qLog(Messaging) << "found in box" << QString(*box->pathName().data());
            return box;
        }
    }

    return NULL;
}

void MailAccount::removeBox(Mailbox *box)
{
    QSettings accountconf("Trolltech","qtmail_account");
    for (int x = 0; x < mailboxes.count(); x++) {
        accountconf.beginGroup( id() + "_" + QString::number(x) );
        accountconf.remove("");
        accountconf.endGroup();
    }

    mailboxes.removeAll(box);
}

/* General */
QString MailAccount::id() const
{
    return ( _mailUserName + "@" + _mailServer);
}

bool MailAccount::hasSettings() const
{
    switch( accountType() ) {
        case MailAccount::SMS:
            return false;
        default:
            return true;
    }
}

#ifdef QTOPIA4_TODO

static QString encipher(const QString& plain)
{
    // mainly, we make it long
    QString cipher;
    int mix=28730492;
    for (int i=0; i<(int)plain.length(); i++) {
        int u = plain[i].unicode();
        int c = u ^ mix;
        QString x = QString::number(c,36);

        cipher.append(QChar('a'+x.length()));
        cipher.append(x);
        mix *= u;
    }
    return cipher;
}

static QString decipher(const QString& cipher)
{
    QString plain;
    int mix=28730492;
    for (int i=0; i<(int)cipher.length();) {
        int l = cipher[i].unicode()-'a';
        QString x = cipher.mid(i+1,l); i+=l+1;
        int u = x.toInt(0,36) ^ mix;
        plain.append(QChar(u));
        mix *= u;
    }
    return plain;
}

#endif

void MailAccount::saveSettings(QSettings *conf)
{
    conf->remove("");
    conf->setValue("accountname", _accountName );
    conf->setValue("name", _userName );
    conf->setValue("email", _emailAddress );
    conf->setValue("mailuser", _mailUserName );
    conf->setValue("mailpassword", _mailPassword); //TODO fix encipher
    conf->setValue("mailserver", _mailServer );
    conf->setValue("mailport", _mailPort);
    conf->setValue("type", _accountType );
    conf->setValue("basefolder", _baseFolder);

    conf->setValue("smtpserver", _smtpServer );
    conf->setValue("smtpport", _smtpPort);
#ifndef QT_NO_OPENSSL
    conf->setValue("smtpUsername",_smtpUsername);
    conf->setValue("smtpPassword",_smtpPassword);
    conf->setValue("smtpAuthentication",_smtpAuthentication);
    conf->setValue("smtpEncryption",_smtpEncryption);
    conf->setValue("mailEncryption",_mailEncryption);
#endif
    conf->setValue("usesig", _useSig);
    conf->setValue("maxmailsize", _maxMailSize);
    conf->setValue("checkinterval", _checkInterval);
    conf->setValue("defaultmailserver", _defaultMailServer);

    QString file = QDir::homePath() + "/Applications/qtmail/sig_" + _mailUserName + "_" + _mailServer;
    QFile f(file);
    if ( f.open(QIODevice::WriteOnly) ) {    // file opened successfully
        QTextStream t( &f );        // use a text stream
        t << _sig;
        f.close();
    }

    conf->setValue("synchronize", _synchronize);
    conf->setValue("deletemail", _deleteMail);

    conf->setValue("networkConfig", _networkCfg);
    conf->setValue("autoDownload", _autoDL);

    if ( _accountType == IMAP ) {
        int count = 0;
        conf->setValue("mailboxes", mailboxes.count() );
        QListIterator<Mailbox*> it(mailboxes);
        while ( it.hasNext() ) {
            Mailbox *box = it.next();
            conf->beginGroup( id() + "_" + QString::number(count) );
            box->saveSettings(conf);
            count++;
            conf->endGroup();
        }
    } else {
        conf->setValue("queuedelete", delList.join(",!;") );
    }
}

void MailAccount::readSettings(QSettings *conf)
{
    _accountName = conf->value("accountname").toString();
    _userName = conf->value("name").toString();
    _emailAddress = conf->value("email").toString();
    _mailUserName = conf->value("mailuser").toString();
    _mailPassword = conf->value("mailpassword").toString(); //TODO fix decipher
    _mailServer = conf->value("mailserver").toString();
    _mailPort = conf->value("mailport", 110).toInt();
    _accountType = static_cast<AccountType>( conf->value("type", POP ).toInt() );
    _baseFolder = conf->value("basefolder").toString();

    _useSig = conf->value("usesig").toBool();
    _maxMailSize = conf->value("maxmailsize", 100*1024).toInt();
    setCheckInterval( conf->value("checkinterval", -1).toInt() );

    // write signature to a file (to resolve problem with config.h)
    QString file = QString(QDir::homePath()) + "/Applications/qtmail/sig_" + _mailUserName + "_" + _mailServer;
    QFile f(file);
    if ( f.open(QIODevice::ReadOnly) ) {    // file opened successfully
        QTextStream t( &f );        // use a text stream
        _sig = t.readAll();
        f.close();
    }

    _smtpServer = conf->value("smtpserver").toString();
    _smtpPort = conf->value("smtpport", 25).toInt();
#ifndef QT_NO_OPENSSL
    _smtpUsername = conf->value("smtpUsername").toString();
    _smtpPassword = conf->value("smtpPassword").toString();

    int index = conf->value("smtpAuthentication",0).toInt();
    _smtpAuthentication = static_cast<AuthType>(index);
    index = conf->value("smtpEncryption",0).toInt();
    _smtpEncryption = static_cast<EncryptType>(index);
    index = conf->value("mailEncryption",0).toInt();
    _mailEncryption = static_cast<EncryptType>(index);
#endif
    _defaultMailServer = conf->value("defaultmailserver", Qt::CaseInsensitive).toBool();

    _synchronize = conf->value("synchronize").toBool();
    _deleteMail = conf->value("deletemail").toBool();

    _networkCfg = conf->value("networkConfig").toString();
    _autoDL = conf->value("autoDownload").toBool();

    QString oldGroup = conf->group();
    if ( _accountType == IMAP ) {
        uint count = conf->value("mailboxes", 0).toInt();
        Mailbox *box;
        for (uint x = 0; x < count; x++) {
            conf->beginGroup( id() + "_" + QString::number(x) );
            box = new Mailbox( this );
            box->readSettings(conf);
            mailboxes.append(box);
            conf->endGroup();
        }
    } else {
        QString queueDelete = conf->value("queuedelete").toString();
        if(!queueDelete.isEmpty())
            delList = queueDelete.split(",!;");
        conf->endGroup();
        conf->beginGroup( id() );
        int count = conf->value("idcount", 0).toInt();
        uidList.clear();
        for (int x = 1; x < (count + 1); x++) {
            uidList.append( conf->value( QString::number(x) ).toString() );
        }
        conf->endGroup();
        conf->beginGroup( oldGroup );
    }

    search->setFromAccount( id() );
}

bool MailAccount::matchesEmail(const QMailMessage& message) const
{
    return search->matches(message);
}

void MailAccount::checkIntervalTimeout()
{
    emit intervalCheck( this );
}

int MailAccount::unreadCount() const
{
    return _unreadCount;
}

void MailAccount::setUnreadCount( int count )
{
    _unreadCount = count;
}

int MailAccount::count() const
{
    return _count;
}

void MailAccount::setCount( int count )
{
    _count = count;
}

QString MailAccount::networkConfig() const
{
    if (_networkCfg.isEmpty()) {
        QSettings dfltConfig("Trolltech", "Network");
        dfltConfig.beginGroup("WAP");
        return dfltConfig.value("DefaultAccount").toString();
    }
    return _networkCfg;
}

