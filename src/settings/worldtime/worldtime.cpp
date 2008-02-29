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
#include <QPainter>


// Local includes
#include "worldtime.h"

// Qtopia includes
#include <qtopiaapplication.h>
#include <QWorldmap>

#include <qtimestring.h>
#include <qtimezone.h>
#include <qtimezoneselector.h>
#include <qtopialog.h>
#ifndef QTOPIA_HOST
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
   currentCombo = -1;

   QSettings cfg("WorldTime");
   cfg.beginGroup("TimeZones");
   visibleZones = cfg.value("Visible", 1).toInt();
        

//   QFontMetrics fm(font);
//   qLog(Time)<< "font Metric height"<<fm.xHeight();
#ifdef Q_WS_QWS
   setWindowTitle(tr("World Time"));
#endif
   while (listCities.count())
   { delete listCities.takeLast(); }
   while (listTimes.count())
   { delete listTimes.takeLast(); }

   // checkSize;

   if ( qApp->desktop()->width() > qApp->desktop()->height() )
     maxVisibleZones = 2;
   else
     maxVisibleZones = CITIES - 2;


    //   float dpi = QApplication::desktop()->screen()->logicalDpiY();
    //float height = qApp->desktop()->height();

   int columns,rows;
      mMode = Minimal;
      columns = 3;
      rows = 3;


   // Don't need a select softkey for touchscreen phones as it does nothing
   if ( Qtopia::mousePreferred() )
      QSoftMenuBar::setLabel( this, Qt::Key_Select, QSoftMenuBar::NoLabel );

   // first, work out current number of columns?
   gl = new QGridLayout(this);
   frmMap = new QWorldmap(this);
   QSizePolicy sp = frmMap->sizePolicy();

    
    float dpi = QApplication::desktop()->screen()->logicalDpiY();
    float height = qApp->desktop()->height();
  
    if ( qApp->desktop()->width() < qApp->desktop()->height() ) {
            sp.setHeightForWidth(true);
        }
    
   frmMap->setSizePolicy(sp);
   gl->addWidget(frmMap, 0, 0, 0, columns, Qt::Alignment(Qt::AlignTop));

   {
      for(int i=0; i < visibleZones + 1; i++) {
         gl->setRowStretch(i, 1);
      }

      gl->addItem(new QSpacerItem(0,100),
                 QSizePolicy::MinimumExpanding, QSizePolicy::Ignored);

      gl->setColumnStretch(1,4);

      for (int i = 0; i < CITIES; i++) {
         QComboBox *combo;
         combo = new QComboBox(this);
         QString d;
         d.setNum(i,10);
         combo->setObjectName(d);

         listBoxes.append(combo);

         connect(listBoxes.at(i), SIGNAL(activated(int)),
                 this, SLOT(slotComboSetZone(int)));

         listTimes.append(new CityInfo(this));

         if( i < visibleZones ) {
           gl->addWidget( listBoxes.at(i), i + 3, 0,  Qt::Alignment(Qt::AlignBottom));
           gl->addWidget( listTimes.at(i), i + 3, 1, Qt::Alignment(Qt::AlignBottom));
         } else {
            listBoxes.at(i)->hide();
            listTimes.at(i)->hide();
         }
      }

   }// else
   {
      // not minimal mode

   }

