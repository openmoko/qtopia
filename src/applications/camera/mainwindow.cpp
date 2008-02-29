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

#include "mainwindow.h"
#include "videocaptureview.h"
#include "ui_camerasettings.h"
#include "thumbbutton.h"
#include "phototimer.h"

#include <qthumbnail.h>
#include <qtopiaapplication.h>
#include <qsoftmenubar.h>
#include <qtopiaipcenvelope.h>
#include <qtopiaservices.h>
#include <qcategorymanager.h>
#include <qstorage.h>

#include <qcontent.h>
#include <qcontentset.h>
#include <qvaluespace.h>

#include <QAction>
#include <QToolButton>
#include <QPushButton>
#include <QSignalMapper>
#include <QImage>
#include <QTimer>
#include <QSettings>
#include <QMessageBox>
#include <QKeyEvent>
#include <QPainter>
#include <QDSAction>
#include <QDSData>
#include <QDSServices>
#include <QDataStream>
#include <QByteArray>
#include <QDesktopWidget>

#ifdef QTOPIA_PHONE
#include <QMenu>
#else
#include <QToolBar>
#endif

#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>

static const int thmarg=2;
static const bool video_supported = false;

CameraMainWindow::CameraMainWindow(QWidget *parent, Qt::WFlags f):
    QMainWindow(parent, f),
    snapRequest( 0 ),
    videoOnSecondary(false),
    m_contextMenuActive(false)
{
    setWindowTitle(tr("Camera"));

    picfile = Qtopia::tempDir() + "image.jpg";
    QWidget *cameraWidget = new QWidget(this);
    camera = new Ui::CameraBase();
    camera->setupUi(cameraWidget);
    setCentralWidget(cameraWidget);

    connect(qApp,
            SIGNAL(contentChanged(const QContentIdList&,QContent::ChangeType)),
            this,
            SLOT(contentChanged(const QContentIdList&,QContent::ChangeType)));

    camcat = QLatin1String("Camera");
    // Ensure the Camera system categoy exists
    QCategoryManager catman("Documents");
    // For new code a more unique id should be used instead of using the untranslated text
    // eg. ensureSystemCategory("com.mycompany.myapp.mycategory", "My Category");
    catman.ensureSystemCategory(camcat, camcat);

    new CameraService(this);

    camera->photo->setFocus();

    connect(camera->photo, SIGNAL(clicked()), this, SLOT(takePhoto()));
    connect(camera->video, SIGNAL(clicked()), this, SLOT(toggleVideo()));

    refocusTimer = new QTimer(this);
    refocusTimer->setSingleShot(true);
    connect(refocusTimer, SIGNAL(timeout()), this, SLOT(takePhotoNow()));

    thumb[0] = camera->thumb1;
    thumb[1] = camera->thumb2;
    thumb[2] = camera->thumb3;
    thumb[3] = camera->thumb4;
    thumb[4] = camera->thumb5;
    cur_thumb = -1;

    QSignalMapper* sm = new QSignalMapper(this);
    for (int i=0; i<nthumb; i++) {
        sm->setMapping(thumb[i],i);
        connect(thumb[i],SIGNAL(clicked()),sm,SLOT(map()));
        thumb[i]->installEventFilter(this);
    }
    connect(sm,SIGNAL(mapped(int)),this,SLOT(thumbClicked(int)));

    QMimeType m( QLatin1String( "image/jpeg" ));
    QContent a = m.application();
    QIcon picViewIcon = a.icon();
    if ( picViewIcon.isNull() )
        picViewIcon = QIcon( QLatin1String( ":icon/view" ));
    a_pview = new QAction( picViewIcon, tr( "View pictures" ), this );

    connect( a_pview, SIGNAL( triggered() ), this, SLOT( viewPictures() ) );

    if ( video_supported ) {
        // If video_supported is ever not a constant false then ensure
        // an application is available to handle it
        if ( a.isValid() )
        {
            a_vview = new QAction( QIcon(":image/"+a.iconName()), QString("%1...").arg(a.name()), this );
            connect( a_vview, SIGNAL( triggered() ), this, SLOT( viewVideos() ) );
        }
    } else {
        a_vview = 0;
    }

    a_timer = new QAction( QIcon( ":icon/wait" ) , tr("Timer"), this );
    connect( a_timer, SIGNAL( triggered() ), this, SLOT( takePhotoTimer() ) );

    a_settings = new QAction( QIcon( ":icon/settings" ) , tr("Settings..."), this );
    connect( a_settings, SIGNAL( triggered() ), this, SLOT( doSettings() ) );

    a_th_edit = new QAction( QIcon(":icon/edit"), tr("Edit"), this );
    connect( a_th_edit, SIGNAL( triggered() ), this, SLOT( editThumb() ) );
    a_th_del = new QAction( QIcon(":icon/trash"), tr("Delete"), this );
    connect( a_th_del, SIGNAL( triggered() ), this, SLOT( delThumb() ) );
    a_th_add = new QAction( QIcon(":image/addressbook/AddressBook"), tr("Save to Contact..."), this );
    connect( a_th_add, SIGNAL( triggered() ), this, SLOT( moveToContact() ) );
    a_send = new QAction( QIcon( ":icon/beam" ), tr("Send to Contact..."), this );
    connect( a_send, SIGNAL( triggered() ), this, SLOT( sendFile() ) );

#ifndef QTOPIA_PHONE
    // Create Toolbars
    QToolBar *bar = new QToolBar( this );
    //bar->setHorizontalStretchable( true ); // TODO: what's the new version?
    bar->setMovable( false );

    addToolBar( bar );

    bar->addAction( a_pview );
    if ( video_supported )
        bar->addAction( a_vview );
    bar->addAction( a_th_edit );
    bar->addAction( a_th_del );
    bar->addAction( a_th_add );
    bar->addAction( a_timer );
    bar->addAction( a_send );
    bar->addAction( a_settings );
#else
    QMenu *contextMenu = QSoftMenuBar::menuFor(this);
    contextMenu->addAction( a_pview );
    if ( video_supported )
        contextMenu->addAction( a_vview );
    contextMenu->addAction( a_th_edit );
    contextMenu->addAction( a_th_del );
    contextMenu->addAction( a_th_add );
    contextMenu->addAction( a_timer );
    contextMenu->addAction( a_send );
    contextMenu->addAction( a_settings );

    connect(contextMenu, SIGNAL(aboutToHide()),
            this, SLOT(contextMenuAboutToHide()));
    connect(contextMenu, SIGNAL(aboutToShow()),
            this, SLOT(contextMenuAboutToShow()));
#endif

    installEventFilter(camera->photo);
    installEventFilter(camera->video);
    camera->photo->installEventFilter(this);
    camera->video->installEventFilter(this);

    // Load the allowable sizes from the camera hardware.
    photo_size = camera->videocaptureview->photoSizes();
    video_size = camera->videocaptureview->videoSizes();

    settingsDialog = new QDialog( this );
    settingsDialog->setModal( true );
    settings = new Ui::CameraSettings();
    settings->setupUi( settingsDialog );
    settingsDialog->setObjectName( "settings" );    // To pick up correct help.
    connect( settings->photo_quality, SIGNAL(valueChanged(int)),
             settings->photo_quality_n, SLOT(setNum(int)) );
    QFileSystemFilter *fsf = new QFileSystemFilter;
    fsf->documents = QFileSystemFilter::Set;
    settings->location->setFilter(fsf);
    // load settings from config
    QSettings cfg("Trolltech","Camera");
    cfg.beginGroup("General");
    QString l = cfg.value("location").toString();
    if ( !l.isEmpty() )
        settings->location->setLocation(l);
    storagepath = settings->location->documentPath();
    cfg.endGroup();
    cfg.beginGroup("Photo");
    int w;
    w = cfg.value("width",camera->videocaptureview->recommendedPhotoSize().width()).toInt();
    for (psize=0; psize<(int)photo_size.count()-1 && photo_size[psize].width() > w;)
        psize++;
    pquality = cfg.value("quality",settings->photo_quality->value()).toInt();
    cfg.endGroup();
    cfg.beginGroup("Video");
    w = cfg.value("width",camera->videocaptureview->recommendedVideoSize().width()).toInt();
    for (vsize=0; vsize<(int)video_size.count()-1 && video_size[vsize].width() > w;)
        vsize++;
    vquality = cfg.value("quality",settings->video_quality->value()).toInt();
    vframerate = cfg.value("framerate",settings->video_framerate->value()).toInt();

    for (int i=0; i<(int)photo_size.count(); i++) {
        settings->photo_size->addItem(QString("%1 x %2").arg(photo_size[i].width()).arg(photo_size[i].height()));
    }
    for (int i=0; i<(int)video_size.count(); i++) {
        settings->video_size->addItem(QString("%1 x %2").arg(video_size[i].width()).arg(video_size[i].height()));
    }

    namehint=0;
    recording = false;

    preview();

    if ( !video_supported ) {
        // Room for longer text
        camera->photo->setText(tr("Take Photo"));
        camera->video->setEnabled(false);
        camera->video->hide();
    }

    if (QApplication::desktop()->numScreens() > 1) {
        // We have a secondary display - watch for the clamshell open/close
        clamshellVsi = new QValueSpaceItem("/Hardware/Devices/ClamshellOpen", this);
        connect(clamshellVsi, SIGNAL(contentsChanged()), this, SLOT(clamshellChanged()));
        if (!clamshellVsi->value().toBool()) {
            videoToScreen(1);
        }
    }

#ifdef Q_WS_QWS
    QtopiaApplication::setPowerConstraint(QtopiaApplication::Disable);
#endif
}

