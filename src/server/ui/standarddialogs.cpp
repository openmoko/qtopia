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

#include "standarddialogs.h"
#include "criticalmemorypopup.h"
#include "desktoppoweralerter.h"
#include "shutdownimpl.h"
#include "applicationlauncher.h"
#include "qterminationhandlerprovider.h"
#include "startupapps.h"
#include <QContent>
#include <QContentSet>
#include <QPowerStatus>
#include <QPowerStatusManager>
#include <QtopiaServiceRequest>

static bool criticalMemoryPopup = true;
static bool shutdownDialog = true;
static bool defaultCrashDialog = true;
static bool preloadCrashDialog = true;
static bool powerAlertDialog = true;
static bool terminationHandlerDialog = true;

// declare StandardDialogsImpl
class StandardDialogsImpl : public QObject
{
Q_OBJECT
public:
    StandardDialogsImpl(QObject *parent = 0);
    virtual ~StandardDialogsImpl();

    void disableCriticalMemoryPopup();
    void disablePowerAlert();
    void disableTerminationHandlerDialog();

private slots:
    void shutdownRequested();
    void applicationTerminated(const QString &name,
                               ApplicationTypeLauncher::TerminationReason,
                               bool filtered);
    void preloadTerminated(const QString &name);
    void powerTimeout();
    void applicationTerminated(const QString &name, const QString &text,
                               const QPixmap &icon, const QString &buttonText,
                               QtopiaServiceRequest &buttonAction);

private:
    CriticalMemoryPopup *cmi;
    DesktopPowerAlerter *dpa;
    QTerminationHandlerProvider *thp;
};

// define StandardDialogsImpl
static StandardDialogsImpl *sdi = 0;

StandardDialogsImpl::StandardDialogsImpl(QObject *parent)
: QObject(parent), cmi(0), dpa(0), thp(0)
{
    sdi = this;

    if(criticalMemoryPopup) {
        cmi = new CriticalMemoryPopup;
    }

    if(shutdownDialog) {
        QObject::connect(QtopiaServerApplication::instance(), SIGNAL(shutdownRequested()), this, SLOT(shutdownRequested()));
    }

    if(defaultCrashDialog) {
        ApplicationLauncher *launcher = qtopiaTask<ApplicationLauncher>();
        if(launcher)
            QObject::connect(launcher, SIGNAL(applicationTerminated(const QString &, ApplicationTypeLauncher::TerminationReason,bool)), this, SLOT(applicationTerminated(const QString &, ApplicationTypeLauncher::TerminationReason,bool)));
    }

    if(preloadCrashDialog) {
        StartupApplications *apps = qtopiaTask<StartupApplications>();
        if(apps)
            QObject::connect(apps, SIGNAL(preloadCrashed(const QString &)), this, SLOT(preloadTerminated(const QString &)));
    }

    if(powerAlertDialog) {
        dpa = new DesktopPowerAlerter(0);
        QTimer *timer = new QTimer(this);
        QObject::connect(timer, SIGNAL(timeout()), this, SLOT(powerTimeout()));
        timer->start(10000);
    }

    if(terminationHandlerDialog) {
        thp = new QTerminationHandlerProvider(this);
        QObject::connect(thp, SIGNAL(applicationTerminated(QString,QString,QPixmap,QString,QtopiaServiceRequest&)), this, SLOT(applicationTerminated(QString,QString,QPixmap,QString,QtopiaServiceRequest&)));
    }
}

StandardDialogsImpl::~StandardDialogsImpl()
{
    if(dpa) { delete dpa; dpa = 0; }
    if(cmi) { delete cmi; cmi = 0; }
}

