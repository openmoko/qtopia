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

#include "ablabel.h"

#include <QDL>
#include <QDLBrowserClient>
#include <qtopia/pim/qcontactmodel.h>
#include <qtopianamespace.h>
#include <qtopialog.h>
#include <qtimestring.h>
#include <qcategorymanager.h>
#include <qthumbnail.h>
#include <qtopiaipcenvelope.h>

#ifdef QTOPIA_PHONE
# include <qsoftmenubar.h>
#endif

#include <QApplication>
#include <QDateTime>
#include <QRegExp>
#include <QTextDocument>
#include <QTextDocumentFragment>
#include <QTextCursor>
#include <QTextTable>
#include <QTextTableCell>
#include <QTextCharFormat>
#include <QTextBlockFormat>
#include <QFont>
#include <QScrollBar>
#include <QKeyEvent>
#include <QDebug>
#include <QDesktopWidget>
#include <QMouseEvent>
#include <QImageReader>


enum {ContactLinkType = QTextCharFormat::UserProperty, ContactPhoneNumber, ContactPhoneType};

/* Helper functions - not universally useful */
static void addTextBreak( QTextCursor &curs)
{
    curs.insertBlock();
    curs.movePosition(QTextCursor::NextBlock);
}

static void addTextLine( QTextCursor& curs, const QString& text, const QTextCharFormat& cf,
        const QTextBlockFormat &bf, const QTextCharFormat& bcf)
{
    if (! text.isEmpty() ) {
        curs.insertBlock(bf, bcf);
        curs.insertText(text, cf);
        curs.movePosition(QTextCursor::NextBlock);
    }
}

static void addImageAndTextLine ( QTextCursor& curs, const QTextImageFormat& imf,
        const QString& text, const QTextCharFormat& cf, const QTextBlockFormat& bf,
        const QTextCharFormat& bcf)
{
    if (! text.isEmpty() ) {
        curs.insertBlock(bf, bcf);
        curs.insertImage(imf);
        curs.insertText(text, cf);
        curs.movePosition(QTextCursor::NextBlock);
    }
}

static void addTextNameValue( QTextCursor& curs, const QString& name,
        const QTextCharFormat &ncf, const QString& value, const QTextCharFormat &vcf,
        const QTextBlockFormat& bf, const QTextCharFormat& bcf)
{
    if (! value.isEmpty() ) {
        curs.insertBlock(bf, bcf);
        curs.insertText(name, ncf);
        curs.insertText(value, vcf);
        curs.movePosition(QTextCursor::NextBlock);
    }
}

/* AbLabel */
AbLabel::AbLabel( QWidget *parent, const char *name )
  : QDLBrowserClient( parent, "contactnotes" )
{
    setObjectName(name);
    setFrameStyle(NoFrame);
    setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );

#ifdef QTOPIA_PHONE
    QSoftMenuBar::setLabel(this, Qt::Key_Back,
        QSoftMenuBar::Back, QSoftMenuBar::AnyFocus);
    QSoftMenuBar::setLabel(this, Qt::Key_Select,
        QSoftMenuBar::NoLabel, QSoftMenuBar::AnyFocus);
#endif

    connect(this, SIGNAL(highlighted(const QString&)),
            this, SLOT(linkHighlighted(const QString&)));

#ifdef QTOPIA_VOIP
    presence = new QPresence(QString(), this);
    if ( presence->available() ) {
        connect(presence, SIGNAL(monitoredPresence(QString,QPresence::Status)),
                this, SLOT(monitoredPresence(QString,QPresence::Status)) );
    } else {
        // We don't have presence capabilities within the system.
        delete presence;
        presence = 0;
    }
    monitoredVoipIdStatus = QPresence::Unavailable;
#endif
}

AbLabel::~AbLabel()
{
#ifdef QTOPIA_VOIP
    if ( presence && !monitoredVoipId.isEmpty() )
        presence->stopMonitoring( monitoredVoipId );
#endif
}

