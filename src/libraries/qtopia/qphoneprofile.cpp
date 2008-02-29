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

#include "qphoneprofile.h"

#include <QSettings>
#include <QHash>
#include <QTranslatableSettings>
#include <QtopiaIpcEnvelope>
#include <QtopiaService>
#include <QtopiaApplication>
#include <QSharedData>
#include <QContentSet>


/*!
  \class QPhoneProfile::Setting
  \brief The QPhoneProfile::Setting class allows applications to add their own settings to a profile.

  Applications may integrate with the Qtopia profiles by adding their own
  settings.  Optionally, when the profile changes the application will be
  notified and instructed to apply the new settings.

  Most profile editing applications (such as Ring Profiles in Qtopia Phone) will
  use the Settings service to interactively acquire settings from applications
  to add in this way, but it is not necessary to do so.

  Application settings are added to a profile through the
  QPhoneProfile::setApplicationSettings() or
  QPhoneProfile::setApplicationSetting() methods and read via the
  QPhoneProfile::applicationSetting() call.
 */

// define QPhoneProfile::Setting
/*!
  Construct a null application setting object.
 */
QPhoneProfile::Setting::Setting()
: notify(false)
{
}

/*!
  Construct an empty application setting object for \a application.
  */
QPhoneProfile::Setting::Setting(const QString &application)
: appName(application), notify(false)
{
}

/*!
  Construct a copy of \a other.
  */
QPhoneProfile::Setting::Setting(const Setting &other)
: appName(other.appName), appTitle(other.appTitle), details(other.details),
  notify(other.notify)
{
}

/*!
  Assign \a other to this object.
  */
QPhoneProfile::Setting &QPhoneProfile::Setting::operator=(const Setting &other)
{
    appName = other.appName;
    appTitle = other.appTitle;
    details = other.details;
    notify = other.notify;
    return *this;
}

/*!
  Returns true if this is a null application setting object.  A null object is
  one that has an empty applicationName().
  */
bool QPhoneProfile::Setting::isNull() const
{
    return appName.isEmpty();
}

/*!
  Returns the application to which this setting object applies, or an empty
  string if this is a null settings object.
 */
QString QPhoneProfile::Setting::applicationName() const
{
    return appName;
}

/*!
  Return a optional visual description for this setting.  Generally this is
  the visual name of the application.
  */
QString QPhoneProfile::Setting::description() const
{
    return appTitle;
}

/*!
  Return the settings data.
 */
QString QPhoneProfile::Setting::data() const
{
    return details;
}

/*!
  Returns true if the application will be notified when the profile changes and
  false otherwise.

  Applications will be notified with a \c {Settings::activateSettings(QString)}
  message when they are to apply settings, with the parameter set to the
  data() portion of the setting.  If the profile changes to one where the
  application has not added a custom setting from one where the application has
  added a custom setting, a \c {Settings::activateDefault()} message will
  be sent.
 */
bool QPhoneProfile::Setting::notifyOnChange() const
{
    return notify;
}

/*!
  Set the application \a name.
  */
void QPhoneProfile::Setting::setApplicationName(const QString &name)
{
    appName = name;
}

/*!
  Set the setting \a description.
  */
void QPhoneProfile::Setting::setDescription(const QString &description)
{
    appTitle = description;
}

/*!
  Set the setting \a data.
 */
void QPhoneProfile::Setting::setData(const QString &data)
{
    details = data;
}

/*!
  Set whether the application is notified on changes to the profile or not.
  If \a notifyOnChange is true applications are notified, otherwise they are
  not.
  */
void QPhoneProfile::Setting::setNotifyOnChange(bool notifyOnChange)
{
    notify = notifyOnChange;
}

/*!
  Returns true if this and \a other are equivalent.
  */
bool QPhoneProfile::Setting::operator==(const Setting &other) const
{
    return appName == other.appName &&
           appTitle == other.appTitle &&
           details == other.details &&
           notify == other.notify;
}

/*!
  Returns true if this and \a other are not equivalent.
 */
bool QPhoneProfile::Setting::operator!=(const Setting &other) const
{
    return !(other == *this);
}

// declare QPhoneProfilePrivate
class QPhoneProfilePrivate : public QSharedData
{
public:
    QPhoneProfilePrivate()
    : mSaveId(-1), mIsSystemProfile(false), mVolume(3), mVibrate(true),
      mCallAlert(QPhoneProfile::Continuous), mMsgAlert(QPhoneProfile::Once),
      mMsgAlertDuration(5000), mAutoAnswer(false), mPlaneMode(false) {}
    QPhoneProfilePrivate &operator=(const QPhoneProfilePrivate &o);

    QString mName;
    int mSaveId;
    bool mIsSystemProfile;
    int mVolume;
    bool mVibrate;
    QPhoneProfile::AlertType mCallAlert;
    QPhoneProfile::AlertType mMsgAlert;
    int mMsgAlertDuration;
    bool mAutoAnswer;
    QContent mCallTone;
    QContent mMessageTone;
    bool mPlaneMode;
    QString mIcon;
    QPhoneProfile::Settings mSettings;
    QPhoneProfile::Schedule mSchedule;
    QString mAccessory;
    QString mSpeedDialInput;
};

