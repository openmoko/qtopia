/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
** 
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
** 
** A copy of the GNU GPL license version 2 is included in this package as 
** LICENSE.GPL.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
** See the GNU General Public License for more details.
**
** In addition, as a special exception Trolltech gives permission to link
** the code of this program with Qtopia applications copyrighted, developed
** and distributed by Trolltech under the terms of the Qtopia Personal Use
** License Agreement. You must comply with the GNU General Public License
** in all respects for all of the code used other than the applications
** licensed under the Qtopia Personal Use License Agreement. If you modify
** this file, you may extend this exception to your version of the file,
** but you are not obligated to do so. If you do not wish to do so, delete
** this exception statement from your version.
** 
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "ablabel.h"

#ifdef QTOPIA_DATA_LINKING
#include <qtopia/qdl.h>
#endif

#include <qtopia/stringutil.h>
#include <qtopia/timestring.h>
#ifndef QTOPIA_DESKTOP
#include <qtopia/qcopenvelope_qws.h>
#endif
#include <qtopia/resource.h>
#include <qtopia/global.h>
#include <qtopia/categories.h>
#include <qtopia/image.h>

#include <qapplication.h>
#include <qdatetime.h>
#include <qsimplerichtext.h>
#include <qregexp.h>
#include <qstylesheet.h>

AbLabel::AbLabel( QWidget *parent, const char *name )
  : QTextBrowser( parent, name )
{
    setFrameStyle(NoFrame);
    setHScrollBarMode( AlwaysOff );
    dirty = TRUE;
    mOnlyActivateDialLinks = FALSE;
#ifdef QTOPIA_DATA_LINKING
    mNotesQC = new QDLClient( this, "contactNotes" );
#endif
#ifdef QTOPIA_VOIP
    line = new PhoneLine
    ( QString::null, this, LINE_CREATION_FLAGS_CREATE_ALL_SUPPORTED_LINES );
    connect( line, SIGNAL(notification(PhoneLine::QueryType,const QString &)),
         this, SLOT(notification(PhoneLine::QueryType,const QString &)) );
    available = FALSE;
#endif

}

AbLabel::~AbLabel()
{
#ifdef QTOPIA_VOIP
    if ( !monitoredUri.isEmpty() ) {
    line->modify( PhoneLine::VoIPStopMonitoring, monitoredUri );
    }
#endif
}

void AbLabel::setSource( const QString& name )
{
#ifndef QTOPIA_DESKTOP
    ServiceRequest req = anchorService(name);
    if ( !req.isNull() ) {
    req.send();
    emit externalLinkActivated();
    } else
#endif
    if( !mOnlyActivateDialLinks )
    {
#ifdef QTOPIA_DATA_LINKING
    if( name.startsWith("qdl://") )
    QDL::activateLink( name, QDL::clients( this )  );
    else
#endif
    QTextBrowser::setSource( name );
    }
}

ServiceRequest AbLabel::anchorService(const QString& name) const
{
    //TODO : This should probably be in a library. 'QDL::activateLink' - handle activation of links in textbrowser
    if ( name.startsWith( "dialer:" ) ) { // No tr
    QString number = name.mid(7);
    if( number.startsWith("phoneType:") )
    {
        number = number.mid(10);
        int f = number.find(":");
        if( f != -1 )
        number = number.mid( f+1 );
        else
        number = QString::null;
    }

    if ( number.length() > 0  ) {
        ServiceRequest req( "Dialer", "dial(QString,QUuid)" ); // No tr
        req << number;
            req << ent.uid();       
        return req;
    }
    }
    else if( !mOnlyActivateDialLinks )
    {
    if( name.startsWith( "email:" ) )
    {
        QString e = name.mid(6);    
        if( !e.isEmpty() )
        {
        ServiceRequest req( "Email", "writeMail(QString,QString)" );
        req << ent.fileAs() << e;
        return req;
        }
    }
    else if( name.startsWith("sms:") )
    {
        QString s = name.mid(4);
        if( !s.isEmpty() )
        {
        ServiceRequest req( "Email", "writeSms(QString,QString)" );
        req << ent.fileAs() << s;
        return req;
        }
    }
    }
    return ServiceRequest();
}

PimContact AbLabel::entry() const
{
    return ent;
}

void AbLabel::init( const PimContact &entry )
{
    ent = entry;
    dirty = TRUE;
}

static bool dialerPresent()
{
#ifdef QTOPIA_DESKTOP
    return FALSE;
#else
    return ( Service::channel( "Dialer" ) != 0 );
#endif
}