void StandardDialogsImpl::powerTimeout()
{
    QTimer *timer = static_cast<QTimer *>(sender());
    Q_ASSERT(timer);
    if(!dpa) {
        timer->stop();
    }

    QPowerStatus status = QPowerStatusManager::readStatus();
    if((status.batteryStatus() == QPowerStatus::VeryLow)) {
        dpa->alert( tr( "Battery is running very low." ), 6 );
    }

    if(status.batteryStatus() == QPowerStatus::Critical ) {
        dpa->alert(  tr( "Battery level is critical!\n"
                        "Please recharge the main battery!" ), 1 );
    }
    if (status.backupBatteryStatus() == QPowerStatus::VeryLow ) {
        dpa->alert( tr( "Back-up battery is very low.\nPlease charge the back-up battery." ), 3 );
    }
}

void StandardDialogsImpl::applicationTerminated(
        const QString &, const QString &text,
        const QPixmap &icon, const QString &buttonText,
        QtopiaServiceRequest &buttonAction)
{
    bool executeAction = !buttonAction.isNull();
    if( !text.isEmpty() ) {
        bool ba = !buttonAction.isNull() && !buttonText.isEmpty();
        QMessageBox* box = new QMessageBox(tr("Application terminated"), text, QMessageBox::Critical, QMessageBox::Ok | QMessageBox::Default, ba ? QMessageBox::No : QMessageBox::NoButton, QMessageBox::NoButton);

        if(ba)
            box->setButtonText(QMessageBox::No, buttonText);
        if(!icon.isNull() )
            box->setIconPixmap(icon);

        executeAction = box->exec() == QMessageBox::No;
    }

    if(executeAction)
        buttonAction.send();
}

void StandardDialogsImpl::disableTerminationHandlerDialog()
{
    if(thp) { delete thp; thp = 0; }
}

void StandardDialogsImpl::disablePowerAlert()
{
    if(dpa) { delete dpa; dpa = 0; }
}

void StandardDialogsImpl::disableCriticalMemoryPopup()
{
    if(cmi) { delete cmi; cmi = 0; }
}

void StandardDialogsImpl::shutdownRequested()
{
    if(shutdownDialog) {
        ShutdownImpl *sd = new ShutdownImpl(0);
        sd->setAttribute(Qt::WA_DeleteOnClose);
        connect(sd, SIGNAL(shutdown(QtopiaServerApplication::ShutdownType)),
                QtopiaServerApplication::instance(), SLOT(shutdown(QtopiaServerApplication::ShutdownType)));
        sd->showMaximized();
    }
}

void StandardDialogsImpl::applicationTerminated(const QString &name,
        ApplicationTypeLauncher::TerminationReason reason,
        bool filtered)
{
    if(ApplicationTypeLauncher::Normal == reason || !defaultCrashDialog || filtered)
        return;

    QContentFilter filter(QContent::Application);
    QContentSet set(filter);
    QContent app = set.findExecutable(name);
    if(app.id() == QContent::InvalidId || !app.isValid()) return;
    QString appname = Qtopia::dehyphenate(app.name());

    QMessageBox::information(0, tr("Application terminated"), tr("<qt><b>%1</b> was terminated due to application error.</qt>").arg(appname));
}

void StandardDialogsImpl::preloadTerminated(const QString &name)
{
    if(!preloadCrashDialog) return;

    QContentFilter filter(QContent::Application);
    QContentSet set(filter);
    QContent app = set.findExecutable(name);
    if(!app.isValid()) return;
    QString appname = Qtopia::dehyphenate(app.name());

    QMessageBox::information(0, tr("Application terminated"), tr("<qt><b>%1</b> was terminated due to application error.  (Fast loading has been disabled for this application. Tap and hold the application icon to reenable it.)</qt>").arg(appname));
}

QTOPIA_TASK(StandardDialogs, StandardDialogsImpl);


// define StandardDialogs
/*!
  \namespace StandardDialogs
  \ingroup QtopiaServer
  \brief The StandardDialogs namespace contains methods to disable Qtopia's standard dialog popups.

  The core Qtopia sub-systems often generate information that needs to be
  displayed to the user.  To ease customizability, all the simple dialogs and
  notifications in Qtopia are collectively managed in one place - the
  \c {StandardDialogs} server task.

  To customize one of these dialogs, an integrator needs only to implement their
  own version and disable Qtopia's default behaviour through one of the methods
  available in the StandardDialogs namespace.

  The documentation for each of the following methods lists both what the
  dialog it disables is, as well as the system behaviour to which it responds.
  The intent is to make the task of replicating each very simple.
 */

