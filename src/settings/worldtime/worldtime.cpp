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

// Qt4 Headers
#include <QGridLayout>
#include <QTimerEvent>
#include <QComboBox>
#include <QStackedWidget>
#include <QDir>
#include <QFile>
#include <QLabel>
#include <QMessageBox>
#include <QMenu>
#include <QRegExp>
#include <QSettings>
#include <QTextStream>
#include <QToolButton>
#include <QDesktopWidget>
#include <QLineEdit>
#include <QSettings>
#include <QDebug>

// Local includes
#include "worldtime.h"

// Qtopia includes
#include <qtopiaapplication.h>
#include <QWorldmap>

#include <qtimestring.h>
#include <qtimezone.h>
#include <qtimezonewidget.h>
#include <qtopialog.h>
#if defined(Q_WS_QWS) && !defined(QT_NO_COP)
#include <qtopiaipcenvelope.h>
#include <qtopiaipcadaptor.h>
#endif
#ifdef QTOPIA_PHONE
#include <qsoftmenubar.h>
#include <QAction>
#endif

WorldTime::WorldTime( QWidget *parent,
                            Qt::WFlags fl )
    : QWidget( parent, fl ),
      strRealTz(0),
      bAdded(false)
{
#ifdef Q_WS_QWS
    setWindowTitle(tr("World Time"));
#endif
    while (listCities.count())
      { delete listCities.takeLast(); }
    while (listTimes.count())
      { delete listTimes.takeLast(); }

    // checkSize;
    float dpi = QApplication::desktop()->screen()->logicalDpiY();
    float height = qApp->desktop()->height();
    int columns,rows;
    if ( ( height / dpi ) < 3 ) {
        mMode = Minimal;
        columns = 2;
        rows = 3;
    } else {
        if ( qApp->desktop()->width() < qApp->desktop()->height() ) {
            mMode = Tall;
            columns = 2;
            rows = 7;
        } else {
            mMode = Wide;
            columns = 4;
            rows = 4;
        }
    }

    // Don't need a select softkey for touchscreen phones as it does nothing
    if ( Qtopia::mousePreferred() )
        QSoftMenuBar::setLabel( this, Qt::Key_Select, QSoftMenuBar::NoLabel );

    // first, work out current number of columns?
    QGridLayout *gl = new QGridLayout(this);
    frmMap = new QWorldmap(this);
    QSizePolicy sp = frmMap->sizePolicy();

    sp.setHeightForWidth(true);
    frmMap->setSizePolicy(sp);
    gl->addWidget(frmMap, 0, 0, 1, columns, Qt::Alignment(Qt::AlignTop));
    if (mMode == Minimal) {
        // all in widgetstack
        // remember, hboxes and ...
        mStack = new QStackedWidget(this);
        mCombo = new QComboBox(this);
#ifndef QTOPIA_PHONE
        tb = new QToolButton(this);
        tb->setIcon(QIcon(":icon/edit"));
        tb->setCheckable(true);
        connect(tb, SIGNAL(pressed()), this, SLOT(beginNewTz()));
        gl->addWidget(mCombo, 1, 0);
        gl->addWidget(tb, 1, 1);
#else
        gl->addWidget(mCombo, 1, 0, 1, 1);
#endif
        gl->addWidget(mStack, 2, 0, 1, 1, Qt::AlignTop);

        gl->setRowStretch(0, 0);
        gl->setRowStretch(1, 0);
        gl->setRowStretch(2, 1);

        for (int i = 0; i < CITIES; i++) {
            listTimes.append(new CityInfo(mStack));
            mStack->addWidget(listTimes.at(i));
        }

        connect(mCombo, SIGNAL(activated(int)), mStack, SLOT(setCurrentIndex(int)));
        connect(mCombo, SIGNAL(activated(int)), this, SLOT(setZone(int)));
        mStack->setCurrentIndex(0);
    } else {
        int i, j;
        mStack = 0;
        mCombo = 0;
        for (i = 0; i < CITIES; i++) {
            listCities.append(new QPushButton(this));
            listCities.at(i)->setIcon(QIcon(":icon/edit"));
            listCities.at(i)->setCheckable(true);
            connect(listCities.at(i), SIGNAL(clicked()), this, SLOT(beginNewTz()));
            listTimes.append(new CityInfo(this));

            if (mMode == Tall) {
                gl->addWidget(listCities.at(i),i+1, 0, (Qt::Alignment)(i==5 ? Qt::AlignTop : 0));
                gl->addWidget(listTimes.at(i), i+1, 1, (Qt::Alignment)(i==5 ? Qt::AlignTop : 0));
            } else {
                gl->addWidget(listCities.at(i),(i+2)/2, 2*(i%2), (Qt::Alignment)(i >= 4 ? Qt::AlignTop : 0));
                gl->addWidget(listTimes.at(i),(i+2)/2, 2*(i%2)+1, (Qt::Alignment)(i >= 4 ? Qt::AlignTop : 0));
            }
            gl->setRowStretch(0, 0);
            gl->setRowStretch(1, 0);
            gl->setRowStretch(2, 2);
            if (mMode == Tall) {
                gl->setRowStretch(3, 0);
                gl->setRowStretch(4, 0);
                gl->setRowStretch(5, 0);
                gl->setRowStretch(6, 1);
                gl->setColumnStretch(0,1);
                gl->setColumnStretch(1,1);
            } else {
                gl->setRowStretch(3, 1);
            }
        }
        for (i = 0; i < CITIES; i++)
            for (j = 0; j < CITIES; j++)
                if (i!= j)
                    connect(listCities.at(i),
                            SIGNAL(toggled(bool)),
                            listCities.at(j),
                            SLOT(setDisabled(bool)));
    }

#ifdef QTOPIA_PHONE
    QMenu *contextMenu = QSoftMenuBar::menuFor(this);
    if (mMode == Minimal) {
        QAction *a = new QAction(QIcon(":icon/edit"),
                tr("Select City"), this);
        connect(a, SIGNAL(triggered()), this, SLOT(beginNewTz()));
        contextMenu->addAction(a);
    }
#endif

    QSettings config("Trolltech","qpe");
    strRealTz = QTimeZone::current().id();
    // append the labels to their respective lists...

    gl->setSpacing(4);
    gl->setMargin(4);

    bAdded = true;
    readInTimes();
    changed = false;
    QObject::connect( qApp, SIGNAL( clockChanged(bool) ),
                      this, SLOT( showTime() ) );
    // now start the timer so we can update the time quickly every second
    timerEvent( 0 );

    frmMap->setFocus();
    connect( frmMap,
             SIGNAL( newZone( const QTimeZone& ) ),
             this,
             SLOT( slotNewTz( const QTimeZone& ) ) );
    connect( frmMap,
             SIGNAL( selectZoneCanceled() ),
             this,
             SLOT( slotNewTzCancelled() ) );
    connect( frmMap,
             SIGNAL( selecting() ),
             this,
             SLOT( editMode() ) );
}

