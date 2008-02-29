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

#include "eventview.h"

#include <qtopia/stringutil.h>
#include <qtopia/timestring.h>
#include <qtopia/pim/event.h>

#ifdef QTOPIA_DATA_LINKING
#include <qtopia/qdl.h>
#endif

#include <qglobal.h>

EventView::EventView( QWidget *parent, const char *name )
    : QTextBrowser( parent, name ),
    previousView( 0 ), mReadOnly( FALSE )
{
#ifdef QTOPIA_PHONE
    setFrameStyle(NoFrame);
#endif

#ifdef QTOPIA_DATA_LINKING
    mNotesQC = new QDLClient( this, "editNote" );
#endif
}

void EventView::setSource( const QString &ahref )
{
#ifdef QTOPIA_DATA_LINKING
    QDL::activateLink( ahref, QDL::clients( this ) );
#else
    (void)ahref;
#endif
}

void EventView::init( const Occurrence &o )
{
    QString text = createPreviewText( o );
    if ( text != previousText )
	setText( text );
    previousText = text;
}

void EventView::keyPressEvent( QKeyEvent *e )
{
    switch ( e->key() ) {
	case Key_Space:
	case Key_Return:
	    // PHONE back is handled in DateBook::closeEvent()
	    emit done();
	    break;
	default:
	    QTextBrowser::keyPressEvent( e );
	    break;
    }
}

QString EventView::createPreviewText( const Occurrence &o )
{
    const PimEvent &ev = o.event();

    QString text;

#ifdef QTOPIA_DATA_LINKING
    if ( !mReadOnly ) {
        mNotesQC->clear();
        QDL::loadLinks( ev.customField( QDL::DATA_KEY ), QDL::clients( this ) );
    }
#endif

    bool badgeIcons = FALSE;
    if ( o.event().timeZone().isValid() ||
         o.event().hasAlarm() ||
         o.event().hasRepeat() ||
         o.event().isException() ) {
        badgeIcons = TRUE;
    }

    if ( badgeIcons )
        text += "<table><tr><td>";

    if ( !ev.description().isEmpty() )
        text += Qtopia::escapeString(ev.description());

    if ( badgeIcons )
        text += "</td><td>";

    if ( o.event().hasRepeat() )
        text += "<img src=\"repeat\">";
    else if ( o.event().isException() )
        text += "<img src=\"repeatException\">";
    if ( o.event().hasAlarm() )
        text += "<img src=\"smallalarm\">";
    if ( o.event().timeZone().isValid() )
        text += "<img src=\"globe\">";

    if ( badgeIcons )
        text += "</td></tr></table>";

#ifndef QTOPIA_DESKTOP
    text += "<p>";
#endif

    if ( !ev.location().isEmpty() )
	text += "<b>" + tr("Location: ") + "</b>" + ev.location() + "<br>";
    if ( ev.isAllDay() ) {
	text += "<b>" + tr("Start: ") + "</b>" +
		TimeString::localYMD( o.startInCurrentTZ().date() ) + "<br>";
	text += "<b>" + tr("End: ") + "</b>" +
		TimeString::localYMD( o.endInCurrentTZ().date() ) + "<br>";
    } else {
	text += "<b>" + tr("Start: ") + "</b>" +
		TimeString::localYMD( o.startInCurrentTZ().date() ) + " " +
	        TimeString::localHM( o.startInCurrentTZ().time() ) + "<br>";
	text += "<b>" + tr("End: ") + "</b>" +
		TimeString::localYMD( o.endInCurrentTZ().date() ) + " " +
	        TimeString::localHM( o.endInCurrentTZ().time() ) + "<br>";
    }
    if ( ev.hasAlarm() )
	text += "<b>" + tr("Alarm: ") + "</b>" +
	    QString("%1").arg( ev.alarmDelay() ) + " " + tr("minutes") + " (" +
	    tr(ev.alarmSound() == PimEvent::Silent ? "Silent" : "Audible") +
	    ")" + "<br>";
    if ( ev.hasRepeat() ) {
	QString word;
	if ( ev.repeatType() == PimEvent::Daily )
	    if ( ev.frequency() > 1 )
		word = tr("every %1 days", "eg. every 2 days");
	    else
		word = tr("every day");
	else if ( ev.repeatType() == PimEvent::Weekly )
	    if ( ev.frequency() > 1 )
		word = tr("every %1 weeks", "eg. every 2 weeks");
	    else
		word = tr("every week");
	else if ( ev.repeatType() == PimEvent::MonthlyDate ||
		  ev.repeatType() == PimEvent::MonthlyDay ||
		  ev.repeatType() == PimEvent::MonthlyEndDay )
	    if ( ev.frequency() > 1 )
		word = tr("every %1 months", "eg. every 2 months");
	    else
		word = tr("every month");
	else if ( ev.repeatType() == PimEvent::Yearly )
	    if ( ev.frequency() > 1 )
		word = tr("every %1 years", "eg. every 2 years");
	    else
		word = tr("every year");
	if ( ev.frequency() > 1 )
	    word = word.arg( ev.frequency() );
	text += "<b>" + tr("Repeat: ") + "</b>";
        QString endword;
	if ( ev.repeatForever() )
            endword = tr("forever");
        else
	    endword = tr("ending on %1", "1=date").arg(TimeString::localYMD(ev.repeatTill()));
        text += tr("From %1, %2, %3", "1=date, 2=every x days/weeks/etc, 3=ending on date/forever")
            .arg(TimeString::localYMD(o.event().startInCurrentTZ().date()))
            .arg(word)
            .arg(endword);
	text += "<br>";
    }
    if ( !ev.notes().isEmpty() )
    {
	QString txt = Qtopia::escapeMultiLineString( ev.notes() );
#ifdef QTOPIA_DATA_LINKING
        if ( !mReadOnly )
            txt = QDL::lidsToAnchors( txt, mNotesQC );
#endif
	text += "<b>" + tr("Notes: ") + "</b>" +
	     txt + "<br>";
    }
    return text.mid(0, text.length() - 4); // don't include the last <br>
}

void EventView::readonly()
{
    mReadOnly = TRUE;
    setFrameStyle(NoFrame);
    setPaper( colorGroup().brush( QColorGroup::Background ) );
}

