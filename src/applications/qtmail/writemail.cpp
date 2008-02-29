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

#include "writemail.h"
#include "accountlist.h"
#include "account.h"
#ifdef QTOPIA_PHONE
#ifndef QTOPIA_NO_MMS
#include "mmscomposer.h"
#endif
#include "composer.h"
#else
#include "pdacomposer.h"
#endif
#include "qtmailwindow.h"

#include <QValueSpaceItem>
#include <qtopiaapplication.h>
#include <QSettings>
#include <QMenu>
#include <qsoftmenubar.h>
#include <qdocumentselector.h>
#include <qtopiaglobal.h>
#include <QTabWidget>
#include <QToolBar>
#include <QMenuBar>
#include <QStackedWidget>
#include <QMessageBox>
#include <QDateTime>
#include <QBoxLayout>
#ifdef QTOPIA_PHONE
#include <QTextEdit>
#endif
#include <QDir>
#include <QFileInfo>
#include <qmimetype.h>
#include <QToolTip>
#include <QWhatsThis>
#include <QStackedWidget>
#ifndef Q_OS_WIN32
#ifndef Q_OS_MACX
#include <sys/vfs.h>  // sharp 1862
#endif
#endif

#ifdef QTOPIA_PHONE
#include "detailspage.h"
#endif

static QString createFrom(QString name, QString email)
{
    QString from;
    if ( !name.trimmed().isEmpty() )
        from = Email::quoteString( name.trimmed() );

    if ( email.isEmpty() )
        return from; // Error no email address specified

    if ( email[0] == '<' )
        from += " " + email;
    else
        from += " <" + email + ">";

    return from;
}


//===========================================================================

SelectListWidget::SelectListWidget( QDialog *parent )
    : QListWidget( parent )
{
    mParent = parent;
    setFrameStyle( QFrame::NoFrame );
    mKeyBackPressed = false;
}

bool SelectListWidget::keyBackPressed()
{
    return mKeyBackPressed;
}

void SelectListWidget::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Back) {
        mKeyBackPressed = true;
        mParent->reject();
        e->accept();
        return;
    }
    QListWidget::keyPressEvent( e );
}

//===========================================================================

WriteMail::WriteMail(QWidget* parent,  const char* name, Qt::WFlags fl )
    : QMainWindow( parent, fl ), mail(),_detailsOnly(false)
{
    setObjectName(name);

    /*
      because of the way QtMail has been structured, even though
      WriteMail is a main window, setting its caption has no effect
      because its parent is also a main window. have to set it through
      the real main window.
    */
#ifdef QTOPIA_DESKTOP
    m_mainWindow = this;
#else
    m_mainWindow = QTMailWindow::singleton();
#endif

    init();
}

WriteMail::~WriteMail()
{
    delete m_composerInterface;
    m_composerInterface = 0;
}

void WriteMail::setAccountList(AccountList *list)
{
    accountList = list;
#ifdef QTOPIA_PHONE
    if (m_detailsPage )
        m_detailsPage->setFromFields( accountList->emailAccounts() );
#else
    m_composerInterface->setFromFields( accountList->emailAccounts() );
#endif
    QStringList::Iterator it;
    QStringList as = accountList->emailAccounts();
}

void WriteMail::init()
{
    widgetStack = new QStackedWidget(this);
    setCentralWidget(widgetStack);

#ifdef QTOPIA_PHONE
    // stage 1 - composing
    m_composerInterface = 0;
    m_composerWidget = 0;
    // setup when a composer is set

    // stage 2 - specifying recipients and other message details
    // also setup when a composer is set
    m_detailsPage = 0;

    keyEventAccepted = false;
#else
    m_composerInterface = new PDAComposer( this, "pdaComposer" );
    connect( m_composerInterface, SIGNAL(contentChanged()),
             this, SLOT(messageChanged()) );
    connect( m_composerInterface, SIGNAL(recipientsChanged()),
             this, SLOT(messageChanged()) );
    m_mainWindow->setWindowTitle( tr("Write Mail") );
#endif

    // stage 3 - sending
    /*
    TODO  : composers should handle this
    wrapLines = new QAction( tr("Wrap lines"), QString::null, 0, this, 0);
    wrapLines->setToggleAction( true );
    connect(wrapLines, SIGNAL( activated() ), this, SLOT( wrapToggled() ) );
    */

#ifdef QTOPIA_PHONE
    m_cancelAction = new QAction(QIcon(":icon/cancel"),tr("Cancel"),this);
#else
    m_cancelAction = new QAction(QIcon(":icon/discard"),tr("Discard"),this);
#endif

    connect( m_cancelAction, SIGNAL(triggered()), this, SLOT(discard()) );

    m_draftAction = new QAction(QIcon(":icon/draft"),tr("Save in drafts"),this);
    connect( m_draftAction, SIGNAL(triggered()), this, SLOT(draft()) );
    m_draftAction->setWhatsThis( tr("Save this message as a draft.") );

    m_previousAction = new QAction( QIcon(":icon/i18n/previous"),tr("Previous"),this);
    connect( m_previousAction, SIGNAL(triggered()), this, SLOT(previousStage()) );
}