// define QPhoneProfilePrivate
QPhoneProfilePrivate &QPhoneProfilePrivate::operator=(const QPhoneProfilePrivate &o)
{
    mName = o.mName;
    mIsSystemProfile = o.mIsSystemProfile;
    mVolume = o.mVolume;
    mVibrate = o.mVibrate;
    mCallAlert = o.mCallAlert;
    mMsgAlert = o.mMsgAlert;
    mMsgAlertDuration = o.mMsgAlertDuration;
    mAutoAnswer = o.mAutoAnswer;
    mCallTone = o.mCallTone;
    mMessageTone = o.mMessageTone;
    mPlaneMode = o.mPlaneMode;
    mIcon = o.mIcon;
    mSaveId = o.mSaveId;
    mSettings = o.mSettings;
    mSchedule = o.mSchedule;
    mAccessory = o.mAccessory;
    mSpeedDialInput = o.mSpeedDialInput;

    return *this;
}

/*!
  \class QPhoneProfile
  \brief The QPhoneProfile class encapsulates a single phone profile
         configuration.
 */

// define QPhoneProfile
static const QString cName("Name"); // no tr
static const QString cSystem("System"); // no tr
static const QString cVolume("Volume"); // no tr
static const QString cVibrate("Vibrate"); // no tr
static const QString cCallA("CallAlert"); // no tr
static const QString cMsgA("MsgAlert"); // no tr
static const QString cMsgADuration("MsgAlertDuration");
static const QString cAutoAnswer("AutoAnswer"); // no tr
static const QString cCallTone("RingTone"); // no tr
static const QString cMessageTone("MessageTone"); // no tr
static const QString cPlaneMode("PlaneMode"); // no tr
static const QString cIcon("Icon"); // no tr
static const QString cAccessory("Accessory"); // no tr
static const QString cSpeedDialInput("SpeedDialInput");
static const QString cDescription("Description"); // no tr
static const QString cData("Data"); // no tr
static const QString cNotify("Notify"); // no tr

static const QContent *systemRingTone = 0;
static const QContent *systemAlertTone = 0;

static QContent findSystemRingTone(const QString &name)
{
    static QContentSet *systemRingTones = 0;
    if (!systemRingTones)
        systemRingTones = new QContentSet( QContentFilter::Category, QLatin1String( "SystemRingtones" ));

    return systemRingTones->findFileName(name);
}

/*!
  \enum QPhoneProfile::AlertType

  Controls how a ring or message tone is played.

  \value Off No tone is played.
  \value Once The tone is played once, from beginning to end.
  \value Continuous The tone is played repeatedly until the user acknowledges
         the alert.
  \value Ascending The tone is played repeatedly with increasing volume each
         time.
  */

/*!
  \typedef QPhoneProfile::Settings

  This is a convenience typedef to encapsulate a mapping between application
  names and their corresponding Setting object.  The exact type is:
  \code
  QMap<QString, QPhoneProfile::Setting>
  \endcode
  */

/*!
  Construct a null QPhoneProfile.
  */
QPhoneProfile::QPhoneProfile()
: d(new QPhoneProfilePrivate)
{
}

/*!
  Construct an empty QPhoneProfile with the specified \a id.
 */
QPhoneProfile::QPhoneProfile(int id)
: d(new QPhoneProfilePrivate)
{
    d->mSaveId = id;
}

/*!
  Construct a copy of \a other.
 */
QPhoneProfile::QPhoneProfile(const QPhoneProfile &other)
: d(other.d)
{
}

/*!
  Destroy the QPhoneProfile instance.
 */
QPhoneProfile::~QPhoneProfile()
{
}

/*!
  \fn bool QPhoneProfile::operator==(const QPhoneProfile &other) const

  Returns true if this profile is equal to \a other.  Equality means all
  the profile fields are equivalent.
 */
bool QPhoneProfile::operator==(const QPhoneProfile &o) const
{
    return d->mName == o.d->mName &&
           d->mIsSystemProfile == o.d->mIsSystemProfile &&
           d->mVolume == o.d->mVolume &&
           d->mVibrate == o.d->mVibrate &&
           d->mCallAlert == o.d->mCallAlert &&
           d->mMsgAlert == o.d->mMsgAlert &&
           d->mMsgAlertDuration == o.d->mMsgAlertDuration &&
           d->mAutoAnswer == o.d->mAutoAnswer &&
           d->mCallTone == o.d->mCallTone &&
           d->mMessageTone == o.d->mMessageTone &&
           d->mPlaneMode == o.d->mPlaneMode &&
           d->mIcon == o.d->mIcon &&
           d->mSaveId == o.d->mSaveId &&
           d->mSettings == o.d->mSettings &&
           d->mSchedule == o.d->mSchedule;
}

/*!
  \fn bool QPhoneProfile::operator!=(const QPhoneProfile &other) const

  Returns true if this profile is not equal to \a other.
  */
