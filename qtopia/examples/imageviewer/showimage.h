/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS.  All rights reserved.
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

#ifndef SHOWIMAGE_H
#define SHOWIMAGE_H

#include <qwidget.h>
#include <qmainwindow.h>
#include <qimage.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qwmatrix.h>
#include <qvaluelist.h>

#include "settingsdialog.h"

class QMenuBar;
class QPopupMenu;
class QWidgetStack;
class FileSelector;
class DocLnk;
class StorageInfo;
class QTimer;
class QAction;

class ImageWidget : public QWidget
{
    Q_OBJECT
public:
    ImageWidget( QWidget *parent=0 ) : QWidget( parent ), pixmap( 0 ) { }
    ~ImageWidget() { }

    void setPixmap( const QPixmap& pm ) { pixmap = pm; }
    void showBusy();
signals:
    void clicked();

protected:
    void paintEvent( QPaintEvent * );
    void mouseReleaseEvent(QMouseEvent* event);

private:
    QPixmap pixmap;
};


class ImagePane : public QWidget
{
    Q_OBJECT
public:
    ImagePane( QWidget *parent=0 );
    ~ImagePane() { }

    void showStatus();
    void hideStatus();
    QLabel  *statusLabel() { return status; }
    void setPixmap( const QPixmap& pm );

    int w() { return imageWidth; }
    int h() { return imageHeight; }

    void showBusy();

protected:
    void keyPressEvent(QKeyEvent *e);
    void closeEvent(QCloseEvent *e);

signals:
    void clicked();
    void keypress(int keycode);

private:
    int		imageWidth;
    int		imageHeight;
    ImageWidget	*image;
    QLabel	*status;
    QVBoxLayout *vb;
};


class ImageViewer : public QMainWindow
{
    Q_OBJECT
public:
    ImageViewer( QWidget *parent=0, const char *name=0, int wFlags=0 );
    ~ImageViewer();
    void show();

protected:
    void resizeEvent( QResizeEvent * );
    void keyPressEvent(QKeyEvent *e);

private:
    void updateCaption( QString name=QString::null );
    bool loadSelected();
    void loadFilename( const QString &file );
    void scale( bool newImage );
    void setScaledImage();
    void convertEvent( QMouseEvent* e, int& x, int& y );
    int h();
    void updateStatus();
    void openFile( const QString &name, const QString &file );
    int  imageIndex(void);
    void setControls();

signals:
    void keypress(int keycode);

private slots:
    void settings();
    void setDocument(const QString& fileref);
    void openFile( const DocLnk &file );
    void open();
    void closeFileSelector();
    void hFlip();
    void vFlip();
    void rot180();
    void rot90();
    void rot270();
    void toggleFullscreen();
    void normalView();
    void fullScreen();
    const QPixmap &scaledPixmap( bool newImage );
    void handleKeypress(int keycode);
    void systemMessage( const QCString &msg, const QByteArray &data);

private:
    QImage  image;	    // the loaded image

    bool needPmScaled0;	    // flags to tell if we need to create scaled pms
    bool needPmScaled90;

    QPixmap pmScaled;	    // the scaled and transformed pixmap
    QPixmap pmScaled0;	    // the scaled pixmap sized for rotation by 0
    QPixmap pmScaled90;	    // the scaled pixmap sized for rotation by 90

    QWMatrix matrix;
    bool rotated90;

    QString filename;

    QMenuBar *menubar;
    QToolBar *toolBar;
    DocLnk  *doc;	    // for accessing properties
    StorageInfo *storage;
    
    QWidgetStack *stack;
    ImagePane *imagePanel;
    FileSelector *fileSelector;
    int imagewidth, imageheight;
    bool isFullScreen;
    bool bFromDocView;	    // a flag to indicate whether or not we were
			    // launched from the document view...

    QPopupMenu *edit;
    QAction *openAction;
    QAction *rotateAction;
    QAction *flipAction;
    QAction *fullscreenAction;
    int	    hflip_id;
    int	    vflip_id;

    int slideDelay;
    bool slideRepeat;
    bool slideReverse;	    // show slideshow in reverse order
    bool rotateOnLoad;	    // rotate by 90 degrees on loading to fit
    bool rotateClockwise;   // rotate clockwise
    bool fastLoad;	    // (sacrifice quality for speed)
    bool smallScale;	    // scale small images to desktop size
    QTimer *slideTimer;
};


#endif // SHOWIMAGE_H
