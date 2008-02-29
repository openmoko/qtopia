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



#include "readmail.h"
#include "maillistview.h"
#include "emailfolderlist.h"
#include "smsclient.h"

#ifdef QTOPIA_DESKTOP
#include <qdapplication.h>
#include <qcopenvelope_qd.h>
#else
#include <qtopiaapplication.h>
#include <qtopiaipcenvelope.h>
#endif
#include <qtopianamespace.h>
#include <qtopiaservices.h>
#include <qsoftmenubar.h>
#include <qthumbnail.h>

#include <qtopia/pim/qcontact.h>

#ifdef QTOPIA_PHONE
#ifndef QTOPIA_NO_MMS
# include <qtopia/smil/smil.h>
# include <qtopia/smil/element.h>
# include <qtopia/smil/timing.h>
# include <qtopia/smil/transfer.h>
#endif
#endif

#include <qlabel.h>
#include <qimage.h>
#include <qaction.h>
#include <qfile.h>
#include <qtextstream.h>

#include <qcursor.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qregexp.h>
#include <qstackedwidget.h>
#include <qmessagebox.h>
#include <qboxlayout.h>
#include <qevent.h>
#include <qimagereader.h>
#include <qalgorithms.h>
#include <QDebug>

Browser::Browser( QWidget *parent  )
        : QTextBrowser( parent )
{
#ifdef QTOPIA_PHONE
    setFrameStyle( NoFrame );
#endif
}

Browser::~Browser()
{
}

void Browser::scrollBy(int dx, int dy)
{
    scrollContentsBy( dx, dy );
}

QVariant Browser::loadResource( int type, const QUrl& name )
{
    if ( resourceMap[name].isNull() )
        return QTextBrowser::loadResource( type, name );
    return resourceMap[name];
}

void Browser::setResource( const QUrl& name, QVariant var )
{
    resourceMap[name] = var;
}

void Browser::setSource(const QUrl &name)
{
    Q_UNUSED(name)
// We deal with this ourselves.
//    QTextBrowser::setSource( name );
}

//===========================================================================

class InfoLabel : public QLabel
{
public:
    InfoLabel( const QString &txt, QWidget *parent = 0, const char *name = 0 )
        : QLabel( txt, parent, Qt::FramelessWindowHint | Qt::Tool )
    {
        setObjectName( name );
        setFrameStyle( QFrame::WinPanel + QFrame::Raised );
        setMargin( 10 );
        resize( sizeHint() );

        startTimer( 2000 );
    }

protected:
    void timerEvent(QTimerEvent *)
    {
        delete this;
    }

    void mousePressEvent(QMouseEvent *)
    {
        delete this;
    }
};

#ifdef QTOPIA_PHONE
#ifndef QTOPIA_NO_MMS
static QMap<SmilDataSource*,QIODevice*> *transfers = 0;
#endif
#endif

//===========================================================================

ReadMail::ReadMail( QWidget* parent,  const QString name, Qt::WFlags fl )
    : QMainWindow(parent, fl)
{
    setObjectName( name );
    plainTxt = false;
    sending = false;
    receiving = false;
    initialized = false;
    _pHeight = 0;
    _pWidth = 0;
    firstRead = false;
    mContactModel = 0;

    init();
}

ReadMail::~ReadMail()
{
}

void ReadMail::init()
{
#if defined(QTOPIA_PHONE) && !defined(QTOPIA_NO_MMS)
    smilView = 0;
#endif

#ifndef QTOPIA_PHONE
#ifdef QTOPIA4_TODO
    bar = new QToolBar(this);
    bar->setMoveable( false );
    bar->setHorizontalStretchable( true );
    menu = new QMenuBar( bar );

    mailMenu = new QPopupMenu(menu);
    menu->insertItem( tr( "&Mail" ), mailMenu);

    viewMenu = new QPopupMenu(menu);
    menu->insertItem( tr( "&View" ), viewMenu);

    statusMenu = new QPopupMenu(menu);

    bar = new QToolBar(this);
#endif
#endif

    getThisMailButton = new QAction( QIcon(":icon/getmail"), tr("Get this mail"), this );
    connect(getThisMailButton, SIGNAL(triggered()), this, SLOT(getThisMail()) );
    getThisMailButton->setWhatsThis( tr("Retrieve this mail from the server.  You can use this option to retrieve individual mails that would normally not be automatically downloaded.") );

    sendThisMailButton = new QAction( QIcon(":icon/sendmail"), tr("Send this mail"), this );
    connect(sendThisMailButton, SIGNAL(triggered()), this, SLOT(sendThisMail()));
    sendThisMailButton->setWhatsThis(  tr("Send this mail.  This option will not send any other mails in your outbox.") );

    replyButton = new QAction( QIcon(":icon/reply"), tr("Reply"), this );
    connect(replyButton, SIGNAL(triggered()), this, SLOT(reply()));
    replyButton->setWhatsThis( tr("Reply to sender only.  Select Reply all from the menu if you want to reply to all recipients.") );

    replyAllAction = new QAction( QIcon(":icon/replytoall"), tr("Reply All"), this );
    connect(replyAllAction, SIGNAL(triggered()), this, SLOT(replyAll()));

    forwardAction = new QAction(tr("Forward"), this );
    connect(forwardAction, SIGNAL(triggered()), this, SLOT(forward()));

    modifyButton = new QAction( QIcon(":icon/edit"), tr("Modify"), this );
    connect(modifyButton, SIGNAL(triggered()), this, SLOT(modify()));
    modifyButton->setWhatsThis( tr("Opens this mail in the composer so that you can make modifications to it.") );

    previousButton = new QAction( QIcon( ":icon/up" ), tr( "Previous" ), this );
    connect( previousButton, SIGNAL(triggered()), this, SLOT( previous() ) );
    previousButton->setWhatsThis( tr("Read the previous mail in the folder.") );

    nextButton = new QAction( QIcon( ":icon/down" ), tr( "Next" ), this );
    connect( nextButton, SIGNAL(triggered()), this, SLOT( next() ) );
    nextButton->setWhatsThis( tr("Read the next mail in the folder.") );

    attachmentsButton = new QAction( QIcon( ":icon/attach" ), tr( "Attachments" ), this );
    connect( attachmentsButton, SIGNAL(triggered()), this,
            SLOT( viewAttachments() ) );
    attachmentsButton->setWhatsThis( tr("View the attachments in the mail.") );

    plainTextButton = new QAction( QIcon( ":icon/txt" ), tr( "Text Format" ), this );
    plainTextButton->setCheckable( true );
    connect( plainTextButton, SIGNAL(triggered()), this, SLOT( shiftText() ) );
    plainTextButton->setWhatsThis( tr("Toggle the display of mail between plain text and rich text.") );

    deleteButton = new QAction( QIcon( ":icon/trash" ), tr( "Delete" ), this );
    connect( deleteButton, SIGNAL(triggered()), this, SLOT( deleteItem() ) );
    deleteButton->setWhatsThis( tr("Move this mail to the trash folder.  If the mail is already in the trash folder it will be deleted. ") );

    printButton = new QAction( QIcon( ":icon/print" ), tr( "Print" ), this );
    connect( printButton, SIGNAL(triggered()), this, SLOT(print()) );

    QFrame *vbox = new QFrame(this);
    QVBoxLayout *vboxLayout = new QVBoxLayout(vbox);
    vboxLayout->setMargin(0);

    views = new QStackedWidget(vbox);
    vboxLayout->addWidget( views );

    emailView = new Browser( views );
    emailView->setObjectName( "emailView" );
    connect(emailView, SIGNAL( anchorClicked(const QUrl&) ),
            this, SLOT( linkClicked(const QUrl&) ) );
    emailView->setWhatsThis( tr("This view displays the contents of the mail.") );
    views->addWidget(emailView);
    views->setCurrentWidget(emailView);

    progressLabel = new QLabel(vbox);
    vboxLayout->addWidget( progressLabel );
    progressLabel->hide();

    setCentralWidget(vbox);

#ifdef QTOPIA_PHONE
    emailView->installEventFilter( this );
    context = QSoftMenuBar::menuFor( this );
#else
#ifdef QTOPIA4_TODO
    previousButton->addTo(viewMenu);
    nextButton->addTo(viewMenu);
    viewMenu->insertSeparator();
    attachmentsButton->addTo(viewMenu);
    viewMenu->insertSeparator();
    plainTextButton->addTo(viewMenu);
#endif
#endif
}

