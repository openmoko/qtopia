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



#include "imapclient.h"
#include "emailhandler.h"
#include "longstream.h"

namespace QtMail
{
    bool compareMailboxes(Mailbox* a, Mailbox* b)
    {
        return (a->pathName() < b->pathName());
    }
}

ImapClient::ImapClient()
{
    connect(&client, SIGNAL(finished(ImapCommand&,OperationState&)),
            this, SLOT( operationDone(ImapCommand&,OperationState&) ) );
    connect(&client, SIGNAL( mailboxListed(QString&,QString&,QString&) ),
            this, SLOT( mailboxListed(QString&,QString&,QString&) ) );
    connect(&client, SIGNAL( messageFetched(Email&) ),
            this, SLOT( messageFetched(Email&) ) );
    connect(&client, SIGNAL( downloadSize(int) ),
            this, SIGNAL( downloadedSize(int) ) );
    connect(&client, SIGNAL( connectionError(int) ),
            this, SLOT( connectionError(int) ) );
}

ImapClient::~ImapClient()
{
}

void ImapClient::newConnection()
{
    if ( client.connected() ) {
        qWarning("socket in use, connection refused");
        return;
    }

    if ( account->mailServer().isEmpty() ) {
        emit mailTransferred(0);
        return;
    }

    status = Init;

    unresolvedUid.clear();
    selected = false;
    messageCount = 0;

    atCurrentBox = 0;
    internalId = QUuid();

    emit updateStatus( tr("DNS lookup" ) );
#ifdef SMTPAUTH
    if(account->mailEncryption() != MailAccount::Encrypt_NONE)
        client.openSecure(account->mailServer(),account->mailPort());
    else
#endif
        client.open(account->mailServer(), account->mailPort() );

}

void ImapClient::operationDone(ImapCommand &command, OperationState &state)
{
    if ( state != OpOk ) {
        switch ( command ) {
            case IMAP_UIDStore:
            {
                // Couldn't set a flag, ignore as we can stil continue
                qWarning("could not store message flag");
                break;
            }

            case IMAP_Login:
            {
                errorHandling(ErrLoginFailed, client.lastError() );
                return;
            }

            case IMAP_Full:
            {
                errorHandling(ErrFileSystemFull, client.lastError() );
                return;
            }

            default:        //default = all critical messages
            {
                errorHandling(ErrUnknownResponse, client.lastError() );
                return;
            }
        }
    }

    switch( command ) {
        case IMAP_Init:
        {
            emit updateStatus( tr("Logging in" ) );
            client.login(account->mailUserName(), account->mailPassword());
            break;
        }
        case IMAP_Login:
        {
            emit updateStatus( tr("Retrieving folders") );
            mailboxNames.clear();

            if ( selected ) {
                fetchNextMail();    //get selected messages only
            } else {
                client.list(account->baseFolder(), "*");        //account->baseFolder() == root folder
            }
            break;
        }
        case IMAP_List:
        {
            removeDeletedMailboxes();
            emit serverFolders();

            // Could be no mailbox has been selected to be stored locally
            if ( !nextMailbox() ) {
//              errorHandling(ErrUnknownResponse, tr("No Mailbox on Server") );
                emit mailTransferred( 0 );
                return;
            }

            emit updateStatus( tr("Checking ") + currentBox->displayName() );
            client.select( currentBox->pathName() );
            break;
        }
        case IMAP_Select:
        {
            handleSelect();
            break;
        }
        case IMAP_UIDSearch:
        {
            handleSearch();
            break;
        }
        case IMAP_UIDFetch:
        {
            handleUidFetch();
            break;
        }

        case IMAP_UIDStore:
        {
            setNextDeleted();
            break;
        }

        case IMAP_Expunge:
        {
            // Deleted messages, we can handle UID now
            handleUid();
            break;
        }

        case IMAP_Logout:
        {
            emit mailTransferred( 0 );
            return;

            break;
        }

        case IMAP_Full:
        {
            qFatal( "Logic error, IMAP_Full" );
            break;
        }

    }
}

void ImapClient::handleUidFetch()
{
    if (status == Fetch) {    //getting headers
        //currentBox->setServerUid( client.mailboxUidList() );
        currentBox->setServerUid( _mailboxUidList );

        if (nextMailbox()) {
            emit updateStatus( tr("Checking ") + currentBox->displayName() );
            client.select( currentBox->pathName() );
        } else {
            emit mailTransferred( 0 );
            return;
        }
    } else if (status == RFC822) {    //getting complete messages
        fetchNextMail();
        return;
    }
}