#ifdef QTOPIA_PHONE
   QMenu *contextMenu = QSoftMenuBar::menuFor(this);
   if (mMode == Minimal) {
       
       QAction *a = new QAction(QIcon(":icon/edit"),
                               tr("Select City"), this);
      connect(a, SIGNAL(triggered()), this, SLOT(beginNewTz()));
      contextMenu->addAction(a);
      contextMenu->addSeparator();
       
      addClockAction = new QAction(QIcon(":icon/new"),
                      tr("Add clock"), this);
       connect(addClockAction, SIGNAL(triggered()), this, SLOT(addClock()));
       contextMenu->addAction(addClockAction);

      removeClockAction = new QAction(QIcon(":icon/reset"),
                      tr("Remove clock"), this);
       connect(removeClockAction, SIGNAL(triggered()), this, SLOT(removeClock()));
       contextMenu->addAction(removeClockAction);

       checkMenu();
   
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
   QObject::connect( qApp, SIGNAL(clockChanged(bool)),
                     this, SLOT(showTime()));

   // now start the timer so we can update the time quickly every second
   timerEvent( 0 );

   listBoxes.at(0)->setFocus();
    
    if( !Qtopia::mousePreferred()) {
        frmMap->setFocus();
    } else {
        frmMap->setContinuousSelect(true);
    }
    //    this->setFocus();
    
   connect( frmMap, SIGNAL(newZone(QTimeZone) ),
            this, SLOT( slotNewTz(QTimeZone)));
    
    if( Qtopia::mousePreferred()) 
        connect( frmMap, SIGNAL(buttonSelected()),
             this, SLOT(selected()));
    
   connect( frmMap, SIGNAL(selectZoneCanceled()),
            this, SLOT(slotNewTzCancelled()));

   connect( frmMap, SIGNAL(selecting()),
            this, SLOT(editMode()));

}

WorldTime::~WorldTime()
{
   if ( changed ) {
      writeTimezoneChanges();
   }
}

void WorldTime::saveChanges()
{
    if (changed) {
      writeTimezoneChanges();
      viewMode();
      readInTimes();
    }
   changed = false;
}

void WorldTime::cancelChanges()
{
    viewMode();
   changed = false;
}