bool QPhoneProfile::operator!=(const QPhoneProfile &o) const
{
    return !(*this == o);
}

/*!
  Assign \a other to this profile.
  */
QPhoneProfile &QPhoneProfile::operator=(const QPhoneProfile &other)
{
    d = other.d;
    return *this;
}

/*!
  Returns true if this profile is null.  A null profile is one with an id
  of -1.
  */
bool QPhoneProfile::isNull() const
{
    return id() == -1;
}

/*!
  Returns the user visible name of the profile.
 */
QString QPhoneProfile::name() const
{
    return d->mName;
}

/*!
  Returns true if this is a system profile.  System profiles can typically not
  be deleted.
 */
bool QPhoneProfile::isSystemProfile() const
{
    return d->mIsSystemProfile;
}

/*!
  Returns the volume for this profile.
 */
int QPhoneProfile::volume() const
{
    return d->mVolume;
}

/*!
  Returns true if vibration is enabled, false if not.
 */
bool QPhoneProfile::vibrate() const
{
    return d->mVibrate;
}

/*!
  Returns the AlertType to use for incoming calls.
  */
QPhoneProfile::AlertType QPhoneProfile::callAlert() const
{
    return d->mCallAlert;
}

/*!
  Returns the AlertType to use for incoming messages.
 */
QPhoneProfile::AlertType QPhoneProfile::msgAlert() const
{
    return d->mMsgAlert;
}

/*!
  Returns the duration in milliseconds to play the message tone.
  */
int QPhoneProfile::msgAlertDuration() const
{
    return d->mMsgAlertDuration;
}

/*!
  Returns true if incoming calls should be automatically answered in this
  profile, false if not.
 */
bool QPhoneProfile::autoAnswer() const
{
    return d->mAutoAnswer;
}

/*!
  Returns the default ring tone to use for incoming calls.
  */
QContent QPhoneProfile::callTone() const
{
    return d->mCallTone;
}

/*!
  Returns the system ring tone to use for incoming calls.
  The system ring tone will be used when the user defined tone cannot be found.
*/
QContent QPhoneProfile::systemCallTone() const
{
    if (!systemRingTone)
        systemRingTone = new QContent(findSystemRingTone(QLatin1String("phonering.wav")));
    return *systemRingTone;
}

/*!
  Returns the default ring tone to use for incoming messages.
 */
QContent QPhoneProfile::messageTone() const
{
    return d->mMessageTone;
}

/*!
  Returns the system message tone to use for incoming messages.
  The system message tone will be used when the user defined tone cannot be found.
*/
QContent QPhoneProfile::systemMessageTone() const
{
    if (!systemAlertTone)
        systemAlertTone = new QContent(findSystemRingTone(QLatin1String("alarm.wav")));
    return *systemAlertTone;
}

/*!
  Returns true if plane mode is on in this profile, false otherwise.
  */
bool QPhoneProfile::planeMode() const
{
    return d->mPlaneMode;
}

/*!
  Return all application settings for this profile.
  */
QPhoneProfile::Settings QPhoneProfile::applicationSettings() const
{
    return d->mSettings;
}

/*!
  Return the setting for \a application for this profile, or a null setting
  if not applicable.
 */
QPhoneProfile::Setting QPhoneProfile::applicationSetting(const QString &application) const
{
    return d->mSettings[application];
}

/*!
  Return the icon to use to identify this profile.
  */
QString QPhoneProfile::icon() const
{
    return d->mIcon;
}

/*!
  Return the auto activation schedule for this profile.
 */
QPhoneProfile::Schedule QPhoneProfile::schedule() const
{
    return d->mSchedule;
}

/*!
  Return the identifier for this profile.
  */
int QPhoneProfile::id() const
{
    return d->mSaveId;
}

/*!
  Return the accessory on which this profile should auto activate, or an empty
  string if there is no such accessory.
 */
QString QPhoneProfile::accessory() const
{
    return d->mAccessory;
}

/*!
  Return the speed dial input on which this profile should activate.
  If there is no speed dial input for the profile, an empty string is returned.
  */
QString QPhoneProfile::speedDialInput() const
{
    return d->mSpeedDialInput;
}

/*!
  Set the profile \a id.
  */
void QPhoneProfile::setId(int id)
{
    d->mSaveId = id;
}

/*!
  Set the profile name to \a name.
  */
void QPhoneProfile::setName(const QString &name)
{
    d->mName = name;
}

/*!
  Set whether this profile is a system profile to \a isSystemProfile.
  */
void QPhoneProfile::setIsSystemProfile(bool isSystemProfile)
{
    d->mIsSystemProfile = isSystemProfile;
}

/*!
  Set the profile \a volume.  Valid values are from 0 to 5.
  */
void QPhoneProfile::setVolume(int volume)
{
    d->mVolume = qMin(5, qMax(0, volume));
}

/*!
  Set the profile vibration to \a vibrate.
  */
void QPhoneProfile::setVibrate(bool vibrate)
{
    d->mVibrate = vibrate;
}

