/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#define QTOPIA_INTERNAL_TZSELECT_INC_LOCAL

#include "tzselect.h"
#include "resource.h"
#include "global.h"
#include "config.h"
#include <qtoolbutton.h>
#include <qfile.h>
#include <qregexp.h>
#include <stdlib.h>

#ifdef Q_WS_QWS
#include <qcopchannel_qws.h>
#endif

/*!
  \class TimeZoneSelector

  \brief The TimeZoneSelector widget allows users to configure their time zone information.

  \ingroup qtopiaemb
*/

QIconSet qtopia_internal_loadIconSet( const QString &pix );

class TimeZoneSelectorPrivate : public QObject
{
#ifdef QTOPIA_DESKTOP
Q_OBJECT
#endif
public:
    TimeZoneSelectorPrivate() : QObject(0), includeLocal(FALSE) {}
    bool includeLocal;

#ifdef QTOPIA_DESKTOP
    void doEmitConfigureTimeZones();
    TZCombo *cmbTz;
signals:
    void configureTimeZones();

public slots:
    void applyZoneSelection();
#endif

};

#ifdef QTOPIA_DESKTOP
void TimeZoneSelectorPrivate::doEmitConfigureTimeZones() {
    emit configureTimeZones();
}
#endif

TZCombo::TZCombo( QWidget *p, const char* n )
    : QComboBox( p, n )
{
    updateZones();
    // check to see if TZ is set, if it is set the current item to that
    QString tz = getenv("TZ");
    if (parent()->inherits("TimeZoneSelector")) {
	if ( ((TimeZoneSelector *)parent())->localIncluded() ) {
	    // overide to the 'local' type.
	    tz = "None"; // No tr
	}
    }
    if ( !tz.isNull() ) {
	int n = 0,
            index = 0;
        for ( QStringList::Iterator it=identifiers.begin(); 
	      it!=identifiers.end(); ++it) {
	    if ( *it == tz )
		index = n;
	    n++;
	}
	setCurrentItem(index);
    } else {
	setCurrentItem(0);
    }



    // listen on QPE/System
#if defined(Q_WS_QWS)
#if !defined(QT_NO_COP)
    QCopChannel *channel = new QCopChannel( "QPE/System", this );
    connect( channel, SIGNAL(received(const QCString&, const QByteArray&)),
	this, SLOT(handleSystemChannel(const QCString&, const QByteArray&)) );
#endif
#endif


}

TZCombo::~TZCombo()
{
}

void TZCombo::updateZones()
{
    QString cur = currentText();
    clear();
    identifiers.clear();
    int curix=0;
    QString tz = getenv("TZ");
    bool tzFound = FALSE; // found the current timezone.
    Config cfg("WorldTime");
    cfg.setGroup("TimeZones");
    int listIndex = 0;
    if (parent()->inherits("TimeZoneSelector")) {
	if ( ((TimeZoneSelector *)parent())->localIncluded() ) {
	    // overide to the 'local' type.
	    identifiers.append( "None" ); // No tr
	    insertItem( tr("None") );
	    if ( cur == tr("None"))
		curix = 0;
	    listIndex++;
	}
    }
    int cfgIndex = 0;
    while (1) {
	QString zn = cfg.readEntry("Zone"+QString::number(cfgIndex), QString::null);
	if ( zn.isNull() ){
	    break;
	}
	if ( zn == tz )
	    tzFound = TRUE;
	QString nm = cfg.readEntry("ZoneName"+QString::number(cfgIndex));
	identifiers.append(zn);
	insertItem(nm);
	if ( nm == cur )
	    curix = listIndex;
	++cfgIndex;
	++listIndex;
    }
    for (QStringList::Iterator it=extras.begin(); it!=extras.end(); ++it) {
	insertItem(*it);
	identifiers.append(*it);
	if ( *it == cur )
	    curix = listIndex;
	++listIndex;
    }
    if ( !tzFound && !tz.isEmpty()) {
	int i =  tz.find( '/' );
	QString nm = tz.mid( i+1 ).replace(QRegExp("_"), " ");
	identifiers.append(tz);
	insertItem(nm);
	if ( nm == cur )
	    curix = listIndex;
	++listIndex;
    }
    setCurrentItem(curix);
}


void TZCombo::keyPressEvent( QKeyEvent *e )
{
    // ### should popup() in Qt 3.0 (it's virtual there)
//    updateZones();
    QComboBox::keyPressEvent(e);
}