CameraMainWindow::~CameraMainWindow()
{
#ifdef Q_WS_QWS
    QtopiaApplication::setPowerConstraint(QtopiaApplication::Enable);
#endif
}

void CameraMainWindow::resizeEvent(QResizeEvent*)
{
    thumbw = width()/5-4;
    thumbh = thumbw*3/4;
    camera->thumbs->setFixedHeight(thumbh+thmarg*2);

    loadThumbs();
}

bool CameraMainWindow::event(QEvent* e)
{
    if ( e->type() == QEvent::WindowActivate ) {
        if ( cur_thumb < 0 )
            camera->videocaptureview->setLive();
    } else if ( e->type() == QEvent::WindowDeactivate ) {
        camera->videocaptureview->setLive(-1);
    }
    return QMainWindow::event(e);
}


bool CameraMainWindow::eventFilter(QObject* o, QEvent* e)
{
    if ( e->type() == QEvent::KeyPress ) {
        QKeyEvent *ke = (QKeyEvent*)e;

#if QTOPIA_PHONE
        if (!ke->isAutoRepeat()) {
            if (ke->key() == Qt::Key_1) {
                takePhoto();
                return true;
            } else if (ke->key() == Qt::Key_2) {
                toggleVideo();
                return true;
            }
        }
#endif
        if ( ke->key() == Qt::Key_Up ) {
            camera->photo->setFocus();
            return true;
        } else if ( ke->key() == Qt::Key_Down ) {
            thumb[0]->setFocus();
            return true;
        } else if ( ke->key() == Qt::Key_Left ) {
            if ( o == camera->video ) {
                camera->photo->setFocus();
                return true;
            } else {
                if ( o == thumb[0] )
                    return true;
                for (int i=1; i<nthumb; ++i) {
                    if ( o == thumb[i] ) {
                        thumb[i-1]->setFocus();
                        return true;
                    }
                }
            }
        } else if ( ke->key() == Qt::Key_Right ) {
            if ( o == camera->photo ) {
                camera->video->setFocus();
                return true;
            } else {
                if ( o == thumb[nthumb-1] )
                    return true;
                for (int i=0; i<nthumb-1; ++i) {
                    if ( o == thumb[i] ) {
                        thumb[i+1]->setFocus();
                        return true;
                    }
                }
            }
        }
    }
    else if (!m_contextMenuActive)
    {
        if (e->type() == QEvent::FocusIn)
        {
            if (o == camera->photo)
                camera->photo->setText(tr("Take Photo"));
            updateActions();
        }
        else if (e->type() == QEvent::FocusOut)
        {
            if (o == camera->photo)
                camera->photo->setText(tr("Activate Camera"));
        }
    }

    return QWidget::eventFilter(o,e);
}

