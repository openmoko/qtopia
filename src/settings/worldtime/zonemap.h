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

#ifndef ZONEMAP_H
#define ZONEMAP_H

#include "stylusnormalizer.h"
#include <qtopia/timezone.h>

#include <qlist.h>
#include <qscrollview.h>
#include <qstring.h>

extern const int iCITYOFFSET;

class QImage;
class QComboBox;
class QLabel;
class QTimer;
class QToolButton;


class ZoneMap : public QScrollView
{
    Q_OBJECT
public:
    ZoneMap( QWidget *parent = 0, const char *name = 0 );
    ~ZoneMap();
    void showZones( void ) const;
    // convert between the pixels on the image and the coordinates in the
    // database
    inline bool zoneToWin( int zoneX, int zoneY, int &winX, int &winY ) const;
    inline bool winToZone( int winX, int winY, int &zoneX, int &zoneY ) const;

public slots:
    void slotZoom( bool setZoom );
    void slotIllum( bool setIllum );
    void slotUpdate( void );
    void slotRedraw( void );
    void slotFindCity( const QPoint &pos );  // Find the closest city

signals:
    void signalTz( const QCString &timezoneID );

protected:
    virtual void viewportMouseMoveEvent( QMouseEvent *event );
    virtual void viewportMousePressEvent( QMouseEvent *event );
    virtual void viewportMouseReleaseEvent( QMouseEvent *event );
    virtual void keyPressEvent( QKeyEvent * );
    virtual void resizeEvent( QResizeEvent *);
    virtual void drawContents( QPainter *p, int cx, int cy, int cw, int ch );

private:
    const TimeZone findCityNear( const TimeZone &city, int key );
    void showCity( const TimeZone &city );
    void drawCities( QPainter *p );	// put all the cities on the map (ugly)
    void drawCity( QPainter *p, const TimeZone &pCity ); // draw the given city on the map
    void zoom( void );  // Zoom the map...
    void makeMap( int width, int height );
    QPixmap* pixCurr; // image to be drawn on the screen
    QLabel* lblCity;    // the "tool-tip" that shows up when you pick a city...
    QToolButton *cmdZoom;   // our zoom option...
    QTimer*	tHide;  // the timer to hide the "tool tip"
    TimeZone m_last;   // the last known good city that was found...
    TimeZone m_repaint; // save the location to maximize the repaint...
    StylusNormalizer norm;

    //the True width and height of the map...
    int wImg;
    int hImg;
    // the pixel points that correspond to (0, 0);
    int ox;
    int oy;

    // the drawable area of the map...
    int drawableW;
    int drawableH;

    bool bZoom; // a flag to indicate zoom is active
    bool bIllum;    // flag to indicat that illumination is active

    TimeZone m_cursor;
};

inline bool ZoneMap::zoneToWin( int zoneX, int zoneY,
                                int &winX, int &winY ) const
{
    winY = oy - ( ( hImg * zoneY ) / 648000 );  // 180 degrees in secs
    winX = ox + ( ( wImg * zoneX ) / 1296000 ); // 360 degrees in secs
    // whoa, some things aren't in the best spots..
    if ( winX > wImg ) {
        winX = wImg - iCITYOFFSET;
    } else if ( winX <= 0 ) {
        winX = iCITYOFFSET;
    }

    if ( winY >= hImg ) {
        winY = hImg - iCITYOFFSET;
    } else if ( winY <= 0 ) {
        winY = iCITYOFFSET;
    }
    // perhaps in the future there will be some real error checking
    // for now just return true...
    return true;
}
 
inline bool ZoneMap::winToZone( int winX, int winY, 
                                int &zoneX, int &zoneY ) const
{
    zoneY = ( 648000 * ( oy - winY ) ) / hImg;
    zoneX = ( 1296000 * ( winX - ox ) ) / wImg;
    // perhaps in the future there will be some real error checking
    // for now just return true...
    return true;
} 

#endif