/*!
  Set the incoming call alert \a type.
  */
void QPhoneProfile::setCallAlert(AlertType type)
{
    d->mCallAlert = type;
}

/*!
  Set the incoming message alert \a type.
  */
void QPhoneProfile::setMsgAlert(AlertType type)
{
    d->mMsgAlert = type;
}

/*!
  Set the duration of \a ms milliseconds to play the message tone.
  */
void QPhoneProfile::setMsgAlertDuration(int ms)
{
    d->mMsgAlertDuration = ms;
}

/*!
  Enables auto answer if \a autoAnswer is true, otherwise disable it.
  */
void QPhoneProfile::setAutoAnswer(bool autoAnswer)
{
    d->mAutoAnswer = autoAnswer;
}

/*!
  Enable plane mode if \a planeMode is true, otherwise disable it.
 */
void QPhoneProfile::setPlaneMode(bool planeMode)
{
    d->mPlaneMode = planeMode;
}

/*!
  Set the application \a settings.
 */
void QPhoneProfile::setApplicationSettings(const Settings &settings)
{
    d->mSettings = settings;
}

/*!
  Add or update a single application \a setting.
 */
void QPhoneProfile::setApplicationSetting(const Setting &setting)
{
    if(!setting.applicationName().isEmpty())
        d->mSettings.insert(setting.applicationName(), setting);
}

/*!
  Set the profile's auto activation \a schedule.
 */
void QPhoneProfile::setSchedule(const Schedule &schedule)
{
    d->mSchedule = schedule;
}

/*!
  \fn void QPhoneProfile::setCallTone(const QContent &tone)

  Set the incoming call \a tone.
  */
void QPhoneProfile::setCallTone(const QContent &l)
{
    if (l.fileKnown())
        d->mCallTone = l;
    else
        d->mCallTone = systemCallTone();
}

/*!
  \fn void QPhoneProfile::setMessageTone(const QContent &tone)

  Set the incoming message \a tone.
 */
void QPhoneProfile::setMessageTone(const QContent &l)
{
    if (l.fileKnown())
        d->mMessageTone = l;
    else
        d->mMessageTone = systemMessageTone();
}

/*!
  Set the auto activation \a accessory.
 */
void QPhoneProfile::setAccessory(const QString &accessory)
{
    d->mAccessory = accessory;
}

/*!
  Set the speed dial input \a input.
  */
void QPhoneProfile::setSpeedDialInput(const QString &input)
{
    d->mSpeedDialInput = input;
}

void QPhoneProfile::read(QTranslatableSettings &c)
{
    d->mName=c.value(cName).toString();
    setIsSystemProfile(c.value(cSystem).toBool());
    setVolume(c.value(cVolume).toInt());
    setVibrate(c.value(cVibrate).toBool());
    setCallAlert((QPhoneProfile::AlertType)c.value(cCallA).toInt());
    setMsgAlert((QPhoneProfile::AlertType)c.value(cMsgA).toInt());
    setMsgAlertDuration(c.value(cMsgADuration).toInt());
    setAutoAnswer(c.value(cAutoAnswer).toBool());
    setPlaneMode(c.value(cPlaneMode).toBool());
    setIcon(c.value(cIcon).toString());
    setAccessory(c.value(cAccessory).toString());
    setSpeedDialInput(c.value(cSpeedDialInput).toString());

    QContent link = QContent(c.value(cCallTone).toString());
    if (link.fileKnown())
        d->mCallTone = link;
    else
       d->mCallTone = systemCallTone();
    link = QContent(c.value(cMessageTone).toString());
    if (link.fileKnown())
        d->mMessageTone = link;
    else
       d->mMessageTone = systemMessageTone();

    QString str = c.value("SettingList").toString();
    QStringList settingList;
    if (!str.isEmpty())
        settingList = str.split( ',' );

    // load associated settings
    Settings settings;
    Setting setting;
    QString settingName, tmp;

    for(int i = 0; i < settingList.size(); i++) {
        settingName = settingList.at(i);
        setting.setApplicationName(settingName);
        c.beginGroup(settingName);
        setting.setDescription(c.value(cDescription).toString());
        setting.setData(c.value(cData).toString());
        setting.setNotifyOnChange(c.value(cNotify, false).toBool());
        c.endGroup();
        settings[setting.applicationName()] = setting;
    }
    setApplicationSettings(settings);

    d->mSchedule = Schedule(c.value("Schedule").toString());
}

/*!
  Set the profile \a icon.
  */
void QPhoneProfile::setIcon(const QString &icon)
{
    d->mIcon = icon;
    if(!d->mIcon.startsWith('/') && !d->mIcon.startsWith(":image/"))
        d->mIcon = ":image/" + d->mIcon;
}