/*  We need to be careful here. Don't allow clicking on any links
    to automatically install anything.  If we want that, we need to
    make sure that the mail doesn't contain mailicious link encoding
*/
void ReadMail::linkClicked(const QUrl &lnk)
{
#ifdef QTOPIA4_TODO
    // Why does lnk have "&amp;" etc. in it?
    QString str = Qtopia::plainString(lnk.toString());
#else
    QString str = lnk.toString();
#endif

    int pos = str.indexOf(";");
    if ( pos != -1 ) {
        QString command = str.left(pos);
        QString param = str.mid(pos + 1);

        if ( command == "attachment" ) { // No tr
            if (param == "view") { // No tr
                viewAttachments();
            } else if ( param.startsWith("scrollto;") ) {
                emailView->scrollToAnchor( param.mid(9) );
#ifndef QTOPIA_NO_MMS
            } else if (param == "play") {
                if (isMms)
                    viewMms();
#endif
            }
        }
    } else {
        if ( str.startsWith("mailto:") )  {
#ifdef QTOPIA_PHONE
            emit mailto( str );
#else
#ifdef QTOPIA4_TODO
            QPopupMenu *pop = new QPopupMenu(this);
            pop->insertItem( tr("Add to Contacts"), 0 );
            pop->insertItem( tr("Write mail to"), 1);

            int id;
            if ( (id = pop->exec( QCursor::pos() ) ) > -1 ) {
                if ( id == 0 ) {
                    if ( !AddressPicker::addressList()->addToContacts( str.mid(7) ) ) {
                        InfoLabel *label = new InfoLabel( tr("The address already exists in Contacts") );
                        QPoint p = QCursor::pos();
                        if ( p.x() + label->width() > qApp->desktop()->width() )
                            p.setX( qApp->desktop()->width() - label->width() );
                        label->move( p );
                        label->show();
                    }
                } else if ( id == 1 ) {
                    emit mailto( str );
                }
            }
#endif
#endif
        } else if ( str.startsWith("http://") ) {
#ifndef QTOPIA_DESKTOP
            {
                QtopiaServiceRequest e( "WebAccess", "openURL(QString)" );
                e << str;
                e.send();
            }
#else
#ifdef QTOPIA_DESKTOP
            // open a url
#else
#warning "should open link some other way if no qcop"
#endif
#endif
        }
#ifndef QTOPIA_DESKTOP
        else if( mail->type() == MailMessage::System && str.startsWith( QLatin1String( "qtopiaservice:" ) ) )
        {
            int commandPos  = str.indexOf( QLatin1String( "::" ) ) + 2;
            int argPos      = str.indexOf( '?' ) + 1;
            QString service = str.mid( 14, commandPos - 16 );
            QString command;
            QStringList args;

            if( argPos > 0 )
            {
                command = str.mid( commandPos, argPos - commandPos - 1 );
                args    = str.mid( argPos ).split( ',' );
            }
            else
                command = str.mid( commandPos );

            QtopiaServiceRequest e( service, command );

            foreach( QString arg, args )
                e << arg;

            e.send();
        }
#endif
    }
}

void ReadMail::updateView()
{
    QString mailStringSize;

    if ( lastMailUuid.isNull() )
        return;

    isMms = false;
    isSmil = false;
    views->setCurrentWidget(emailView);

    mail->readFromFile();
    if ( !mail->status(EFlag_Read) ) {
        mail->setStatus(EFlag_Read, true );
        mail->setDirty( true );
        firstRead = true;
    }
    else
        firstRead = false;

    //report currently viewed mail so that it will be
    //placed first in the queue of new mails to download.
    emit viewingMail(mail);

    if ( mail->status(EFlag_Incoming) ) {

        if ( !mail->status(EFlag_Downloaded) ) {

            uint mailSize = mail->size();
            if (mailSize < 1024) {
                mailStringSize.setNum(mailSize);
                mailStringSize += " Bytes";
            } else if (mailSize < 1024*1024) {
                mailStringSize.setNum( (mailSize / 1024) );
                mailStringSize += " Kb";
            } else {
                float f = (float) mailSize / (1024*1024);
                mailStringSize.setNum(f , 'g', 3 );
                mailStringSize += " Mb";
            }
        }
    }

#ifndef QTOPIA_NO_MMS
    QString mmsType = mail->header("X-Mms-Message-Type");
    if (mmsType.contains("m-retrieve-conf")
        || mmsType.contains("m-send-req")) {
        isMms = true;
        if (mail->header("Content-Type").contains("multipart/related")) {
            isSmil = true;
        }
    }
#endif
    plainTextButton->setVisible(!isMms && mail->type() != MailMessage::System );
    forwardAction->setVisible(mail->type() == MailMessage::Email );
    replyAllAction->setVisible(mail->type() == MailMessage::Email );

    if ( !plainTxt || isMms ) {                                         //use RichText, inline pics etc.
        emailView->setHtml( fancyText(mailStringSize) );
    } else {                                            // show plain txt mail
        emailView->setPlainText( normalText(mailStringSize) );
    }
    //force an extra resize to fire so scrollbars are actually sized properly on the first go
    //Bug in QTextEdit
    emailView->append("");
}

void ReadMail::resizeEvent(QResizeEvent *e)
{
    _pWidth = e->size().width();
    _pHeight = e->size().height();

    updateView();
}

void ReadMail::keyPressEvent(QKeyEvent *e)
{
    switch( e->key() ) {
        case Qt::Key_A:
            if ( attachmentsButton->isEnabled() )
                viewAttachments();
            break;
        case Qt::Key_P:
            if ( previousButton->isEnabled() )
                previous();
            break;
        case Qt::Key_N:
            if ( nextButton->isEnabled() )
                next();
            break;
        case Qt::Key_Delete:
            deleteItem();
            break;
        case Qt::Key_R:
            reply();
            break;
        case Qt::Key_F:
            forward();
            break;
        case Qt::Key_T:
            shiftText();
            plainTextButton->toggle();
            break;
        case Qt::Key_E:
            if ( modifyButton->isEnabled() )
                modify();
        default:
            QMainWindow::keyPressEvent( e );
    }
}