void AbLabel::linkHighlighted(const QString& link)
{
    mLink = link;

    if (link.isEmpty()) {
        QSoftMenuBar::setLabel(this, Qt::Key_Select,
            QSoftMenuBar::NoLabel, QSoftMenuBar::AnyFocus);
    } else {
        // XXX annoyingly, QTextBrowser doesn't expose the QTextCharFormat
        // of the highlighted link, so we can't use our custom properties.
        if (link.startsWith("dialer:"))
            QSoftMenuBar::setLabel(this, Qt::Key_Select,
                "phone/calls", tr("Dial"), QSoftMenuBar::AnyFocus); 
        else if (link.startsWith("email:"))
            QSoftMenuBar::setLabel(this, Qt::Key_Select,
                "email", tr("Email"), QSoftMenuBar::AnyFocus);
        else if (link.startsWith("sms:"))
            QSoftMenuBar::setLabel(this, Qt::Key_Select,
                "email", tr("Text"), QSoftMenuBar::AnyFocus); // email icon looks better
        else // QDL etc
            QSoftMenuBar::setLabel(this, Qt::Key_Select,
                QSoftMenuBar::Select, QSoftMenuBar::AnyFocus);
    }
}

void AbLabel::setSource(const QUrl & name)
{
    linkClicked(name.toString());
}

QtopiaServiceRequest AbLabel::anchorService(const QString& name) const
{
    if ( name.startsWith( "dialer:" ) ) // No tr
    {
        QString number = name.mid(7);
        if( number.startsWith("phoneType:") )
        {
            number = number.mid(10);
            int f = number.indexOf(":");
            if( f != -1 )
                number = number.mid( f+1 );
            else
                number = QString();
        }

        if ( number.length() > 0  )
        {
            QtopiaServiceRequest req( "Dialer", "dial(QString,QUniqueId)" ); // No tr
            req << number;
            req << ent.uid();
            return req;
        }
    }
    else if( name.startsWith( "email:" ) )
    {
        QString e = name.mid(6);
        if( !e.isEmpty() )
        {
            QtopiaServiceRequest req( "Email", "writeMail(QString,QString)" );
            req << ent.label() << e;
            return req;
        }
    }
    else if( name.startsWith("sms:") )
    {
        QString s = name.mid(4);
        if( !s.isEmpty() )
        {
            QtopiaServiceRequest req( "SMS", "writeSms(QString,QString)" );
            req << ent.label() << s;
            return req;
        }
    }
    return QtopiaServiceRequest();
}

QContact AbLabel::entry() const
{
    return ent;
}

inline static bool dialerPresent()
{
    return !QtopiaService::channel( "Dialer" ).isEmpty();
}

void AbLabel::init( const QContact &entry )
{
    QPalette thisPalette = qApp->palette(this);
    QFont defaultFont = QApplication::font(this);
    cfNormal.setFont(defaultFont);

    cfBold = cfNormal;
    cfItalic = cfNormal;
    cfSmall = cfNormal;
    cfSmallBold = cfNormal;
    cfAnchor = cfNormal;
    cfBoldUnderline = cfNormal;

    cfItalic.setFontItalic(true);

    cfBold.setFontWeight(80);

    cfBoldUnderline.setFontWeight(80);
    cfBoldUnderline.setFontUnderline(true);
    cfBoldUnderline.setUnderlineStyle(QTextCharFormat::SingleUnderline);

    cfSmall.setFontPointSize(cfNormal.fontPointSize() * 0.8);

    cfSmallBold.setFontWeight(80);
    cfSmallBold.setFontPointSize(cfNormal.fontPointSize() * 0.8);

    cfAnchor.setFontUnderline(true);
    cfAnchor.setUnderlineStyle(QTextCharFormat::DashUnderline);
    cfAnchor.setProperty(QTextFormat::IsAnchor, true);
    cfAnchor.setForeground(thisPalette.color(QPalette::Link));

    bfCenter.setAlignment(Qt::AlignHCenter);

    tfNoBorder.setCellPadding(4);
    tfNoBorder.setCellSpacing(0);
    tfNoBorder.setBorder(0);
    tfNoBorder.setAlignment(Qt::AlignHCenter);

    ent = entry;
    mLink = QString();
    setDocument(createContactDocument(ent, dialerPresent()));
    verifyLinks();

    // moves focus to first link in document
    focusNextChild();
}

