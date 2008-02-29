/**********************************************************************
** Copyright (C) 2000-2004 Trolltech AS.  All rights reserved.
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

#ifndef FLATSTYLE_H
#define FLATSTYLE_H

#include <qwindowsstyle.h>
#include <qtopia/styleinterface.h>

class FlatStylePrivate;

class QTOPIA_PLUGIN_EXPORT FlatStyle : public QWindowsStyle
{
public:
    FlatStyle();
    virtual ~FlatStyle();
    virtual void polish( QPalette &p );
    virtual void polish( QWidget *w );
    virtual void unPolish( QWidget *w );

    int defaultFrameWidth () const;
    void drawItem( QPainter *p, int x, int y, int w, int h,
                    int flags, const QColorGroup &g, bool enabled,
		    const QPixmap *pixmap, const QString& text, int len, const QColor* penColor );
    void drawPanel ( QPainter * p, int x, int y, int w, int h,
		    const QColorGroup &, bool sunken=FALSE, int lineWidth = 1, const QBrush * fill = 0 );
    void drawButton( QPainter *p, int x, int y, int w, int h,
		    const QColorGroup &g, bool sunken, const QBrush* fill );
    void drawButtonMask ( QPainter * p, int x, int y, int w, int h );
    void drawBevelButton( QPainter *p, int x, int y, int w, int h,
		    const QColorGroup &g, bool sunken=FALSE, const QBrush* fill=0 );
    void drawToolButton( QPainter *p, int x, int y, int w, int h,
		    const QColorGroup &g, bool sunken=FALSE, const QBrush* fill=0 );
    void drawPushButton( QPushButton *btn, QPainter *p );
    void drawPushButtonLabel( QPushButton *btn, QPainter *p );
    QRect comboButtonRect( int x, int y, int w, int h);
    QRect comboButtonFocusRect( int x, int y, int w, int h);
    void drawComboButton( QPainter *p, int x, int y, int w, int h,
                    const QColorGroup &g, bool sunken, bool, bool enabled,
		    const QBrush *fill );
    void drawExclusiveIndicator ( QPainter * p, int x, int y, int w, int h,
		    const QColorGroup & g, bool on, bool down = FALSE, bool enabled = TRUE );
    void drawIndicator ( QPainter * p, int x, int y, int w, int h,
		    const QColorGroup & g, int state, bool down = FALSE, bool enabled = TRUE );
    void scrollBarMetrics( const QScrollBar*, int&, int&, int&, int&);
    void drawScrollBarControls( QPainter*,  const QScrollBar*, int sliderStart, uint controls, uint activeControl );
    ScrollControl scrollBarPointOver( const QScrollBar* sb, int sliderStart, const QPoint& p );
    void drawRiffles( QPainter* p,  int x, int y, int w, int h,
                          const QColorGroup &g, bool horizontal );
    int sliderLength() const;
    void drawSlider( QPainter *p, int x, int y, int w, int h,
		    const QColorGroup &g, Orientation, bool tickAbove, bool tickBelow );
    void drawSliderMask( QPainter *p, int x, int y, int w, int h,
		    Orientation, bool tickAbove, bool tickBelow );
    void drawSliderGrooveMask( QPainter *p, int x, int y, int w, int h,
		    const QColorGroup& , QCOORD c, Orientation orient );
    void drawSliderGroove ( QPainter * p, int x, int y, int w, int h, const QColorGroup & g, QCOORD c, Orientation );
    void drawTab( QPainter *, const QTabBar *, QTab *, bool selected );
    void polishPopupMenu ( QPopupMenu * );
    int extraPopupMenuItemWidth( bool checkable, int maxpmw, QMenuItem*, const QFontMetrics& );
    int popupMenuItemHeight( bool checkable, QMenuItem*, const QFontMetrics& );
    void drawPopupMenuItem( QPainter* p, bool checkable, int maxpmw, int tab, QMenuItem* mi,
				       const QPalette& pal,
				       bool act, bool enabled, int x, int y, int w, int h);

    int buttonMargin() const;
    QSize scrollBarExtent() const;
    void getButtonShift( int &x, int &y );

private:
    FlatStylePrivate *d;
    bool revItem;
    bool fillBtnBorder;
    QColor btnBg;
    // Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    FlatStyle( const FlatStyle & );
    FlatStyle& operator=( const FlatStyle & );
#endif
};


class QTOPIA_PLUGIN_EXPORT FlatStyleImpl : public StyleInterface
{
public:
    FlatStyleImpl();
    virtual ~FlatStyleImpl();

    QRESULT queryInterface( const QUuid&, QUnknownInterface** );
    Q_REFCOUNT

    virtual QStyle *style();
    virtual QString name() const;

private:
    FlatStyle *flat;
    ulong ref;
};

#endif // FLATSTYLE_H
