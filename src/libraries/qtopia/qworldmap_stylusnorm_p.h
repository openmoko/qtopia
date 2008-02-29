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

#ifndef QWORLDMAPSTYLUSNORM_P_H
#define QWORLDMAPSTYLUSNORM_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qtopia API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

// Qt includes
#include <QWidget>
#include <QPoint>
#include <QTime>

// ============================================================================
//
// _StylusEvent
//
// ============================================================================

class _StylusEvent
{
public:
    _StylusEvent( const QPoint &pt = QPoint( 0, 0 ) );
    ~_StylusEvent();
    QPoint point( void ) const { return _pt; };
    QTime time( void ) const { return _t; };
    void setPoint( int x, int y) { _pt.setX( x ); _pt.setY( y ); };
    void setPoint( const QPoint &newPt ) { _pt = newPt; };
    void setTime( QTime newTime ) { _t = newTime; };

private:
    QPoint _pt;
    QTime _t;
};

// ============================================================================
//
// StylusNormalizer
//
// ============================================================================

class StylusNormalizer : public QWidget
{
    Q_OBJECT
public:
    explicit StylusNormalizer( QWidget *parent = 0 );
    ~StylusNormalizer();
    void start();
    void stop();
    void addEvent( const QPoint &pt );   // add a new point in

signals:
    void signalNewPoint( const QPoint &p );

private slots:
    void slotAveragePoint( void );  // return an averaged point

private:
    enum {SAMPLES = 10};
    _StylusEvent _ptList[SAMPLES];
    int _next;
    QTimer *_tExpire;
    bool bFirst;    // the first item added in...
};

#endif // QWORLDMAPSTYLUSNORM_P_H
