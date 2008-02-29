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
#include <QTextCursor>
#include <QTextTable>
#include <QTextTableCell>
#include <QTextCharFormat>
#include <QTextCharFormat>
#include <QFont>
#include <QScrollBar>
#include <QKeyEvent>
#include <QDebug>
#include <QDesktopWidget>
#include <QMouseEvent>
#include <QImageReader>

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
        QSoftMenuBar::Select, QSoftMenuBar::AnyFocus);
#endif

    connect(this, SIGNAL(highlighted(const QString&)),
            this, SLOT(linkSelected(const QString&)));

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

void AbLabel::linkSelected(const QString& link)
{
    mLink = link;
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
    // TODO: Replace the code that generates the display as HTML with
    // code that generates the display as a QTextDocument for speed.

    ent = entry;
    mLink = QString();
    setHtml(contactToRichText(ent, dialerPresent()));
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

typedef enum {NoLink = 0, Dialer, Messaging} LinkType;

static QString phoneToRichText( const QString& pm, const QString& num, LinkType link, int phoneType = -1 )
{
    if( num.isEmpty() )
        return num;
    QString img = "<img src=\"" + pm + "\"> ";
    QString name = Qt::escape(num);
    QString r = img + name;
    QString escnum = num;
    escnum = Qt::escape( escnum.replace( QRegExp(" "), "-" ) );
    if ( link == Dialer )
        r = "<a href=\"dialer:" +
        (phoneType != -1 ? "phoneType:" + QString::number( phoneType ) + ":" : QString()) + escnum + "\">" + r + "</a>";
#ifdef QTOPIA_PHONE
    // sms only available on phone
    else if ( link == Messaging )
        r = "<a href=\"sms:" + escnum + "\">" + r + "</a>";
#endif
    if( !Qtopia::mousePreferred() )
        r = "<small>"+r+"</small>";
    return r + "<br>";
}
static QString phoneToRichText( const QString& pm, const QString& num, bool dialer, int phoneType = -1 )
{
    return phoneToRichText(pm,num,dialer ? Dialer : NoLink, phoneType);
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

QString AbLabel::contactToRichText(const QContact & contact, bool dialer)
{
    QString text;
    QString value, comp, state;
    QString baseDirStr = Qtopia::applicationFileName( "addressbook", "contactimages/" );
    loadLinks( contact.customField( QDL::CLIENT_DATA_KEY ) );

    //
    //  Thumbnail image
    //

    QString thumb = contact.portraitFile();
    QString thumbtxt;
    if( !thumb.isEmpty() )
    {
        thumb = baseDirStr + thumb;
        QFontMetrics fm(font());
        int thh = 3*fm.height();
        QThumbnail thumbnail( thumb );
        QSize dims = thumbnail.actualSize( QSize( thh*3/2, thh ) );
        QString iml = "<img src=\"%1\" width=\"%2\" height=\"%3\">&nbsp;";
        iml = iml.arg(Qt::escape(thumb)).arg(dims.width()).arg(dims.height());
        thumbtxt = iml;
    }

    //
    //  Name
    //

    QString nametxt;
    if ( !(value = contact.label()).isEmpty() )
        nametxt = "<b>" + Qt::escape(value) + "</b>";

    //
    //  Also, name pronunciation
    //

    if ( !(value = contact.firstNamePronunciation()).isEmpty() )
    {
        nametxt += " <i>\"" + Qt::escape(value);
        if( contact.lastNamePronunciation().isEmpty() )
            nametxt += "\"";
        nametxt += "</i>";
    }


    if ( !(value = contact.lastNamePronunciation()).isEmpty() )
    {
        nametxt += " <i>";
        if( contact.firstNamePronunciation().isEmpty() )
            nametxt += "\"";
        nametxt +=  Qt::escape(value) + "\"</i>";
    }

    if ( thumbtxt.isEmpty() ) {
        text += "<center>"+nametxt+"</center>";
    } else {
        text += "<table cellspacing=0 cellpadding=0>";
        text += "<tr><td>"+thumbtxt+"<td>"+nametxt;
        text += "</table>";
    }

    //
    //  Job (if this is a business contact)
    //

    QCategoryManager c("addressbook", 0);
    QString bcatid  = "Business"; // no tr
    bool isBus = contact.categories().contains( bcatid );

    QString job;
    if( isBus )
    {
        QStringList lines;
        QString str;
        str = contact.jobTitle();
        if ( !str.isEmpty() )
            lines += Qt::escape(str);
        str = contact.company();
        if( !str.isEmpty() )
            lines += "<b>" + Qt::escape(str) + "</b>";
        if ( lines.count() ) {
            if ( !nametxt.isEmpty() )
                job += " ";
            job += "<small>"+lines.join(" ")+"</small>";
        }
        if ( !job.isEmpty() )
            job += "<br>";
    }
    text += "<center>"+job+"</center>";

    //
    //  Voice and messaging...
    //

    if( isBus )
    {
        text += busPhoneRichText( contact, dialer );
        text += homePhoneRichText( contact, dialer );
        text += phoneToRichText( ":icon/businessmessage",
            contact.businessMobile(),
            Messaging, QContactModel::BusinessMobile );
        text += phoneToRichText( ":icon/homemessage",
            contact.homeMobile(), Messaging, QContactModel::HomeMobile );
    }
    else
    {
        text += homePhoneRichText( contact, dialer );
        text += busPhoneRichText( contact, dialer );
        text += phoneToRichText( ":icon/homemessage",
            contact.homeMobile(), Messaging, QContactModel::HomeMobile );
        text += phoneToRichText( ":icon/businessmessage",
            contact.businessMobile(), Messaging, QContactModel::BusinessMobile );
    }

    text += emailRichText( contact );

#ifdef QTOPIA_VOIP
    // If we have VoIP presence capabilities within the system,
    // then set up monitoring and convert the URI into a dialer link.
    if ( presence )
        text += voipIdRichText( contact );
#endif

    // XXX For now, no 'Messaging' support for faxes,
    // XXX but it's completely feasible to add.
    text += phoneToRichText( ":icon/businessfax", contact.businessFax(),
        false, QContactModel::BusinessFax );
    text += phoneToRichText( ":icon/homefax", contact.homeFax(),
        false, QContactModel::HomeFax );

    if( isBus )
    {
        text += businessRichText( contact, dialer );
        text += personalRichText( contact, dialer );
    }
    else
    {
        text += personalRichText( contact, dialer );
        text += businessRichText( contact, dialer );
    }

    value = contact.notes();
    if ( !value.isEmpty() )
    {
        text += value;
    }

    /*// photo last
    QString pho = contact.customField( "photofile" );
    if( !pho.isEmpty() )
    {
        pho = baseDirStr + pho;

        //  Cannot use widget size, since it doesn't have the
        //  right geometry at this point.
        int dw = QApplication::desktop()->width()*7/8;
        QSize sh(dw,dw);
        QImageReader reader( pho );
        QSize dims = reader.size() * 2;
        if( dims.width() > sh.width() || dims.height() > sh.height() ) {
            dims.scale( sh, Qt::KeepAspectRatio );
        }
        QString img = "<img width=%1 height=%2 src=\"" // No tr
            + Qt::escape( pho ) + "\">" + "<br>";
        img = img.arg(dims.width()).arg(dims.height());
        text += img;
    }*/

    return text;
}

QString AbLabel::busPhoneRichText( const QContact &contact, bool dialer )
{
    QString text;

    text += phoneToRichText( ":icon/businessphone",
        contact.businessPhone(), dialer, QContactModel::BusinessPhone );
    text += phoneToRichText( ":icon/businessmobile",
        contact.businessMobile(), dialer, QContactModel::BusinessMobile );
    text += phoneToRichText( ":icon/businesspager",
        contact.businessPager(), false, QContactModel::BusinessPager );

    return text;
}

QString AbLabel::homePhoneRichText( const QContact &contact, bool dialer )
{
    QString text;

    text += phoneToRichText( ":icon/homephone",
        contact.homePhone(), dialer, QContactModel::HomePhone );
    text += phoneToRichText( ":icon/homemobile",
        contact.homeMobile(), dialer, QContactModel::HomeMobile );

    return text;
}

QString AbLabel::businessRichText( const QContact &contact, bool /* dialer */ )
{
    QString text;
    QString value, comp, state;
    QString str;
    QString title;

    if ( !(value = contact.jobTitle()).isEmpty() )
        text += Qt::escape(value) + "<br>";

    if ( !(value = contact.department()).isEmpty() ) {
        text += "<i>" + Qt::escape(value) + "</i>";
        text += "<br>";
    }
    comp = contact.company();
    if ( !comp.isEmpty() )
        text += "<b>" + Qt::escape(comp) + "</b> ";
    if ( !(value = contact.companyPronunciation()).isEmpty() )
        text += "<i>\"" + Qt::escape(value) + "\"</i>";
    if( !comp.isEmpty() || !value.isEmpty() )
        text += "<br>";



    if ( !contact.businessStreet().isEmpty() || !contact.businessCity().isEmpty() ||
         !contact.businessZip().isEmpty() || !contact.businessCountry().isEmpty() ) {
        text += "<br><b>" + qApp->translate( "QtopiaPim",  "Address: " ) + "</b><br>";
    }

    if ( !(value = contact.businessStreet()).isEmpty() )
        text += Qt::escape(value) + "<br>";
    state =  contact.businessState();
    if ( !(value = contact.businessCity()).isEmpty() ) {
        text += Qt::escape(value);
        if ( !state.isEmpty() )
            text += ", " + Qt::escape(state);
        text += "<br>";
    } else if ( !state.isEmpty() )
        text += Qt::escape(state) + "<br>";
    if ( !(value = contact.businessZip()).isEmpty() )
        text += Qt::escape(value) + "<br>";
    if ( !(value = contact.businessCountry()).isEmpty() )
        text += Qt::escape(value) + "<br>";

    str = contact.businessWebpage();
    if ( !str.isEmpty() )
        text += "<b>" + qApp->translate( "QtopiaPim","Web Page: ") + "</b>"
                + Qt::escape(str) + "<br>";
    str = contact.office();
    if ( !str.isEmpty() )
        text += "<b>" + qApp->translate( "QtopiaPim","Office: ") + "</b>"
                + Qt::escape(str) + "<br>";
    str = contact.profession();
    if ( !str.isEmpty() )
        text += "<b>" + qApp->translate( "QtopiaPim","Profession: ") + "</b>"
                + Qt::escape(str) + "<br>";
    str = contact.assistant();
    if ( !str.isEmpty() )
        text += "<b>" + qApp->translate( "QtopiaPim","Assistant: ") + "</b>"
                + Qt::escape(str) + "<br>";
    str = contact.manager();
    if ( !str.isEmpty() )
        text += "<b>" + qApp->translate( "QtopiaPim","Manager: ") + "</b>"
                + Qt::escape(str) + "<br>";
    if( !text.isEmpty() )
    {
        QString pp = ":icon/addressbook/business";
        QString title;
        title = "<p align=\"center\"><img src=\"" + pp + "\"><b><u>" + qApp->translate( "QtopiaPim", "Business Details" ) + "</u></b></p>";
        text =  title + "<br>" + text;
    }
    return text;
}

QString AbLabel::personalRichText( const QContact &contact, bool /* dialer */ )
{
    QString text;
    QString value, state;
    QString str;

    // home address
    if ( !contact.homeStreet().isEmpty() || !contact.homeCity().isEmpty() ||
         !contact.homeZip().isEmpty() || !contact.homeCountry().isEmpty() ) {
        text += "<b>" + qApp->translate( "QtopiaPim",  "Address: " ) + "</b>";
        text +=  "<br>";
    }
    if ( !(value = contact.homeStreet()).isEmpty() )
        text += Qt::escape(value) + "<br>";
    state =  contact.homeState();
    if ( !(value = contact.homeCity()).isEmpty() ) {
        text += Qt::escape(value);
        if ( !state.isEmpty() )
            text += ", " + Qt::escape(state);
        text += "<br>";
    } else if (!state.isEmpty())
        text += Qt::escape(state) + "<br>";
    if ( !(value = contact.homeZip()).isEmpty() )
        text += Qt::escape(value) + "<br>";
    if ( !(value = contact.homeCountry()).isEmpty() )
        text += Qt::escape(value) + "<br>";

    str = contact.homeWebpage();
    if ( !str.isEmpty() )
        text += "<b>" + qApp->translate( "QtopiaPim","Web Page: ") + "</b>"
                + Qt::escape(str) + "<br>";


    str = contact.gender();
    if ( !str.isEmpty() && str.toInt() != 0 ) {
        if ( str.toInt() == 1 )
            str = qApp->translate( "QtopiaPim", "Male" );
        else if ( str.toInt() == 2 )
            str = qApp->translate( "QtopiaPim", "Female" );
        text += "<b>" + qApp->translate( "QtopiaPim","Gender: ") + "</b>" + str + "<br>";
    }
    str = contact.spouse();
    if ( !str.isEmpty() )
        text += "<b>" + qApp->translate( "QtopiaPim","Spouse: ") + "</b>"
                + Qt::escape(str) + "<br>";

    str = contact.children();
    if ( !str.isEmpty() )
        text += "<b>" + qApp->translate( "QtopiaPim", "Children:") +QLatin1String(" ")+ "</b>"
                + Qt::escape(str) + "<br>";

    if ( contact.birthday().isValid() ) {
        str = QTimeString::localYMD( contact.birthday() );
        if ( !str.isEmpty() )
            text += "<b>" + qApp->translate( "QtopiaPim","Birthday: ") + "</b>"
                + Qt::escape(str) + "<br>";
    }
    if ( contact.anniversary().isValid() ) {
        str = QTimeString::localYMD( contact.anniversary() );
        if ( !str.isEmpty() )
            text += "<b>" + qApp->translate( "QtopiaPim","Anniversary: ") + "</b>"
                + Qt::escape(str) + "<br>";
    }

    if( !text.isEmpty() )
    {
        QString pp = ":icon/home";
        QString title = "<p align=\"center\"><img src=\"" + pp + "\"><b><u>" +
            qApp->translate( "QtopiaPim", "Personal Details" ) + "</b></u></p>";
        text = title + "<br>" + text;
    }
    return text;
}

QString AbLabel::emailRichText( const QContact &contact )
{
    QString text;
    QStringList emails = contact.emailList();
    QStringList::Iterator it;
    QString e;
    bool notFirst = false;
    for( it = emails.begin() ; it != emails.end() ; ++it )
    {
        QString trimmed = (*it).trimmed();
        if(!trimmed.isEmpty())
        {
            QString smallopen, smallclose;
            if( !Qtopia::mousePreferred() ) {
                smallopen = "<small>";
                smallclose = "</small>";
            }
            notFirst = true;
            text += "<a href=\"email:" + Qt::escape(trimmed) + "\">";
            text += "<img src=\":icon/email\">" + smallopen;
            text += Qt::escape(trimmed);
            text += smallclose + "</a>";
            text += "<br>";
        }
    }
    return text;
}

#ifdef QTOPIA_VOIP
QString AbLabel::voipIdRichText( const QContact &contact )
{
    QString text;
    QString voipId = contact.customField("VOIP_ID");
    QString e;
    QString hrefStr;
    QString imageStr;

    hrefStr = "<a href=\"dialer:";
    QString smallopen, smallclose;
    if ( !voipId.isEmpty() ) {
        if ( !Qtopia::mousePreferred() ) {
            smallopen = "<small>";
            smallclose = "</small>";
        }
        if ( presence && monitoredVoipId != voipId ) {
            if ( !monitoredVoipId.isEmpty() ) {
                presence->stopMonitoring( monitoredVoipId );
            }
            monitoredVoipId = voipId;
            presence->startMonitoring( monitoredVoipId );
            monitoredVoipIdStatus = presence->monitoredUriStatus( voipId );
        }
        if ( monitoredVoipIdStatus == QPresence::Available ) {
            text += hrefStr + Qt::escape((voipId).simplified()) + "\">"
            + "<img src=\":icon/online\">" + smallopen
            + Qt::escape((voipId).simplified())
            + smallclose + "</a>";
            text += "<br>";
        } else {
            text += hrefStr + Qt::escape((voipId).simplified()) + "\">"
            + "<img src=\":icon/offline\">" + smallopen
            + Qt::escape((voipId).simplified())
            + smallclose + "</a>";
            text += "<br>";
        }
    }
    return text;
}

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
    setHtml(contactToRichText(ent, dialerPresent()));
    // moves focus to first link in document
    focusNextChild();
}


#endif