int ReadMail::pWidth()
{
    if ( _pWidth )
        return _pWidth;

    return width();
}

int ReadMail::pHeight()
{
    if ( _pHeight )
        return _pHeight;

    return height();
}

QString ReadMail::fancyText(QString mailSize)
{
    int i;
    QString subj, text1, text2, sepText;

    subj += "<b><big><center><font color=\"#0000FF\">" +
             Qt::escape( mail->subject() ) +
             "</font></center></big></b><br>";

    if (!mail->from().isEmpty() && mail->from() != "\"\" <>") { // ugh
        text1 += "<b>"+tr("From")+": </b>";
        text1 += refMailTo( mail->from() ) + "<br>";
    }

    if (mail->to().count() > 0) {
        text1 +="<b>"+tr("To")+": </b>";
        text1 += listRefMailTo( mail->to() );
    }

    if (mail->cc().count() > 0) {
        text1 += "<br><b>CC: </b>";
        text1 += listRefMailTo( mail->cc() );
    }
    if (mail->bcc().count() > 0) {
        text1 += "<br><b>BCC: </b>";
        text1 += listRefMailTo( mail->bcc() );
    }
    if ( !mail->replyTo().isEmpty() ) {
        text1 += "<br><b>"+tr("Reply-To")+": </b>";
        text1 += refMailTo( mail->replyTo() );
    }

    text1 += "<br><b>"+tr("Date")+": </b> ";
    if ( !mail->dateTime().isNull() ) {
        text1 += Qt::escape( QTimeString::localYMDHMS( mail->dateTime(), QTimeString::Long ) );
    } else {
        text1 += Qt::escape( mail->dateString() );
    }

    if (isMms && isSmil) {
        text2 = "<a href=\"attachment;play\">";
        text2 += "<b>" + tr("<b>Play MMS") + "</b>";
        text2 += "</a>";
        sepText += "<hr><br>";
        bool showAtt = false;
        for (i = 0; i < (int)mail->messagePartCount(); i++) {
            MailMessagePart &part = mail->messagePartAt(i);
            if (part.contentType().toLower() == "text/x-vcard"
                || part.contentType().toLower() == "text/x-vcalendar") {
                if (!showAtt) {
                    text2 += sepText + "<b>"+tr("Attachments")+": </b><br>";
                    showAtt = true;
                }
                text2 += "<a href=\"attachment;view\"> " +
                         Qt::escape(part.prettyName()) + "</a> ";
            }
        }
    } else if (mail->messagePartCount() > 0) {
#ifdef QTOPIA_PHONE
        sepText += "<hr><br>";
#else
#ifdef QTOPIA4_TODO
        sepText += "<br>";
        text2 += "<b>"+tr("Attachments")+": </b>";

        for (i = 0; i < (int)mail->messagePartCount(); i++ ) {
            MailMessagePart &part = mail->messagePartAt( i );

            if ( (part.contentType().startsWith("text") || part.contentType().startsWith("image") ) ) { // No tr
                text2 += "<a href=\"attachment;scrollto;" + Qt::escape( part.prettyName() ) + "\"> " +
                         Qt::escape(part.prettyName() ) + "</a> ";
            } else {
                text2 += "<a href=\"attachment;view\"> " +
                         Qt::escape(part.prettyName() ) + "</a> ";
            }
        }
        text2 += "<hr><br>";
#endif
#endif

        text2 += formatText( mail->plainTextBody() );
        if ( mail->plainTextBody().isEmpty() ) {
            text2 += mail->htmlBody();
            initImages();
        }

        for ( i = 0; i < (int)mail->messagePartCount(); i++ ) {
            MailMessagePart &part = mail->messagePartAt( i );

        QString attachmentName = part.prettyName();

#ifdef QTOPIA_PHONE
        bool isFilePart = !part.name().isEmpty();
        bool hasContentLocation = !part.contentLocation().isEmpty();
        if ( isFilePart || hasContentLocation) {
            if(!isFilePart)
                attachmentName = part.contentLocation();
#endif
            text2 += "<a name=\"" + Qt::escape( attachmentName ) + "\"> </a>"; // No tr

            text2 += "<br><hr><b>" + tr("Attachment") +
                    ": </b> <a href=\"attachment;view\"> " +
                    Qt::escape( attachmentName ) + " </a> <hr>";
#ifdef QTOPIA_PHONE
        }
#endif

            if (part.contentType().startsWith("text") ) { // No tr
                QString temp;
                if ( ( temp = part.decodedBody() ) != QString() ) {
                    if ( part.contentType().indexOf("html") > -1) {
                        text2 += temp + "<br>";
#ifdef QTOPIA_PHONE
                    } else if ( !isFilePart ) {
                        text2 += formatText( temp );
#endif
                    } else {
                        text2 += "<pre> " + Qt::escape(temp) + "</pre>";
                    }
                } else {
                    text2 += "<b>"+tr("Could not locate file")+"</b><br>";
                }

            } else if (part.contentType().startsWith("image") ) { // No tr

                QString str;

                if ( mail->multipartRelated() ) {
                    str = part.contentID();
                    if ( str.length() > 0 && str[0] == '<' )
                        str = str.mid(1);
                    if ( str.length() > 0 && str[(int)str.length() - 1] == '>' )
                        str = str.left(str.length() - 1);
                    str = "cid:" + str;
                } else {
                    str = QString("%1_%2").arg(i).arg(part.filename());
                    text2 += "<img src =\"" + str + "\"> </img>";
                }
                QFile oldFile( part.storedFilename().trimmed() );
                bool exists = oldFile.exists();
                mail->validateFile( part );
                QThumbnail thumb( part.storedFilename().trimmed() );
                QSize size = thumb.actualSize();
                int maxsize = qMin(emailView->width(), emailView->height());
                QPixmap pixmap;
                if ( size.width() > maxsize || size.height() > maxsize )
                    pixmap = thumb.pixmap( QSize( maxsize, maxsize ));
                else
                    pixmap = thumb.pixmap( size );
                emailView->setResource( QUrl( str ), QVariant( pixmap.toImage() ) );
                if (!exists)
                    oldFile.remove();
            }
        }
    }
    else if( mail->type() == MailMessage::System )
    {
        text2 += mail->plainTextBody();
        sepText += "<hr><br>";
    }
    else {
        sepText += "<hr><br>";
        if ( mail->status(EFlag_Downloaded) || !mail->status(EFlag_Incoming) ) {
            if ( mail->plainTextBody().isEmpty() ) {
                text2 += mail->htmlBody();
                initImages();
            }
#ifndef QTOPIA_NO_SMS
            else if ( mail->plainTextBody().startsWith( SmsClient::vCardPrefix() ) ) {
                QString copy = mail->plainTextBody();
                QList<QContact> contacts
                    = QContact::readVCard(
                        copy.remove( SmsClient::vCardPrefix() ).toLatin1() );

                if ( contacts.count() == 0 ) {
                    // Invalid VCard data, so just show raw data
                    text2 += formatText( mail->plainTextBody() );
                } else if ( contacts.count() == 1 ) {
                    QString name = tr( "Message contains vCard for %1" );
                    if ( !contacts[0].nickname().isEmpty() ) {
                        text2 += formatText( name.arg( contacts[0].nickname() ) );
                    } else if ( !contacts[0].firstName().isEmpty() &&
                                !contacts[0].lastName().isEmpty() ) {
                        text2 += formatText ( name.arg( contacts[0].firstName() +
                                            " " +
                                            contacts[0].lastName() ) );
                    } else if ( !contacts[0].firstName().isEmpty() ) {
                        text2 += formatText( name.arg( contacts[0].firstName() ) );
                    } else {
                        text2 += formatText(
                                    tr( "Message contains vCard for a contact" ) );
                    }
                } else if ( contacts.count() > 1 ) {
                    text2 += formatText(
                                tr( "Message contains vCard for multiple contacts" ) );
                }
            }
#endif
            else {
                text2 += formatText( mail->plainTextBody() );
            }
        } else {
            text2 += "<b> "+tr("Awaiting download")+" </b><br>";
            text2 += tr("Size of mail") + ": " + mailSize;
        }
    }

#ifdef QTOPIA_PHONE
    return subj + text2 + sepText + "<font size=\"-5\">" + text1 + "</font>";
#else
    return subj + text1 + sepText + text2;
#endif
}