void QPhoneProfile::write(QSettings &c) const
{
    // do not write system profile's name.
    // it should be translated into different languages
    if (!isSystemProfile())
        c.setValue(cName, d->mName);
    c.setValue(cSystem, isSystemProfile());
    c.setValue(cVolume, volume());
    c.setValue(cVibrate, vibrate());
    c.setValue(cCallA, (int)callAlert());
    c.setValue(cMsgA, (int)msgAlert());
    c.setValue(cMsgADuration, msgAlertDuration());
    c.setValue(cAutoAnswer, autoAnswer());
    c.setValue(cPlaneMode, planeMode());
    c.setValue(cIcon, icon());
    c.setValue(cAccessory, accessory());
    c.setValue(cSpeedDialInput, speedDialInput());

    if ( d->mCallTone.fileKnown() )
        c.setValue(cCallTone, d->mCallTone.file());
    if ( d->mMessageTone.fileKnown() )
        c.setValue(cMessageTone, d->mMessageTone.file());

    Settings s = applicationSettings();
    int sCount = s.count();
    if ( sCount > 0 ) {
        QStringList sList;
        QStringList keys = s.keys();
        QString currentKey;
        for ( int j = 0; j < s.count(); j++ ) {
            currentKey = keys.value( j );
            Setting setting = s[currentKey];
            c.beginGroup(setting.applicationName());
            c.setValue(cDescription, setting.description());
            c.setValue(cData, setting.data());
            c.setValue(cNotify, setting.notifyOnChange());
            sList += s[currentKey].applicationName();
            c.endGroup();
        }
        c.setValue( "SettingList", sList.join(QString(',')) );
    } else {
        c.setValue( "SettingList", QString() );
    }

    c.setValue("Schedule", d->mSchedule.toString());
}


/*!
  \class QPhoneProfile::Schedule
  \brief The QPhoneProfile::Schedule class controls timed auto-activation of a
         profile.

  Qtopia profiles may be automatically activated on certain times and dates.
  The QPhoneProfile::Schedule class represents the times at which the profile
  should be activated.  A profile's schedule can be read through the
  QPhoneProfile::schedule() method, and set through a
  QPhoneProfile::setSchedule() call.
 */
// define QPhoneProfile::Schedule
/*!
  Construct a new, empty schedule.
  */
QPhoneProfile::Schedule::Schedule()
: _active(false), _days(0)
{
}

/*! \internal */
QPhoneProfile::Schedule::Schedule(const QString &str)
: _active(false), _days(0)
{
    fromString(str);
}

/*!
  Construct a copy of \a other.
  */
QPhoneProfile::Schedule::Schedule(const Schedule &other)
: _active(other._active), _time(other._time), _days(other._days)
{
}

/*!
  Assign \a other to this schedule.
  */
QPhoneProfile::Schedule::Schedule &QPhoneProfile::Schedule::operator=(const Schedule &other)
{
    _active = other._active;
    _time = other._time;
    _days = other._days;
    return *this;
}

/*!
  \fn bool QPhoneProfile::Schedule::operator==(const Schedule &other) const

  Return true of \a other is equivalent to this schedule.
  */
bool QPhoneProfile::Schedule::operator==(const QPhoneProfile::Schedule &other) const
{
    return _active == other._active &&
           _time == other._time &&
           _days == other._days;
}

/*!
  \fn bool QPhoneProfile::Schedule::operator!=(const Schedule &other) const

  Return true of \a other is not equivalent to this schedule.
  */
bool QPhoneProfile::Schedule::operator!=(const QPhoneProfile::Schedule &other) const
{
    return !(other == *this);
}

/*!
  Returns true if the schedule is active.  Inactive schedules will be ignored
  by the auto activation system.
 */
bool QPhoneProfile::Schedule::isActive() const
{
    return _active;
}

/*!
  If \a active is true, the schedule is enabled, otherwise it is disabled.
 */
void QPhoneProfile::Schedule::setActive(bool active)
{
    _active = active;
}

/*!
  Returns the time of day the profile will be activated.
 */
QTime QPhoneProfile::Schedule::time() const
{
    return _time;
}

/*!
  Sets the activation time of day to \a time.
  */
void QPhoneProfile::Schedule::setTime(const QTime &time)
{
    _time = time;
}

inline static unsigned char dayToMask(Qt::DayOfWeek day)
{
    unsigned char rv = 1;
    if ((int)day > 0 && day <= Qt::Sunday)
        rv <<= ((int)day - 1);
    return rv;
}

/*!
  Returns true of the profile will be activated on \a day.
 */
bool QPhoneProfile::Schedule::scheduledOnDay(Qt::DayOfWeek day) const
{
    return _days & dayToMask(day);
}

/*!
  Returns the list of days on which the profile will be auto activated.
 */
QList<Qt::DayOfWeek> QPhoneProfile::Schedule::scheduledOnDays() const
{
    QList<Qt::DayOfWeek> rv;

    unsigned char day = 1;
    for(int ii = 1; ii <= 7; ++ii) {
        if(_days & day)
            rv.append((Qt::DayOfWeek)ii);
        day <<= 1;
    }

    return rv;
}

/*!
  Sets the profile to activate on \a day.  If other days are set they will not
  be affected.
  */
void QPhoneProfile::Schedule::setScheduledDay(Qt::DayOfWeek day)
{
    _days |= dayToMask(day);
}

