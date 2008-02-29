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
#include "qtopiapowermanager.h"

#include <QDebug>
#include <QList>
#include <QMap>
#include <QSettings>
#include <QtAlgorithms>
#include <QPowerStatus>
#include <QPowerStatusManager>
#include <QtopiaServiceRequest>
#include <qtopianetwork.h>
#include <qtopiaipcenvelope.h>
#include <qvaluespace.h>
#include <qwindowsystem_qws.h>
#include "applicationlauncher.h"
#include "systemsuspend.h"
#include "qtopiapowermanagerservice.h"

#if 0
class ScreenSaverPrivate {
public:
    enum QSAction {
        DimLight = 0,       //dim the background light
        LightOff = 1,       //turn light off
        Suspend = 2,        //suspend device (uses APM)
        HomeScreen = 3    //go back to HomeScreen
        //AppShutdown = 4    //shutdown running applications
    };

};
#endif

static bool forced_off = false;
static int currentBacklight = -1;
QValueSpaceObject *QtopiaPowerManager::m_vso = 0;

/*  Apply light/power settings for current power source */
static void applyLightSettings(QPowerStatus *p)
{
    int initbright, intervalDim, intervalLightOff;
    bool dim, lightoff;
#ifdef QTOPIA_PHONE
    bool home;
    int intervalHome;
#else
    bool suspend;
    int intervalSuspend;
#endif

    {
        QSettings config("Trolltech","qpe");
        bool defsus;
        if ( p->acStatus() == QPowerStatus::Online ) {
            config.beginGroup("ExternalPower");
            defsus = false;
        } else {
            config.beginGroup("BatteryPower");
            defsus = true;
        }

        intervalDim = config.value( "Interval_Dim", 20 ).toInt();
        config.setValue("Interval_Dim", intervalDim);
        intervalLightOff = config.value("Interval_LightOff", 30).toInt();
        config.setValue("Interval_LightOff", intervalLightOff);
        initbright = config.value("Brightness", 255).toInt();
        config.setValue("Brightness", initbright);
        dim = config.value("Dim", true).toBool();
        config.setValue("Dim", dim);
        lightoff = config.value("LightOff", false ).toBool();
        config.setValue("LightOff", lightoff);
#ifndef QTOPIA_PHONE
        intervalSuspend = config.value("Interval", 240).toInt();
        config.setValue("Interval", intervalSuspend);
        suspend = config.value("Suspend", defsus ).toBool();
        config.setValue("Suspend", suspend);
#else
        config.endGroup();
        config.beginGroup("HomeScreen");
        intervalHome = config.value( "Interval_HomeScreen", 300 ).toInt();
        home = config.value( "ShowHomeScreen", false ).toBool();
        config.setValue("Interval_HomeScreen", intervalHome);
        config.setValue("ShowHomeScreen", home);
#endif
        config.sync(); //write out for syncronisation with light app
    }

    int i_dim =      (dim ? intervalDim : 0);
    int i_lightoff = (lightoff ? intervalLightOff : 0);

#ifndef QTOPIA_PHONE
    int i_suspend =  (suspend ? intervalSuspend : 0);
#else
    int i_home = ( home ? intervalHome : 0);
#endif

    QtopiaServiceRequest eB("QtopiaPowerManager", "setBacklight(int)" );
    eB << -3; //forced on
    eB.send();

    QtopiaServiceRequest e("QtopiaPowerManager", "setIntervals(int,int,int)" );
#ifndef QTOPIA_PHONE
    e << i_dim << i_lightoff << i_suspend;
#else
    e << i_dim << i_lightoff << i_home;
#endif
    e.send();
}

//--------------------------------------------------
// customised power management for Phone and PDA

static QtopiaPowerManager *g_qtopiaPowerManager = 0;

/*!
  \internal

  Constructor.
  */
QtopiaPowerManager::QtopiaPowerManager() : m_powerConstraint(QtopiaApplication::Enable), m_dimLightEnabled(true), m_lightOffEnabled(true) {
    g_qtopiaPowerManager = this;
    if (!m_vso)
        m_vso = new QValueSpaceObject("/Hardware/Display");
    setBacklight(-3); //forced on

    // Create the screen saver and the associated service.
    QWSServer::setScreenSaver(this);
    (void)new QtopiaPowerManagerService( this, this );

    QtopiaPowerConstraintManager *tsmMonitor = new QtopiaPowerConstraintManager(this);
    connect( tsmMonitor, SIGNAL(forceSuspend()),
             qtopiaTask<SystemSuspend>(), SLOT(suspendSystem()) );
}