void CameraMainWindow::updateActions()
{
    bool p=false,v=false;
    QWidget *foc = focusWidget();
    if ( foc == camera->photo ) {
        p = true; v = false;
    } else if ( foc == camera->video ) {
        v = true; p = false;
    }
    a_pview->setVisible(p);
    if ( video_supported )
        a_vview->setVisible(v);
    a_settings->setVisible(p || v);
    bool th=!p && !v;
    if ( th ) {
        int i;
        for (i=0; i<nthumb; i++) {
            if ( thumb[i] == foc ) {
                selectThumb(i);
                break;
            }
        }
        if ( i==nthumb || thumb[i]->icon().isNull() )
            selectThumb(-1);
    } else {
        selectThumb(-1);
    }
}

void CameraMainWindow::viewPictures()
{
    QtopiaServiceRequest req("PhotoEdit","showCategory(QString)");
    req << camcat;
    req.send();
}

void CameraMainWindow::viewVideos()
{
    QMimeType m( QLatin1String( "video/mpeg" ));
    QContent a = m.application();
    if ( a.isValid() )
        a.execute();
}

void CameraMainWindow::doSettings()
{
    bool v = video_supported;
#ifdef QTOPIA_PHONE
    bool p;
    p = a_pview->isEnabled();
    v = v && a_vview && a_vview->isVisible();
    if (p)
        settings->photo->show();
    else
        settings->photo->hide();
#endif
    if (v)
        settings->video->show();
    else
        settings->video->hide();
    settings->photo_size->setCurrentIndex(psize);
    settings->video_size->setCurrentIndex(vsize);
    settings->photo_quality->setValue(pquality);
    settings->video_quality->setValue(vquality);
    settings->video_framerate->setValue(vframerate);
    settings->video_quality_n->setFixedWidth(fontMetrics().width("100"));
    settings->photo_quality_n->setFixedWidth(fontMetrics().width("100"));

    if ( QtopiaApplication::execDialog(settingsDialog) ) {
        confirmSettings();
    } else {
        settings->location->setLocation(storagepath);
    }
}