/*!
  Sets the profile not to activate on \a day.
 */
void QPhoneProfile::Schedule::unsetScheduledDay(Qt::DayOfWeek day)
{
    _days &= ~dayToMask(day);
}

/*!
  Sets the profile to activate on no days.
  */
void QPhoneProfile::Schedule::clearScheduledDays()
{
    _days = 0;
}

/*!
  \internal

  Format is:

  \c {[on|off],<integer 0-6 for each scheduled day>,<time in minutes>,}

  For example, a schedule of 8:06 on Tuesday, Wednesday and Sunday would be

  \c {on,2,3,7,486,}
 */
QString QPhoneProfile::Schedule::toString() const
{
    QString rv;
    if(isActive())
        rv = QString("on,");
    else
        rv = QString("off,");

    unsigned char days = _days;
    for(int ii = (int)Qt::Monday; ii <= (int)Qt::Sunday; ++ii) {
        if(days & 0x01) {
            rv.append(QString::number(ii));
            rv.append(",");
        }
        days >>= 1;
    }

    int minutes = time().hour() * 60 + time().minute();
    rv.append(QString::number(minutes));

    rv.append(","); // Backwards compatibility

    return rv;
}

/*!
  \internal
  */
void QPhoneProfile::Schedule::fromString(const QString &str)
{
    *this = QPhoneProfile::Schedule();

    QStringList lst = str.split(',', QString::SkipEmptyParts);
    if(lst.isEmpty())
        return;

    if(lst.first() == "on") {
        setActive(true);
    } else if(lst.first() == "off") {
        setActive(false);
    }

    lst.pop_front();

    if(lst.isEmpty())
        return;

    int time = lst.last().toInt();
    setTime(QTime(time / 60, time % 60));

    lst.pop_back();

    for(int ii = 0; ii < lst.count(); ++ii) {
        int day = lst.at(ii).toInt();
        if(day >= Qt::Monday && day <= Qt::Sunday)
            setScheduledDay((Qt::DayOfWeek)day);
    }

    return;
}

// declare QPhoneProfileManagerPrivate
class QPhoneProfileManagerPrivate
{
public:
    QPhoneProfileManagerPrivate()
    : m_selected(-1), m_planeMode(false),
      m_planeModeAvailable(false), m_maxId(-1),
      m_constructing(true) {}

    QPhoneProfileManagerPrivate(const QPhoneProfileManagerPrivate &o)
    : m_selected(-1), m_planeMode(o.m_planeMode),
      m_planeModeAvailable(o.m_planeModeAvailable), m_maxId(o.m_maxId),
      m_constructing(o.m_constructing) {}

    int m_selected;
    bool m_planeMode;
    bool m_planeModeAvailable;
    int m_maxId;
    bool m_constructing;

    typedef QHash<int, QPhoneProfile> Profiles;
    Profiles m_profiles;
};

/*!
  \class QPhoneProfileManager
  \brief The QPhoneProfileManager class allows applications to control phone profiles.

  Qtopia's phone profiles are stored in the \c {Trolltech/PhoneProfile}
  configuration file.  A device may have any number of integrator or user
  defined profiles.  Each profile has a unique integer identifier which is used
  to refer to it by the system.

  A profile can be manually activated through the
  QPhoneProfileManager::activateProfile() method.  Alternatively a profile may
  be set to auto-activate at certain times, controlled through by
  QPhoneProfile::schedule(), or when a phone accessory is attached.

 */

// define QPhoneProfileManager
/*!
  Construct a new QPhoneProfileManager with the specified \a parent.
 */
QPhoneProfileManager::QPhoneProfileManager(QObject *parent)
: QObject(parent), d(new QPhoneProfileManagerPrivate)
{
    sync();

    QtopiaChannel *channel = new QtopiaChannel("QPE/PhoneProfiles", this);
    QObject::connect(channel, SIGNAL(received(QString,QByteArray)),
                     this, SLOT(profMessage(QString,QByteArray)));

    d->m_constructing = false;
}

/*!
  Destroy the QPhoneProfileManager instance.
 */
QPhoneProfileManager::~QPhoneProfileManager()
{
    delete d;
}

void QPhoneProfileManager::loadConfig()
{
    d->m_profiles.clear();

    QTranslatableSettings c("Trolltech","PhoneProfile"); // no tr
    c.beginGroup("Profiles"); // no tr
    d->m_selected = c.value("Selected").toInt();
    d->m_planeMode = c.value("PlaneMode", false).toBool();
    d->m_planeModeAvailable = c.value("PlaneModeAvailable", false).toBool();

    QStringList profiles = c.value("Profiles").toString().split(','); // no tr
    for(QStringList::Iterator it = profiles.begin();
        it != profiles.end(); ++it) {

        c.endGroup();
        c.beginGroup("Profile " + (*it));

        int id = (*it).toInt();
        if (id > d->m_maxId)
            d->m_maxId = id;

        QPhoneProfile rpp(id);
        rpp.read(c);

        d->m_profiles.insert(id, rpp);
    }
}