void AbLabel::sync()
{
    if ( !dirty )
    return;
    dirty = FALSE;
    QString text = contactToRichText( ent, dialerPresent() );
    setText( text );
}

void AbLabel::keyPressEvent( QKeyEvent *e )
{
#ifdef QTOPIA_PHONE
    //trick the text browser into activating a call 
    QKeyEvent *oldEvent = 0;
    if( e->key() == Key_Yes || e->key() == Key_Call )
    {
    oldEvent = e;
    e = new QKeyEvent( QEvent::KeyPress, Key_Select, 0, 0 );
    if( oldEvent->key() == Key_Call )
        mOnlyActivateDialLinks = TRUE;
    }
    QTextBrowser::keyPressEvent(e);
    if( oldEvent )
    {
    if( e->isAccepted() )
        oldEvent->accept();
    delete e;
    e = oldEvent;
    mOnlyActivateDialLinks = FALSE;
    return;
    }
#else
    switch( e->key() ) {
    case Key_Space:
    case Key_Return:
    emit okPressed();
    break;
    case Key_Left:
    case Key_Up:
    emit previous();
    break;
    case Key_Right:
    case Key_Down:
    emit next();
    break;
    default:
    QTextBrowser::keyPressEvent( e );
    }
#endif
}

enum LinkType { NoLink, Dialer, Messaging};
static QString phoneToRichText( const QString& pm, const QString& num, LinkType link, int phoneType = -1 )
{
    if ( num.isEmpty() )
    return num;
    QString img = "<img src=\"" + ("addressbook/"+pm) + "\"> ";
    QString name = Qtopia::escapeString(num);
    QString r = img + name;
    QString escnum = num;
    escnum = Qtopia::escapeString( escnum.replace( QRegExp(" "), "-" ) );
    if ( link == Dialer )
    r = "<a href=\"dialer:" + 
    (phoneType != -1 ? "phoneType:" + QString::number( phoneType ) + ":" : QString::null) + escnum + "\">" + r + "</a>";
#ifdef QTOPIA_PHONE
    // sms only available on phone
    else if ( link == Messaging )
    r = "<a href=\"sms:" + escnum + "\">" + r + "</a>";
#endif
    if( !Global::mousePreferred() )
    r = "<small>"+r+"</small>";
    return r + "<br>";
}
static QString phoneToRichText( const QString& pm, const QString& num, bool dialer, int phoneType = -1 )
{
    return phoneToRichText(pm,num,dialer ? Dialer : NoLink, phoneType);
}