/*!
  The critical memory popup dialog appears whenever the memory available to the
  device becomes dangerously low.  The default implementation is a standard
  QMessageBox alerting the user to close any unnecessary applications.

  The dialog appears 20 seconds after the MemoryMonitor::memoryState() becomes
  MemoryMonitor::MemVeryLow.  The 20 second delay is an attempt to prevent
  the dialog from exacerbating the situation.

  Invoking this method will disable the default critical memory popup.
 */
void StandardDialogs::disableCriticalMemoryPopup()
{
    criticalMemoryPopup = false;
    if(sdi) sdi->disableCriticalMemoryPopup();
}

/*!
  The shutdown dialog allows users to terminate Qtopia, restart Qtopia, shutdown
  the system or reboot the system.  The default implementation is a simple
  dialog allowing the user to select from one of these four options.

  The shutdown dialog appears in response to the
  QtopiaServerApplication::shutdownRequested() signal.  Once the user has
  selected their desired option, the dialog invokes the
  QtopiaServerApplication::shutdown() method and closes.

  Invoking this method will disable the shutdown dialog.
 */
void StandardDialogs::disableShutdownDialog()
{
    shutdownDialog = false;
}

/*!
  The default crash handler dialog appears whenever an unfiltered application
  crash occurs.  An application crash may be filtered by the
  ApplicationTerminationHandler interface.  The default implementation is a
  standard QMessageBox alerting the user to the name of the crashed application.

  The dialog appears whenever the ApplicationLauncher::applicationTerminated()
  signal is emitted with the filtered parameter set to false.

  Invoking this method will disable the default crash dialog.
 */
void StandardDialogs::disableDefaultCrashDialog()
{
    defaultCrashDialog = false;
}

/*!
  The preload crash dialog appears whenever a preloaded application crashes.
  The default implementation is a standard QMessageBox alerting the user to the
  name of the crashed application, and informing them that it has been removed
  from the preload list.

  The dialog appears whenever the StartupApplications::preloadCrashed() signal
  is emitted.

  Invoking this method will diable the preload crash dialog.
 */
void StandardDialogs::disablePreloadCrashDialog()
{
    preloadCrashDialog = false;
}

/*!
  The power alert dialog appears whenever the device battery runs low.  The
  default implementation is a standard QMessageBox alerting the user to the
  low power situation.

  The dialog appears:
  \list 1
  \i Immediately when the QPowerStatus::batteryStatus() is reported as Critical.
  \i 30 seconds after the QPowerStatus::backupBatteryStatus() is reported as
     VeryLow.
  \i 60 seconds after the QPowerStatus::batteryStatus() is reported as VeryLow.
  \endlist

  The dialog will be redisplayed every five minutes until the power state
  changes or a higher priority (as listed) case occurs.  The default
  implementation uses the QPowerStatusManager to determine this information.

  Invoking this method will disable the power alert dialog.
*/
void StandardDialogs::disablePowerAlertDialog()
{
    powerAlertDialog = false;
    if(sdi) sdi->disablePowerAlert();
}

/*!
  Applications can install custom termination messages using the
  QTerminationHandler class.  The default implementation displays a standard
  QMessageBox alerting th euser to the termination and the specified reason.

  The dialog appears whenever the QTerminationHandlerProvider class emits the
  QTerminationHandlerProvider::applicationTerminated() signal.

  Invoking this method will disable the default termination handler dialog.
 */
void StandardDialogs::disableTerminationHandlerDialog()
{
    terminationHandlerDialog = false;
    if(sdi) sdi->disableTerminationHandlerDialog();
}

#include "standarddialogs.moc"