void WriteMail::nextStage()
{
#ifdef QTOPIA_PHONE
    QWidget *curPage = widgetStack->currentWidget();
    if (!curPage) {
        composeStage();
    } else if (curPage == m_composerWidget) {
        if (!m_composerInterface)
            qWarning( "BUG: NULL composer interface in composer stage" );
        if (!m_composerInterface->hasContent()) {
            discard();
            //QTimer::singleShot( 0, this, SIGNAL(discardMail()) );
            return;
        }
        detailsStage();
    } else if (curPage == m_detailsPage) {
        sendStage();
    } else {
        qWarning("BUG: WriteMail::nextStage() called in unknown stage.");
    }
#endif
}

void WriteMail::sendStage()
{
    enqueue();
}

void WriteMail::detailsStage()
{
#ifdef QTOPIA_PHONE
    if (!changed() && !_detailsOnly) {
        discard();
        return;
    }
    m_detailsPage->setType( m_composerInterface->type() );
    if (_detailsOnly) //fix resize problem if not showing composer
        widgetStack->setCurrentWidget(m_composerWidget);
    widgetStack->setCurrentWidget(m_detailsPage);
    QTimer::singleShot( 0, this, SLOT(updateUI()) );
#endif
}

void WriteMail::composeStage()
{
#ifdef QTOPIA_PHONE
    if (composer().isEmpty())
        setComposer( ComposerFactory::defaultInterface() );
    widgetStack->setCurrentWidget(m_composerWidget);
    setComposerFocus();
    QTimer::singleShot( 0, this, SLOT(updateUI()) );
#endif
}

bool WriteMail::hasRecipients() const // Also check from field is not empty
{
#ifdef QTOPIA_PHONE
    if ((m_composerInterface
         && m_composerInterface->type() == MailMessage::Email)
         && m_detailsPage->from().trimmed().isEmpty() )
        return false;
    return !m_detailsPage->to().trimmed().isEmpty();
#else
    if (m_composerInterface->from().trimmed().isEmpty())
        return false;
    return !m_composerInterface->to().trimmed().isEmpty()
        || !m_composerInterface->cc().trimmed().isEmpty()
        || !m_composerInterface->bcc().trimmed().isEmpty();
#endif
}

bool WriteMail::isComplete() const
{
    return changed() && hasRecipients();
}

/*  QtMailWindow calls this method when it receives a close event, meaning the user
    just closed the window.  For the PDA Autosave is called, where the emailclient
    needs to determine what to do ( add to draft or outbox)
*/
bool WriteMail::tryAccept()
{
    int r = -1;
    // ideally, you'd also check to see if the message is the same as it was
    // when we started working on it
    if (hasMessageChanged) {
        r = QMessageBox::warning( this, tr("Save to drafts"),
                                  "<qt>" + tr("Do you wish to save the message"
                                              " to drafts?") + "</qt>",
                                  tr("Yes"), tr("No"),
                                  0, 0, 1 );
    }
    if (!r)
        draft();
    else // empty or user chose not to save
        discard();
    return true;
}

void WriteMail::previousStage()
{
#ifdef QTOPIA_PHONE
    QWidget *curPage = widgetStack->currentWidget();
    if (curPage == m_composerWidget)
        return; // no previous
    if (!curPage)
        composeStage();
    else if (curPage == m_detailsPage)
        composeStage();
    else
        qWarning("BUG: WriteMail::nextStage() called in unknown stage.");
#endif
}