void AbLabel::linkClicked(const QString& link)
{
    QtopiaServiceRequest req = anchorService(link);

    if ( !req.isNull() ) {
        req.send();
        emit externalLinkActivated();
    }
}

void AbLabel::keyPressEvent( QKeyEvent *e )
{
#ifdef QTOPIA_PHONE
    QKeyEvent *oldEvent = 0;
    switch(e->key())
    {
        case Qt::Key_Back:
            emit backClicked();
            return;
        case Qt::Key_Call:
            if ( mLink.startsWith( "dialer:" ) ) {
                linkClicked( mLink );
                return;
            }
    }

    QTextBrowser::keyPressEvent(e);

    if( oldEvent )
    {
        if( e->isAccepted() )
            oldEvent->accept();
        delete e;
        e = oldEvent;
        return;
    }
#else
    switch( e->key() )
    {
        case Qt::Key_Space:
        case Qt::Key_Return:
        case Qt::Key_Call:
            emit okPressed();
            break;
        case Qt::Key_Left:
        case Qt::Key_Up:
            emit previous();
            break;
        case Qt::Key_Right:
        case Qt::Key_Down:
            emit next();
            break;
        default:
            QTextBrowser::keyPressEvent( e );
    }
#endif
}

void AbLabel::addPhoneFragment( QTextCursor &curs, const QString& img, const QString& num, LinkType link, int phoneType)
{
    if ( ! num.isEmpty() ) {
        QString escnum = num;
        escnum = Qt::escape( escnum.replace( QRegExp(" "), "-" ) );

        QTextCharFormat cfMyAnchor(cfAnchor);
        QTextImageFormat cfMyAnchorImage;

        switch (link)
        {
            case Dialer:
                {
                    QString urlStr = "dialer:";
                    if (phoneType != -1)
                        urlStr += "phoneType:" + QString::number(phoneType) + ":";
                    urlStr += escnum;
                    cfMyAnchor.setProperty(QTextFormat::AnchorHref, urlStr);
                }
                cfMyAnchor.setProperty(ContactLinkType, Dialer);
                cfMyAnchor.setProperty(ContactPhoneNumber, escnum);
                cfMyAnchor.setProperty(ContactPhoneType, phoneType);
                break;

#ifdef QTOPIA_PHONE
            case Messaging:
                cfMyAnchor.setProperty(QTextFormat::AnchorHref, QString("sms:") + escnum);
                cfMyAnchor.setProperty(ContactLinkType, Messaging);
                cfMyAnchor.setProperty(ContactPhoneNumber, escnum);
                break;
#endif
            default:
                cfMyAnchor = cfNormal;
                break;
        }
        cfMyAnchorImage.merge(cfMyAnchor);
        cfMyAnchorImage.setName(img);

        addImageAndTextLine(curs, cfMyAnchorImage, num,
                cfMyAnchor, bfCenter, Qtopia::mousePreferred() ? cfSmall : cfNormal);
    }
}