/*!
  \reimp
  */
void QtopiaPowerManager::powerStatusChanged(const QPowerStatus &ps)
{
    QPowerStatus status = ps;
    applyLightSettings(&status);
}
/*!
    \fn void QtopiaPowerManager::setIntervals(int* a, int size)

    This function sets the internal timeouts for the power manager.
    It expects an array \a a containing the timeout values and \a size
    being the number of entries in \a a. This allows any arbitrary number of
    power saving levels.

    This function needs to be reimplemented by subclasses.
*/
void QtopiaPowerManager::setIntervals(int *v, int size)
{
    Q_UNUSED(v);
    Q_UNUSED(size);
}

/*!
    \fn void QtopiaPowerManager::setDefaultIntervals()

    Resets the intervals to the default configuration for the power manager.
    The default values are defined in configuration files and can be edited
    by the user using the Power Management application.
*/
void QtopiaPowerManager::setDefaultIntervals() {
#ifdef QTOPIA_PHONE
    int v[4];
    v[3]=-1;
#else
    int v[3];
#endif
    v[0]=-1;
    v[1]=-1;
    v[2]=-1;
    setIntervals(v, sizeof(v)/sizeof(int));
}

/*! \internal

   Returns \a interval if the given power manager setting is enabled. If the given
   interval is <0 it returns the value given by \a cfg.
 */
int QtopiaPowerManager::interval(int interval, QSettings& cfg, const QString &enable,
        const QString& value, int deflt)
{
    if ( !enable.isEmpty() && cfg.value(enable,false).toBool() == false )
        return 0;

    if ( interval < 0 ) {
        // Restore screen blanking and power saving state
        interval = cfg.value( value, deflt ).toInt();
    }
    return interval;
}

/*!
  \internal

  Sets the back light to \a bright.

  \sa backlight
  */
void QtopiaPowerManager::setBacklight(int bright)
{
    if ( bright == -3 ) {
        // Forced on
        forced_off = false;
        bright = -1;
    }
    if ( forced_off && bright != -2 )
        return;
    if ( bright == -2 ) {
        // Toggle between off and on
        bright = currentBacklight ? 0 : -1;
        forced_off = !bright;
    }
    if ( bright == -1 ) {
        // Read from config
        QSettings config("Trolltech","qpe");
        QPowerStatus ps = QPowerStatusManager::readStatus();
        if (ps.acStatus() == QPowerStatus::Online)
            config.beginGroup( "ExternalPower" );
        else
            config.beginGroup( "BatteryPower" );
        bright = config.value("Brightness",255).toInt();
    }
    qpe_setBrightness(bright);
    currentBacklight = bright;

    if (m_vso)
        m_vso->setAttribute("0/Backlight", bright);
}

/*!
  \internal

  Returns the current level of the backlight. The return value has a range between
  0 and 255 ( 255 being the brightest backlight setting ).

  \sa setBacklight
  */
int QtopiaPowerManager::backlight()
{
    if (currentBacklight == -1) {
        QSettings config("Trolltech","qpe");
        QPowerStatus ps = QPowerStatusManager::readStatus();
        if (ps.acStatus() == QPowerStatus::Online)
            config.beginGroup( "ExternalPower" );
        else
            config.beginGroup( "BatteryPower" );
        currentBacklight = config.value("Brightness", 255).toInt();
    }
    return currentBacklight;
}

/*!
  Activates or deactivates the power manager given the flag \a on.
  When activated, power management is enabled. When deactivated, power management is completely disabled.
  */
void QtopiaPowerManager::setActive(bool on)
{
    QWSServer::screenSaverActivate(on);
}

/*!
  \internal

  Applies the power constraint \a c to this power manager.
  */
void QtopiaPowerManager::setConstraint(QtopiaApplication::PowerConstraint c)
{
    m_powerConstraint = c;
}

/*!
    \fn void QtopiaPowerManager::restore()

    Restores the state of the device when power saving is active. This
    usually happens when the user interacts with the device.
*/
void QtopiaPowerManager::restore()
{
    if ( backlight() <= 1 ) //if dimmed or off
        setBacklight(-1);
}

