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

#include "storagemonitor.h"
#include <QMessageBox>

#ifdef QTOPIA_PHONE
# include "phone/phonelauncher.h"
# include "phone/homescreen.h"
# include <qsoftmenubar.h>
# include <qvaluespace.h>
#endif

#include <qtopiaipcenvelope.h>
#include <qtopiaservices.h>
#include <qstorage.h>

#include <QSettings>
#include <qtopiaapplication.h>
#include <QTimer>

#ifdef QTOPIA_PHONE
class StorageAlertBox : public QMessageBox
{
    public:
        StorageAlertBox(const QString &caption, const QString &text, Icon icon,
                int button0, int button1, int button2,
                QWidget *parent = 0, Qt::WFlags f = Qt::MSWindowsFixedSizeDialogHint)
           : QMessageBox(caption, text, icon, button0, button1, button2, parent, f)
        {
        };
    protected:
        void showEvent(QShowEvent *se) {
            QSoftMenuBar::setLabel(this, Qt::Key_Back, "trash", "");
            QMessageBox::showEvent(se);
        };

};
#endif

/*!
  \class StorageMonitor
  \ingroup QtopiaServer::Task
  \brief The StorageMonitor class periodically checks filesystems for available space and prompts to start the cleanup wizard if low.
  */

/*! \internal */
StorageMonitor::StorageMonitor(QObject *o)
: QObject(o), sinfo(0), box(0), suppressNotification(false)
{
    QSettings cfg("Trolltech","qpe");
    cfg.beginGroup("StorageMonitor");
    if (!cfg.value("Enabled", true).toBool()) {
        QtopiaServerApplication::taskValueSpaceSetAttribute("StorageMonitor",
                                                            "InUse",
                                                            "No");
        return;
    } else {
        QtopiaServerApplication::taskValueSpaceSetAttribute("StorageMonitor",
                                                            "InUse",
                                                            "Yes");
    }


    sinfo = new QStorageMetaInfo( this );
    QtopiaChannel *channel = new QtopiaChannel("QPE/System", this);
    connect(channel, SIGNAL(received(const QString&,const QByteArray&)),
            this, SLOT(systemMsg(const QString&,const QByteArray&)) );

    QTimer *storageTimer = new QTimer( this );
    connect( storageTimer, SIGNAL(timeout()), this, SLOT(checkAvailStorage()));
    storageTimer->start(cfg.value("UpdatePeriod", 15).toInt()*1000);
}

/*! \internal */
StorageMonitor::~StorageMonitor()
{
    if (box)
        delete box;
}

/*! \internal */
long StorageMonitor::fileSystemMetrics(const QFileSystem *fs)
{
    long mult = 0;
    long div = 0;
    if (fs == 0)
        return 0;

    if ( fs->blockSize() ) {
        mult = fs->blockSize() / 1024;
        div = 1024 / fs->blockSize();
    }
    if ( !mult ) mult = 1;
    if ( !div ) div = 1;

    return (fs->availBlocks() * mult / div);
}

/*! \internal */
void StorageMonitor::checkAvailStorage()
{
    if (suppressNotification)
        return;

    bool haveWritableFS = false;
    long availStorage = 0;

    QFileSystemFilter fsf;
    fsf.documents = QFileSystemFilter::Set;
    foreach ( QFileSystem *fs, sinfo->fileSystems(&fsf) ) {
        availStorage += fileSystemMetrics(fs);
        if( fs->isWritable() )
            haveWritableFS = true;
    }

    if( !haveWritableFS )
        return; // no writable filesystems, lack of free space is irrelevant

    //for now read config file each time until we have notification in place
    QSettings cfg("Trolltech","qpe");
    cfg.beginGroup("StorageMonitor");
    int notificationLimit = cfg.value("MinimalStorageLimit", 20).toInt();
    if (notificationLimit >= 0 && availStorage <= notificationLimit ) {
        QString msg = tr("<qt>The device has no free storage space. "
               "Please delete unwanted documents.</qt>");
        outOfSpace(msg);
    }
}

/*! \internal */
void StorageMonitor::systemMsg(const QString &msg, const QByteArray &data)
{
    QDataStream stream( data );

    if ( msg == "outOfDiskSpace(QString)" ) {
        QString text;
        stream >> text;
        if (!suppressNotification)
            outOfSpace( text );
    } else if ( msg == "checkDiskSpace()" ) {
        checkAvailStorage();
    }

}

static bool existingMessage = false;

void StorageMonitor::outOfSpace(QString &msg)
{
    if (existingMessage)
        return;

    existingMessage = suppressNotification = true;

    // start cleanup wizard
    if (!box) {
#ifdef QTOPIA_PHONE
        box = new StorageAlertBox(tr("Out of storage space"), "", QMessageBox::Critical,
                QMessageBox::Ok | QMessageBox::Default,
                QMessageBox::No, QMessageBox::NoButton);
#else
        box = new QMessageBox(tr("Out of storage space"), "", QMessageBox::Critical,
                QMessageBox::Ok | QMessageBox::Default,
                QMessageBox::No, QMessageBox::NoButton);
#endif
        box->setButtonText(QMessageBox::No, tr("Cleanup"));
    }
    box->setText(msg);
#ifdef QTOPIA_PHONE
    switch (QtopiaApplication::execDialog(box)) {
#else
    switch (box->exec()) {
#endif
        case QMessageBox::Ok:
            break;
        case QMessageBox::No:
            showCleanupWizard();
            break;
    }

    //suppress next msg for 30 mins to avoid annoying msgbox
    QTimer::singleShot(30*60*1000, this, SLOT(showNotification()));
    existingMessage = false;
}

void StorageMonitor::showNotification()
{
    suppressNotification = false;
}

void StorageMonitor::showCleanupWizard()
{
    QtopiaServiceRequest req("CleanupWizard", "showCleanupWizard()");
    req.send();
}

QTOPIA_TASK(StorageMonitor, StorageMonitor);
