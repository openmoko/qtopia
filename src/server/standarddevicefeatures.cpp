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

#include "standarddevicefeatures.h"
#include "qtopiaserverapplication.h"
#include "phone/cameramonitor.h"

#include <qvaluespace.h>

#include <qwindowsystem_qws.h>
#include <QDebug>
#include <QtopiaFeatures>
#include <QPowerStatus>

static bool batteryMonitor = true;
static bool cameraMonitor = true;
static bool clamFlipKeyMonitor = true;
static bool uiTimeValues = true;
static bool inputFeatures = true;

class BatteryMonitorPrivate;
class BatteryMonitor : public QObject
{
    Q_OBJECT

public:
    BatteryMonitor(QObject *p);
    virtual ~BatteryMonitor();

    void setUpdateTimer(int duration);

    int value() const;
    bool charging() const;

signals:
    void valueChanged(int);
    void chargingChanged(bool);

private:
    BatteryMonitorPrivate *d;
    Q_DISABLE_COPY(BatteryMonitor);
};

class TimeControl : public QObject
{
Q_OBJECT
public:
    TimeControl(QObject *parent = 0);

    void setSlowUpdate(bool);

protected:
    virtual void timerEvent(QTimerEvent *);

private slots:
    void doTimeTick();
    void dateFormatChanged();
    void clockChanged(bool);

private:
    QValueSpaceObject timeValueSpace;
    bool m_slowUpdates;
    int m_timeId;
};

class StandardDeviceFeaturesImpl : public QObject, public QWSServer::KeyboardFilter
{
    Q_OBJECT
public:
    StandardDeviceFeaturesImpl(QObject *parent=0);

    void disableBatteryMonitor();
    void disableCameraMonitor();
    void disableClamshellMonitor();
    void disableUITimeValues();

    // key filter.
    bool filter(int unicode, int keycode, int modifiers, bool press,
                bool autoRepeat);

private slots:
    void backlightChanged();

private:
    BatteryMonitor *battery;
    CameraMonitor *camera;
    QValueSpaceItem *backlightVsi;
    QValueSpaceObject *clamshellVso;
    TimeControl *time;
    bool clamOpen;
};

static StandardDeviceFeaturesImpl *sdfi = 0;

StandardDeviceFeaturesImpl::StandardDeviceFeaturesImpl(QObject *parent)
: QObject(parent), battery(0), camera(0), clamshellVso(0), time(0), clamOpen(true)
{
    sdfi = this;

    if (batteryMonitor) {
        // BatteryMonitor keeps the value space updated correctly
        battery = new BatteryMonitor(this);
        battery->setUpdateTimer(10000);
    }

    if (cameraMonitor) {
        // CameraMonitor keeps the camera QtopiaFeatures upto date
        camera = new CameraMonitor(this);
    }

    if (clamFlipKeyMonitor) {
        qwsServer->addKeyboardFilter(this);
        clamshellVso = new QValueSpaceObject("/Hardware/Devices");
        clamshellVso->setAttribute("ClamshellOpen", clamOpen);
    }

    backlightVsi = new QValueSpaceItem("/Hardware/Display/0", this);
    connect(backlightVsi, SIGNAL(contentsChanged()), this, SLOT(backlightChanged()));

    if(uiTimeValues)
        time = new TimeControl(this);

    if(inputFeatures) {
        QSettings btnCfg(Qtopia::defaultButtonsFile(), QSettings::IniFormat);
        btnCfg.beginGroup("Device");
        QStringList ilist = btnCfg.value("Input").toString().split(',', QString::SkipEmptyParts);
        if (!ilist.isEmpty()) {
            for(int ii = 0; ii < ilist.count(); ++ii)
                QtopiaFeatures::setFeature(ilist.at(ii));
        } else {
            if (Qtopia::mousePreferred()) {
                //# ifdef QPE_NEED_CALIBRATION
                QtopiaFeatures::setFeature("Calibrate");
                //# endif
                QtopiaFeatures::setFeature("Touchscreen");
            } else {
                QtopiaFeatures::setFeature("Keypad");
            }
        }
    }
}

void StandardDeviceFeaturesImpl::disableUITimeValues()
{
    delete time;
    time = 0;
}

void StandardDeviceFeaturesImpl::disableBatteryMonitor()
{
    delete battery;
    battery = 0;
}


void StandardDeviceFeaturesImpl::disableCameraMonitor()
{
    delete camera;
    camera = 0;
}

void StandardDeviceFeaturesImpl::disableClamshellMonitor()
{
    delete clamshellVso;
    clamshellVso = 0;
}