/*!
    \fn bool QtopiaPowerManager::save(int level)

    This function is called by Qtopia when a timeout has occurred
    and dynamically maps \a level onto a power saving action.

    This function needs to be reimplemented by subclasses.
*/
bool QtopiaPowerManager::save(int level)
{
    Q_UNUSED(level);
    return false;
}

//------------------------------------------------------------

/*!
  \class QtopiaPowerManager
  \ingroup QtopiaServer
  \brief The QtopiaPowerManager class implements default device power management behaviour.
   Qtopia provides an implementation for a phone and PDA device.
   These implementations support the following three timeouts and actions.

   PhonePowerManager (phone/phonepowermanager.cpp)
   \list
    \o show homescreen (and close open applications)
    \o dim backlight
    \o turn backlight off
   \endlist

   PDAPowerManager (pda/pdapowermanager.cpp)
   \list
    \o dim backlight
    \o turn backlight off
    \o suspend device (apm suspend)
   \endlist

   To extend Qtopia's default behaviour it is necessary to subclass either PhonePowerManager or PDAPowerManager.
   A minimal subclass has to reimplement the following functions:

   \list
   \o \c{setIntervals(int*, int)}
   \o \c{save(int)}
   \endlist

   \sa setIntervals(), save()
*/


// XXX : not used?
//void qpe_setBacklight (int bright) { ScreenSaver::setBacklight(bright); }

#if 0

/*!
    \fn ScreenSaver* ScreenSaver::createScreenSaver()

    Returns a new instance of the ScreenSaver. Any existing screen saver instance
    is deleted by QWSServer when we install the new instance. QWSServerPrivate
    automatically takes ownership of the screen saver pointer when calling
    QWSServer::setScreenSaver.
*/

ScreenSaver * ScreenSaver::createScreenSaver()
{
    // It is assumed that the returned pointer is used to call
    // QWSServer::setScreenSaver(). The QWSServer takes ownership of the
    // instance and deletes it if necessary (see qwindowsystem_qws.cpp).
#ifdef QTOPIA_PHONE
    return new QPhoneScreenSaver;
#else
    return new QPDAScreenSaver;
#endif
}
#endif



/**********************************************************/

#ifdef QTOPIA_MAX_SCREEN_DISABLE_TIME
#define QTOPIA_MIN_SCREEN_DISABLE_TIME ((int) 300)  // min 5 minutes before forced suspend kicks in
#endif

static QtopiaPowerConstraintManager *g_managerinstance= 0;

/*!
  \ingroup QtopiaServer
    \class QtopiaPowerConstraintManager
    \brief The QtopiaPowerConstraintManager class keeps track of power management constraints set by Qtopia applications.

    In some use cases Qtopia applications may want to disable
    power saving acitivities in order to perform their tasks (e.g. videos app
    disables the dimming of the backlight when it plays a video). This monitor
    keeps track of all of all constraints set by applications and applies a common denominator
    to the QtopiaPowerManager .

    QtopiaPowerConstraintManager is a singleton, which you can access through QtopiaPowerConstraintManager::instance().
*/

/*!
  \fn void QtopiaPowerConstraintManager::forceSuspend()

  Emitted when the device was forced to suspend due to the forced-suspend timeout.
  This timeout is set by defining the QTOPIA_MAX_SCREEN_DISABLE_TIME, usually in custom.h.
  \sa {Hardware Configuration}
*/

/*!
  Constructs the power constraint manager. \a parent is passed to QObject.
  */
QtopiaPowerConstraintManager::QtopiaPowerConstraintManager(QObject *parent)
    : QObject(parent)
{
    g_managerinstance = this;
    currentMode = QtopiaApplication::Enable;
    timerId = 0;


    ApplicationLauncher *launcher = qtopiaTask<ApplicationLauncher>();
    if(launcher) {
        QObject::connect(launcher, SIGNAL(applicationTerminated(const QString &, ApplicationTypeLauncher::TerminationReason)), this, SLOT(applicationTerminated(const QString &)));
    }
}

/*!
    The application \a app temporarily requests \a mode.
*/
void QtopiaPowerConstraintManager::setConstraint(int mode, const QString &app)
{
    removeOld(app);
    switch(mode) {
        case QtopiaApplication::Disable:
            sStatus[0].append(app);
            break;
        case QtopiaApplication::DisableLightOff:
            sStatus[1].append(app);
            break;
        case QtopiaApplication::DisableReturnToHomeScreen:
            sStatus[2].append(app);
            break;
        case QtopiaApplication::DisableSuspend:
            sStatus[3].append(app);
            break;
        case QtopiaApplication::Enable:
            // need to reset the screen saver timer, and this is how you do that!
            g_qtopiaPowerManager->setDefaultIntervals();
            break;
        default:
            qWarning("Unrecognized temp power setting.  Ignored");
            return;
    }
    updateAll();
}