QString ReadMail::normalText(QString mailSize)
{
    QString text = tr("Subject")+": " + mail->subject() + "\n";
    text += tr("From")+": " + mail->fromName() + " " + mail->fromEmail() + "\n";
    text += tr("To")+": ";
    text += mail->to().join(", ");
    if (mail->cc().count() > 0) {
        text += "\nCC: ";
        text += mail->cc().join(", ");
    }
    if (mail->bcc().count() > 0) {
        text += "\nBCC: ";
        text += mail->bcc().join(", ");
    }
    if ( !mail->replyTo().isEmpty() ) {
        text += "\nReply-To: ";
        text += mail->replyTo();
    }

    text += "\n"+tr("Date")+": ";
    if ( !mail->dateTime().isNull() ) {
        text += QTimeString::localYMDHMS( mail->dateTime(), QTimeString::Long ) + "\n";
    } else {
        text += mail->dateString() + "\n";
    }


    if ( mail->messagePartCount() > 0 ) {
        text += tr("Attachments")+": ";
        for ( uint i = 0; i < mail->messagePartCount(); i++ ) {
            MailMessagePart &part = mail->messagePartAt( i );
            text += part.prettyName() + " ";
        }

        text += "\n\n";
    } else text += "\n";

    if ( !mail->status(EFlag_Incoming) ) {
        text += mail->plainTextBody();
    } else if ( mail->status(EFlag_Downloaded) ) {
        text += mail->plainTextBody();
    } else {
        text += "\n"+tr("Awaiting download")+"\n";
        text += tr("Size of mail")+": " + mailSize;
    }

    return text;
}

QString ReadMail::formatText(QString txt)
{
    QStringList p, out;

    p = txt.split("\n\n");
    QStringList::Iterator it = p.begin();

    uint lineCharLength;
    if ( fontInfo().pointSize() >= 10 ) {
        lineCharLength = width() / (fontInfo().pointSize() - 4 );
    } else {
        lineCharLength = width() / ( fontInfo().pointSize() - 3 );
    }

    //Maintain original linelengths if display width allows it
    if (!mail->header("X-Sms-Type").isNull()) {
        while ( it != p.end() ) {
            out.append( smsBreakReplies( *it) );
            it++;
        }
    } else if ( lineCharLength >= 70 ) {
        while ( it != p.end() ) {
            out.append( noBreakReplies( *it) );
            it++;
        }
    } else {
        while ( it != p.end() ) {
            out.append( handleReplies( *it ) );
            it++;
        }
    }

    QString ret = encodeUrlAndMail( out.join("<br><br>") );
    if (!mail->header("X-Sms-Type").isNull())
        ret.replace( QRegExp( "\\s" ), "&nbsp;" );

    return ret;
}

/*  Preserve white space, add linebreaks so text is wrapped to
    fit the display width */
QString ReadMail::smsBreakReplies(QString txt)
{
    QString str = "";
    QStringList p = txt.split("\n");

    QStringList::Iterator it = p.begin();
    while ( it != p.end() ) {
        str += buildParagraph( *it, "", true ) + "<br>";
        it++;
    }

    return str;
}


/*  Maintains the original linebreaks, but colours the lines
    according to reply level    */
QString ReadMail::noBreakReplies(QString txt)
{
    QString str = "";
    QStringList p = txt.split("\n");

    int x, levelList;
    QStringList::Iterator it = p.begin();
    while ( it != p.end() ) {

        x = 0;
        levelList = 0;
        while (x < (int)(*it).length() ) {
            if ( (*it)[x] == '>' ) {
                levelList++;
            } else if ( (*it)[x] == ' ' ) {
            } else break;

            x++;
        }

        if (levelList == 0 ) {
            str += Qt::escape(*it) + "<br>";
        } else {
            if ( levelList % 2 == 0 ) {
                str += "<font color=\"#0000FF\">";
            } else {
                str += "<font color=\"#FF0000\">";
            }
            str += Qt::escape(*it) + "</font><br>";
        }

        it++;
    }

    return str;
}

/*  This one is a bit complicated.  It divides up all lines according
    to their reply level, defined as count of ">" before normal text
    It then strips them from the text, builds the formatted paragraph
    and inserts them back into the beginning of each line.  Probably not
    to speed efficient on large texts, but this manipulation greatly increases
    the readability (trust me, I'm using this program for my daily email reading..)
*/
QString ReadMail::handleReplies(QString txt)
{
    QStringList out;
    QStringList p = txt.split("\n");
    QList<uint> levelList;
    QStringList::Iterator it = p.begin();
    int lastLevel = 0, level = 0;

    QString str, line;
    while ( it != p.end() ) {
        line = (*it).trimmed();
        if ( line.startsWith(">") ) {
            level = 0;
            for (int x = 0; x < (int)line.length(); x++) {
                if ( line[x] == ' ') {  //do nothing
                } else if ( line[x] == '>' ) {
                    level++;
                    if ( (level > 1 ) && (line[x-1] != ' ') ) {
                        line.insert(x, ' ');    //we need it to be "> > " etc..
                        x++;
                    }
                } else {
                    // make sure it follows style "> > This is easier to format"
                    line.insert(x, ' ');
                    break;
                }
            }
        } else {
            level = 0;
        }

        if ( level != lastLevel ) {
            if ( !str.isEmpty() ) {
                out.append( str );
                levelList.append( lastLevel );
            }

            str = "";
            lastLevel = level;
            it--;
        } else {
            str += line.mid( level * 2) + "\n";
        }

        it++;
    }
    if ( !str.isEmpty() ) {
        out.append( str );
        levelList.append( level );
    }

    //str = "<nobr>"; // let textview control linebreaks
    str = "";
    lastLevel = 0;
    int pos = 0;
    it = out.begin();
    while ( it != out.end() ) {
        if ( levelList[pos] == 0 ) {
            str += buildParagraph( *it, "" ) + "<br>";
        } else {
            QString pre = "";
            QString preString = "";
            for ( int x = 0; x < (int)levelList[pos]; x++) {
                pre += "&gt; ";
                preString += "> ";
            }

            // single lines ">" are dropped, so we insert this for better clarity
            if ( (int)levelList[pos] < lastLevel ) {
                str += "<font color=\"FF0000\">" + pre +"</font> <br>";
            }

            p = buildParagraph( *it, preString ).split("<br>" );

            if ( levelList[pos] % 2 == 0 ) {
                str += "<font color=\"#0000FF\">";
            } else {
                str += "<font color=\"#FF0000\">";
            }
            str += pre + p.join("<br> " + pre) + "</font><br>";
        }

        lastLevel = levelList[pos];
        pos++;
        it++;
    }

    if ( str.indexOf("<br>", -4) != -1 ) {
        str.truncate( str.length() - 4 );   //remove trailing br
    }
    //str += "</nobr>"; //let textview control linebreaks.

    return str;
}

