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
#ifndef QWORLDMAP_H
#define QWORLDMAP_H

#define QABS(a) ((a) >= 0 ? (a) : -(a))
#define QMAX(a,b)       ((a) > (b) ? (a) : (b))
#define QMIN(a,b)       ((a) < (b) ? (a) : (b))

// Qtopia includes
#include <QTimeZone>

// Qt includes
#include <QKeyEvent>
#include <QLabel>
#include <QMouseEvent>
#include <QPixmap>
#include <QResizeEvent>
#include <QAbstractScrollArea>

// Forward class declarations
class QWorldmapPrivate;

// ============================================================================
//
// QWorldmap
//
// ============================================================================

class QTOPIA_EXPORT QWorldmap : public QAbstractScrollArea
{
    Q_OBJECT

public:
    explicit QWorldmap( QWidget *parent = 0);
    ~QWorldmap();

    int heightForWidth( int w ) const;
    bool isZoom() const;
    bool isDaylight() const;
    bool isReadOnly() const;
    QTimeZone zone() const;

    virtual bool event(QEvent *event);

public slots:
    void selectNewZone();
    void toggleZoom();
    void select();
    void setDaylight( const bool show );
    void setZone( const QTimeZone& zone );
    void setZone( const QPoint& pos );
    void setReadOnly( const bool readOnly = true );
    void setContinuousSelect(const bool selectMode = false);

signals:
    void selecting();
    void newZone( const QTimeZone& zone );
    void selectZoneCanceled();
    void buttonSelected();

protected:
    virtual void keyPressEvent( QKeyEvent * );
    virtual void keyReleaseEvent( QKeyEvent * );
    virtual void resizeEvent( QResizeEvent *);
    virtual void paintEvent( QPaintEvent *);
    virtual void mouseMoveEvent( QMouseEvent *event );
    virtual void mousePressEvent( QMouseEvent *event );
    virtual void mouseReleaseEvent( QMouseEvent *event );
    virtual void scrollContentsBy( int dx, int dy );

private slots:
    void update( void );
    void redraw( void );
    void initCities();
    void cursorTimeout();
    void cityLabelTimeout();
    void selectCanceled();

private:
    bool selectionMode;
#ifdef DEBUG_QWORLDMAP
    void testAccess();
    void drawCities( QPainter *p );
#endif
    void updateCursor();
    void setCursorPoint( int ox, int oy, QString city = QString() );
    void showCity( const QTimeZone &city );
    void drawCity( QPainter *p, const QTimeZone &pCity );
    void makeMap( int width, int height );

    void zoneToZoomedWin( int zoneX, int zoneY, int &winX, int &winY );
    void startSelecting();
    void stopSelecting();

    QWorldmapPrivate* d;
    void setCityLabelText();
};

#endif //QWORLDMAP_H