void StandardDeviceFeaturesImpl::backlightChanged()
{
    if(batteryMonitor) {
        int backlight = backlightVsi->value("Backlight", 255).toInt();
        if (backlight <= 1)
            battery->setUpdateTimer(60000);
        else
            battery->setUpdateTimer(10000);
    }
}

bool StandardDeviceFeaturesImpl::filter(int unicode, int keycode,
                                    int modifiers, bool press, bool autoRepeat)
{
    Q_UNUSED(unicode);
    Q_UNUSED(modifiers);
    Q_UNUSED(autoRepeat);

    if (clamFlipKeyMonitor && keycode == Qt::Key_Flip) {
        clamOpen = !press;
        clamshellVso->setAttribute("ClamshellOpen", clamOpen);
    }

    return false;
}

QTOPIA_TASK(StandardDeviceFeatures, StandardDeviceFeaturesImpl);

/*!
  \namespace StandardDeviceFeatures
  \ingroup QtopiaServer
  \brief The StandardDeviceFeatures namespace contains methods to disable
  Qtopia's standard device feature handling.

  The core Qtopia sub-systems rely on various device specific status
  information being maintained in the value space.
  To ease customizability, all the simple hardware device monitoring
  in Qtopia are collectively managed in one place - the
  \c {StandardDeviceFeatures} server task.

  To customize hardware device monitoring, an integrator needs only to implement their
  own version and disable Qtopia's default behaviour through one of the methods
  available in the StandardDeviceFeatures namespace.

  The documentation for each of the following methods lists both what
  device it monitors, as well as the expected result when the device state
  changes.  The intent is to make the task of replicating each very simple.
 */

/*!
  The battery monitor is responsible for maintaining the battery state in the
  value space.  The values that must be maintained are:

  \list
  \i /Accessories/Battery/Charging - true if the battery is being charged.
  \i /Accessories/Battery/Charge - the percentage of charge remaining (0-100).
  \i /Accessories/Battery/VisualCharge - the percentage to display in the
     user interface.  This may not reflect the true percentage exactly, e.g.
     a charging state is often indicated by progressively increasing the
     value.
  \endlist

  Invoking this method will disable the default battery monitoring.
 */
void StandardDeviceFeatures::disableBatteryMonitor()
{
    batteryMonitor = false;
    if (sdfi) sdfi->disableBatteryMonitor();
}

/*!
  The camera monitor detects when camera are available on the device and
  advertises them using the QtopiaFeatures API as the "Camera" feature.

  Invoking this method will disable the default camera monitoring.
 */
void StandardDeviceFeatures::disableCameraMonitor()
{
    cameraMonitor = false;
    if (sdfi) sdfi->disableCameraMonitor();
}

/*!
  The clamshell monitor is responsible monitoring when a clamshell style
  phone (flip phone) is opened or closed and updating the value space.
  The value that must be maintained is:

  \list
  \i /Hardware/Device/ClamshellOpen - true if the clamshell is in an open state.
  \endlist

  The default clamshell monitoring uses Qt::Key_Flip press events
  to indicate a closed state and Qt::Key_Flip release events to indicate
  an open state.

  Invoking this method will disable the default clamshell monitoring.
 */
void StandardDeviceFeatures::disableClamshellMonitor()
{
    clamFlipKeyMonitor = false;
    if (sdfi) sdfi->disableClamshellMonitor();
}

/*!
  The current system date and time is kept updated in the following value space
  keys:

  \table
  \header \o Key \o Description
  \row \o \c {/UI/DisplayTime/Time} \o The current time with minute resolution (eg "22:13").
  \row \o \c {/UI/DisplayTime/Date} \o The current date in expanded form (eg. "31 Aug 06").
  \row \o \c {/UI/DisplayTime/BriefDate} \o The current date in reduced form (eg. "31/08/06").
  \endtable

  While it is possible, and correct, for applications to source this
  information directly through the QTime and QDate APIs, these values can be
  used for displaying the time in title bars or other incidental locations.
  By doing so, these time or date displays will remain in sync with one
  another.

  Invoking this method will disable the creation and updating of the listed
  value space keys.
*/
void StandardDeviceFeatures::disableUITimeValues()
{
    uiTimeValues = false;
    if (sdfi) sdfi->disableUITimeValues();
}

