/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#ifndef SAMPLEWINDOW_H
#define SAMPLEWINDOW_H

#include "samplewindow.h"
#include <QWidget>
#include <qwindowdecorationinterface.h>

class QMenu;
class QMenuBar;

class SampleWindow : public QWidget
{
    Q_OBJECT
public:
    SampleWindow( QWidget *parent );

    QSize sizeHint() const;
    void setFont( const QFont &f );
    void setDecoration( QWindowDecorationInterface *i );
    void paintEvent( QPaintEvent * );
    void fixGeometry();
    void setUpdatesEnabled( bool e );

protected:
    void init();
    bool eventFilter( QObject *, QEvent *e );
    void changeEvent( QEvent *e );
    void resizeEvent( QResizeEvent *re );

    QWindowDecorationInterface *iface;
    QWindowDecorationInterface::WindowData wd;
    QWidget *container;
    QMenu *popup;
    QMenuBar *mb;
    int th;
    int tb;
    int lb;
    int rb;
    int bb;
    QSize desktopSize;
};

#endif