QString ReadMail::buildParagraph(QString txt, QString prepend, bool preserveWs)
{
    QStringList out;

    //use escape here so we don't clutter our <br>
    QStringList p;
    if (preserveWs) {
        txt = Qt::escape( txt );
        p = txt.split(" ", QString::KeepEmptyParts);
    } else {
        txt = Qt::escape( txt.simplified() );
        p = txt.split(" ");
    }
    
    return p.join(" ");
    
    //for richtext/html contents, we cannot assume a font point size,
    //as this might change according to the markup. 
    //Compromise and only indent the first line to indicate reply level
    
    
//     QStringList::Iterator it = p.begin();
// 
//     QString str;
//     QFontMetrics fm( emailView->font() );
// //    int w = emailView->viewport()->width()-20;
//     int w = pWidth() - 20;
//     while ( it != p.end() ) {
// 
//         if ( fm.width( prepend + (*it) + str) > w ) {
//             if ( str.isEmpty() ) {
//                 out.append( *it );
//             } else {
//                 out.append( str );
//                 str = *it;
//             }
//         } else if ( str.length() == 0 ) {
//             str = *it;
//         } else {
//             str += " " + *it;
//         }
//         it++;
//     }
// 
//     if ( !str.isEmpty() )
//         out.append( str );
// 
//     return out.join("<br>");
}

/*  This one is called after Qt::escape, so if the email address is of type<some@rtg> we
    have to remove the safe characters at the beginning and end.  It's not a foolproof method, but
    it should handle nearly all cases.  To make it foolproof add methods to determine legal/illegal
    characters in the url/email addresses.
*/
QString ReadMail::encodeUrlAndMail(QString txt)
{
    QString emailAddress, url;
    QString str(txt);
    const QString validChars = QString(".!#$%'*+-/=?^_`{|}~");

    // Find and encode email addresses
    int pos = 0;
    while ( ( pos = str.indexOf('@', pos) ) != -1 ) {
        int beg = pos - 1;
        while ( beg >= 0 &&
                ( str[beg].isLetterOrNumber() || (validChars.indexOf( str[beg] ) != -1 )))
            beg--;
        if (beg < 0)
            beg = 0;
        if ( !str[beg].isLetterOrNumber() && (validChars.indexOf( str[beg] ) == -1 ))
            beg++;

        int endPos = pos + 1;
        if (endPos >= (int)str.length())
            endPos = pos;
        while ( endPos < (int)str.length() &&
                ( str[endPos].isLetterOrNumber() || (validChars.indexOf( str[endPos] ) != -1 )))
            endPos++;
        if ( !str[endPos].isLetterOrNumber() && (validChars.indexOf( str[endPos] ) == -1 ))
            endPos--;

        emailAddress = str.mid(beg, endPos - beg + 1);
        if ( emailAddress.startsWith("&lt;") ) {
            emailAddress = emailAddress.right( emailAddress.length() - 4);
            beg += 4;

            if ( emailAddress.indexOf("&gt;", -4) != -1 ) {
                emailAddress = emailAddress.left( emailAddress.length() - 4 );
                endPos -= 4;
            }
        }

        if ( emailAddress.lastIndexOf('.', -1) > endPos - pos ) {       //Scan for . after @ to verify that it is an email address

            QString s = refMailTo(emailAddress);
            str.replace(beg, endPos - beg + 1, s);

            pos = beg + s.length();
        } else {
            pos = endPos + 1;
        }
    }

    // Find and encode http addresses
    pos = 0;
    const QString httpStr = "http://";
    const QString wwwStr = "www.";

    while ( ( ( str.indexOf(httpStr, pos) ) != -1 ) ||
            ( ( str.indexOf(wwwStr, pos) ) != -1 ) ) {
        int httpPos = str.indexOf(httpStr, pos);
        int wwwPos = str.indexOf(wwwStr, pos);
        int endPos = 0;
        QString urlPrefix;

        if ( (httpPos != -1) && ((wwwPos == -1) || (httpPos < wwwPos)) ) {
            pos = httpPos;
            endPos = pos + httpStr.length();
        } else {
            pos = wwwPos;
            endPos = pos + wwwStr.length();
            urlPrefix = "http://";
        }

        while ( endPos < (int)str.length() &&
                ( str[endPos].isLetterOrNumber() || (validChars.indexOf( str[endPos] ) != -1 )))
            endPos++;
        if (endPos >= (int)str.length() ||
            (!str[endPos].isLetterOrNumber() && (validChars.indexOf( str[endPos] ) == -1 )))
            endPos--;

        url = str.mid(pos, endPos - pos + 1);

        if ( url.indexOf('.') > -1 ) {  //Scan for . after // to verify that it is an url (weak, I know)
            QString s = "<a href=\"" + urlPrefix + url + "\"> " + url + " </a>";
            str.replace(pos, endPos - pos + 1, s);

            pos += s.length();
        } else {
            pos = endPos + 1;
        }
    }

    return str;
}

QString ReadMail::listRefMailTo(QStringList list)
{
    QString str;
    for (QStringList::Iterator it = list.begin(); it != list.end(); ++it)
    {
        if ( str.isEmpty() ) {
            str += refMailTo( *it );
        } else {
            str += ", " + refMailTo( *it );
        }
    }

    return str;
}

QString ReadMail::refMailTo(QString adr)
{
    int type = adr.indexOf("/TYPE=");
    if (type > 0)
        adr.truncate(type);

    if (!mContactModel)
        mContactModel = new QContactModel( this );

    QString name = Qt::escape( mail->displayName( mContactModel, adr ) );
    return "<a href=\"mailto:" + Qt::escape(adr) + "\"> " + name + " </a>";
}

//update view with current EmailListItem (item)
void ReadMail::update(MailListView *view)
{
    mailView = view;
    EmailListItem *current = (EmailListItem *) view->currentItem();

    if ( !current || !view->isItemSelected( current ) ) {
        close();
        return;
    }

    mail = current->mail();
    lastMailUuid = mail->uuid();
    QString mailbox = mailView->currentMailbox();

#ifdef QTOPIA_PHONE
    context->clear();

    if ( hasGet(mailbox) )
        context->addAction( getThisMailButton );
    else if ( hasSend(mailbox) )
        context->addAction( sendThisMailButton );

    if ( hasReply(mailbox) ) {
        context->addAction( replyButton );
        context->addAction( replyAllAction );
        context->addAction( forwardAction );
    }

    if ( hasEdit(mailbox) )
        context->addAction( modifyButton );

    context->addAction( deleteButton );
    context->addAction( plainTextButton );
    context->addAction( printButton );

    addDialActions();

    //what about next/prev/attachments/status?
#else
#ifdef QTOPIA4_TODO
    bar->clear();

    if ( hasGet(mailbox) )
        getThisMailButton->addTo(bar);

    if ( hasSend(mailbox) )
        sendThisMailButton->addTo(bar);

    if ( hasReply(mailbox) )
        replyButton->addTo(bar);

    if ( hasEdit(mailbox) )
        modifyButton->addTo(bar);

    previousButton->addTo(bar);
    nextButton->addTo(bar);
    deleteButton->addTo(bar);

    buildMenu(mailbox);
#endif
#endif

    updateView();
    updateButtons();
}