bool WriteMail::buildMail()
{
    // TODO - should check formatting of recipients?
    /*
      if (!isComplete()) {
      qWarning("BUG: build mail called on incomplete mail");
      return false;
      }
    */

    m_composerInterface->getContent( mail );

#ifdef QTOPIA_PHONE
    m_detailsPage->getDetails( mail );
#endif

    mail.setDateTime( QDateTime::currentDateTime() );

    //FIXME : DetailsPage::getDetails should do this, but needs account list
    QString fromName, fromEmail = mail.from();
    QListIterator<MailAccount*> itAccount = accountList->accountIterator();
    while (itAccount.hasNext()) {
        MailAccount* current = itAccount.next();
        if (current->emailAddress() == fromEmail ||
            current->userName().toLower() == fromEmail.toLower() ) {
            fromName = current->userName();
            fromEmail = current->emailAddress();
            break;
        }
    }
    mail.setFrom( createFrom( fromName, fromEmail ) );
    mail.setStatus(EFlag_Outgoing | EFlag_Downloaded, true);

    return true;
}

/*  TODO: connect this method to the input widgets so we can actually know whether
    the mail was changed or not */
bool WriteMail::changed() const
{
    if (!m_composerInterface || !m_composerInterface->hasContent())
        return false;

    return true;
}

// sharp 1839 to here
static void checkOutlookString(QString &str)
{
    int  pos = 0;
    int  newPos;
    QString  oneAddr;

    QStringList  newStr;
    if (str.indexOf(";") == -1) {
        // not Outlook style
        return;
    }

    while ((newPos = str.indexOf(";", pos)) != -1) {
        if (newPos > pos + 1) {
            // found some string
            oneAddr = str.mid(pos, newPos-pos);

            if (oneAddr.indexOf("@") != -1) {
                // not Outlook comment
                newStr.append(oneAddr);
            }
        }
        if ((pos = newPos + 1) >= str.length()) {
            break;
        }
    }

    str = newStr.join(", ");
}

void WriteMail::attach( const QContent &dl )
{
    if (m_composerInterface) {
        m_composerInterface->attach( dl );
    } else {
        qWarning("WriteMail::attach called with no composer interface present.");
    }
}

void WriteMail::attach( const QString &fileName )
{
    if (m_composerInterface) {
        m_composerInterface->attach( fileName );
    } else {
        qWarning("WriteMail::attach called with no composer interface present.");
    }
}

