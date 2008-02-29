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

#ifndef STATEWIDGET_H
#define STATEWIDGET_H

#include "playercontrol.h"

#include <qmediawidgets.h>

#include <QtGui>

class StateWidget : public QWidget
{
    Q_OBJECT
public:
    StateWidget( PlayerControl* control, QWidget* parent = 0 );

private slots:
    void setState( PlayerControl::State state );
    void setStopped();

protected:
    void keyPressEvent( QKeyEvent* e );
    void mousePressEvent( QMouseEvent* e );
    void mouseReleaseEvent( QMouseEvent* e );

private:
    void togglePlaying();

    PlayerControl *m_control;
    QMediaStateLabel *m_label;

    QTimer *m_holdtimer;
};

#endif // STATEWIDGET_H