WorldTime::~WorldTime()
{
    if ( changed ) {
        writeTimezoneChanges();
    }
}

void WorldTime::saveChanges()
{
    if (changed)
        writeTimezoneChanges();

    changed = false;
}

void WorldTime::cancelChanges()
{
    changed = false;
}

void WorldTime::writeTimezoneChanges(void)
{
    QSettings cfg("Trolltech","WorldTime");

    cfg.beginGroup("TimeZones");

    int i;
    bool realTzWritten = false;
    for ( i = 0;  i < CITIES; i++) {
        if ( !strCityTz[i].isNull() ) {
            cfg.setValue("Zone"+QString::number(i), strCityTz[i]);
            if ( strCityTz[i] == strRealTz )
                realTzWritten = true;
        }
    }
    /*if ( realTzWritten )
        cfg.remove("Zone"+QString::number(CITIES));
    else
        cfg.setValue("Zone"+QString::number(CITIES), strRealTz);*/

    cfg.sync(); // ensure that config file is written immediately
#ifndef QTOPIA_DESKTOP
    QtopiaIpcEnvelope ( "QPE/System", "timeZoneListChange()" );
#else
    emit timeZoneListChange();
#endif

    changed = false;
}

void WorldTime::timerEvent( QTimerEvent *)
{
    if ( timerId ){
        killTimer( timerId );
        timerId = 0;
    }
    // change the time again!!
    showTime();
    int ms = 1000 - QTime::currentTime().msec();
    ms += (60-QTime::currentTime().second())*1000;
    timerId = startTimer( ms );
}

void WorldTime::showTime( void )
{
    QDateTime curUtcTime = QTimeZone::utcDateTime();

    for (int i=0; i<CITIES; i++)
        listTimes.at(i)->setUtcTime(curUtcTime);
}

void WorldTime::setZone( const int index )
{
    frmMap->setZone(
        QTimeZone( strCityTz[index].toLatin1() ) );
}

void WorldTime::editMode()
{
    if ( mMode == Minimal ) {
        mCombo->setFocusPolicy( Qt::NoFocus );
    } else {
        for ( int i=0; i<CITIES; i++ ) {
            listCities.at(i)->setFocusPolicy( Qt::NoFocus );
        }
    }
}