// type 1 = reply, 2 = reply all, 3 = forward
void WriteMail::reply(const Email &replyMail, int type)
{
    int pos;

    // work out the kind of mail to response
    // type of reply depends on the type of message
    // a reply to an mms is just a new mms message with the sender as recipient
    // a reply to an sms is a new sms message with the sender as recipient

    mail = replyMail;
    QUuid id;
    mail.setUuid( id );
    QString fromEmail = mail.fromEmail();
    QString fromName = mail.fromName();
    QString fromBody = mail.plainTextBody();
    QStringList fileNames;

    if ((replyMail.type() ==  MailMessage::Email) &&
        (type == 3)) {// forwarded email
        for ( int i = 0; i < (int)mail.messagePartCount(); i++ ) {
            MailMessagePart &part = mail.messagePartAt( i );
            mail.validateFile( part );
            fileNames.append( part.storedFilename() );
        }
    }

#ifdef QTOPIA_PHONE
    newMail(ComposerFactory::defaultInterface( replyMail ));
#else
    newMail();
#endif

#ifdef QTOPIA_PHONE
    if(replyMail.type() == MailMessage::SMS) {
        // SMS
        if (!fromEmail.isEmpty()) {
            QString from = fromEmail;
            if (from.right( 4 ) == "@sms") {
                int len = replyMail.fromEmail().length();
                from = replyMail.fromEmail().left( len - 4 );
            }
            m_detailsPage->setTo( from );
        }
    } else if (replyMail.type() == MailMessage::MMS) {
        // MMS
        if (type == 3) {
            if (replyMail.subject().left(4).toLower() != "fwd:"
                && replyMail.subject().left(4).toLower() != "fw:") {
                m_detailsPage->setSubject("Fwd: " + replyMail.subject() );
            } else {
                m_detailsPage->setSubject( replyMail.subject() );
            }
            m_composerInterface->setMailMessage( mail );
            detailsStage();
        } else {
            m_detailsPage->setTo(replyMail.fromEmail());
            if (replyMail.subject().left(3).toLower() != "re:")
                m_detailsPage->setSubject("Re: " + replyMail.subject());
            else
                m_detailsPage->setSubject(replyMail.subject());
            if (type == 2) {
                QStringList all = replyMail.to();
                all += replyMail.cc();

                QString cc = all.join(", ");
                checkOutlookString( cc );

                m_detailsPage->setCc( cc );
            }
        }
    } else
#endif
    if( replyMail.type() ==  MailMessage::Email ) {
        // EMAIL
        if ( type == 3 ) {
            if (replyMail.subject().left(4).toLower() != "fwd:") {
#ifdef QTOPIA_PHONE
                m_detailsPage->setSubject("Fwd: " + replyMail.subject() );
#else
                m_composerInterface->setSubject("Fwd: " + replyMail.subject() );
#endif
            } else {
#ifdef QTOPIA_PHONE
                m_detailsPage->setSubject( replyMail.subject() );
#else
                m_composerInterface->setSubject( replyMail.subject() );
#endif
            }

            // Restore attachments
            for ( int i = 0; i < fileNames.count(); i++ )
                attach( fileNames[i] );
        } else {
            if (replyMail.subject().left(3).toLower() != "re:") {
#ifdef QTOPIA_PHONE
                m_detailsPage->setSubject( "Re: " + replyMail.subject() );
#else
                m_composerInterface->setSubject( "Re: " + replyMail.subject() );
#endif
            } else {
#ifdef QTOPIA_PHONE
                m_detailsPage->setSubject( replyMail.subject() );
#else
                m_composerInterface->setSubject( replyMail.subject() );
#endif
            }

            mail.removeAllMessageParts();

            QString str;

            if ( replyMail.replyTo().isEmpty() ) {
                str = replyMail.fromEmail();
            } else {
                str = replyMail.replyTo();
                mail.setReplyTo( "" );
            }

            checkOutlookString(str);
#ifdef QTOPIA_PHONE
            m_detailsPage->setTo( str );
#else
            m_composerInterface->setTo( str );
#endif

            mail.setInReplyTo( mail.messageId() );

            if ( type == 2 ) {  //reply all
                QStringList all = replyMail.to();
                all += replyMail.cc();

                QString cc = all.join(", ");
                checkOutlookString( cc );

#ifdef QTOPIA_PHONE
                m_detailsPage->setCc( cc );
#else
                m_composerInterface->setCc( cc );
#endif
            }
        }
        QString str;
        if (type == 3) {
            str = "\n------------ Forwarded Message ------------\n";
            str += "Date: " + replyMail.dateString() + "\n";
            str += "From: " + replyMail.fromName() + "<"
                   + replyMail.fromEmail() + ">\n";
            str += "To: " + replyMail.to().join(", ") + "\n";
            str += "Subject: " + replyMail.subject() + "\n";

            str += "\n" + replyMail.plainTextBody();
        } else {
            if (!replyMail.dateTime().isNull()) {
                str = "\nOn " + QTimeString::localYMDHMS(replyMail.dateTime(), QTimeString::Long) + ", " + fromName + " wrote:\n> ";
            } else {
                str = "\nOn " + replyMail.dateString() + ", "
                      + replyMail.fromName() + " wrote:\n> ";
            }

            pos = str.length();
            str += replyMail.plainTextBody();

            while (pos != -1) {
                pos = str.indexOf('\n', pos);
                if (pos != -1)
                    str.insert(++pos, "> ");
            }
        }

        mail.setPlainTextBody( str );
        mail.setFrom("");
#ifdef QTOPIA_PHONE
        if (m_composerInterface->inherits( "TextComposerInterface" )) {
            TextComposerInterface *iface;
            iface = (TextComposerInterface *)m_composerInterface;
            iface->setText( fromBody );
        } else {
            qWarning("Don't know how to set text of unknown email "
                     "composer type.");
        }
#else
        m_composerInterface->setText( fromBody );
#endif
    }

    //accountId stored in mail, but email address used for selecting
    //account, so loop through and find the matching account
    QListIterator<MailAccount*> it = accountList->accountIterator();
    while (it.hasNext()) {
        MailAccount* current = it.next();
        if (current->id() == mail.fromAccount()) {
#ifdef QTOPIA_PHONE
        m_detailsPage->setFrom( current->emailAddress() );
#else
        m_composerInterface->setFrom( current->emailAddress() );
#endif
        break;
        }
    }

    // ugh. we need to do this everywhere
    hasMessageChanged = false;
}