void ReadMail::buildMenu(const QString &mailbox)
{
#ifdef QTOPIA_PHONE
    QString unused = mailbox;
#else
#ifdef QTOPIA4_TODO
    mailMenu->clear();
    statusMenu->clear();

    if ( hasGet(mailbox) ) {
        getThisMailButton->addTo( mailMenu );
        mailMenu->insertSeparator();
    }

    if ( hasSend(mailbox) ) {
        sendThisMailButton->addTo( mailMenu );
        mailMenu->insertSeparator();
    }

    if ( hasReply(mailbox) ) {
        replyButton->addTo( mailMenu );
        replyAllAction->addTo( mailMenu );
        forwardAction->addTo( mailMenu );
    }

    if ( hasEdit(mailbox) ) {
        modifyButton->addTo( mailMenu );
    }

    mailMenu->insertSeparator();
    mailMenu->insertItem(tr("Set status"), statusMenu);
    mailMenu->insertSeparator();
    deleteButton->addTo(mailMenu);

    statusMenu->insertItem(tr("Unread"), this, SLOT( setStatus(int) ), 0, 1);
    statusMenu->insertItem(tr("Replied"), this, SLOT( setStatus(int) ), 0, 2);
    statusMenu->insertItem(tr("Forwarded"), this, SLOT( setStatus(int) ), 0, 3);

    statusMenu->insertItem(tr("Sent"), this, SLOT( setStatus(int) ), 0, 4);
    statusMenu->insertItem(tr("Unsent"), this, SLOT( setStatus(int) ), 0, 5);
#endif
#endif
}

void ReadMail::mailUpdated(Email *mailIn)
{
    if ( lastMailUuid == mailIn->uuid() ) {
        mail = mailIn;
        updateView();
        updateButtons();
    } else {
        updateButtons();
    }
}

void ReadMail::showMail(Email *mailIn)
{
    // mailIn must be an SMS currently
    mail = mailIn;
    lastMailUuid = mailIn->uuid();
    updateView();
    
    // Precomputed button visibility settings
    // Only known to be correct for SMS messages
    getThisMailButton->setVisible(false);
    sendThisMailButton->setVisible(false);
    modifyButton->setVisible(false);
    attachmentsButton->setVisible( mail->messagePartCount() );
    nextButton->setVisible(false); // no mailView exists yet
    previousButton->setVisible(false); // no mailView exists yet
    progressLabel->hide();
    context->clear();
    context->addAction( replyButton );
    context->addAction( replyAllAction );
    context->addAction( forwardAction );
    context->addAction( deleteButton );
    context->addAction( plainTextButton );
    context->addAction( printButton );
    addDialActions();
}

#ifdef QTOPIA_PHONE
bool ReadMail::eventFilter( QObject *obj, QEvent *e )
{
    if (obj == emailView && e->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = (QKeyEvent*)e;
        switch( keyEvent->key() ) {
        case Qt::Key_Left:
            emailView->scrollBy(-100,0);
            keyEvent->accept();
            return true;
            break;
        case Qt::Key_Right:
            emailView->scrollBy(100,0);
            keyEvent->accept();
            return true;
            break;
        default:
            break;
        }
#ifndef QTOPIA_NO_MMS
    } else if (smilView && obj == smilView && e->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = (QKeyEvent*)e;
        switch( keyEvent->key() ) {
            case Qt::Key_Select:
                advanceMmsSlide();
                break;
        }
#endif
    }
    return QMainWindow::eventFilter( obj, e );
}
#endif

void ReadMail::closeEvent( QCloseEvent *e )
{
    //check for read reply flag

#ifdef QTOPIA_PHONE
#ifndef QTOPIA_NO_MMS
    QString mmsType = mail->header("X-Mms-Message-Type");
    QString msgClass = mail->header("X-Mms-Message-Class");
    QString readReply = mail->header("X-Mms-Read-Reply");

    if (mmsType.contains("m-retrieve-conf") && !msgClass.contains("Auto")
        && readReply.contains("Yes") && firstRead) {
        emit readReplyRequested(mail);
        }

    if (smilView && views->currentWidget() == smilView) {
        views->setCurrentWidget(emailView);
        smilView->reset();
        e->ignore();
        return;
    }
#endif
#endif
    cleanup();
    emit cancelView();
}

void ReadMail::cleanup()
{
    emailView->setPlainText("");
}

//gets next item in listview, exits if there is no next
void ReadMail::next()
{
    EmailListItem *item = (EmailListItem *) mailView->currentItem();
    if (item && mailView->isItemSelected(item) &&
        mailView->row( item ) + 1 < mailView->rowCount() )
        item = (EmailListItem* )mailView->item( mailView->row( item ) + 1, 0 );
    else
       item = 0;

    if (item != NULL) {
        mailView->clearSelection();
        mailView->setItemSelected( item, true );
        mailView->setCurrentItem( item );

        bool inbox = mail->status(EFlag_Incoming) > 0;
        mail = item->mail();
        lastMailUuid = mail->uuid();

        if ( inbox == (mail->status(EFlag_Incoming) > 0) ) {
            updateView();
        } else {
            update( mailView );
        }

        updateButtons();
    }
}

//gets previous item in listview, exits if there is no previous
void ReadMail::previous()
{
    EmailListItem *item = (EmailListItem *) mailView->currentItem();
    if (item && mailView->isItemSelected(item) &&
        mailView->row( item ) > 0 )
        item = (EmailListItem* )mailView->item( mailView->row( item ) - 1, 0 );
    else
        item = 0;

    if (item != NULL) {
        mailView->clearSelection();
        mailView->setItemSelected( item, true );
        mailView->setCurrentItem( item );

        bool inbox = mail->status(EFlag_Incoming) > 0;
        mail = item->mail();
        lastMailUuid = mail->uuid();

        if ( inbox == (mail->status(EFlag_Incoming) > 0) ) {
            updateView();
        } else {
            update( mailView );
        }

        updateButtons();
    }
}

//deletes item, tries bringing up next or previous, exits if unsucessful
void ReadMail::deleteItem()
{
    EmailListItem *item = (EmailListItem *) mailView->currentItem();
    emit removeItem(item);
    emailView->setFocus();
}

