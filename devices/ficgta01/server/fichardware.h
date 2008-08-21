/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
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

#ifndef FICHARDWARE_H
#define FICHARDWARE_H

#ifdef QT_QWS_FICGTA01

#include <QObject>
#include <QProcess>

#include <qvaluespace.h>
#include <linux/input.h>

class QBootSourceAccessoryProvider;
class QPowerSourceProvider;

class QSocketNotifier;
class QtopiaIpcAdaptor;
class QSpeakerPhoneAccessoryProvider;

class Ficgta01Hardware : public QObject
{
    Q_OBJECT

public:
    Ficgta01Hardware();
    ~Ficgta01Hardware();

private:
     QValueSpaceObject vsoPortableHandsfree;
     QValueSpaceObject vsoUsbCable;
     QValueSpaceObject vsoNeoHardware;
     QtopiaIpcAdaptor *adaptor;

     void findHardwareVersion();

     QtopiaIpcAdaptor         *audioMgr;
//     QAudioStateConfiguration *audioConf;
    
 
private slots:
     void headphonesInserted(bool);
     void cableConnected(bool);
     void shutdownRequested();
};

#endif

#endif