void WriteMail::modify(Email *previousMail)
{
    QString recipients = "";

#ifdef QTOPIA_PHONE
    newMail( ComposerFactory::defaultInterface( previousMail->type() ));
#else
    newMail();
#endif
    mail.setUuid( previousMail->uuid() );
    mail.setFrom( previousMail->from() );

#ifdef QTOPIA_PHONE
    m_detailsPage->setFrom( previousMail->fromEmail() );
    m_detailsPage->setSubject( previousMail->subject() );
    m_composerInterface->setMailMessage( *previousMail );
    m_detailsPage->setTo( previousMail->to().join(", ") );
    m_detailsPage->setCc( previousMail->cc().join(", ") );
    m_detailsPage->setBcc( previousMail->bcc().join(", ") );
#else
    m_composerInterface->setFrom( previousMail->fromEmail() );
    m_composerInterface->setSubject( previousMail->subject() );
    m_composerInterface->setText( previousMail->plainTextBody() );
    m_composerInterface->setTo( previousMail->to().join(", ") );
    m_composerInterface->setCc( previousMail->cc().join(", ") );
    m_composerInterface->setBcc( previousMail->bcc().join(", ") );

    // Restore attachments
    for ( int i = 0; i < (int)previousMail->messagePartCount(); i++ ) {
      MailMessagePart &part = previousMail->messagePartAt( i );
      previousMail->validateFile( part );
      attach( part.storedFilename() );
    }

#endif

    // ugh. we need to do this everywhere
    hasMessageChanged = false;
}

void WriteMail::setRecipient(const QString &recipient)
{
#ifdef QTOPIA_PHONE
    m_detailsPage->setTo( recipient );
#else
    m_composerInterface->setTo( recipient );
#endif
}

void WriteMail::setBody(const QString &text)
{
#ifdef QTOPIA_PHONE
    if (!m_composerInterface)
        return;
    if (m_composerInterface->type() == MailMessage::MMS)
        return;
    if (m_composerInterface->inherits( "TextComposerInterface" )) {
        TextComposerInterface *iface = (TextComposerInterface *)m_composerInterface;
        iface->setText( text );
    } else {
        qWarning("Can't set body of mail with unknown composer interface.");
    }
#else
    m_composerInterface->setText( text );
#endif
}

bool WriteMail::hasContent()
{
    // Be conservative when returning false, which means the message can
    // be discarded without user confirmation.
    if (!m_composerInterface)
        return true;
    return m_composerInterface->hasContent();
}

#ifdef QTOPIA_PHONE
void WriteMail::setSmsRecipient(const QString &recipient)
{
  m_detailsPage->setTo( recipient );
}
#endif

void WriteMail::setRecipients(const QString &emails, const QString & numbers)
{
    QString to;
    to += emails;
#ifdef QTOPIA_PHONE
    to = to.trimmed();
    if (to.right( 1 ) != "," && !numbers.isEmpty()
        && !numbers.trimmed().startsWith( "," ))
        to += ", ";
    to +=  numbers;
    m_detailsPage->setTo( to );
#else
    Q_UNUSED(numbers);
    m_composerInterface->setTo( to );
#endif
}

void WriteMail::reset()
{
    mail = MailMessage();
    mail.removeAllMessageParts();
    mail.setAllStatusFields(0);
    mail.setServerUid("");
    mail.setMessageId("");
    mail.setInReplyTo("");
    mail.setMultipartRelated( false );
    QUuid id;
    mail.setUuid( id );
    mail.setAllStatusFields(0);
    mail.setTo( QStringList() );
    mail.setCc( QStringList() );
    mail.setBcc( QStringList() );

#ifdef QTOPIA_PHONE
    if (m_detailsPage)
        m_detailsPage->clear();
#else
    m_composerInterface->clear();
#endif

#ifdef QTOPIA_PHONE
    if (m_composerInterface) {
        m_composerInterface->deleteLater();
        m_composerInterface = 0;
        m_composerWidget = 0;
    }
#endif

    hasMessageChanged = false;
}

