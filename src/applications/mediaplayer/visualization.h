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

#ifndef VISUALIZATION_H
#define VISUALIZATION_H

#include <QtGui>

class Flake;

class FlakeFactory
{
public:
    FlakeFactory( int maxpool );

    ~FlakeFactory();

    Flake* create();

    void retireAll();

    int count() { return m_count; }
    Flake* flakes() const { return m_pool; }

private:
    int m_count;
    Flake *m_pool;
};

class VisualizationWidget : public QWidget
{
    Q_OBJECT
public:
    VisualizationWidget( QWidget* parent = 0 );

    void setActive( bool active );

protected:
    void paintEvent( QPaintEvent* e );

    // Create new set of flakes every tick
    void timerEvent( QTimerEvent* e );

private:
    bool m_isactive;

    int m_generate;
    int m_update;

    FlakeFactory m_flakefactory;

    QPixmap m_flakepixmap;
    int m_halfwidth;
};

#endif // VISUALIZATION_H