void ReadMail::updateButtons()
{
    EmailListItem *current = (EmailListItem *) mailView->currentItem();

    if ( !current || !mailView->isItemSelected( current ) ) {
        close();
        return;
    }

    /*  Safety precaution.  The mail might have been moved internally/externally
        away from the mailbox.  Verify that we actually still have access to the
        same mail   */
    mail = current->mail();
    if ( mail->uuid() != lastMailUuid ) {
        update( mailView );
        return;
    }

    QString mailbox = mailView->currentMailbox();

    if ( hasGet(mailbox) ) {
        bool off = mail->status(EFlag_Downloaded) || receiving;
        getThisMailButton->setVisible( !off );
    }

    if ( hasSend(mailbox) ) {
        if ( mail->status(EFlag_Sent) || sending ) {
            sendThisMailButton->setVisible(false);
        } else if ( mail->unfinished() ) {
            sendThisMailButton->setVisible(false);
        } else {
            sendThisMailButton->setVisible(true);
        }
    }

    if ( hasEdit(mailbox) )
        modifyButton->setVisible( !( mail->status(EFlag_Sent) || sending ) );

    attachmentsButton->setVisible( mail->messagePartCount() );

    nextButton->setVisible(mailView->row(current) + 1 < mailView->rowCount());
    previousButton->setVisible(mailView->row(current) > 0);

    if ( current )
        current->updateState();

    if (sending || receiving)
        progressLabel->show();
    else
        progressLabel->hide();
}

void ReadMail::shiftText()
{
    plainTxt = !plainTxt;

    if ( plainTxt ) {
        //reset fonts etc to defaults for text view
        emailView->setDocument(new QTextDocument(emailView));
    } else {
        QFont font;
        emailView->setFont( font );
    }

    updateView();
}

void ReadMail::print()
{
    QtopiaServiceRequest srv2( "Print", "printHtml(QString)" );
    srv2 << emailView->toHtml();
    srv2.send();
}

void ReadMail::viewAttachments()
{
    ViewAtt dlg(mail, mail->status(EFlag_Incoming));
#ifdef QTOPIA_DESKTOP
    dlg.exec();
#else
    QtopiaApplication::execDialog(&dlg);
#endif
}

void ReadMail::viewMms()
{
#ifdef QTOPIA_PHONE
#ifndef QTOPIA_NO_MMS
    QString start = "<presentation-part>";
    QString content = mail->header("Content-Type");
    QStringList params = content.split(';');
    QStringList::Iterator it;
    for (it = params.begin(); it != params.end(); ++it) {
        int pos = (*it).indexOf("start=");
        if (pos > 0) {
            start = (*it).mid(pos+6);
            if (start[0] == '\"')
                start = start.mid(1);
            if (start[start.length()-1] == '\"')
                start.truncate(start.length()-1);
            break;
        }
    }

    if (!smilView) {
        smilView = new SmilView(views);
        smilView->setGeometry(views->rect()); // will be using rect() to tweak smil layout
        connect(smilView, SIGNAL(transferRequested(SmilDataSource*, const QString&)),
                this, SLOT(requestTransfer(SmilDataSource*, const QString&)));
        connect(smilView, SIGNAL(transferCancelled(SmilDataSource*, const QString&)),
                this, SLOT(cancelTransfer(SmilDataSource*, const QString&)));
        connect(smilView, SIGNAL(finished()), this, SLOT(mmsFinished()));
        views->addWidget(smilView);
        smilView->setFocusPolicy(Qt::StrongFocus);
        smilView->installEventFilter(this);
        QSoftMenuBar::setLabel(smilView, QSoftMenuBar::menuKey(), QSoftMenuBar::NoLabel);
        QSoftMenuBar::setLabel(smilView, Qt::Key_Select, QSoftMenuBar::Next);
    }

    bool ok = false;
    for ( uint i = 0; i < mail->messagePartCount(); i++ ) {
        MailMessagePart &part = mail->messagePartAt( i );
        if (part.contentID() == start) {
            QString smil = part.decodedBody();
            qWarning() << smil;
            smilView->setSource(smil);
            ok = true;
            break;
        }
    }
    if (ok && smilView && smilView->rootElement()) {
        tweakSmil(smilView->rootElement());
        smilView->play();
        views->setCurrentWidget(smilView);
    } else {
        QMessageBox::warning(this, tr("Cannot view MMS"),
            tr("<qt>Cannot play improperly formatted MMS</qt>"), QMessageBox::Ok, QMessageBox::NoButton);
    }
#endif
#endif
}

#ifdef QTOPIA_PHONE
#ifndef QTOPIA_NO_MMS
void ReadMail::tweakSmil(SmilElement *smil)
{
    // Try to make sure the layout works on our display
    SmilElement *layout = smil->findChild(QString(), "layout", true);
    if (!layout)
        return;

    QRect rl = rect();
    SmilElement *rootLayout = layout->findChild(QString(), "root-layout");
    if (rootLayout) {
        if (rootLayout->rect().width() > smilView->width()
            || rootLayout->rect().height() > smilView->height()) {
            rootLayout->setRect(QRect(0, 0, smilView->width(), smilView->height()));
        }
        rl = rootLayout->rect();
    }

    SmilElement *imageLayout = layout->findChild("Image", "region");
    if (!imageLayout)
        imageLayout = layout->findChild("image", "region");

    SmilElement *textLayout = layout->findChild("Text", "region");
    if (!textLayout)
        textLayout = layout->findChild("text", "region");

    if (imageLayout && textLayout) {
        QRect il = imageLayout->rect();
        QRect tl = textLayout->rect();
        if (il.bottom() > tl.top() || il.right() > rl.right()
                || tl.right() > rl.right() || il.bottom() > rl.bottom()
                || tl.bottom() > rl.bottom()) {
            // Not going to fit - use our preferred sizes.
            il = tl = rl;
            il.setBottom(il.top() + rl.height()*2/3);
            tl.setTop(il.bottom()+1);
            tl.setHeight(rl.height() - il.height());
            imageLayout->setRect(il);
            textLayout->setRect(tl);
        }
    }
}

void ReadMail::advanceMmsSlide()
{
    // Try to advance to the next slide
    SmilElement *smil = smilView->rootElement();
    if (!smil)
        return;
    SmilElement *body = smil->findChild(QString(), "body", true);
    if (!body)
        return;
    SmilElementList::ConstIterator it;
    for (it = body->children().begin(); it != body->children().end(); ++it) {
        SmilElement *e = *it;
        if (e->name() == "par") {
            if (e->state() == SmilElement::Active) {
                // This should be the current active slide
                SmilTimingAttribute *at = (SmilTimingAttribute*)e->module("Timing");
                Duration d(at->startTime.elapsed());    // i.e. end now.
                e->setCurrentEnd(d);
                break;
            }
        }
    }
}
#endif
#endif

void ReadMail::mmsFinished()
{
#ifdef QTOPIA_PHONE
#ifndef QTOPIA_NO_MMS
    views->setCurrentWidget(emailView);
#endif
#endif
}

void ReadMail::requestTransfer(SmilDataSource*
#ifdef QTOPIA_PHONE
 dataSource, const QString &src
#else
 , const QString&
#endif
    )
{
#ifdef QTOPIA_PHONE
#ifndef QTOPIA_NO_MMS
    bool isId = false;
    QString source = src;
    if (source.startsWith("cid:")) {
        source = source.mid(4);
        isId = true;
    }
    for ( uint i = 0; i < mail->messagePartCount(); i++ ) {
        MailMessagePart &part = mail->messagePartAt( i );
        if ((isId && part.contentID() == source)
            || part.contentLocation() == source) {
            dataSource->setMimeType(part.contentType());
            QBuffer *data = new QBuffer();
            data->setData(part.decodedBody().toLatin1());
            data->open(QIODevice::ReadOnly);
            dataSource->setDevice(data);
            if (!transfers)
                transfers = new QMap<SmilDataSource*,QIODevice*>;
            (*transfers)[dataSource] = data;
            break;
        }
    }
#endif
#endif
}