/*!
  Returns a QPhoneProfile instance for the profile with the specified \a id,
  or a null QPhoneProfile if no such profile exists.
 */
QPhoneProfile QPhoneProfileManager::profile(int id) const
{
    QPhoneProfileManagerPrivate::Profiles::Iterator iter =
        d->m_profiles.find(id);
    if(iter == d->m_profiles.end())
        return QPhoneProfile();
    else
        return *iter;
}

/*!
  Return all configured phone profiles.
 */
QList<QPhoneProfile> QPhoneProfileManager::profiles() const
{
    QList<QPhoneProfile> rv;

    for(QPhoneProfileManagerPrivate::Profiles::ConstIterator iter =
            d->m_profiles.begin();
            iter != d->m_profiles.end();
            ++iter)
        rv.append(*iter);

    return rv;
}

/*!
  Return a list of ids for all configured profiles.
 */
QList<int> QPhoneProfileManager::profileIds() const
{
    return d->m_profiles.keys();
}

/*!
  Return true if plane mode is active, false if not.  Plane mode is active
  if either planeModeOverride() is true or QPhoneProfile::planeMode() for
  the active profile is true.
 */
bool QPhoneProfileManager::planeMode() const
{
    return d->m_planeMode || activeProfile().planeMode();
}

/*!
  Returns true if plane mode override is on, false otherwise.  The plane mode
  override allows plane mode to be enabled, even if the active profile does
  not specify it as such.
 */
bool QPhoneProfileManager::planeModeOverride() const
{
    return d->m_planeMode;
}

/*!
  Set the current plane mode override state to \a mode.
 */
void QPhoneProfileManager::setPlaneModeOverride(bool mode)
{
    d->m_planeMode = mode;
    QSettings c("Trolltech", "PhoneProfile");
    c.beginGroup("Profiles");
    c.setValue("PlaneMode", d->m_planeMode);

    QtopiaIpcEnvelope env("QPE/PhoneProfiles", "profileChanged()");
}

/*!
  Returns true if plane mode is available, false if not.
 */
bool QPhoneProfileManager::planeModeAvailable() const
{
    return d->m_planeModeAvailable;
}

/*!
  Returns the currently active profile, or a null profile if not profile is
  active.
 */
QPhoneProfile QPhoneProfileManager::activeProfile() const
{
    return profile(d->m_selected);
}

/*!
  \overload

  Activate the \a profile.  If \a profile has not been saved, the saved version
  with the same QPhoneProfile::id() will be activated.

  This call is equivalent to \c {activateProfile(profile.id())}.
 */
bool QPhoneProfileManager::activateProfile(const QPhoneProfile &profile)
{
    return activateProfile(profile.id());
}

/*!
  \fn bool QPhoneProfileManager::activateProfile(int profile)

  Activate the \a profile id.  If \a profile is -1 or the specified profile
  does not exist, a null (default values) profile will be activated.
 */
bool QPhoneProfileManager::activateProfile(int newProf)
{
    // Find the new profile
    QPhoneProfile newProfile = profile(newProf);

    // Find the existing profile
    QPhoneProfile oldProfile = activeProfile();

    activateProfile(newProfile, oldProfile);

    return (newProfile.id() != -1) || (newProf == -1);
}

void QPhoneProfileManager::activateProfile(const QPhoneProfile &newProfile,
                                           const QPhoneProfile &oldProfile)
{
    // Update settings file
    {
        QSettings cfg("Trolltech", "PhoneProfile");
        cfg.beginGroup("Profiles");
        cfg.setValue("Selected", newProfile.id());
    }

    // Send changed message
    {
        QtopiaIpcEnvelope e( "QPE/PhoneProfiles", "profileChanged()" );
    }

    // Update external application settings
    activateSettings(newProfile.applicationSettings(),
                     oldProfile.applicationSettings());

    // Update me :)
    d->m_selected = newProfile.id();
}

void QPhoneProfileManager::activateSettings(const QPhoneProfile::Settings &current, const QPhoneProfile::Settings &previous)
{
    // list of all available settings
    QStringList allapps = QtopiaService::apps("Settings");

    // count of current profile's associated sttings
    foreach(QPhoneProfile::Setting setting, current) {
        if(setting.notifyOnChange()) {
            // activate the setting with the saved details
            QtopiaIpcEnvelope e("QPE/Application/" + setting.applicationName(),
                                "Settings::activateSettings(QString)");
            e << setting.data();
        }

        // remove from the list of all settings
        allapps.removeAll(setting.applicationName());
    }

    // if the previous profile has associated settings
    // that are not included in the current profile,
    // activate the default state for each of them
    foreach(QPhoneProfile::Setting setting, previous) {
        if(setting.notifyOnChange() &&
           allapps.contains(setting.applicationName())) {
            QtopiaIpcEnvelope e("QPE/Application/" + setting.applicationName(),
                                "Settings::activateDefault()");
        }
    }

}

/*!
  Save the specified \a profile.

  Saving a profile will overwrite any previously saved information.
 */