void CameraMainWindow::confirmSettings()
{
    storagepath = settings->location->documentPath();
    psize = settings->photo_size->currentIndex();
    vsize = settings->video_size->currentIndex();
    pquality = settings->photo_quality->value();
    vquality = settings->video_quality->value();
    vframerate = settings->video_framerate->value();

    // save settings
    QSettings cfg("Trolltech","Camera");
    cfg.beginGroup("General");
    cfg.setValue("location",storagepath);
    cfg.endGroup();
    cfg.beginGroup("Photo");
    cfg.setValue("width",photo_size[psize].width());
    cfg.setValue("quality",pquality);
    cfg.endGroup();
    cfg.beginGroup("Video");
    cfg.setValue("width",video_size[vsize].width());
    cfg.setValue("quality",vquality);
    cfg.setValue("framerate",vframerate);

    loadThumbs();

    preview();
}

void CameraMainWindow::loadThumbs()
{
    QContentSet     set;

    set.addCriteria(QContentFilter::Category, camcat, QContentFilter::And);
    set.addCriteria(QContentFilter::MimeType, "image/jpeg", QContentFilter::And);

    set.setSortOrder(QStringList("time desc"));

    QContentSetModel    model(&set);

    for (int i = 0; i < nthumb; ++i)
    {
        QPixmap pm;

        if (i < model.rowCount()) {
            picturefile[i] = model.content(i);

            pm = QThumbnail(picturefile[i].file()).pixmap(QSize(thumbw, thumbh));
        }

        thumb[i]->setIcon(pm);
        thumb[i]->setEnabled(!pm.isNull());
    }

    if ( cur_thumb >= 0 )
        selectThumb(cur_thumb);

    if ( !camera->videocaptureview->available() ) {
        camera->photo->setEnabled(false);
        camera->video->setEnabled(false);
        if (model.rowCount() == 0) {
            thumb[0]->setEnabled(false);
        } else {
            thumb[0]->setFocus();
            thumb[0]->setEnabled(true);
        }
    }

    updateActions();
}