bool AbLabel::decodeHref(const QString& href, QtopiaServiceRequest* req, QString* pm) const
{
    //  FIXME: The way this works is just stupid.
    //          Shouldn't have to decode HTML to figure out what
    //          could have easily been stored.

    if ( req )
        *req = anchorService(href);
    if ( pm ) {
        bool sms = false;
        QString num, email;
        if ( href.left(7) == "dialer:" ) { // No tr
            num = href.mid(7);
            if( num.startsWith("phoneType:") )
            {
                num = num.mid(10);
                int f = num.indexOf(":");
                if( f != -1 )
                    num.mid( f + 1 );
                else
                    num = QString();

                f = num.indexOf(":");
                if( f != -1 )
                    num = num.mid(f + 1);
            }
        } else if ( href.left(6) == "email:" ) {
            email = href.mid(6);
            if ( email.right(4) == "@sms" ) {
                sms = true;
                num = email.left(email.length()-4);
                email = QString();
            }
        }
        QString f;
        if ( !email.isEmpty() && ent.emailList().contains(email) )
            f = "email";
        else if ( !num.isNull() ) {
            if ( num == ent.businessMobile().replace( QRegExp(" "), "-" ) )
                f = sms ? "businessmessage" : "businessmobile";
            else if ( num == ent.businessPhone().replace( QRegExp(" "), "-" ) )
                f = "businessphone";
            else if ( num == ent.homeMobile().replace( QRegExp(" "), "-" ) )
                f = sms ? "homemessage" : "homemobile";
            else if ( num == ent.homePhone().replace( QRegExp(" "), "-" ) )
                f = "homephone";
        }
        if ( !f.isNull() )
            *pm = f;
    }
    return (!req || !req->isNull()) && (!pm || !pm->isNull());
}

QString AbLabel::encodeHref() const
{
    QString num;
    LinkType link = NoLink;
    int phoneType = -1;
    bool isBus = ent.categories().contains( "Business" );
    if ( isBus ) {
        // Check business details first, then home details
        if ( !ent.businessPhone().isEmpty() ) {
            num = ent.businessPhone();
            link = Dialer;
            phoneType = QContactModel::BusinessPhone;
        } else if ( !ent.businessMobile().isEmpty() ) {
            num = ent.businessMobile();
            link = Dialer;
            phoneType = QContactModel::BusinessMobile;
        } else if ( !ent.homePhone().isEmpty() ) {
            num = ent.homePhone();
            link = Dialer;
            phoneType = QContactModel::HomePhone;
        } else if ( !ent.homeMobile().isEmpty() ) {
            num = ent.homeMobile();
            link = Dialer;
            phoneType = QContactModel::HomeMobile;
        }
    } else {
        // Check home details first, then business details
        if ( !ent.homePhone().isEmpty() ) {
            num = ent.homePhone();
            link = Dialer;
            phoneType = QContactModel::HomePhone;
        } else if ( !ent.homeMobile().isEmpty() ) {
            num = ent.homeMobile();
            link = Dialer;
            phoneType = QContactModel::HomeMobile;
        } else if ( !ent.businessPhone().isEmpty() ) {
            num = ent.businessPhone();
            link = Dialer;
            phoneType = QContactModel::BusinessPhone;
        } else if ( !ent.businessMobile().isEmpty() ) {
            num = ent.businessMobile();
            link = Dialer;
            phoneType = QContactModel::BusinessMobile;
        }
    }

    QString escnum = num;
    escnum = Qt::escape( escnum.replace( QRegExp(" "), "-" ) );

    QString href;
    if ( link == Dialer ) {
        href = "dialer:";
        if ( phoneType != -1 )
            href += "phoneType:" + QString::number( phoneType ) + ":";
        href += escnum;
    }
#ifdef QTOPIA_PHONE
    // sms only available on phone
    else if ( link == Messaging ) {
        href = "sms:" + escnum;
    }
#endif

    return href;
}

void AbLabel::addNameFragment( QTextCursor &curs, const QContact &contact)
{
    QString value;

    //  Name
    if ( !(value = contact.label()).isEmpty() ) {
        curs.insertText(value, cfBold);
    }

    //  Also, name pronunciation
    if ( !(value = contact.firstNamePronunciation()).isEmpty() )
    {
        curs.insertText(" \"" + value, cfItalic);
        if( contact.lastNamePronunciation().isEmpty() )
            curs.insertText("\"", cfItalic);
    }

    if ( !(value = contact.lastNamePronunciation()).isEmpty() )
    {
        curs.insertText(" ");
        if( contact.lastNamePronunciation().isEmpty() )
            curs.insertText("\"", cfItalic);
        curs.insertText(value + "\"", cfItalic);
    }
    curs.insertText(" ", cfNormal);
}