bool AbLabel::decodeHref(const QString& href, ServiceRequest* req, QString* pm) const
{
    if ( req )
    *req = anchorService(href);
    if ( pm ) {
    bool sms = FALSE;
    QString num, email;
    if ( href.left(7) == "dialer:" ) { // No tr
        num = href.mid(7);
        if( num.startsWith("phoneType:") )
        {
        num = num.mid(10);
        int f = num.find(":");
        if( f != -1 )
            num.mid( f+1 );
        else
            num = QString::null;
        }
    } else if ( href.left(6) == "email:" ) {
        email = href.mid(6);
        if ( email.right(4) == "@sms" ) {
        sms = TRUE;
        num = email.left(email.length()-4);
        email = QString::null;
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

QString AbLabel::contactToRichText( const PimContact & contact, bool dialer )
{
    QString text;
    QString value, comp, state;
    QString baseDirStr = Global::applicationFileName( "addressbook", "contactimages/" );

#ifdef QTOPIA_DATA_LINKING
    mNotesQC->clear();
    QDL::loadLinks( contact.customField( QDL::DATA_KEY ), QDL::clients( this ) );
#endif

    QString thumb = contact.customField( "photofile" );
    QString thumbtxt;
    if( !thumb.isEmpty() )
    {
    thumb = baseDirStr + thumb;
    // Should be using a 'portrait', but that would mean
    // adding 'portrait' intelligence to the MimeSourceFactory.
    QSimpleRichText srt("A<br><small>B<br>C</small>",font());
    srt.setWidth(100);
    int thh = srt.height();
    QSize dims = Image::loadScaledImageSize( thumb, thh*3/2, thh );
    QString iml = "<img src=\"%1\" width=\"%2\" height=\"%3\">";
    iml = iml.arg(Qtopia::escapeString(thumb)).arg(dims.width()).arg(dims.height());
    thumbtxt = iml;
    }

    // name
    QString nametxt;
    if ( !(value = contact.fileAs()).isEmpty() )
    nametxt = "<b>" + Qtopia::escapeString(value) + "</b>";
    // also part of name is how to pronounce it.

    if ( !(value = contact.firstNamePronunciation()).isEmpty() )
    {
    nametxt += " <i>\"" + Qtopia::escapeString(value); 
    if( contact.lastNamePronunciation().isEmpty() )
        nametxt += "\"";
    nametxt += "</i>";
    }

    if ( !(value = contact.lastNamePronunciation()).isEmpty() )
    {
    nametxt += " <i>";
    if( contact.firstNamePronunciation().isEmpty() )
        nametxt += "\"";
    nametxt +=  Qtopia::escapeString(value) + "\"</i>";
    }

    if ( thumbtxt.isEmpty() ) {
    text += "<center>"+nametxt+"</center>";
    } else {
    text += "<table cellspacing=0 cellpadding=0>";
    text += "<tr><td>"+thumbtxt+"<td>"+nametxt;
        text += "</table>";
    }

    // job
    Categories c;
    bool lo = c.load( categoryFileName() );
    int bcatid  = 0;
    if( lo )
    bcatid = c.id( "addressbook", "_Business");
    bool isBus = contact.categories().contains( bcatid );
    
    QString job;
    if( isBus ) {
    QStringList lines;
    QString str;
    str = contact.jobTitle();
    if ( !str.isEmpty() )
        lines += Qtopia::escapeString(str);
    str = contact.company();
    if( !str.isEmpty() )
        lines += "<b>" + Qtopia::escapeString(str) + "</b>";
    if ( lines.count() ) {
        if ( !nametxt.isEmpty() )
        job += " ";
        job += "<small>"+lines.join(" ")+"</small>";
    }
    }
    text += "<center>"+job+"</center>";

    // voice and messaging...
    if( isBus )
    {
    text += busPhoneRichText( contact, dialer );
    text += homePhoneRichText( contact, dialer );
    text += phoneToRichText( "businessmessage", contact.businessMobile(), Messaging, PimContact::BusinessMobile );
    text += phoneToRichText( "homemessage", contact.homeMobile(), Messaging, PimContact::HomeMobile );
    }
    else
    {
    text += homePhoneRichText( contact, dialer );
    text += busPhoneRichText( contact, dialer );
    text += phoneToRichText( "homemessage", contact.homeMobile(), Messaging, PimContact::HomeMobile );
    text += phoneToRichText( "businessmessage", contact.businessMobile(), Messaging, PimContact::BusinessMobile );
    }

    text += emailRichText( contact );

#ifdef QTOPIA_VOIP
    text += voipIdRichText( contact, dialer);
#endif

    // XXX For now, no 'Messaging' support for faxes,
    // XXX but it's completely feasible to add.
    text += phoneToRichText( "businessfax", contact.businessFax(), FALSE, PimContact::BusinessFax );
    text += phoneToRichText( "homefax", contact.homeFax(), FALSE, PimContact::HomeFax );

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

    if ( (value = contact.notes()) ) 
    {
    value =  Qtopia::escapeMultiLineString(value);
#ifdef QTOPIA_DATA_LINKING
    text += QDL::lidsToAnchors( value, mNotesQC );
#endif
    }

    // photo last
    QString pho = contact.customField( "photofile" );
    if( !pho.isEmpty() )
    {
    pho = baseDirStr + pho;

    // ### Cannot use widget size, since it doesn't have the
    // ### right geometry at this point.
    int dw = qApp->desktop()->width()*7/8;
    QSize sh(dw,dw);

    QSize dims = Image::loadScaledImageSize(pho, sh.width(), sh.height(), 2 );
    QString img = "<img width=%1 height=%2 src=\"" // No tr
        + Qtopia::escapeString( pho ) + "\">" + "<br>";
    img = img.arg(dims.width()).arg(dims.height());
    text += img;
    }
    return text;
}

QString AbLabel::busPhoneRichText( const PimContact &contact, bool dialer )
{
    QString text;

    text += phoneToRichText( "businessphone", contact.businessPhone(), dialer, PimContact::BusinessPhone );
    text += phoneToRichText( "businessmobile", contact.businessMobile(), dialer, PimContact::BusinessMobile );
    text += phoneToRichText( "businesspager", contact.businessPager(), FALSE, PimContact::BusinessPager );

    return text;
}

QString AbLabel::homePhoneRichText( const PimContact &contact, bool dialer )
{
    QString text;

    text += phoneToRichText( "homephone", contact.homePhone(), dialer, PimContact::HomePhone );
    text += phoneToRichText( "homemobile", contact.homeMobile(), dialer, PimContact::HomeMobile );

    return text;
}

QString AbLabel::businessRichText( const PimContact &contact, bool /* dialer */ )
{
    QString text;
    QString value, comp, state;
    QString str;
    QString title;

    if ( !(value = contact.jobTitle()).isEmpty() )
    text += Qtopia::escapeString(value) + "<br>";

    if ( !(value = contact.department()).isEmpty() ) {
    text += "<i>" + Qtopia::escapeString(value) + "</i>";
    text += "<br>";
    }
    comp = contact.company();
    if ( !comp.isEmpty() )
    text += "<b>" + Qtopia::escapeString(comp) + "</b> ";
    if ( !(value = contact.companyPronunciation()).isEmpty() )
    text += "<i>\"" + Qtopia::escapeString(value) + "\"</i>";
    if( !comp.isEmpty() || !value.isEmpty() )
    text += "<br>";



    if ( !contact.businessStreet().isEmpty() || !contact.businessCity().isEmpty() ||
     !contact.businessZip().isEmpty() || !contact.businessCountry().isEmpty() ) {
    text += "<br><b>" + qApp->translate( "QtopiaPim",  "Address: " ) + "</b><br>";
    }

    if ( !(value = contact.businessStreet()).isEmpty() )
    text += Qtopia::escapeString(value) + "<br>";
    state =  contact.businessState();
    if ( !(value = contact.businessCity()).isEmpty() ) {
    text += Qtopia::escapeString(value);
    if ( state )
        text += ", " + Qtopia::escapeString(state);
    text += "<br>";
    } else if ( !state.isEmpty() )
    text += Qtopia::escapeString(state) + "<br>";
    if ( !(value = contact.businessZip()).isEmpty() )
    text += Qtopia::escapeString(value) + "<br>";
    if ( !(value = contact.businessCountry()).isEmpty() )
    text += Qtopia::escapeString(value) + "<br>";



    str = contact.businessWebpage();
    if ( !str.isEmpty() )
    text += "<b>" + qApp->translate( "QtopiaPim","Web Page: ") + "</b>"
        + Qtopia::escapeString(str) + "<br>";
    str = contact.office();
    if ( !str.isEmpty() )
    text += "<b>" + qApp->translate( "QtopiaPim","Office: ") + "</b>"
        + Qtopia::escapeString(str) + "<br>";
    str = contact.profession();
    if ( !str.isEmpty() )
    text += "<b>" + qApp->translate( "QtopiaPim","Profession: ") + "</b>"
        + Qtopia::escapeString(str) + "<br>";
    str = contact.assistant();
    if ( !str.isEmpty() )
    text += "<b>" + qApp->translate( "QtopiaPim","Assistant: ") + "</b>"
        + Qtopia::escapeString(str) + "<br>";
    str = contact.manager();
    if ( !str.isEmpty() )
    text += "<b>" + qApp->translate( "QtopiaPim","Manager: ") + "</b>"
        + Qtopia::escapeString(str) + "<br>";
    if( !text.isEmpty() )
    {
    QString pp = Resource::findPixmap( "addressbook/business" );
    QString title;
    title = "<p align=\"center\"><img src=\"" + pp + "\"><b><u>" + qApp->translate( "QtopiaPim", "Business Details" ) + "</u></b></p>";
    text =  title + text;
    }
    return text;
}

QString AbLabel::personalRichText( const PimContact &contact, bool /* dialer */ )
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
    text += Qtopia::escapeString(value) + "<br>";
    state =  contact.homeState();
    if ( !(value = contact.homeCity()).isEmpty() ) {
    text += Qtopia::escapeString(value);
    if ( !state.isEmpty() )
        text += ", " + Qtopia::escapeString(state);
    text += "<br>";
    } else if (!state.isEmpty())
    text += Qtopia::escapeString(state) + "<br>";
    if ( !(value = contact.homeZip()).isEmpty() )
    text += Qtopia::escapeString(value) + "<br>";
    if ( !(value = contact.homeCountry()).isEmpty() )
    text += Qtopia::escapeString(value) + "<br>";

    str = contact.homeWebpage();
    if ( !str.isEmpty() )
    text += "<b>" + qApp->translate( "QtopiaPim","Web Page: ") + "</b>"
        + Qtopia::escapeString(str) + "<br>";


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
        + Qtopia::escapeString(str) + "<br>";
    if ( contact.birthday().isValid() ) {
    str = TimeString::localYMD( contact.birthday() );
    if ( !str.isEmpty() )
        text += "<b>" + qApp->translate( "QtopiaPim","Birthday: ") + "</b>"
        + Qtopia::escapeString(str) + "<br>";
    }
    if ( contact.anniversary().isValid() ) {
    str = TimeString::localYMD( contact.anniversary() );
    if ( !str.isEmpty() )
        text += "<b>" + qApp->translate( "QtopiaPim","Anniversary: ") + "</b>"
        + Qtopia::escapeString(str) + "<br>";
    }

    if( !text.isEmpty() )
    {
    QString pp = Resource::findPixmap( "home" );
    QString title = "<p align=\"center\"><img src=\"" + pp + "\"><b>" + qApp->translate( "QtopiaPim", "Personal Details" ) + "</b></p>";
    text = title + text;
    }
    return text;
}

QString AbLabel::emailRichText( const PimContact &contact )
{
    QString text;
    QStringList emails = contact.emailList();
    QStringList::Iterator it;
    QString e;
    bool notFirst = FALSE;
    for( it = emails.begin() ; it != emails.end() ; ++it )
    {
    QString smallopen, smallclose;
    if( !Global::mousePreferred() ) {
        smallopen = "<small>";
        smallclose = "</small>";
    }
    notFirst = TRUE;
    text += "<a href=\"email:" + Qtopia::escapeString((*it).stripWhiteSpace()) + "\">"
        + "<img src=addressbook/email>" + smallopen + 
        + Qtopia::escapeString((*it).stripWhiteSpace())
        + smallclose + "</a>";
    text += "<br>";
    }
    return text;
}

QString AbLabel::voipIdRichText( const PimContact &contact, bool dialer )
{
#ifdef  QTOPIA_VOIP
  Q_UNUSED(dialer)
  QString text;
  QString voipId = contact.customField("VOIP_ID");
  QString e;
  QString hrefStr;
  QString imageStr;

  hrefStr = "<a href=\"dialer:";
  QString smallopen, smallclose;
  if ( !voipId.isEmpty() )
  {
     if( !Global::mousePreferred() )
     {
            smallopen = "<small>";
            smallclose = "</small>";
     }
     if ( monitoredUri != voipId ) {
    if ( !monitoredUri.isEmpty() ) {
        line->modify( PhoneLine::VoIPStopMonitoring, monitoredUri );
    }
    monitoredUri = voipId;
    line->modify( PhoneLine::VoIPStartMonitoring, monitoredUri );
    available = FALSE;  // We don't know the state yet.
     }
     if( available ) {
            text +=hrefStr + Qtopia::escapeString((voipId).stripWhiteSpace()) + "\">"
            +"<img src=addressbook/online>" + smallopen +
            + Qtopia::escapeString((voipId).stripWhiteSpace())
            + smallclose+ "</a>";
            text += "<br>";
     } else {
            text +=hrefStr + Qtopia::escapeString((voipId).stripWhiteSpace()) + "\">"
            +"<img src=addressbook/offline>" + smallopen +
            + Qtopia::escapeString((voipId).stripWhiteSpace())
            + smallclose+ "</a>";
            text += "<br>";
      }  
   }
    return text;
#else
// FIXME please
//    Q_UNUSED(contact)
    Q_UNUSED(dialer)
    return QString("");
#endif
}

void AbLabel::notification( PhoneLine::QueryType type, const QString &value )
{
#ifdef QTOPIA_VOIP
    if ( type == PhoneLine::VoIPMonitoredPresence ) {
    // Monitoring information received from the phone server.
    qDebug( "VoIP presence report: %s", value.latin1() );
    int index = value.find( QChar(',') );
    if ( index < 0 ) {
        // Value is not in the correct format.
        return;
    }
    QString status = value.left( index );
    QString uri = value.mid( index + 1 );
    if ( uri != monitoredUri ) {
        // This is not the URI that we were interested in monitoring.
        // Probably some other application asked to monitor the URI.
        return;
    }
    bool newAvailable = ( status == "available" );
    if ( newAvailable == available ) {
        // The availability status has not changed, so don't resync.
        return;
    }
    available = newAvailable;

    // Reload the current contact.
    dirty = TRUE;
    sync();
    }
#else
// FIXME please
//    Q_UNUSED(type)
//    Q_UNUSED(value)
#endif
}
