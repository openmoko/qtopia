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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <qpixmap.h>
#include "ui_camerabase.h"
#include "ui_camerasettings.h"
#include <qmainwindow.h>
#include <qcontent.h>
#include <QDSActionRequest>
#include <QtopiaAbstractService>

class QAction;
class QTimer;
class QValueSpaceItem;


class CameraMainWindow : public QMainWindow
{
    Q_OBJECT
public:
    CameraMainWindow( QWidget *parent=0, Qt::WFlags fl=0 );
    ~CameraMainWindow();

public slots:
    void takePhoto();
    void toggleVideo();
    void selectThumb(int i);
    void thumbClicked(int i);
    void getImage( const QDSActionRequest& request );

private slots:
    void viewPictures();
    void viewVideos();
    void doSettings();
    void editThumb();
    void delThumb();
    void moveToContact();
    void takePhotoNow();
    void takePhotoTimer();
    void sendFile();
    void contentChanged(const QContentIdList&, QContent::ChangeType);
    void clamshellChanged();
    void contextMenuAboutToShow();
    void contextMenuAboutToHide();

private:
    bool event(QEvent* e);
    void updateActions();
    void resizeEvent(QResizeEvent*);

    bool eventFilter(QObject*, QEvent*);
    QString nextFileName();
    void loadThumbs();
    void pushThumb(const QContent& f, const QImage& img);
    static const int nthumb = 5;
    QToolButton* thumb[nthumb];
    QContent picturefile[nthumb];
    int cur_thumb;
    void delThumb(int th);

    // Settings
    void confirmSettings();
    Ui::CameraSettings *settings;
    QDialog *settingsDialog;
    QString storagepath;
    QString media;
    int thumbw;
    int thumbh;
    int psize;
    int vsize;
    int pquality;
    int vquality;
    int vframerate;

    // Snap
    QSize snap_max;
    void setSnapMode( bool snapMode );

    Ui::CameraBase *camera;

    int namehint;
    QAction *a_pview, *a_vview, *a_timer, *a_settings;
    QAction *a_th_edit, *a_th_del, *a_th_add;
    QAction *a_send;
    QList<QSize> photo_size;
    QList<QSize> video_size;

    QTimer *refocusTimer;
    QString picfile;

    QDSActionRequest* snapRequest;

    bool recording;
    void stopVideo();
    void startVideo();

    void preview();

    void videoToScreen(int screen);
    bool videoOnSecondary;
    QValueSpaceItem *clamshellVsi;

    QString camcat;
    bool    m_contextMenuActive;
};

class CameraService : public QtopiaAbstractService
{
    Q_OBJECT
    friend class CameraMainWindow;
private:
    CameraService( CameraMainWindow *parent )
        : QtopiaAbstractService( "Camera", parent )
        { this->parent = parent; publishAll(); }

public:
    ~CameraService();

public slots:
    void getImage( const QDSActionRequest& request );
    void shutter();

private:
    CameraMainWindow *parent;
};


#endif