/*!
  The QtopiaFeatures class allows applications to query certain properties about
  the Qtopia configuration.  Based on the configured input modes, the following
  features are set automatically:

  \table
  \header \o Feature \o Description
  \row \o \c {Touchscreen} \o The primary input for the device is via a
  touchscreen.  That is, \c {Qtopia::mousePreferred() == true}.
  \row \o \c {Calibrate} \o The touch screen device requires calibration.  This
  is set if \c {Touchscreen} is set.
  \row \o \c {KeyPad} \o The primary input for the device is via a keypad.  That
  is, \c {Qtopia::mousePreferred() == false}.
  \endtable.

  The device integrator can override these default features by setting the
  \c {Device/Input} value in Qtopia's defaultbuttons configuration file.  The
  value should be a comma separated list of the features to set.  If \i {any}
  features are specified in this manner, the automatic features above are not
  set.

  Invoking this method will disable the setting of features automatically.
 */
void StandardDeviceFeatures::disableInputFeatures()
{
    Q_ASSERT(!sdfi && !"StandardDeviceFeatures::disableInputFeatures() must be called before the instantiation of the StandardDeviceFeatures task.");
    inputFeatures = false;
}

void TimeControl::setSlowUpdate(bool updates)
{
    m_slowUpdates = updates;
    if (m_timeId)
        killTimer(m_timeId);
    m_timeId = startTimer( 5000 );
}

void TimeControl::timerEvent(QTimerEvent *)
{
    doTimeTick();
}

void TimeControl::doTimeTick()
{
    static bool sameMinute = false;
    QTime now = QDateTime::currentDateTime().time();
    if ( m_slowUpdates) {
        if ( sameMinute ){ // update time shortly after minute changes
            if ( m_timeId )
                killTimer( m_timeId );
            m_timeId = startTimer( 60000 );
            sameMinute = false;
        }
        else {
            int tdelta = 60 - now.second();
            if ( tdelta < 55) {
                if ( m_timeId )
                    killTimer( m_timeId );
                m_timeId = startTimer( (tdelta+1)*1000 );
                sameMinute = true;
            }
        }
    }

    clockChanged(QTimeString::currentAMPM());
    dateFormatChanged();
}

void TimeControl::dateFormatChanged()
{
    QString ldate = QTimeString::localYMD(QDate::currentDate(), QTimeString::Short);
    QString sdate = QTimeString::numberDateString(QDate::currentDate());
    timeValueSpace.setAttribute("Date", ldate);
    timeValueSpace.setAttribute("BriefDate", sdate);
}

void TimeControl::clockChanged(bool)
{
    timeValueSpace.setAttribute("Time",
            QTimeString::localHM(QDateTime::currentDateTime().time(), QTimeString::Short));
}

TimeControl::TimeControl(QObject *parent)
: QObject(parent), timeValueSpace("/UI/DisplayTime"), m_slowUpdates(false),
  m_timeId(0)
{
    setSlowUpdate( true );
    timerEvent(0);
    QObject::connect(qApp, SIGNAL(timeChanged()), this, SLOT(doTimeTick()));
    QObject::connect(qApp, SIGNAL(dateFormatChanged()), this, SLOT(dateFormatChanged()));
    QObject::connect(qApp, SIGNAL(clockChanged(bool)), this, SLOT(clockChanged(bool)));
}

//#define QTOPIA_ATCBC_BATTERY

// declare BatteryMonitorPrivate
class BatteryMonitorPrivate : public QObject
{
Q_OBJECT
public:
    BatteryMonitorPrivate(QObject *parent);
    void setUpdateTimer(int duration);

signals:
    void valueChanged(int);
    void chargingChanged(bool);

protected:
    void timerEvent(QTimerEvent *e);

private slots:
#ifdef QTOPIA_ATCBC_BATTERY
    void queryResult( QPhoneLine::QueryType type, const QString& value );
#endif

private:
    void doValueChanged(int newActual, int newPercent);
    void setCharging(bool charge);

private:
    int chargeId;
    int updateId;
    bool initialUpdate;
    int duration;

#ifdef QTOPIA_ATCBC_BATTERY
    QPhoneLine *line;
#endif
    QPowerStatus ps;

    QValueSpaceObject vso;

public:
    bool charging;
    int actualPercent;
    int percent;
};

// define BatteryMonitorPrivate
BatteryMonitorPrivate::BatteryMonitorPrivate(QObject *p)
: QObject(p), chargeId(0), initialUpdate(true),
  duration(10000), vso("/Accessories/Battery"),
  charging(true), actualPercent(-1), percent(-1)
{
#ifdef QTOPIA_ATCBC_BATTERY
    line = new QPhoneLine( QString(), this );
    connect(line, SIGNAL(queryResult(QPhoneLine::QueryType,const QString&)),
            this, SLOT(queryResult(QPhoneLine::QueryType,const QString&)));
    connect(line, SIGNAL(notification(QPhoneLine::QueryType,const QString&)),
            this, SLOT(queryResult(QPhoneLine::QueryType,const QString&)));
#endif

    doValueChanged(0, 0);
    setCharging(false);
    updateId = startTimer(0);
}