void CameraMainWindow::delThumb(int th)
{
    switch(QMessageBox::warning(0, tr("Confirmation"),
            tr("<qt>Delete '%1'?</qt>", "%1 = file name").arg(picturefile[th].name()),
            QMessageBox::Yes,
            QMessageBox::No))
    {
        case QMessageBox::Yes:
            picturefile[th].removeFiles();

            // Rhys Hack - if we have just deleted the last image and there
            // is no camera connected, then exit the application.  This
            // avoids a focus problem where it is impossible to exit with
            // the back button due to the focus being in a stupid place.
            loadThumbs();
            if ( !camera->videocaptureview->available() &&
                 !(thumb[0]->isEnabled()) ) {
                close();
            }
            break;
        default:
            //nothing
            break;
    }
}

void CameraMainWindow::pushThumb(const QContent& f, const QImage& img)
{
    for (int i=nthumb; --i; ) {
        bool en = thumb[i-1]->isEnabled();
        thumb[i]->setEnabled(en);
        picturefile[i] = picturefile[i-1];
        QIcon icon = thumb[i-1]->icon();
        if ( en && !icon.isNull() ) {
            thumb[i]->setIcon(icon);
        } else {
            thumb[i]->setText("");
        }
    }
    thumb[0]->setIcon(QPixmap::fromImage(img.scaled(
            QSize( thumbw, thumbh ), Qt::KeepAspectRatio, Qt::SmoothTransformation)));
    thumb[0]->setEnabled(true);
    picturefile[0]=f;
}

void CameraMainWindow::takePhotoTimer()
{
    PhotoTimerDialog* dialog = new PhotoTimerDialog( this );
    if ( dialog->exec() == QDialog::Accepted ) {
        PhotoTimer* timer = new PhotoTimer( dialog->timeout(),
                                            dialog->number(),
                                            dialog->interval(),
                                            this );
        connect( timer, SIGNAL( takePhoto() ), this, SLOT( takePhotoNow() ) );
        timer->show();
    }

    delete dialog;
}

void CameraMainWindow::takePhoto()
{
    if (camera->photo != focusWidget())
    {
        camera->photo->setFocus();
        return;
    }
    QSize size = photo_size[psize];
    if ( size == camera->videocaptureview->captureSize() ||
        camera->videocaptureview->refocusDelay() == 0 )
    {

        // We can grab the current image immediately.
        takePhotoNow();

    } else {

        // Change the camera size and then wait for the camera to refocus.
        camera->videocaptureview->setCaptureSize( size );
        refocusTimer->start( camera->videocaptureview->refocusDelay() );

    }
}

void CameraMainWindow::takePhotoNow()
{
    QImage img = camera->videocaptureview->image();

    if ( snapRequest != 0 ) {
        // Rescale the image and pop it into a QDSData object
        QImage scaledimg = img.scaled( snap_max,
                                       Qt::KeepAspectRatio,
                                       Qt::SmoothTransformation);
        QByteArray savedImageData;
        {
            QDataStream stream( &savedImageData, QIODevice::WriteOnly );
            stream << QPixmap::fromImage( scaledimg );
        }
        QDSData snappedImage( savedImageData, QMimeType( "image/x-qpixmap" ) );

        // Send response with the data
        snapRequest->respond( snappedImage );

        // Reset snap mode
        setSnapMode( false );
        delete snapRequest;
        snapRequest = 0;

        // Finished serving QDS request so close the application
        close();
    } else {
        QContent f;
        QList<QString> c;

        f.setType("image/jpeg");
        f.setName(tr("Photo, %1","date")
                .arg(QTimeString::localYMD(QDate::currentDate(),QTimeString::Short)));
        f.setMedia( settings->location->installationPath() );

        c.append( camcat );
        f.setCategories(c);

        QImage  temp = img.convertToFormat(QImage::Format_RGB32);

        temp.save(f.file(),"JPEG",pquality);

        f.commit();

        pushThumb(f,img);
    }

    preview();
}

