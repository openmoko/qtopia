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

#ifndef IMAGESCREEN_H
#define IMAGESCREEN_H

#include <QWidget>
#include <QKeyEvent>
#include <QContent>
#include <QImage>

class IViewer;
class QSlider;

class ImageScreen : public QWidget 
{
    Q_OBJECT
public:
    ImageScreen(IViewer *viewer);
    void setImage(const QContent &content);
private:
    void createActions();
    void createMenu();
    void paintEvent(QPaintEvent *event);
    void doBack();
protected:
    void keyPressEvent (QKeyEvent *event);
private slots:
    void onRotateLeft();
    void onRotateRight();
    void onZoomIn();
    void onZoomOut();
    void onShowInfo();
    void onFullScreen();
private:
    IViewer *_viewer;
    QImage  *_image;
    QContent _content;
    qreal    _rotation;
    qreal    _zoom;
    QAction *_rotateLeftAction;
    QAction *_rotateRightAction;
    QAction *_zoomInAction;
    QAction *_zoomOutAction;
    QAction *_showInfoAction;
    QAction *_fullScreenAction;
    QSlider *_zoomSlider;
};

#endif