void QPhoneProfileManager::saveProfile(const QPhoneProfile &profile)
{
    if(profile.id() == -1 /* XXX || !profile.mTainted */)
        return;

    d->m_profiles.insert(profile.id(), profile);

    // Actually write to disk
    QSettings c("Trolltech", "PhoneProfile");
    c.remove("Profile " + QString::number(profile.id()));
    c.beginGroup("Profile " + QString::number(profile.id()));
    profile.write(c);
    c.endGroup();
    c.beginGroup("Profiles");
    QStringList profs =
        c.value("Profiles").toString().split(',', QString::SkipEmptyParts);
    if(!profs.contains(QString::number(profile.id()))) {
        profs.append(QString::number(profile.id()));
        c.setValue("Profiles", profs.join(QString(',')));
    }

    if(profile.id() > d->m_maxId)
        d->m_maxId = profile.id();

    QtopiaIpcEnvelope env("QPE/PhoneProfiles", "profileChanged()");
}

/*!
  Remove \a profile.  If \a profile is a null profile, no action will be
  taken.

  If \a profile is active, the default profile (QPhoneProfile::id() == 1)
  will be activated.
 */
void QPhoneProfileManager::removeProfile(const QPhoneProfile &profile)
{
    if(-1 == profile.id())
        return;

    bool wasActive = false;
    if(activeProfile().id() == profile.id()) {
        wasActive = true;
        if(activeProfile().id() == 1) {
            activateProfile(QPhoneProfile());
        } else {
            activateProfile(this->profile(1));
        }
    }

    QPhoneProfileManagerPrivate::Profiles::Iterator iter =
        d->m_profiles.find(profile.id());
    if(iter != d->m_profiles.end()) {
        d->m_profiles.erase(iter);
        QSettings c("Trolltech", "PhoneProfile");
        c.remove("Profile " + QString::number(profile.id()));
        // remove id from id list
        c.beginGroup("Profiles"); // no tr
        QStringList profiles = c.value("Profiles").toString().split(','); // no tr
        profiles.removeAll(QString::number(profile.id()));
        c.setValue("Profiles", profiles.join(QString(',')));
    }

    if(!wasActive)
        QtopiaIpcEnvelope env("QPE/PhoneProfiles", "profileChanged()");
}

/*!
  Returns a new QPhoneProfile instance with its id set to the next available
  integer.
  */
QPhoneProfile QPhoneProfileManager::newProfile()
{
    return QPhoneProfile(++d->m_maxId);
}

void QPhoneProfileManager::profMessage(const QString &msg,
                                       const QByteArray &)
{
    if("profileChanged()" == msg)
        sync();
}

/*!
  Force the re-reading of the profile information.
  */
void QPhoneProfileManager::sync()
{
    if(d->m_constructing) {
        loadConfig();
        return;
    }

    bool oldPlaneMode = planeMode();
    QPhoneProfileManagerPrivate old(*d);
    loadConfig();

    // Work out what changed
    bool activeChanged = old.m_selected != d->m_selected;
    bool hasPlaneModeChanged = oldPlaneMode != planeMode();

    QList<QPhoneProfile> added;
    QList<QPhoneProfile> changed;

    for(QPhoneProfileManagerPrivate::Profiles::ConstIterator iter = d->m_profiles.begin(); iter != d->m_profiles.end(); ++iter) {

        QPhoneProfileManagerPrivate::Profiles::Iterator olditer = old.m_profiles.find(iter.key());

        if(olditer == old.m_profiles.end()) {
            added.append(*iter);
        } else if(*olditer != *iter) {
            if(iter->id() == d->m_selected)
                activeChanged = true;
            changed.append(*iter);
        }

        old.m_profiles.erase(olditer);
    }


    for(QPhoneProfileManagerPrivate::Profiles::ConstIterator iter = old.m_profiles.begin(); iter != old.m_profiles.end(); ++iter) {
        emit profileRemoved(*iter);
    }

    for(int ii = 0; ii < changed.count(); ++ii)
        emit profileUpdated(changed.at(ii));

    for(int ii = 0; ii < added.count(); ++ii)
        emit profileAdded(added.at(ii));

    if(activeChanged)
        emit activeProfileChanged(activeProfile());

    if(hasPlaneModeChanged)
        emit planeModeChanged(planeMode());
}

/*!
  \fn void QPhoneProfileManager::planeModeChanged(bool enabled)

  Emitted whenever the plane mode state changes.  \a enabled will be set to
  true when plane mode is on, and false otherwise.
 */

/*!
  \fn void QPhoneProfileManager::activeProfileChanged(const QPhoneProfile &profile)

  Emitted whenever the active \a profile changes or is updated.
  */

/*!
  \fn void QPhoneProfileManager::profileUpdated(const QPhoneProfile &profile)

  Emitted whenever \a profile is updated.
 */

/*!
  \fn void QPhoneProfileManager::profileAdded(const QPhoneProfile &profile)

  Emitted when a new \a profile is added.
 */

/*!
  \fn void QPhoneProfileManager::profileRemoved(const QPhoneProfile &profile)

  Emitted when \a profile is removed.
 */

