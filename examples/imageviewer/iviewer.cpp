/****************************************************************************
**
** Copyright (C) 2007-2008 TROLLTECH ASA. All rights reserved.
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

#include "iviewer.h"
#include "listscreen.h"
#include "imagescreen.h"
#include <QtopiaApplication>
#include <QTimer>
#include <QKeyEvent>


IViewer::IViewer(QWidget *parent, Qt::WFlags /*f*/) 
: QStackedWidget(parent) 
{
    _listScreen  = 0;
    _imageScreen = 0;
    setCurrentWidget(listScreen()); 
}

ListScreen* IViewer::listScreen() 
{
    if (!_listScreen) {
        _listScreen = new ListScreen(this);
        addWidget(_listScreen);
    }
    return _listScreen;
}

ImageScreen* IViewer::imageScreen() 
{
    if (!_imageScreen) {
        _imageScreen = new ImageScreen(this);
        addWidget(_imageScreen);
    }
    return _imageScreen;
}

void IViewer::toggleFullScreen() 
{
    QString title = windowTitle(); 
    setWindowTitle( QLatin1String("_allow_on_top_"));
    setWindowState(windowState() ^ Qt::WindowFullScreen);
    setWindowTitle(title);
}

void IViewer::keyPressEvent(QKeyEvent *ke)
{
    if (ke->key() == Qt::Key_Back) {
        qDebug() << "Iviewer handles back event";
    }
    QWidget::keyPressEvent(ke);
}