void ImapClient::handleSearch()
{
    switch(_searchStatus)
    {
    case All:
    {
      //deal with possbily buggy SEARCH command implementations

        if (client.mailboxUidList().count() == 0 &&
            client.mailboxUidList().count() != client.exists()) {
          qWarning("Inconsistent UID SEARCH result. Trying SEEN/UNSEEN");
            _searchStatus = Seen;
            client.uidSearch(MFlag_Seen);
        } else {
            _mailboxUidList = client.mailboxUidList();
            uniqueUidList = currentBox->getNewUid( _mailboxUidList );
            if (!messagesToDelete())
                handleUid();
        }
        break;
    }
    case Seen:
    {
        _mailboxUidList = client.mailboxUidList();
        uniqueUidList = currentBox->getNewUid( _mailboxUidList );
        _searchStatus = Unseen;
        client.uidSearch(MFlag_Unseen);
        break;
    }
    case Unseen:
    {
        _mailboxUidList += client.mailboxUidList();
        uniqueUidList += currentBox->getNewUid(client.mailboxUidList());
        _searchStatus = All; //reset
        if (!messagesToDelete())
            handleUid();
        break;
    }
    default:
        qWarning("Unknown search status");
    }
}

void ImapClient::handleUid()
{
    if (uniqueUidList.count() > 0) {
        emit updateStatus( tr("Previewing ") + QString::number( uniqueUidList.count() ) );

        status = Fetch;
        client.uidFetch(uniqueUidList.first(), uniqueUidList.last(),
                        F_UID | F_RFC822_SIZE | F_RFC822_HEADER );

    } else if (nextMailbox()) {
        emit updateStatus( tr("Checking ") + currentBox->displayName() );
        client.select( currentBox->pathName() );
    } else {
        emit mailTransferred( 0 );
    }

//    currentBox->setServerUid( client.mailboxUidList() );
}

bool ImapClient::messagesToDelete()
{
    if (account->deleteMail()) {
        delList = currentBox->msgToDelete();
        if (delList.count() == 0)
            return false;

        setNextDeleted();
        return true;
    }

    return false;
}

void ImapClient::setNextDeleted()
{
    if (delList.count() > 0) {
        msgUidl = delList.first();
        delList.removeAll( msgUidl );
        //precaution, don't download a header to a message we mark as deleted
        uniqueUidList.removeAll( msgUidl );

        emit updateStatus( tr("Deleting message %1").arg(msgUidl) );
        currentBox->msgDeleted( msgUidl );
        client.uidStore(msgUidl, MFlag_Deleted );
    } else {
        // All messages flagged as deleted, expunge them
        client.expunge();
    }
}

void ImapClient::handleSelect()
{
    /*  We arrive here when we want to get a single mail, need to change
        folder, and the correct folder has been selected.  Isn't async. prog. fun? */
    if (status == RFC822) {
        emit updateStatus( tr("Completing %1 / %2").arg(messageCount).arg(mailList->count()) );
        client.uidFetch( msgUidl, msgUidl, F_UID | F_RFC822_SIZE | F_RFC822 );
        return;
    }

    if (client.exists() > 0) {
        _searchStatus = All;
        client.uidSearch(1, client.exists() );  //get all uids just to be safe
    } else if (nextMailbox()) {
        emit updateStatus( tr("Checking ") + currentBox->displayName() );
        client.select( currentBox->pathName() );
    } else {    //last box checked
        emit mailTransferred( 0 );
    }
}

bool ImapClient::nextMailbox()
{
    bool found = false;

    while (!found) {
        if (atCurrentBox >= (uint)account->mailboxes.count())
            return false;

        currentBox = account->mailboxes.at(atCurrentBox);
        if (currentBox == NULL) {
            return false;
        }

        found = currentBox->localCopy();
        atCurrentBox++;
    }

    return true;
}

