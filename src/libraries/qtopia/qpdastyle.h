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

#ifndef QPDASTYLE_H
#define QPDASTYLE_H

#include <qtopiastyle.h>

class QPdaStylePrivate;

class QPdaStyle : public QtopiaStyle
{
    Q_OBJECT
public:
    QPdaStyle();
    virtual ~QPdaStyle();

    int pixelMetric(PixelMetric metric, const QStyleOption *option = 0, const QWidget *widget = 0) const;

private:
    QPdaStylePrivate *d;
};

#ifdef QTOPIA4_TODO

#include <qtopiaglobal.h>

#ifndef QT_H
#include <qwindowsstyle.h>
#endif // QT_H

#if QT_VERSION >= 0x030000

class QTOPIA_EXPORT QPEStyle : public QWindowsStyle
{
public:
    QPEStyle();
    virtual ~QPEStyle();

    virtual void drawPrimitive( PrimitiveElement pe, QPainter *p, const QRect &r, const QColorGroup &cg, SFlags flags=Style_Default, const QStyleOption & = QStyleOption::Default) const;
    virtual void drawControl( ControlElement ce, QPainter *p, const QWidget *widget, const QRect &r, const QColorGroup &cg, SFlags how=Style_Default, const QStyleOption & = QStyleOption::Default) const;
    virtual void drawComplexControl( ComplexControl control, QPainter *p, const QWidget *widget, const QRect &r, const QColorGroup &cg, SFlags how=Style_Default, SCFlags sub=SC_All, SCFlags subActive=SC_None, const QStyleOption & = QStyleOption::Default) const;
    virtual int pixelMetric( PixelMetric metric, const QWidget *widget=0 ) const;
    virtual QSize sizeFromContents( ContentsType contents, const QWidget *widget, const QSize &contentsSize, const QStyleOption & = QStyleOption::Default) const;
};

#else

class  QTOPIA_EXPORT QPEStyle : public QWindowsStyle
{
public:
    QPEStyle();
    virtual ~QPEStyle();
    virtual void polish( QPalette &p );
    virtual void polish( QWidget *w );
    virtual void unPolish( QWidget *w );

    int defaultFrameWidth () const;
    void drawPanel ( QPainter * p, int x, int y, int w, int h,
                    const QColorGroup &, bool sunken=false, int lineWidth = 1, const QBrush * fill = 0 );
    void drawButton( QPainter *p, int x, int y, int w, int h,
                    const QColorGroup &g, bool sunken, const QBrush* fill );
    void drawButtonMask ( QPainter * p, int x, int y, int w, int h );
    void drawBevelButton( QPainter *p, int x, int y, int w, int h,
                    const QColorGroup &g, bool sunken=false, const QBrush* fill=0 );
    QRect comboButtonRect( int x, int y, int w, int h);
    QRect comboButtonFocusRect( int x, int y, int w, int h);
    void drawComboButton( QPainter *p, int x, int y, int w, int h,
                    const QColorGroup &g, bool sunken, bool, bool enabled,
                    const QBrush *fill );
    void drawExclusiveIndicator ( QPainter * p, int x, int y, int w, int h,
                    const QColorGroup & g, bool on, bool down = false, bool enabled = true );
    void drawIndicator ( QPainter * p, int x, int y, int w, int h,
                    const QColorGroup & g, int state, bool down = false, bool enabled = true );
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
    void drawTab( QPainter *, const QTabBar *, QTab *, bool selected );
    void polishPopupMenu ( QMenu * ); // Do not call in subclasses
    int extraPopupMenuItemWidth( bool checkable, int maxpmw, QMenuItem*, const QFontMetrics& );
    int popupMenuItemHeight( bool checkable, QMenuItem*, const QFontMetrics& );
    void drawPopupMenuItem( QPainter* p, bool checkable, int maxpmw, int tab, QMenuItem* mi,
                                       const QPalette& pal,
                                       bool act, bool enabled, int x, int y, int w, int h);

    int buttonMargin() const;
    QSize scrollBarExtent() const;

private:        // Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QPEStyle( const QPEStyle & );
    QPEStyle& operator=( const QPEStyle & );
#endif
};

#endif

#endif

#endif // QPESTYLE_H
