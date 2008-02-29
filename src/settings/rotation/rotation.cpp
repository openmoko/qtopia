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

#include "settings.h"

#include <qtopia/global.h>
#include <qtopia/fontmanager.h>
#include <qtopia/config.h>
#include <qtopia/applnk.h>
#include <qtopia/qpeapplication.h>
#if defined(Q_WS_QWS) && !defined(QT_NO_COP)
#include <qtopia/qcopenvelope_qws.h>
#endif

#include <qlabel.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qtabwidget.h>
#include <qslider.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qdatastream.h>
#include <qmessagebox.h>
#include <qcombobox.h>
#include <qspinbox.h>
#include <qlistbox.h>
#include <qdir.h>
#if QT_VERSION >= 0x030000
#include <qstylefactory.h>
#endif

//#if defined(QT_QWS_IPAQ) || defined(QT_QWS_SL5XXX)
//#include <unistd.h>
//#include <linux/fb.h>
//#include <sys/types.h>
//#include <sys/stat.h>
//#include <fcntl.h>
//#include <sys/ioctl.h>
//#endif
#include <stdlib.h>


RotationSettings::RotationSettings( QWidget* parent,  const char* name, WFlags fl )
    : RotationSettingsBase( parent, name, TRUE, fl )
{
    reset();
#ifdef SINGLE_APP
#ifdef QT_DEMO_SINGLE_FLOPPY
    rotation_message->setText(
        tr("<p>Rotation is not enabled in this demo, however this is a supported feature of Qt/Embedded."));
#else
    rotation_message->setText(
        tr("<p>Rotation will not apply until you restart the server"));
#endif
#endif
}

RotationSettings::~RotationSettings()
{
}

void RotationSettings::accept()
{
    applyRotation();
    QDialog::accept();
}

static int initrot=-1;

void RotationSettings::applyRotation()
{
    if ( rot270->isChecked() )
        setRotation( 270 );
    else if ( rot180->isChecked() )
        setRotation( 180 );
    else if ( rot90->isChecked() )
        setRotation( 90 );
    else if ( rot0->isChecked() )
        setRotation( 0 );
}

void RotationSettings::setRotation(int r)
{
    QPEApplication::setDefaultRotation((r+initrot)%360);
}

void RotationSettings::reject()
{
    reset();
    QDialog::reject();
}

void RotationSettings::reset()
{
    QString d = getenv("QWS_DISPLAY");
    int r=0;
    if ( d.contains("Rot90") ) { // No tr
        r = 90;
    } else if ( d.contains("Rot180") ) { // No tr
        r = 180;
    } else if ( d.contains("Rot270") ) { // No tr
        r = 270;
    }
    if ( initrot == -1 )
        initrot = r;
    r = (r-initrot+360) % 360;
    rot0->setChecked(r==0);
    rot90->setChecked(r==90);
    rot180->setChecked(r==180);
    rot270->setChecked(r==270);
}


void RotationSettings::done(int r) { 
  QDialog::done(r);
  close();
}
