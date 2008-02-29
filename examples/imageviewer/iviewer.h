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

#ifndef IVIEWER_H
#define IVIEWER_H

#include <QStackedWidget>
#include <QWidget>

class ListScreen;
class ImageScreen;
class QKeyEvent;

class IViewer : public QStackedWidget 
{
    Q_OBJECT
public:
    IViewer(QWidget *parent=0, Qt::WFlags f=0);
    ImageScreen *imageScreen();
    ListScreen  *listScreen();
    void toggleFullScreen();
    void keyPressEvent(QKeyEvent *ke);
private:
    ListScreen  *_listScreen;
    ImageScreen *_imageScreen;

};

#endif