QTextDocument * AbLabel::createContactDocument(const QContact &contact, bool dialer)
{
    QString baseDirStr = Qtopia::applicationFileName( "addressbook", "contactimages/" );
    loadLinks( contact.customField( QDL::CLIENT_DATA_KEY ) );

    QTextDocument *doc = new QTextDocument(this);
    QTextCursor curs(doc);

    QString value;
    QTextBlockFormat bfHVCenter;
    bfHVCenter.setAlignment(Qt::AlignCenter);
    bfHVCenter.setNonBreakableLines(false);

    /* add thumbnail and name */
    curs.setBlockFormat(bfCenter);

    /* check if we need to have a thumbnail */
    QPixmap thumb = contact.thumbnail();
    if( !thumb.isNull() )
    {
        QVariant thumbV = thumb;
        QTextImageFormat img;
        doc->addResource(QTextDocument::ImageResource, QUrl("addressbookdetailthumbnail"), thumbV);
        img.setName("addressbookdetailthumbnail"); // No tr

        QTextTable *tt = curs.insertTable(1, 2, tfNoBorder);

        QTextCursor tCursor = tt->cellAt(0,0).firstCursorPosition();
        tCursor.setBlockFormat(bfHVCenter);
        tCursor.insertImage(img);

        tCursor = tt->cellAt(0, 1).firstCursorPosition();
        tCursor.setBlockFormat(bfHVCenter);
        addNameFragment(tCursor, contact);
    } else {
        addNameFragment(curs, contact);
    }
    curs.movePosition(QTextCursor::NextBlock);

    addTextBreak(curs);

    //  Job (if this is a business contact)
    QCategoryManager c("addressbook", 0);
    QString bcatid  = "Business"; // no tr
    bool isBus = contact.categories().contains( bcatid );

    if( isBus )
    {
        curs.insertBlock(bfCenter);
        value = contact.jobTitle();
        if ( !value.isEmpty() )
            curs.insertText(value + " ", cfSmall);

        value = contact.company();
        if( !value.isEmpty() ) {
            curs.insertText(value, cfSmallBold);
        }
        curs.movePosition(QTextCursor::NextBlock);
    }

    //
    //  Voice and messaging...
    //
    if( isBus )
    {
        addBusinessPhoneFragment( curs, contact, dialer );
        addHomePhoneFragment( curs, contact, dialer );
        addPhoneFragment( curs, ":icon/businessmessage",
            contact.businessMobile(), Messaging, QContactModel::BusinessMobile );

        addPhoneFragment( curs, ":icon/homemessage",
            contact.homeMobile(), Messaging, QContactModel::HomeMobile );
    }
    else
    {
        addHomePhoneFragment( curs, contact, dialer );
        addBusinessPhoneFragment( curs, contact, dialer );

        addPhoneFragment( curs, ":icon/homemessage",
            contact.homeMobile(), Messaging, QContactModel::HomeMobile );
        addPhoneFragment( curs, ":icon/businessmessage",
            contact.businessMobile(), Messaging, QContactModel::BusinessMobile );
    }

    addEmailFragment( curs, contact );

#ifdef QTOPIA_VOIP
    // If we have VoIP presence capabilities within the system,
    // then set up monitoring and convert the URI into a dialer link.
    if ( presence )
        addVoipFragment( curs, contact );
#endif

    // XXX For now, no 'Messaging' support for faxes,
    // XXX but it's completely feasible to add.
    addPhoneFragment(curs, ":icon/businessfax", contact.businessFax(),
        dialer ? Dialer : NoLink, QContactModel::BusinessFax );
    addPhoneFragment( curs,  ":icon/homefax", contact.homeFax(),
        dialer ? Dialer : NoLink, QContactModel::HomeFax );

    if( isBus ) {
        addBusinessFragment( curs, contact, dialer );
        addPersonalFragment( curs, contact, dialer );
    } else {
        addPersonalFragment( curs, contact, dialer );
        addBusinessFragment( curs, contact, dialer );
    }

    value = contact.notes();
    if ( !value.isEmpty() )
    {
        // XXX add a Notes: header
        addTextBreak(curs);
        curs.insertBlock();
        curs.insertHtml(value);
        curs.movePosition(QTextCursor::NextBlock);
    }

    return doc;
}