void BatteryMonitorPrivate::setCharging(bool charge)
{
    if(charge != charging) {
        charging = charge;
        vso.setAttribute("Charging", charging);
        emit chargingChanged(charging);
    }
}

void BatteryMonitorPrivate::doValueChanged(int newActual, int)
{
    if(newActual != actualPercent) {
        actualPercent = newActual;
        vso.setAttribute("Charge", actualPercent);
        emit valueChanged(actualPercent);
    }
}

void BatteryMonitorPrivate::setUpdateTimer(int d)
{
    duration = d;
    if(!initialUpdate) {
        if (updateId)
            killTimer(updateId);

        updateId = startTimer(duration);
    }
}

void BatteryMonitorPrivate::timerEvent(QTimerEvent *e)
{
    if (!e)
        return;

    if (initialUpdate) {
        initialUpdate = false;
        setUpdateTimer(duration);
    }
#ifdef QTOPIA_ATCBC_BATTERY
    line->query( QPhoneLine::BatteryCharge );
#else
    QPowerStatus oldPs = ps;
    ps = QPowerStatusManager::readStatus();
    if ( oldPs != ps ) {
        int newpercent = ps.batteryPercentRemaining();

        if (!charging && ps.batteryStatus() == QPowerStatus::Charging) {
            setCharging(true);
            doValueChanged(newpercent, 0);
        } else if ( charging && ps.batteryStatus() != QPowerStatus::Charging ) {
            setCharging(false);
            doValueChanged(newpercent, newpercent);
        }
    }
#endif
}

#ifdef QTOPIA_ATCBC_BATTERY
void BatteryMonitorPrivate::queryResult(QPhoneLine::QueryType type,
                                        const QString& value)
{
    // The "AT+CBC" command responds with "n,m" where n is one
    // of the following: 0 - ME powered by battery, 1 - ME has
    // a battery but is not currently powered by it (i.e. a power
    // cord is plugged in), 2 - ME does not have a battery, and
    // 3 - something else wrong with the ME.  If n is zero, then
    // m will be 1-100 for a power percentage, 0 for "empty"
    // or <0 for "invalid".
    if ( type == QPhoneLine::BatteryCharge ) {
        int posn = value.indexOf( ',' );
        if ( posn != -1 ) {
            int st = value.left( posn ).toInt();
            int level;
            int posn2 = value.indexOf( ',', posn + 1 );
            if ( posn2 != -1 )
                level = value.mid( posn + 1, posn2 - posn - 1 ).toInt();
            else
                level = value.mid( posn + 1 ).toInt();
            if(level > 100)
                level = 100;

            if ( st != 0 && level < 99 ) {
                // The phone is not powered by a battery.
                st = -1;

                // We cannot detect the current battery level.  We assume
                // that a power cord is connected and set the value to 100%.
                level = 100;

                // power cord is plugged in -> activate charging
                if (!charging) {
                    setCharging(true);
                    chargeId = startTimer( 500 );
                    doValueChanged(level, 0);
                }
            } else if(charging) {
                setCharging(false);
                if (chargeId) {
                    killTimer(chargeId);
                    chargeId = 0;
                }
                doValueChanged(level, level);
            } else {
                doValueChanged(level, level);
            }
        }
    }
}
#endif

// define BatteryMonitor

/*
  \class BatteryMonitor
  \brief The BatteryMonitor class updates the value space with the current
         battery value.

  In addition to the signals and slots, this class also exports the following
  value space items:

  \table
  \header
    \o ValueSpace item
    \o Value
  \row
    \o \c {/Accessories/Battery/Charge}
    \o BatteryMonitor::value()
  \row
    \o \c {/Accessories/Battery/Charging}
    \o BatteryMonitor::charging()
  \endtable

 */
BatteryMonitor::BatteryMonitor(QObject *p)
: QObject(p)
{
    d = new BatteryMonitorPrivate(this);
    QObject::connect(d, SIGNAL(valueChanged(int)),
                     this, SIGNAL(valueChanged(int)));
    QObject::connect(d, SIGNAL(chargingChanged(bool)),
                     this, SIGNAL(chargingChanged(bool)));
}

BatteryMonitor::~BatteryMonitor()
{
}

/*!
  Default \a duration is 10000 milliseconds.
 */
void BatteryMonitor::setUpdateTimer(int duration)
{
    Q_ASSERT(duration > 0);
    d->setUpdateTimer(duration);
}

int BatteryMonitor::value() const
{
    return d->actualPercent;
}

bool BatteryMonitor::charging() const
{
    return d->charging;
}
#include "standarddevicefeatures.moc"
