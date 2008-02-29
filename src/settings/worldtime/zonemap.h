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

#ifndef ZONEMAP_H
#define ZONEMAP_H

#include "stylusnormalizer.h"
#include <qtopia/timezone.h>

#include <qlist.h>
#include <qscrollview.h>
#include <qstring.h>
#include <qvector.h>

//#define TEST_ACCESS_TO_CITIES
extern const int iCITYOFFSET;

class QImage;
class QComboBox;
class QLabel;
class QTimer;
class QToolButton;

struct CityPos
{
    int lat;
    int lon;
    QCString id;
};

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

    void beginEditing();

    int heightForWidth(int) const;

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
    virtual void keyReleaseEvent( QKeyEvent * );
    virtual void resizeEvent( QResizeEvent *);
    virtual void drawContents( QPainter *p, int cx, int cy, int cw, int ch );

private slots:
    void initCities();

    void cursorTimeout();

private:
#ifdef TEST_ACCESS_TO_CITIES
    void testAccess();
#endif
    void updateCursor();
    void setCursorPoint( int, int );
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
    uint minMovement;
    uint maxMovement;

    // the drawable area of the map...
    int drawableW;
    int drawableH;

    bool bZoom; // a flag to indicate that zoom is active
    bool bIllum;    // flag to indicate that illumination is active

    TimeZone m_cursor;
    int m_cursor_x;
    int m_cursor_y;

    QVector<CityPos> cities;
    bool citiesInit;

    QTimer *cursorTimer;
    int accelHori;
    int accelVert;
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