void ImapClient::fetchNextMail()
{
    QString *mPtr;

    if (messageCount == 0) {
        messageCount = 1;
        mPtr = mailList->first();
    } else {
        mPtr = mailList->next();
        messageCount++;
    }

    // Need twice the size of the mail free, extra 10kb is a margin of safety
    if (!LongStream::freeSpace( "", mailList->currentSize() * 2 + 1024*10 )) {
        errorHandling(ErrFileSystemFull, client.lastError() );
        return;
    }

    if (mPtr == NULL) {
        emit mailTransferred( messageCount );
        return;
    }

    status = RFC822;

    // Get next message in queue, but verify it's still on the server
    currentBox = account->getMailboxRefByMsgUid(*mPtr, mailList->currentMailbox() );
    while ((currentBox == NULL) && (mPtr != NULL)) {
        qLog(Messaging) << "ImapClient::fetchNextMail: cant find uid" << qPrintable(*mPtr);
        unresolvedUid.append(*mPtr);
        mPtr = mailList->next();
        messageCount++;
        if (mPtr != NULL)
            currentBox = account->getMailboxRefByMsgUid(*mPtr, mailList->currentMailbox() );
    }

    if (currentBox != NULL) {
        internalId = mailList->currentId();

        if ((currentBox->pathName()) == client.selected()) {
            emit updateStatus( tr("Completing %1 / %2").arg(messageCount).arg(mailList->count()) );
            msgUidl = *mPtr;
            client.uidFetch( msgUidl, msgUidl, F_UID | F_RFC822_SIZE | F_RFC822 );
            return;

        } else {
            msgUidl = *mPtr;
            client.select( currentBox->pathName() );
            return;
        }
    } else {
	if (!mPtr)
	    errorHandling(ErrUnknownResponse, tr( "Message not found" ) );
        emit mailTransferred( messageCount );
    }
}

/*  Mailboxes retrived from the server goes here.  If the INBOX mailbox
    is new, it means it is the first time the account is used.  Set the
    local copy flag to on so that the user can get messages without
    needing to configure it first.
*/
void ImapClient::mailboxListed(QString &flags, QString &del, QString &name)
{
    Mailbox *box;
    QStringList list = name.split(del);
    QString item;

    bool needSort = false;

    for ( QStringList::Iterator it = list.begin(); it != list.end(); ++it ) {
        item += *it;
        if ( ( box = account->getMailboxRef(item) ) == NULL ) {
            box = new Mailbox(account, flags, del, item);
//          if ( item == "INBOX" )
                box->setLocalCopy( true );

            account->mailboxes.append(box);
            needSort = true;
        } else {
//      box->setflags....
//      box->setDel...
        }

        // sanders - get mail in all folders
//      box->setLocalCopy( true );
        item += del;
    }

    if (needSort)
        qSort(account->mailboxes.begin(),account->mailboxes.end(),QtMail::compareMailboxes);

    mailboxNames.append(name);
}

void ImapClient::messageFetched(Email& mail)
{     //set some other parameters

    mail.setFromAccount( account->id() );
    mail.setFromMailbox( currentBox->pathName() );

    if ( status == Fetch ) {
        mail.setUuid( QUuid());
        mail.setStatus(EFlag_Downloaded, false);
    } else {
        mail.setUuid( internalId );
        mail.setStatus(EFlag_Downloaded, true);
    }

    emit newMessage(mail);

}

void ImapClient::setAccount(MailAccount *_account)
{
    account = _account;
}

/* DNS lookup failures and socket errors are reported here */
void ImapClient::connectionError(int e)
{
   //Socket already closed in protocol, just emit an error
   QString msg = tr("Connection failed");

   emit errorOccurred(e, msg);
}

void ImapClient::errorHandling(int status, QString msg)
{
    if ( client.connected() ) {
        client.close();
        emit updateStatus(tr("Error Occurred"));
        emit errorOccurred(status, msg);
    }
}

void ImapClient::quit()
{
    emit updateStatus( tr("Logging out") );
    client.logout();
}

void ImapClient::setSelectedMails(MailList *list, bool connected)
{
    selected = true;
    mailList = list;

    messageCount = 0;
    mailDropSize = 0;

    if ( connected ) {
        fetchNextMail();
    }
}

/*  removes any mailboxes form the client list which no longer is
    registered on the server.  Note that this will not apply to
    mailboxes which are newly created by the user.
*/
void ImapClient::removeDeletedMailboxes()
{
    QList<Mailbox*>::iterator box;
    bool exists;

    for (box = account->mailboxes.begin(); box != account->mailboxes.end();
        box++) {

        if ( !(*box)->userCreated() ) {
            exists = false;
            for (QStringList::Iterator it = mailboxNames.begin();
                 it != mailboxNames.end(); ++it) {
                if ( *it == (*box)->pathName() ) {
                    exists = true;
                } else if ( *it == (*box)->nameChanged() ) {
                    (*box)->changeName(*it, false);
                    exists = true;
                }
            }
            if (!exists) {
                account->removeBox((*box));
                box = account->mailboxes.begin();
            }
        }
    }
}
