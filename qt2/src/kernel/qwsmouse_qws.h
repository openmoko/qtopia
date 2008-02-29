/****************************************************************************
** $Id: qt/src/kernel/qwsmouse_qws.h   2.3.12   edited 2005-10-27 $
**
** Definition of Qt/FB central server classes
**
** Created : 991025
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Qt/Embedded may use this file in accordance with the
** Qt Embedded Commercial License Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef QWSMOUSE_H
#define QWSMOUSE_H

#ifndef QT_H
#include <qobject.h>
#include <qpointarray.h>
#endif // QT_H

class QWSPointerCalibrationData
{
public:
    enum Location { TopLeft = 0, BottomLeft = 1, BottomRight = 2, TopRight = 3,
		    Center = 4, LastLocation = Center };
    QPoint devPoints[5];
    QPoint screenPoints[5];
};

class QWSMouseHandler : public QObject {
    Q_OBJECT
public:
    QWSMouseHandler();
    virtual ~QWSMouseHandler();

    virtual void clearCalibration() {}
    virtual void calibrate( QWSPointerCalibrationData * ) {}
    virtual void getCalibration( QWSPointerCalibrationData * ) {}

signals:
    void mouseChanged(const QPoint& pos, int bstate);
};

class QCalibratedMouseHandler : public QWSMouseHandler
{
    Q_OBJECT
public:
    QCalibratedMouseHandler();

    virtual void clearCalibration();
    virtual void calibrate( QWSPointerCalibrationData * );
    virtual void getCalibration( QWSPointerCalibrationData * );

protected:
    void readCalibration();
    void writeCalibration();
    QPoint transform( const QPoint & );
    bool sendFiltered( const QPoint &, int button );
    void setFilterSize( int );

private:
    int a, b, c;
    int d, e, f;
    int s;
    QPointArray samples;
    unsigned int currSample;
    unsigned int numSamples;
};

#endif