void CameraMainWindow::setSnapMode(bool snapMode)
{
    if (snapMode) {

        camera->thumbs->hide();
        camera->video->hide();

        // in snapshot mode, change back to "cancel taking a photo"
#ifdef QTOPIA_PHONE
        QSoftMenuBar::setLabel(this, Qt::Key_Back, QSoftMenuBar::Cancel);
#endif
    } else {

        camera->thumbs->show();

        if (video_supported)
            camera->video->show();
        else
            camera->video->hide();

        // normal back button
#ifdef QTOPIA_PHONE
        QSoftMenuBar::setLabel(this, Qt::Key_Back, QSoftMenuBar::Back);
#endif
    }
}

void CameraMainWindow::toggleVideo()
{
    if ( recording )
        stopVideo();
    else
        startVideo();
    recording = !recording;
    camera->video->setText(recording ? tr("Stop") : tr("Video"));
    camera->photo->setEnabled(!recording);
    for (int i=0; i<nthumb; i++)
        thumb[i]->setEnabled(!recording && !thumb[i]->icon().isNull());
}

void CameraMainWindow::startVideo()
{
    // XXX eg. MJPG
}
void CameraMainWindow::stopVideo()
{
}

void CameraMainWindow::thumbClicked(int i)
{
    selectThumb(i);
}

void CameraMainWindow::editThumb()
{
    picturefile[cur_thumb].execute();
}

void CameraMainWindow::selectThumb(int i)
{
    cur_thumb = i;
    if ( i >= 0 ) {
        QImage img( picturefile[i].file() );
        camera->videocaptureview->setStill(img);
        thumb[i]->setFocus();
    } else {
        camera->videocaptureview->setLive();
    }
    a_th_edit->setVisible(i>=0);
    a_th_del->setVisible(i>=0);
    a_th_add->setVisible(i>=0);
    a_send->setVisible(i>=0);
}

void CameraMainWindow::moveToContact()
{
    /*if ( cur_thumb >= 0 ) {
        QtopiaServiceRequest e("Contacts","setContactImage(QString)");
        e << picturefile[cur_thumb].file();
        e.send();
    }*/

    if ( cur_thumb >= 0 ) {
        // Find a suitable QDS service
        QDSServices services( QString( "image/x-qpixmap" ) );

        // Select the first service to create the action (assuming there
        // is only be one address book type application on the device)
        QDSAction action( services.findFirst( "setContactImage" ) );
        if ( !action.isValid() ) {
            qWarning( "Camera found no service to set the contact image" );
            return;
        }

        // Create a data object to encapsulate the picture, and send it using
        // the action.
        QByteArray pixArray;
        {
            QDataStream stream( &pixArray, QIODevice::WriteOnly );
            QPixmap pix( picturefile[cur_thumb].file() );
            stream << pix;
        }
        QDSData pixData( pixArray, QMimeType( "image/x-qpixmap" ) );

        if ( action.exec( pixData ) != QDSAction::Complete ) {
            qWarning( "Camera unable to set contact image" );
            return;
        }
    }
}

void CameraMainWindow::delThumb()
{
    if ( cur_thumb >= 0 ) {
        int d = cur_thumb;
        if ( cur_thumb > 0 )
            selectThumb(cur_thumb-1);
        delThumb(d);
    }
}

void CameraMainWindow::contentChanged(const QContentIdList&, QContent::ChangeType)
{
    loadThumbs();
}

void CameraMainWindow::getImage( const QDSActionRequest& request )
{
    if ( !request.isValid() ) {
        qWarning( "Camera: received invalid QDS request" );
        return;
    }

    if ( snapRequest != 0 ) {
        qWarning( "Camera: already processing another QDS request" );
        return;
    }

    // Read snap parameters from request
    QDataStream stream( request.requestData().toIODevice() );
    stream >> snap_max;

    // Set the camera for snap mode
    snapRequest = new QDSActionRequest( request );
    setSnapMode( true );
    showMaximized();
}