void AbLabel::addBusinessPhoneFragment( QTextCursor &curs, const QContact &contact, bool dialer )
{
    addPhoneFragment( curs, ":icon/businessphone",
        contact.businessPhone(), dialer ? Dialer:NoLink, QContactModel::BusinessPhone );
    addPhoneFragment( curs, ":icon/businessmobile",
        contact.businessMobile(), dialer ? Dialer:NoLink, QContactModel::BusinessMobile );
    addPhoneFragment( curs, ":icon/businesspager",
        contact.businessPager(), NoLink, QContactModel::BusinessPager );
}

void AbLabel::addHomePhoneFragment( QTextCursor &curs, const QContact &contact, bool dialer )
{
    addPhoneFragment( curs, ":icon/homephone",
        contact.homePhone(), dialer ? Dialer:NoLink, QContactModel::HomePhone );
    addPhoneFragment( curs, ":icon/homemobile",
        contact.homeMobile(), dialer ? Dialer:NoLink, QContactModel::HomeMobile );
}

void AbLabel::addEmailFragment( QTextCursor &curs, const QContact &contact )
{
    QStringList emails = contact.emailList();
    QStringList::Iterator it;
    for( it = emails.begin() ; it != emails.end() ; ++it )
    {
        QString trimmed = (*it).trimmed();
        if(!trimmed.isEmpty())
        {
            QTextCharFormat cfMyAnchor(cfAnchor);
            QTextImageFormat cfMyImageAnchor;

            cfMyAnchor.setProperty(QTextFormat::AnchorHref, QString("email:") + Qt::escape(trimmed));
            cfMyAnchor.setProperty(ContactLinkType, Email);

            cfMyImageAnchor.merge(cfMyAnchor);
            cfMyImageAnchor.setName(":icon/email");

            addImageAndTextLine(curs, cfMyImageAnchor, trimmed,
                    cfMyAnchor, bfCenter, Qtopia::mousePreferred() ? cfNormal : cfSmall);
        }
    }
}

#ifdef QTOPIA_VOIP
void AbLabel::addVoipFragment( QTextCursor &curs, const QContact &contact )
{
    QString voipId = contact.customField("VOIP_ID");

    if ( !voipId.isEmpty() ) {
        if ( presence && monitoredVoipId != voipId ) {
            if ( !monitoredVoipId.isEmpty() ) {
                presence->stopMonitoring( monitoredVoipId );
            }
            monitoredVoipId = voipId;
            presence->startMonitoring( monitoredVoipId );
            monitoredVoipIdStatus = presence->monitoredUriStatus( voipId );
        }

        QTextCharFormat cfMyAnchor(cfAnchor);
        QTextImageFormat cfMyImageAnchor;

        cfMyAnchor.setProperty(QTextFormat::AnchorHref,
                QString("dialer:") + Qt::escape(voipId.simplified()));
        cfMyAnchor.setProperty(ContactLinkType, Dialer);
        cfMyAnchor.setProperty(ContactPhoneNumber, voipId.simplified());

        cfMyImageAnchor.merge(cfMyAnchor);

        if ( monitoredVoipIdStatus == QPresence::Available )
            cfMyImageAnchor.setName(":icon/online");
        else
            cfMyImageAnchor.setName(":icon/offline");

        addImageAndTextLine(curs, cfMyImageAnchor, voipId.simplified(),
                cfMyAnchor, bfCenter, Qtopia::mousePreferred() ? cfNormal : cfSmall);
    }
}
#endif

