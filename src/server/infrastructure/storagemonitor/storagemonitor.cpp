/****************************************************************************
**
** This file is part of the Qt Extended Opensource Package.
**
** Copyright (C) 2008 Trolltech ASA.
**
** Contact: Qt Extended Information (info@qtextended.org)
**
** This file may be used under the terms of the GNU General Public License
** version 2.0 as published by the Free Software Foundation and appearing
** in the file LICENSE.GPL included in the packaging of this file.
**
** Please review the following information to ensure GNU General Public
** Licensing requirements will be met:
**     http://www.fsf.org/licensing/licenses/info/GPLv2.html.
**
**
****************************************************************************/

#include "storagemonitor.h"
#include "outofspace.h"

#include <QtopiaTimer>
#include <qvaluespace.h>
#include "qtopiaserverapplication.h"
#include <qtopiaipcenvelope.h>
#include <qtopiaservices.h>
#include <qstorage.h>
#include <QSettings>
#include <QTimer>

/*!
    \class StorageMonitor
    \inpublicgroup QtBaseModule
    \ingroup QtopiaServer::Task
    \brief The StorageMonitor class periodically checks filesystems for available space and prompts to start the cleanup wizard if low.

    Periodically the StorageMonitor class examines the writable filesystems for
    free space.  The polling frequency can be controlled through configuration.
    If the amount of free space, summed across all writable filesystems, is found
    to be below a configurable limit, a dialog box is displayed prompting the
    user to delete unwanted documents.  If an application providing the
    \c {CleanupWizard} service is installed, the user can also directly launch
    the cleanup wizard.

    As it is primarily a graphical feature, to preserve power the storage monitor
    suspends polling while the device's screen is turned off, on the rationale
    that the user is probably not present or interacting with the device.

    The StorageMonitor class is configured through the \c {Trolltech/qpe} settings
    file.  The following keys control the configuration.

    \table
    \header \o Key \o Description
    \row \o \c {StorageMonitor/Enabled} \o Set to false if the storage monitor is to be disabled.  By default this value is true and the storage monitor is enabled.
    \row \o \c {StorageMonitor/UpdatePeriod} \o The rate, in seconds, at which the storage monitor polls the filesystems.  By default this is 60 seconds.
    \row \o \c {StorageMonitor/MinimalStorageLimit} \o When the total free space summed across all writable filesystems is below this value, in percent, the storage monitor dialog will be triggered.  By default this is 10 percent.
    \endtable

    Polls, by default, every 60 seconds, but pauses when screen is off or there
    are no writable file systems.  Checks for CleanupWizard service.

    This class is part of the Qt Extended server and cannot be used by other Qt Extended applications.
*/

/*! \internal */
StorageMonitor::StorageMonitor(QObject *o)
:   QObject(o), sinfo(0), box(0), storageTimer(0),
    minimalStorageLimit(-1), pollFrequency(-1),
    channel(0)
{
    QSettings cfg("Trolltech","qpe");
    cfg.beginGroup("StorageMonitor");
    setEnabled(cfg.value("Enabled", true).toBool());
}

/*! \internal */
StorageMonitor::~StorageMonitor()
{
}

/*! \internal */
void StorageMonitor::fileSystemMetrics(const QFileSystem *fs, long &available, long &total)
{
    long mult = 0;
    long div = 0;
    if (fs == 0)
        return;

    if ( fs->blockSize() ) {
        mult = fs->blockSize() / 1024;
        div = 1024 / fs->blockSize();
    }
    if ( !mult ) mult = 1;
    if ( !div ) div = 1;

    available += fs->availBlocks() * mult / div;
    total += fs->totalBlocks() * mult / div;
}

/*! \internal */
void StorageMonitor::checkAvailStorage()
{
    bool haveWritableFS = false;
    long availStorage = 0;
    long totalStorage = 0;

    QFileSystemFilter fsf;
    fsf.documents = QFileSystemFilter::Set;
    foreach ( QFileSystem *fs, sinfo->fileSystems(&fsf) ) {
        if( fs->isWritable() ) {
            fileSystemMetrics(fs, availStorage, totalStorage);
            haveWritableFS = true;
        }
    }
    if( !haveWritableFS ) {
        return; // no writable filesystems, lack of free space is irrelevant
    }

    //for now read config file each time until we have notification in place
    if(minimalStorageLimit < 0) {
        QSettings cfg("Trolltech","qpe");
        cfg.beginGroup("StorageMonitor");
        minimalStorageLimit = cfg.value("MinimalStorageLimit", 10).toInt();
    }

    int percentAvailable = availStorage / (totalStorage/100);
    if (minimalStorageLimit >= 0 && percentAvailable <= minimalStorageLimit) {
        QString msg = tr("Less than %1% of the available storage space is free. "
                         "Please delete unwanted documents.").arg(percentAvailable);
        outOfSpace(msg);
    } else {
        storageTimer->start(pollFrequency, QtopiaTimer::PauseWhenInactive);
    }
}