void WriteMail::newMail(const QString& cmpsr, bool detailsOnly)
{
#ifndef QTOPIA_PHONE
    Q_UNUSED(numbers);
#endif
    reset();
    _detailsOnly = detailsOnly;

#ifdef QTOPIA_PHONE
    QString comp = cmpsr;
    if (comp.isNull()) {
        if (!selectComposer()) {
            setComposer(ComposerFactory::defaultInterface());
            QTimer::singleShot(0,this,SLOT(discard()));
            return;
        }
    } else {
        setComposer( comp );
    }
    if( m_composerInterface )
        m_detailsPage->setType( m_composerInterface->type() );
    if(_detailsOnly)
        detailsStage();
    else
        composeStage();
//     TODO  : attachments
//     addAtt->clear();
    QSoftMenuBar::setLabel(this, Qt::Key_Back, QSoftMenuBar::Next);
#endif
}

//updates actions, labels, captions based on the current stage
void WriteMail::updateUI()
{
#ifdef QTOPIA_PHONE
    QWidget *curPage = widgetStack->currentWidget();
    m_previousAction->setVisible( curPage != m_composerWidget && !_detailsOnly);

    if (curPage == m_detailsPage)
      m_mainWindow->setWindowTitle( m_composerInterface->nickName() + " " + tr("Details") );
    else if (curPage == m_composerWidget)
      m_mainWindow->setWindowTitle( tr("Create") + " " + m_composerInterface->nickName() );
    else
      m_mainWindow->setWindowTitle( tr("Write Message") );

    if (curPage == m_detailsPage)
        if( hasRecipients() )
            QSoftMenuBar::setLabel( this, Qt::Key_Back, "qtmail/enqueue", tr("Send") );
        else
            QSoftMenuBar::setLabel( this, Qt::Key_Back, "qtmail/draft", tr("Draft") );
    else if(curPage == m_composerWidget && m_composerInterface
            && !m_composerInterface->hasContent() )
        QSoftMenuBar::setLabel(this, Qt::Key_Back, QSoftMenuBar::Cancel);
    else
        QSoftMenuBar::setLabel(this, Qt::Key_Back, QSoftMenuBar::Next);
#endif
}

void WriteMail::discard()
{
#ifdef QTOPIA_PHONE
    // The follow if clause fixes a crash for the case when an email is being
    // composed and while the textwidget has focus cancel is selected from
    // the context menu
    if (m_composerInterface)
        m_composerInterface->widget()->hide();
#endif

    emit discardMail();
    // Must delete the composer widget after emitting the discardMail signal
    reset();
}

void WriteMail::enqueue()
{
    if (!isComplete()) {
        if (!changed()) {
            QString title = tr("Unmodified");
            QString temp = tr("<qt>The unmodified message has been discarded "
                              "without being sent</qt>" );
            QMessageBox::warning(qApp->activeWindow(),
                                 tr("Incomplete message"),
                                 temp,
                                 tr("OK") );
            discard();
        } else {
            QString title;
            QString temp;
            title = tr("No recipients" );
            temp = tr("<qt>The message could not be sent as no recipients "
                      "have been entered. The message has been saved in "
                      "the drafts folder</qt>" );
            QMessageBox::warning(qApp->activeWindow(),
                                 tr("Incomplete message"),
                                 temp,
                                 tr("OK") );
            draft();
        }
        return;
    }

    QValueSpaceItem item("/UI/Profile/PlaneMode");
    if (item.value().toBool()) {
        // Cannot send right now, in plane mode!
        QMessageBox::information(this, tr("Airplane Safe Mode"),
                                 tr("<qt>Cannot send message while "
                                    "in Airplane Safe Mode.</qt>"));
        return;
    }

    if (buildMail())
        emit enqueueMail(mail);
    // Prevent double deletion of composer textedit that leads to crash on exit
    reset();
}

void WriteMail::draft()
{
  if (!changed()) {
      discard();
      return;
  }
  if (buildMail())
      emit saveAsDraft( mail );
}

void WriteMail::keyPressEvent(QKeyEvent *e)
{
#ifdef QTOPIA_PHONE
    if (e->key() == Qt::Key_Back || e->key() == Qt::Key_Yes) {
        keyEventAccepted = false;
        /*
          don't call nextStage directly here, because it
          might delete the composer that this key press
          originated from.
        */
        QTimer::singleShot( 0, this, SLOT(nextStage()) );
        e->accept();
        if (!Qtopia::mousePreferred())
            keyEventAccepted = true;
    } else
#endif
        QMainWindow::keyPressEvent( e );
}