void AbLabel::addBusinessFragment( QTextCursor &curs, const QContact &contact, bool /* dialer */ )
{
    QString value, value2;
    QString str;

    /* Save our position before we add the header, in case we need to remove it afterwards */
    int posBefore = curs.position();

    /* Add a header */
    curs.insertBlock(bfCenter);
    curs.insertImage(":icon/addressbook/business");
    curs.insertText(qApp->translate( "QtopiaPim", "Business Details"), cfBoldUnderline);
    curs.movePosition(QTextCursor::NextBlock);

    /* and save the new position, so we can tell if we added anything */
    int posAfter = curs.position();

    /* And add stuff */
    addTextLine(curs, contact.jobTitle(), cfNormal, bfCenter, cfNormal);
    addTextLine(curs, contact.department(), cfItalic, bfCenter, cfNormal);

    value = contact.company();
    value2 = contact.companyPronunciation();

    if ( !value.isEmpty() || !value2.isEmpty()) {
        curs.insertBlock(bfCenter);
        if ( !value.isEmpty()) {
            curs.insertText(value, cfBold);
            curs.insertText(" ", cfNormal);
        }
        if ( !value2.isEmpty() )
            curs.insertText("\"" + value2 + "\"", cfItalic);
        curs.movePosition(QTextCursor::NextBlock);
    }

    if ( !contact.businessStreet().isEmpty() || !contact.businessCity().isEmpty() ||
         !contact.businessZip().isEmpty() || !contact.businessCountry().isEmpty() ) {

        // glom the city/state together
        value = contact.businessCity();
        value2 = contact.businessState();
        str = value;
        if ( !value2.isEmpty() && !str.isEmpty())
            str += ", ";
        str += value2;

        addTextBreak(curs);
        addTextLine(curs, qApp->translate( "QtopiaPim",  "Address: " ), cfBold, bfNormal, cfNormal);
        addTextLine(curs, contact.businessStreet(), cfNormal, bfNormal, cfNormal);
        addTextLine(curs, str, cfNormal, bfNormal, cfNormal);
        addTextLine(curs, contact.businessZip(), cfNormal, bfNormal, cfNormal);
        addTextLine(curs, contact.businessCountry(), cfNormal, bfNormal, cfNormal);
        addTextBreak(curs);
    }

    addTextNameValue(curs, qApp->translate( "QtopiaPim","Web Page: "), cfBold,
        contact.businessWebpage(), cfNormal, bfNormal, cfNormal);

    addTextNameValue(curs, qApp->translate( "QtopiaPim","Office: "), cfBold,
        contact.office(), cfNormal, bfNormal, cfNormal);

    addTextNameValue(curs, qApp->translate( "QtopiaPim","Profession: "), cfBold,
        contact.profession(), cfNormal, bfNormal, cfNormal);

    addTextNameValue(curs, qApp->translate( "QtopiaPim","Assistant: "), cfBold,
        contact.assistant(), cfNormal, bfNormal, cfNormal);

    addTextNameValue(curs, qApp->translate( "QtopiaPim","Manager: "), cfBold,
        contact.manager(), cfNormal, bfNormal, cfNormal);

    /* Finally, see if we need to remove our header */
    if (curs.position() == posAfter) {
        curs.setPosition(posBefore, QTextCursor::KeepAnchor);
        curs.removeSelectedText();
    }
}