/*! \internal */
void StorageMonitor::systemMsg(const QString &msg, const QByteArray &data)
{
    QDataStream stream( data );

    if ( msg == "outOfDiskSpace(QString)" ) {
        QString text;
        stream >> text;
        outOfSpace(text);
    } else if ( msg == "checkDiskSpace()" ) {
        checkAvailStorage();
    }
}

void StorageMonitor::outOfSpace(QString &msg)
{
    if (box)
        return;

    box = new OutOfSpace(msg);

    int disableFor = 0;

    QSettings cfg("Trolltech","qpe");
    cfg.beginGroup("StorageMonitor");

    switch (QtopiaApplication::execDialog(box)) {
    case OutOfSpace::CleanupNow:
        showCleanupWizard();
        break;
    case OutOfSpace::HourDelay:
        disableFor = 60 * 60 * 1000;
        break;
    case OutOfSpace::DayDelay:
        disableFor = 24 * 60 * 60 * 1000;
        break;
    case OutOfSpace::WeekDelay:
        disableFor = 7 * 24 * 60 * 60 * 1000;
        break;
    case OutOfSpace::Never:
        setEnabled(false);
        cfg.setValue("Enabled", false);
        break;
    default:
        storageTimer->start(pollFrequency, QtopiaTimer::PauseWhenInactive);
    };

    if (disableFor) {
        storageTimer->start(disableFor, QtopiaTimer::PauseWhenInactive);
        cfg.setValue("DisableUntil", QDateTime::currentDateTime().addMSecs(disableFor).toTime_t());
    }

    delete box;
    box = 0;
}

void StorageMonitor::availableDisksChanged()
{
    // QStorageMonitor emits disksChanged() signal twice during startup and after
    // waking up from suspend.  We don't want the "out of space" message shown again
    // within a short period of time the minimal delay should be the current
    // storageTimer interval or pollFrequency
    const int minimumDelay = qMax(pollFrequency, storageTimer ? storageTimer->interval() : 0);

    if (metaInfoUpdate.secsTo(QTime::currentTime())*1000 > minimumDelay)
        checkAvailStorage();

    metaInfoUpdate = QTime::currentTime();
}

void StorageMonitor::showCleanupWizard()
{
    QtopiaServiceRequest req("CleanupWizard", "showCleanupWizard()");
    req.send();
}

void StorageMonitor::setEnabled(bool enabled)
{
    QtopiaServerApplication::taskValueSpaceSetAttribute("StorageMonitor", "InUse",
                                                        enabled ? "Yes" : "No");

    if (enabled) {
        QSettings cfg("Trolltech", "qpe");
        cfg.beginGroup("StorageMonitor");

        pollFrequency = cfg.value("UpdatePeriod", 60).toInt() * 1000;

        sinfo = QStorageMetaInfo::instance();
        metaInfoUpdate = QTime::currentTime();
        channel = new QtopiaChannel("QPE/System", this);
        connect(channel, SIGNAL(received(QString,QByteArray)),
                this, SLOT(systemMsg(QString,QByteArray)) );

        uint disableUntil = cfg.value("DisableUntil",
                                        QDateTime::currentDateTime().toTime_t()).toUInt();
        int disableFor = QDateTime::currentDateTime().secsTo(
                            QDateTime::fromTime_t(disableUntil)) * 1000;

        storageTimer = new QtopiaTimer(this);
        storageTimer->setSingleShot(true);
        connect(storageTimer, SIGNAL(timeout()), this, SLOT(checkAvailStorage()));
        storageTimer->start(qMax(disableFor, pollFrequency), QtopiaTimer::PauseWhenInactive);

        connect(sinfo, SIGNAL(disksChanged()), this, SLOT(availableDisksChanged()));
    } else {
        if (channel) {
            delete channel;
            channel = 0;
        }
        if (storageTimer) {
            delete storageTimer;
            storageTimer = 0;
        }
        disconnect(sinfo, SIGNAL(disksChanged()), this, SLOT(availableDisksChanged()));
    }
}

QTOPIA_TASK(StorageMonitor, StorageMonitor);