void CameraMainWindow::preview()
{
    if ( camera->videocaptureview->refocusDelay() > 200 ) {
        camera->videocaptureview->setCaptureSize( photo_size[psize] );
    } else {
        camera->videocaptureview->setCaptureSize( camera->videocaptureview->recommendedPreviewSize() );
    }
}

void CameraMainWindow::sendFile()
{
    if ( cur_thumb >= 0) {
        //copy file
        QFile input(picturefile[cur_thumb].file());
        if(!input.open(QIODevice::ReadOnly)){
            return; //error
        }
        QFile output(picfile);
        if(!output.open(QIODevice::WriteOnly)){
            return;
        }

        const int BUFFER_SIZE = 1024;
        qint8 buffer[BUFFER_SIZE];

        QDataStream srcStr(&input);
        QDataStream destStr(&output);

        while(!srcStr.atEnd()) {
            int i = 0;
            while(!srcStr.atEnd() && i < BUFFER_SIZE){
                srcStr >> buffer[i];
                i++;
            }
            for(int k = 0; k < i; k++) {
                destStr << buffer[k];
            }
        }

        QtopiaServiceRequest e("Email","writeMessage(QString,QString,QStringList,QStringList)");
        e << QString() << QString() << QStringList() << QStringList( QString( picfile ) );
        e.send();
    }
}

void CameraMainWindow::videoToScreen(int screen)
{
    QDesktopWidget *desktop = QApplication::desktop();
    if (desktop->screenNumber(camera->videocaptureview) == screen)
        return;

    if (screen == desktop->primaryScreen()) {
        videoOnSecondary = false;
        camera->videocaptureview->hide();
        camera->videocaptureview->setParent(camera->videocapturecontainer);
        camera->videocapturecontainer->layout()->addWidget(camera->videocaptureview);
        camera->videocaptureview->show();
    } else {
        videoOnSecondary = true;
        camera->videocaptureview->setFocusPolicy(Qt::NoFocus);
        camera->videocaptureview->setParent(0, Qt::Tool | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);
        camera->videocaptureview->setGeometry(desktop->screenGeometry(screen));
        camera->videocaptureview->show();
    }
}

void CameraMainWindow::clamshellChanged()
{
    if (clamshellVsi->value().toBool()) {
        videoToScreen(QApplication::desktop()->primaryScreen());
    } else {
        videoToScreen(1);
    }
}

void CameraMainWindow::contextMenuAboutToShow()
{
    m_contextMenuActive = true;
}

void CameraMainWindow::contextMenuAboutToHide()
{
    m_contextMenuActive = false;
}

void ThumbButton::drawButtonLabel( QPainter *p )
{
    QIcon ic = icon();
    if ( !ic.isNull() ) {
        ic.paint( p, rect() );
    }
}

ThumbButton::ThumbButton( QWidget *parent ) : QToolButton(parent)
{
}

/*!
    \service CameraService Camera
    \brief Provides the Qtopia Camera service.

    The \i Camera service enables applications to access features of
    the Camera application.
*/

/*!
    \internal
*/
CameraService::~CameraService()
{
}

/*!
    Instructs the camera to take a photo of the dimensions provided in \a request.

    This slot corresponds to a QDS service with a request data type of
    "x-size/x-qsize" and a response data type of "image/x-qpixmap".

    This slot corresponds to the QCop service message
    \c{Camera::getImage(QDSActionRequest)}.
*/
void CameraService::getImage( const QDSActionRequest& request )
{
    parent->getImage( request );
}

/*!
  Instructs the camera to activate the shutter.  If the application isn't visible, this
  raise the camera and puts it into photo mode.  Otherwise it takes a photo.
*/

void CameraService::shutter() {
    if (parent->isVisible() && parent->isActiveWindow())
        parent->takePhoto();
    else {
        parent->showMaximized();
        parent->raise();
        parent->activateWindow();
    }
}