void ReadMail::cancelTransfer(SmilDataSource *dataSource, const QString &src)
{
    QString unused = src;
#if defined(QTOPIA_PHONE) && !defined(QTOPIA_NO_MMS)
    if (!transfers)
        return;
    if (transfers->contains(dataSource))
        transfers->take(dataSource)->deleteLater();
#else
    Q_UNUSED(dataSource)
#endif
}


void ReadMail::reply()
{
    emit resendRequested(*mail, 1);
}

void ReadMail::replyAll()
{
    emit resendRequested(*mail, 2);
}

/*  Need to install the files first in order for them to be available
    as a mimecoded attachment.  This does however make all the attached files
    appear in thew Documents tab, which may or may not be logical */
void ReadMail::forward()
{
//    MailMessagePart part;
//    for ( uint i = 0; i < mail->messagePartCount(); i++ ) {
//        part = mail->messagePartAt( i );
//        mail->setAttachmentInstalled(part.name(), true );
//    }

    emit resendRequested(*mail, 3);
}

void ReadMail::setStatus(int id)
{
    uint prevStatus = mail->allStatusFields();

    switch( id ) {
        case 1:
        {
            mail->setStatus(EFlag_Replied | EFlag_RepliedAll | EFlag_Forwarded, false);
            mail->setStatus(EFlag_Read, false);
            break;
        }
        case 2:
        {
            mail->setStatus(EFlag_Replied | EFlag_RepliedAll | EFlag_Forwarded, false);
            mail->setStatus(EFlag_Replied, true );
            break;
        }
        case 3:
        {
            mail->setStatus(EFlag_Replied | EFlag_RepliedAll, false);
            mail->setStatus(EFlag_Forwarded, true);
            break;
        }
        case 4: mail->setStatus(EFlag_Sent, true ); break;
        case 5: mail->setStatus(EFlag_Sent, false ); break;
    }

    if ( mail->allStatusFields() != prevStatus)
        mail->setDirty( true );

    updateButtons();
}

void ReadMail::modify()
{
    emit modifyRequested(mail);
}

void ReadMail::getThisMail()
{
    emit getMailRequested(mail);
}

void ReadMail::sendThisMail()
{
    emit sendMailRequested(mail);
}

void ReadMail::isSending(bool on)
{
    sending = on;
    if ( isVisible() )
        updateButtons();
}

void ReadMail::isReceiving(bool on)
{
    receiving = on;
    if ( isVisible() )
        updateButtons();
}

void ReadMail::setProgressText(const QString &txt)
{
    progressLabel->setText(txt);
    if (txt.isNull())
        progressLabel->hide();
}

void ReadMail::initImages()
{
    if (initialized)
        return;

    initialized = true;
    // Add the predefined smiley images for EMS messages.
    emailView->setResource( QUrl( "x-sms-predefined:ironic" ),
                            QVariant( QImage( ":image/smiley/ironic" ) ) );
    emailView->setResource( QUrl( "x-sms-predefined:glad" ),
                            QVariant( QImage( ":image/smiley/glad" ) ) );
    emailView->setResource( QUrl( "x-sms-predefined:skeptical" ),
                            QVariant( QImage( ":image/smiley/skeptical" ) ) );
    emailView->setResource( QUrl( "x-sms-predefined:sad" ),
                            QVariant( QImage( ":image/smiley/sad" ) ) );
    emailView->setResource( QUrl( "x-sms-predefined:wow" ),
                            QVariant( QImage( ":image/smiley/wow" ) ) );
    emailView->setResource( QUrl( "x-sms-predefined:crying" ),
                            QVariant( QImage( ":image/smiley/crying" ) ) );
    emailView->setResource( QUrl( "x-sms-predefined:winking" ),
                            QVariant( QImage( ":image/smiley/winking" ) ) );
    emailView->setResource( QUrl( "x-sms-predefined:laughing" ),
                            QVariant( QImage( ":image/smiley/laughing" ) ) );
    emailView->setResource( QUrl( "x-sms-predefined:indifferent" ),
                            QVariant( QImage( ":image/smiley/indifferent" ) ) );
    emailView->setResource( QUrl( "x-sms-predefined:kissing" ),
                            QVariant( QImage( ":image/smiley/kissing" ) ) );
    emailView->setResource( QUrl( "x-sms-predefined:confused" ),
                            QVariant( QImage( ":image/smiley/confused" ) ) );
    emailView->setResource( QUrl( "x-sms-predefined:tongue" ),
                            QVariant( QImage( ":image/smiley/tongue" ) ) );
    emailView->setResource( QUrl( "x-sms-predefined:angry" ),
                            QVariant( QImage( ":image/smiley/angry" ) ) );
    emailView->setResource( QUrl( "x-sms-predefined:glasses" ),
                            QVariant( QImage( ":image/smiley/glasses" ) ) );
    emailView->setResource( QUrl( "x-sms-predefined:devil" ),
                            QVariant( QImage( ":image/smiley/devil" ) ) );
}

bool ReadMail::hasGet(const QString &mailbox)
{
    return mailbox.startsWith("inbox");
}

bool ReadMail::hasSend(const QString &mailbox)
{
    return mailbox.startsWith("outbox");
}

bool ReadMail::hasEdit(const QString &mailbox)
{
    return mailbox.startsWith("outbox") || mailbox.startsWith("drafts"); // No tr
}

bool ReadMail::hasReply(const QString &mailbox)
{
    return !mailbox.startsWith("outbox");
}

void ReadMail::addDialActions()
{
#ifdef QTOPIA_PHONE
    // Delete the actions that were left over from last time.
    qDeleteAll(dialActions.begin(), dialActions.end());
    dialActions.clear();

    // If the message is SMS, then find all numbers enclosed in
    // quotes and add them as "Dial nnn" menu items.
    if (mail->status(EFlag_TypeSms)) {
        mail->readFromFile();
        QString body = mail->plainTextBody();
        QStringList split = body.split( QChar('"') );
        QRegExp numberRegex( "\\+?[0-9*#]+" );
        for ( int posn = 1; posn < split.size(); posn += 2 ) {
            QString num = split[posn];
            if ( numberRegex.exactMatch( num ) ) {
                QAction *action = new QAction
                    ( tr("Dial %1", "%1=number").arg(num), this );
                action->setData( QVariant(num) );
                connect( action, SIGNAL(triggered()),
                         this, SLOT(dialEmbeddedNumber()) );
                dialActions.append( action );
            }
        }
    }

    // Add the new actions to the context menu.
    foreach ( QAction *action, dialActions )
        context->addAction(action);
#endif
}

void ReadMail::dialEmbeddedNumber()
{
    QAction *action = qobject_cast<QAction *>( sender() );
    if ( action ) {
        QString number = action->data().toString();
        if ( !number.isEmpty() ) {
            QtopiaServiceRequest req( "Dialer", "showDialer(QString)" );
            req << number;
            req.send();
        }
    }
}