void WorldTime::viewMode()
{
    if ( mMode == Minimal ) {
        mCombo->setFocusPolicy( Qt::StrongFocus );
    } else {
        for ( int i=0; i<CITIES; i++ ) {
            listCities.at(i)->setFocusPolicy( Qt::StrongFocus );
        }
    }
}

void WorldTime::beginNewTz()
{
    QString selectedTz;
    if ( mMode == Minimal ) {
        selectedTz = strCityTz[mCombo->currentIndex()].toLatin1();
    } else {
        for ( int i=0; i<CITIES; i++ ) {
            if ( listCities.at(i)->isChecked() )
                selectedTz = strCityTz[i];
        }
    }

    frmMap->setZone( QTimeZone( selectedTz.toLatin1() ) );
    frmMap->setFocus();
    frmMap->selectNewZone();
}

void WorldTime::slotNewTz( const QTimeZone& zone )
{
    // determine what to do based on what button is pressed...
    QTimeZone curZone;
    int i = 0;

    viewMode();

    if ((mMode == Minimal)
#ifndef QTOPIA_PHONE
            &&(tb->isChecked())
#endif
       )
    {
        for (i = 0; i < CITIES; i++) {
            if (strCityTz[i] == zone.id() && i != mCombo->currentIndex()) {
                // city chose is already in the list.  Don't just abort, swap
                // them instead, as that is the most likely desired result from
                // the user.
                strCityTz[i] = strCityTz[mCombo->currentIndex()];
                curZone = QTimeZone( strCityTz[i].toLatin1() );
                mCombo->setItemText(i, curZone.city());
                listTimes.at(i)->setZone(strCityTz[i]);
                break;
            }
        }

        i = mCombo->currentIndex();

        strCityTz[i] = zone.id();
        curZone = QTimeZone( zone.id().toLatin1() );
        mCombo->setItemText(i, curZone.city());
        mCombo->update();
        listTimes.at(i)->setZone( zone.id() );
        changed = true;
#ifndef QTOPIA_PHONE
        tb->setChecked( false );
#endif
    }
    if ( mMode != Minimal) {
        for(i=0;i<CITIES;i++) {
            if ( strCityTz[i] == zone.id() && !listCities.at(i)->isChecked() ) {
                return;
            }
        }
        qLog(Time) << "Change to new zone";
        for(i=0;i<CITIES;i++) {
          if ( listCities.at(i)->isChecked() ) {
              strCityTz[i] = zone.id();
              curZone = QTimeZone( zone.id().toLatin1() );
              listCities.at(i)->setText( curZone.city() );
              listTimes.at(i)->setZone( zone.id() );
              listCities.at(i)->toggle();
              listCities.at(i)->setFocus();
              // we can actually break, since there is only one button
              // that is ever pressed!
              changed = true;
              update();
              break;
          }
        }
    }

#ifndef QTOPIA_DESKTOP
    if (changed)
        writeTimezoneChanges();
#endif

    showTime();
}

void WorldTime::slotNewTzCancelled()
{
    QString currTz;
    viewMode();
    if ( mMode == Minimal ) {
        currTz = strCityTz[mCombo->currentIndex()].toLatin1();
    } else {
        for ( int i=0; i<CITIES; i++ ) {
            if ( listCities.at(i)->isChecked() )
                currTz = strCityTz[i];
        }
    }

    frmMap->setZone( QTimeZone( currTz.toLatin1() ) );
    slotNewTz( QTimeZone( currTz.toLatin1() ) );
}

void WorldTime::readInTimes( void )
{
    QSettings cfg("Trolltech","WorldTime");
    cfg.beginGroup("TimeZones");

    int i;
    QString zn;
    if (mCombo) mCombo->clear();
    zn = cfg.value("Zone0", QString() ).toString();
    if ( zn.isNull() ) {
        // WorldTime.conf doesn't exist, create default one
        cfg.setValue("Zone0", "America/Los_Angeles");
        cfg.setValue("Zone1", "Europe/Oslo");
        cfg.setValue("Zone2", "Asia/Tokyo");
        cfg.setValue("Zone3", "Asia/Hong_Kong");
        cfg.setValue("Zone4", "Australia/Brisbane");
        cfg.setValue("Zone5", "America/New_York");
        cfg.sync();
    }

    for (i = 0; i < CITIES; i++ ) {
        zn = cfg.value("Zone"+QString::number(i), QString()).toString();
        if ( zn.isNull() )
            break;
        QString nm = QTimeZone( zn.toLatin1() ).city();
        strCityTz[i] = zn;
        if (mMode == Minimal)
            mCombo->addItem( nm, i);
        else
            listCities.at(i)->setText(nm);
        listTimes.at(i)->setZone(zn);
    }
}