void AbLabel::addPersonalFragment( QTextCursor& curs, const QContact &contact, bool /* dialer */ )
{
    QString value, value2;
    QString str;

    /* Save our position before we add the header, in case we need to remove it afterwards */
    int posBefore = curs.position();

    /* Add a header */
    curs.insertBlock(bfCenter);
    curs.insertImage(":icon/home");
    curs.insertText(qApp->translate( "QtopiaPim", "Personal Details"), cfBoldUnderline);
    curs.movePosition(QTextCursor::NextBlock);

    /* and save the new position, so we can tell if we added anything */
    int posAfter = curs.position();

    // home address
    if ( !contact.homeStreet().isEmpty() || !contact.homeCity().isEmpty() ||
         !contact.homeZip().isEmpty() || !contact.homeCountry().isEmpty() ) {
        // glom the city/state together
        value = contact.homeCity();
        value2 = contact.homeState();
        str = value;
        if ( !value2.isEmpty() && !str.isEmpty())
            str += ", ";
        str += value2;

        addTextLine(curs, qApp->translate( "QtopiaPim",  "Address: " ), cfBold, bfNormal, cfNormal);
        addTextLine(curs, contact.homeStreet(), cfNormal, bfNormal, cfNormal);
        addTextLine(curs, str, cfNormal, bfNormal, cfNormal);
        addTextLine(curs, contact.homeZip(), cfNormal, bfNormal, cfNormal);
        addTextLine(curs, contact.homeCountry(), cfNormal, bfNormal, cfNormal);
        addTextBreak(curs);
    }

    addTextNameValue(curs, qApp->translate( "QtopiaPim","Web Page: "), cfBold,
            contact.homeWebpage(), cfNormal, bfNormal, cfNormal);

    str = contact.gender();
    if ( !str.isEmpty() && str.toInt() != 0 ) {
        if ( str.toInt() == 1 )
            str = qApp->translate( "QtopiaPim", "Male" );
        else if ( str.toInt() == 2 )
            str = qApp->translate( "QtopiaPim", "Female" );
        else
            str = QString();
        addTextNameValue(curs, qApp->translate( "QtopiaPim","Gender: "), cfBold,
                str, cfNormal, bfNormal, cfNormal);
    }

    addTextNameValue(curs, qApp->translate( "QtopiaPim","Spouse: "), cfBold,
            contact.spouse(), cfNormal, bfNormal, cfNormal);

    addTextNameValue(curs, qApp->translate( "QtopiaPim","Children:") + " ", cfBold,
            contact.children(), cfNormal, bfNormal, cfNormal);

    if ( contact.birthday().isValid() ) {
        addTextNameValue(curs, qApp->translate( "QtopiaPim","Birthday: "), cfBold,
            QTimeString::localYMD( contact.birthday() ), cfNormal, bfNormal, cfNormal);
    }
    if ( contact.anniversary().isValid() ) {
        addTextNameValue(curs, qApp->translate( "QtopiaPim","Anniversary: "), cfBold,
            QTimeString::localYMD( contact.anniversary()), cfNormal, bfNormal, cfNormal);
    }

    /* Finally, see if we need to remove our header */
    if (curs.position() == posAfter) {
        curs.setPosition(posBefore, QTextCursor::KeepAnchor);
        curs.removeSelectedText();
    }
}

#ifdef QTOPIA_VOIP
void AbLabel::monitoredPresence( const QString& uri, QPresence::Status status )
{
    // monitoring information received from the phone server
    qLog(Sip) << "VoIP presence report: " << uri << (int)status;

    if ( uri != monitoredVoipId ) {
        // This is not the URI that we were interested in monitoring.
        // Probably some other application asked to monitor the URI.
        return;
    }
    if ( status == monitoredVoipIdStatus ) {
        // The availability status has not changed, so don't resync.
        return;
    }
    monitoredVoipIdStatus = status;

    // FIXME: The following is inefficient. Ideally we would simply flip the
    // presence icon without rebuilding the enitre contact HTML.

    // reload the current contact
    setDocument(createContactDocument(ent, dialerPresent()));
    // moves focus to first link in document
    focusNextChild();
}
#endif