bool WriteMail::selectComposer()
{
#ifdef QTOPIA_PHONE
    QDialog *dlg = new QDialog( this);
    dlg->setObjectName("selectComposer");
    dlg->setWindowTitle( tr("Select Type") );
    QtopiaApplication::setMenuLike( dlg, true );
    QVBoxLayout *l = new QVBoxLayout( dlg );
    SelectListWidget *lb = new SelectListWidget( dlg );
    lb->setSpacing(2);
    lb->setFrameStyle(QFrame::NoFrame);
    l->addWidget( lb );
    QList<QString> interfaces = ComposerFactory::interfaces();
    QList<QString>::ConstIterator it;
    for( it = interfaces.begin() ; it != interfaces.end() ; ++it ) {
        // list is order dependent
        QIcon icon = ComposerFactory::displayIcon( *it );
        QListWidgetItem* lwItem;
        lwItem = new QListWidgetItem(ComposerFactory::fullName( *it ), lb);
        lwItem->setIcon(icon);
        if( *it == composer() )
            lb->setCurrentRow( lb->count() -1 );
    }
    if (lb->currentRow() == -1)
        lb->setCurrentRow(0);
    connect( lb, SIGNAL(itemActivated(QListWidgetItem*)), dlg, SLOT(accept()));
    dlg->showMaximized();
    int result = QtopiaApplication::execDialog( dlg ) ;
    bool ok = result == QDialog::Accepted && lb->currentRow() != -1;
    // The dialog always seems to be accepted even when Key_Back is pressed
    // hence explicitly check to see if keyBack was pressed and if so
    // treat the dialog as rejected.
    ok &= !lb->keyBackPressed();
    if (ok) {
        QString inf = interfaces[lb->currentRow()];
        setComposer( inf );
    }
    delete dlg;
    return ok;
#else
    return false;
#endif
}

#ifdef QTOPIA_PHONE

void WriteMail::setComposer( const QString &id )
{
    if (m_composerInterface && m_composerInterface->id() == id)
        return;
    if (m_composerInterface) {
        delete m_composerInterface;
        m_composerInterface = 0;
    }
    if (m_composerWidget)
        widgetStack->removeWidget(m_composerWidget);
    m_composerWidget = 0;
    m_composerInterface = ComposerFactory::create( id, this );
    if (!m_composerInterface)
        return;
    m_composerWidget = m_composerInterface->widget();
    widgetStack->addWidget(m_composerWidget);
    connect( m_composerInterface, SIGNAL(contentChanged()),
             this, SLOT(updateUI()) );
    connect( m_composerInterface, SIGNAL(contentChanged()),
             this, SLOT(messageChanged()) );

    // add standard actions to context menu for this composer
    QMenu* menu = QSoftMenuBar::menuFor(m_composerInterface->widget());

    menu->addSeparator();
    menu->addAction(m_previousAction);
    menu->addAction(m_draftAction);
    menu->addAction(m_cancelAction);

    m_composerInterface->widget()->show();
    m_composerInterface->widget()->setFocus();

    m_detailsPage = new DetailsPage( this );
    widgetStack->addWidget(m_detailsPage);
    connect( m_detailsPage, SIGNAL(recipientsChanged()), this, SLOT(updateUI()) );
    m_detailsPage->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding ) );

    if (accountList)
        m_detailsPage->setFromFields( accountList->emailAccounts() );

    QMenu *detailsMenu = QSoftMenuBar::menuFor( m_detailsPage );
    detailsMenu->addSeparator();
    detailsMenu->addAction(m_previousAction);
    detailsMenu->addAction(m_draftAction);
    detailsMenu->addAction(m_cancelAction);

    QTimer::singleShot( 0, this, SLOT(updateUI()) );
}

void WriteMail::setComposerFocus()
{
    if (m_composerInterface)
        m_composerWidget->setFocus();
}

QString WriteMail::composer() const
{
    QString id;
    if (m_composerInterface)
        id = m_composerInterface->id();
    return id;
}

bool WriteMail::keyPressAccepted()
{
    if (keyEventAccepted) {
        keyEventAccepted = false;
        return true;
    }
    return false;
}

#endif // QTOPIA_PHONE

void WriteMail::messageChanged()
{
    hasMessageChanged = true;
}


