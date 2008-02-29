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

#ifndef COLORSELECTOR_H
#define COLORSELECTOR_H

#include <qpushbutton.h>

class ColorSelectorPrivate;

class ColorSelector : public QWidget
{
    Q_OBJECT
public:
    ColorSelector( QWidget *parent, const char *name=0, WFlags f=0 );
    ~ColorSelector();

    QColor color() const;
    QSize sizeHint() const;

    void setDefaultColor( const QColor & );
    const QColor &defaultColor() const;

public slots:
    void setColor( const QColor &c );

signals:
    void selected( const QColor &c );

protected:
    void paintEvent( QPaintEvent * );
    void mousePressEvent( QMouseEvent * );
    void mouseMoveEvent( QMouseEvent * );
    void mouseReleaseEvent( QMouseEvent * );
    void keyPressEvent( QKeyEvent * );
    void showEvent( QShowEvent * );
    QRect rectOfColor( int ) const;

private:
    QColor col;
    ColorSelectorPrivate *d;
};

class ColorButtonPrivate;

class ColorButton : public QPushButton
{
    Q_OBJECT
public:
    ColorButton( QWidget *parent, const char *name=0 );
    ColorButton( const QColor &c, QWidget *parent, const char *name=0 );
    ~ColorButton();

    QColor color() const { return col; }

    void setDefaultColor( const QColor & );
    const QColor &defaultColor() const;

public slots:
    void setColor( const QColor &c );

signals:
    void selected( const QColor &c );

private slots:
    void colorSelected( const QColor & );

protected:
    void drawButtonLabel( QPainter * );

private:
    void init();

private:
    QColor col;
    ColorButtonPrivate *d;
};

#endif