void TZCombo::mousePressEvent(QMouseEvent*e)
{
    // ### should popup() in Qt 3.0 (it's virtual there)
//    updateZones();
    QComboBox::mousePressEvent(e);
}

QString TZCombo::currZone() const
{
    return identifiers[currentItem()];
}

void TZCombo::setCurrZone( const QString& id )
{
    for (int i=0; i< count(); i++) {
	if ( identifiers[i] == id ) {
	    setCurrentItem(i);
	    return;
	}
    }
    insertItem(id);
    setCurrentItem( count() - 1);
    identifiers.append(id);
    extras.append(id);
}



void TZCombo::handleSystemChannel(const QCString&msg, const QByteArray&)
{
    if ( msg == "timeZoneListChange()" ) {
	updateZones();
    }
}

/*!
    Creates a new TimeZoneSelector with parent \a p and name \a n.  The combobox will be
    populated with the available timezones.
*/

TimeZoneSelector::TimeZoneSelector(QWidget* p, const char* n) :
    QHBox(p,n)
{
    d = new TimeZoneSelectorPrivate();

    // build the combobox before we do any updates...
    cmbTz = new TZCombo( this, "timezone combo" );

    cmdTz = new QToolButton( this, "timezone button" ); // No tr
    cmdTz->setIconSet( qtopia_internal_loadIconSet( "citytime_icon" ) );
    cmdTz->setMaximumSize( cmdTz->sizeHint() );
#ifdef QTOPIA_DESKTOP
    d->cmbTz = cmbTz;
#endif

    cmdTz->setFocusPolicy(QWidget::TabFocus);
    setTabOrder(cmbTz, cmdTz);

    // set up a connection to catch a newly selected item and throw our
    // signal
    QObject::connect( cmbTz, SIGNAL( activated( int ) ),
                      this, SLOT( slotTzActive( int ) ) );
    QObject::connect( cmdTz, SIGNAL( clicked() ),
                      this, SLOT( slotExecute() ) );
}

/*!
  Destroys a TimeZoneSelector.
*/

TimeZoneSelector::~TimeZoneSelector()
{
    delete d;
}

void TimeZoneSelector::setLocalIncluded(bool b)
{
    d->includeLocal = b;
    cmbTz->updateZones();
}

bool TimeZoneSelector::localIncluded() const 
{
    return d->includeLocal;
}

/*!
  Returns the currently selected timezone as a string in location format, e.g.
  \code Australia/Brisbane \endcode
*/

QString TimeZoneSelector::currentZone() const
{
    return cmbTz->currZone();
}

/*!
  Sets the current timezone to \a id.
*/

void TimeZoneSelector::setCurrentZone( const QString& id )
{
    cmbTz->setCurrZone( id );
}

/*! \fn void TimeZoneSelector::signalNewTz( const QString& id )
  This signal is emitted when a timezone has been selected by the user. The id
  is a \l QString in location format, eg \code Australia/Brisbane \endcode
*/

void TimeZoneSelector::slotTzActive( int )
{
    emit signalNewTz( cmbTz->currZone() );
}

/*!
  \internal
*/

void TimeZoneSelector::slotExecute( void )
{
#ifdef Q_WS_QWS
    // execute the city time application...
    Global::execute( "worldtime" );
#else
#ifdef QTOPIA_DESKTOP
    d->doEmitConfigureTimeZones();
#endif
#endif
}

#ifdef QTOPIA_DESKTOP
// Called when WorldTime has rewritten its configuration
void TimeZoneSelectorPrivate::applyZoneSelection()
{
   cmbTz->updateZones();
}
#endif

// This should never be called as these string are not translated
QStringList timezoneDefaults( void )
{
    QStringList tzs;
    // load up the list just like the file format (citytime.cpp)
    // BEGIN no tr
    tzs.append( "America/New_York" );
    tzs.append( "New York" );
    tzs.append( "America/Los_Angeles" );
    tzs.append( "Los Angeles" );
    tzs.append( "Australia/Brisbane" );
    tzs.append( "Brisbane" );
    tzs.append( "Europe/Oslo" );
    tzs.append( "Oslo" );
    tzs.append( "Asia/Tokyo" );
    tzs.append( "Tokyo" );
    tzs.append( "Asia/Hong_Kong" );
    tzs.append( "Hong Kong" );
    // END no tr
    return tzs;
}

#ifdef QTOPIA_DESKTOP
#include "tzselect.moc"
#endif