void WorldTime::writeTimezoneChanges()
{
    qLog(Time)<<"write";
   QSettings cfg("WorldTime");
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

   cfg.sync(); // ensure that config file is written immediately

   listBoxes.at(currentCombo)->setEditable( false);
    this->setFocus();
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


void WorldTime::slotComboSetZone( const int index )
{
    frmMap->setReadOnly(false);
    
   currentComboIndex = index;

   QTimeZone zone( strCityTz[index].toLatin1());
   frmMap->setZone(zone);
    
    if( !Qtopia::mousePreferred()) 
        frmMap->setFocus();
   frmMap->selectNewZone();

   QComboBox *sendBox = qobject_cast<QComboBox *>(sender());
   currentCombo = QString(sendBox->objectName()).toInt();
    //    qLog(Time)<<"currentCombo"<<currentCombo << "currentComboIndex"<<currentComboIndex;

   QSettings cfg("WorldTime");
   cfg.setValue("Clock"+sendBox->objectName(), QString::number(index ));

   listTimes.at( currentCombo )->setZone( zone.id());

}

void WorldTime::editMode()
{
    if( Qtopia::mousePreferred())
        return;
    frmMap->setReadOnly(false);
    if ( mMode == Minimal ) {
      for ( int i=0; i<CITIES; i++ )
         listBoxes.at(i)->setFocusPolicy( Qt::NoFocus );
   } else {
      for ( int i=0; i<CITIES; i++ ) {
         listCities.at(i)->setFocusPolicy( Qt::NoFocus );
      }
   }
    //  frmMap->setFocus();
}

void WorldTime::viewMode()
{
    if( !frmMap->isReadOnly())
        frmMap->setReadOnly(true);
    
   if ( mMode == Minimal ) {
      for ( int i=0; i<CITIES; i++ )
         listBoxes.at(i)->setFocusPolicy( Qt::StrongFocus );
   } else {
      for ( int i=0; i<CITIES; i++ ) {
         listCities.at(i)->setFocusPolicy( Qt::StrongFocus );
      }
   }
    this->setFocus();
}

void WorldTime::beginNewTz()
{
   QString selectedTz;
   if ( mMode == Minimal ) {
       for ( int i=0; i<CITIES; i++ )
           if(listBoxes.at(i)->hasFocus()) {
              currentCombo = i;
           }
   } else {
      for ( int i=0; i<CITIES; i++ ) {
         if ( listCities.at(i)->isChecked() )
              currentCombo = i;
      }
   }

   if(currentCombo == -1) currentCombo = 0;
   currentComboIndex = getCurrentComboIndex(currentCombo);

   selectedTz = strCityTz[ listBoxes.at(currentCombo)->currentIndex()].toLatin1();

   frmMap->setZone( QTimeZone( selectedTz.toLatin1() ) );
    if( !Qtopia::mousePreferred()) 
        frmMap->setFocus();
   frmMap->selectNewZone();

}

void WorldTime::beginNewTz(int combo)
{
    Q_UNUSED(combo);
//   qLog(Time)<<"beginNewTz "<<combo;
}

void WorldTime::slotNewTz( const QTimeZone& zone )
{
     if( Qtopia::mousePreferred()) {
        if( frmMap->isReadOnly())
            frmMap->setReadOnly(false);
        this->setFocus();
        return;
    }
    

   QTimeZone curZone;
   int i = 0;

    // qLog(Time)<< "slotNewTz index:"<<  currentComboIndex <<" combo: "<< currentCombo <<"zone.id"<<zone.id();
    if(currentCombo == -1) {
//         if( Qtopia::mousePreferred())
//             if( frmMap->isReadOnly())
//                 frmMap->setReadOnly(false);
        return;
    }
    
   if(strCityTz[currentComboIndex] == zone.id()) { 
       changed = false;
       viewMode();
      return; 
   }

   if ((mMode == Minimal) ) {
      for (i = 0; i < CITIES; i++) {
        if ( i == currentCombo) {
            strCityTz[currentComboIndex] = zone.id();
            listBoxes.at(i)->setEditable( true );
            listBoxes.at(i)->setItemText( currentComboIndex, zone.city());
            listBoxes.at(i)->lineEdit()->setText(zone.city());
            listBoxes.at(i)->lineEdit()->setCursorPosition(0);
            listTimes.at(i)->setZone( zone.id());
        }
      }

      changed = true;
#ifndef QTOPIA_PHONE
      tb->setChecked( false );
#endif
   }

   if ( mMode != Minimal) {
      //not minimal
   }
viewMode();
#ifndef QTOPIA_DESKTOP
#endif
}

void WorldTime::slotNewTzCancelled()
{
   QString currTz;
   viewMode();
   if (-1 == currentCombo) return;
   currTz = strCityTz[listBoxes.at(currentCombo)->currentIndex()].toLatin1();

   frmMap->setZone( QTimeZone( currTz.toLatin1() ) );
   slotNewTz( QTimeZone( currTz.toLatin1() ) );
   currentCombo = -1;
   changed = false;
   readInTimes();
}

void WorldTime::readInTimes( void )
{
   QSettings cfg("WorldTime");
   cfg.beginGroup("TimeZones");

   int i;
   QString zn;
   zn = cfg.value("Zone0", QString() ).toString();

   if ( zn.isEmpty() ) {
      // WorldTime.conf doesn't exist, create default one
      cfg.setValue("Zone0", "UTC");
      cfg.setValue("Zone1", "Europe/Oslo");
      cfg.setValue("Zone2", "Asia/Tokyo");
      cfg.setValue("Zone3", "Asia/Hong_Kong");
      cfg.setValue("Zone4", "Australia/Brisbane");
      cfg.setValue("Zone5", "America/New_York");
      cfg.sync();
   }

   QStringList cityZonesList;
//create zoneslist
   for (i = 0; i < CITIES; i++ ) {
      zn = cfg.value("Zone" + QString::number(i), QString(i)).toString();
      if ( zn.isEmpty() )
         break;
      QString nm =  zn.section("/",-1) ;
      nm = nm.replace("_"," ");
      strCityTz[i] = zn;

      if (mMode == Minimal) {
         cityZonesList << nm;
      }
      else
         listCities.at(i)->setText(nm);
   }

   int visibleZones = cfg.value("Visible", 2).toInt();

   for (i = 0; i < CITIES; i++ ) {
      //insert zonelist into combo's and set current
      listBoxes.at(i)->clear();
      listBoxes.at(i)->insertItems(0,cityZonesList);
      cfg.endGroup();
      int index = cfg.value("Clock"+QString::number(i), QString::number(i)).toInt();
      listBoxes.at(i)->setCurrentIndex( index);
      cfg.beginGroup("TimeZones");
      zn = cfg.value("Zone" + QString::number(index), QString(index)).toString();


      listTimes.at(i)->setZone(zn);
   }
}

void WorldTime::addClock()
{
  if(visibleZones < maxVisibleZones) {

      QSettings cfg("WorldTime");
      cfg.beginGroup("TimeZones");
      visibleZones = cfg.value("Visible",1 ).toInt();
      gl->addWidget( listBoxes.at(visibleZones ), visibleZones + 3, 0, 0);
      gl->addWidget( listTimes.at(visibleZones ), visibleZones + 3, 1, Qt::Alignment(Qt::AlignBottom));
      visibleZones++;

      cfg.setValue("Visible", visibleZones);

      for (int i = 0; i < CITIES; i++) {
         if( i < visibleZones ) {
            listBoxes.at(i)->show();
            listTimes.at(i)->show();
         } else {
            listBoxes.at(i)->hide();
            listTimes.at(i)->hide();
         }
         update();
      }
   }
    checkMenu();
}

void WorldTime::removeClock()
{

  if(visibleZones > 1) {
    QSettings cfg("WorldTime");
   cfg.beginGroup("TimeZones");
   visibleZones = cfg.value("Visible", 1).toInt();
   visibleZones--;

   for (int i = 0; i < CITIES; i++) {
      if( i < visibleZones ) {
         listBoxes.at(i)->show();
         listTimes.at(i)->show();
      } else {
         listBoxes.at(i)->hide();
         listTimes.at(i)->hide();
      }
   }
   cfg.setValue("Visible", visibleZones);
   cfg.endGroup();
   cfg.remove("Clock"+QString::number(visibleZones));
  }
      checkMenu();
}

int WorldTime::getCurrentComboIndex(int clock)
{
   QSettings cfg("WorldTime");
   cfg.endGroup();
   return cfg.value("Clock"+QString::number(clock)).toInt();
}

void WorldTime::keyReleaseEvent( QKeyEvent *ke )
{
    switch(ke->key())  {
    case  Qt::Key_Select:
        saveChanges();
        break;
    };

}

void WorldTime::applyChange()
{
    changed = true;
    saveChanges();
    if(frmMap->isZoom())
        frmMap->toggleZoom();
}

void WorldTime::selected()
{
    QTimeZone zone = frmMap->zone();
    int i = 0;
    
    if(currentCombo == -1) return;
    if(strCityTz[currentComboIndex] == zone.id()) { 
        viewMode();
        return; }
    
    if ((mMode == Minimal) ) {
        for (i = 0; i < CITIES; i++) {
            if ( i == currentCombo) {
                strCityTz[currentComboIndex] = zone.id();
                listBoxes.at(i)->setEditable( true );
                listBoxes.at(i)->setItemText( currentComboIndex, zone.city());
                listBoxes.at(i)->lineEdit()->setText(zone.city());
                listBoxes.at(i)->lineEdit()->setCursorPosition(0);
                listTimes.at(i)->setZone( zone.id());
            }
        }
    }
    
    //  this->setFocus();
    applyChange();
}

void WorldTime::checkMenu()
{
    if(visibleZones > 1) {
        if( !removeClockAction->isVisible()) {
            removeClockAction->setVisible(true);
        }
    } else {
        if( removeClockAction->isVisible()) {
            removeClockAction->setVisible(false);
        }
    }
    
    if(visibleZones == maxVisibleZones) {
        if(addClockAction->isVisible())
            addClockAction->setVisible(false);
    } else {
        if( !addClockAction->isVisible())
            addClockAction->setVisible(true);
    }    
}