// Returns true if app had set a temp Mode earlier
bool QtopiaPowerConstraintManager::removeOld(const QString &pid)
{
    for (int i = 0; i < 4; i++) {
        int idx = sStatus[i].indexOf(pid);
        if ( idx != -1 ) {
            sStatus[i].removeAt( idx );
            return true;
        }
    }
    return false;
}

void QtopiaPowerConstraintManager::updateAll()
{
    int mode = QtopiaApplication::Enable;
    if ( sStatus[0].count() ) {
        mode = QtopiaApplication::Disable;
    } else if ( sStatus[1].count() ) {
        mode = QtopiaApplication::DisableLightOff;
    } else if ( sStatus[2].count() ) {
        mode = QtopiaApplication::DisableReturnToHomeScreen;
    } else if ( sStatus[3].count() ) {
        mode = QtopiaApplication::DisableSuspend;
    }

    if ( mode != currentMode ) {
#ifdef QTOPIA_MAX_SCREEN_DISABLE_TIME
        if ( currentMode == QtopiaApplication::Enable) {
            int tid = timerValue();
            if ( tid )
                timerId = startTimer( tid * 1000 );
        } else if ( mode == QtopiaApplication::Enable ) {
            if ( timerId ) {
                killTimer(timerId);
                timerId = 0;
            }
        }
#endif
        currentMode = mode;
        if ( g_qtopiaPowerManager ) {
            g_qtopiaPowerManager->setConstraint( (QtopiaApplication::PowerConstraint) mode );
        }
    }
}

/*!
  The application \a app has been terminated.
  Any power consdtraint applied by this application will
  be removed.
*/
void QtopiaPowerConstraintManager::applicationTerminated(const QString &app)
{
    if ( removeOld(app) )
        updateAll();
}

int QtopiaPowerConstraintManager::timerValue()
{
    int tid = 0;
#ifdef QTOPIA_MAX_SCREEN_DISABLE_TIME
    tid = QTOPIA_MAX_SCREEN_DISABLE_TIME;

    char *env = getenv("QTOPIA_DISABLED_APM_TIMEOUT");
    if ( !env )
        return tid;

    QString strEnv = env;
    bool ok = false;
    int envTime = strEnv.toInt(&ok);

    if ( ok ) {
        if ( envTime < 0 )
            return 0;
        else if ( envTime <= QTOPIA_MIN_SCREEN_DISABLE_TIME )
            return tid;
        else
            return envTime;
    }
#endif

    return tid;
}

/*!
  \reimp
  */
void QtopiaPowerConstraintManager::timerEvent(QTimerEvent *t)
{
#ifdef QTOPIA_MAX_SCREEN_DISABLE_TIME
    if ( timerId && (t->timerId() == timerId) ) {

        /*  Clean up    */
        killTimer(timerId);
        timerId = 0;
        currentMode = QtopiaApplication::Enable;
        if ( g_qtopiaPowerManager ) {
            g_qtopiaPowerManager->setConstraint
                ( (QtopiaApplication::PowerConstraint) currentMode );
        }

        // signal starts on a merry-go-round, which ends up in Desktop::togglePower()
        emit forceSuspend();
        // if we have apm we are asleep at this point, next line will be executed when we
        // awake from suspend.
        if ( QFile::exists( "/proc/apm" ) ) {
            QTime t;
            t = t.addSecs( timerValue() );
            QString str = tr("<qt>The running applications disabled power saving "
                             "for more than the allowed time (%1)."
                             "<br>The system was forced to suspend</qt>", "%1 = time span").arg( t.toString() );
            QMessageBox::information(0, tr("Forced suspend"), str);
        }

        // Reset all requests.
        for (int i = 0; i < 4; i++)
            sStatus[i].clear();

        updateAll();
    }
#else
    Q_UNUSED(t);
#endif
}

/*!
  Returns the singleton instance of the QtopiaPowerConstraintManager.
  */
QtopiaPowerConstraintManager *QtopiaPowerConstraintManager::instance()
{
    return g_managerinstance;
}


