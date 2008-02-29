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

#ifndef _QPHONEPROFILE_H_
#define _QPHONEPROFILE_H_

#include <QSharedDataPointer>
#include <QString>
#include <QContent>

class QSettings;
class QTranslatableSettings;
class QPhoneProfilePrivate;
class QTOPIA_EXPORT QPhoneProfile {
public:
    class QTOPIA_EXPORT Setting {
    public:
        Setting();
        explicit Setting(const QString &);
        Setting(const Setting &);
        Setting &operator=(const Setting &);

        bool isNull() const;

        bool operator==(const Setting &) const;
        bool operator!=(const Setting &) const;

        QString applicationName() const;
        QString description() const;
        QString data() const;
        bool notifyOnChange() const;

        void setApplicationName(const QString &);
        void setDescription(const QString &);
        void setData(const QString &);
        void setNotifyOnChange(bool);

    private:
        QString appName;
        QString appTitle;
        QString details;
        bool notify;
    };
    typedef QMap<QString,Setting> Settings;

    class QTOPIA_EXPORT Schedule {
    public:
        Schedule();
        explicit Schedule(const QString &);
        Schedule(const Schedule &);
        Schedule &operator=(const Schedule &);
        bool operator==(const Schedule &) const;
        bool operator!=(const Schedule &) const;

        bool isActive() const;
        void setActive(bool);

        QTime time() const;
        void setTime(const QTime &);

        QList<Qt::DayOfWeek> scheduledOnDays() const;
        bool scheduledOnDay(Qt::DayOfWeek) const;
        void setScheduledDay(Qt::DayOfWeek);
        void unsetScheduledDay(Qt::DayOfWeek);
        void clearScheduledDays();

        QString toString() const;
        void fromString(const QString &);

    private:
        bool _active;
        QTime _time;
        unsigned char _days;
    };

    enum AlertType {
        Off,
        Once,
        Continuous,
        Ascending
    };

    QPhoneProfile();
    explicit QPhoneProfile(int id);
    QPhoneProfile(const QPhoneProfile &);
    QPhoneProfile &operator=(const QPhoneProfile &);
    virtual ~QPhoneProfile();

    bool operator==(const QPhoneProfile &) const;
    bool operator!=(const QPhoneProfile &) const;

    bool isNull() const;

    QString name() const;
    bool isSystemProfile() const;
    int volume() const;
    bool vibrate() const;
    AlertType callAlert() const;
    AlertType msgAlert() const;
    int msgAlertDuration() const;
    bool autoAnswer() const;
    QContent callTone() const;
    QContent systemCallTone() const;
    QContent messageTone() const;
    QContent systemMessageTone() const;
    bool planeMode() const;
    Settings applicationSettings() const;
    Setting applicationSetting(const QString &) const;
    QString icon() const;
    Schedule schedule() const;
    int id() const;
    QString accessory() const;
    QString speedDialInput() const;

    void setId(int id);
    void setIcon(const QString &);
    void setName(const QString &);
    void setIsSystemProfile(bool);
    void setVolume(int);
    void setVibrate(bool);
    void setCallAlert(AlertType);
    void setMsgAlert(AlertType );
    void setMsgAlertDuration(int);
    void setAutoAnswer(bool);
    void setPlaneMode(bool);
    void setApplicationSettings(const Settings &);
    void setApplicationSetting(const Setting &);
    void setSchedule(const Schedule &);
    void setCallTone(const QContent &);
    void setMessageTone(const QContent &);
    void setAccessory(const QString &);
    void setSpeedDialInput(const QString &);

private:
    QSharedDataPointer<QPhoneProfilePrivate> d;
    void read(QTranslatableSettings &);
    void write(QSettings &) const;
    friend class QPhoneProfileManager;
};

class QPhoneProfileManagerPrivate;
class QTOPIA_EXPORT QPhoneProfileManager : public QObject
{
Q_OBJECT
public:
    explicit QPhoneProfileManager(QObject *parent = 0);
    virtual ~QPhoneProfileManager();

    QPhoneProfile activeProfile() const;
    bool activateProfile(const QPhoneProfile &);
    bool activateProfile(int);

    bool planeMode() const;
    bool planeModeOverride() const;
    void setPlaneModeOverride(bool);

    bool planeModeAvailable() const;

    QList<QPhoneProfile> profiles() const;
    QList<int> profileIds() const;

    QPhoneProfile profile(int) const;
    void saveProfile(const QPhoneProfile &);
    void removeProfile(const QPhoneProfile &);

    QPhoneProfile newProfile();

    void sync();

signals:
    void planeModeChanged(bool);
    void activeProfileChanged(const QPhoneProfile &);
    void profileUpdated(const QPhoneProfile &);
    void profileAdded(const QPhoneProfile &);
    void profileRemoved(const QPhoneProfile &);

private slots:
    void profMessage(const QString &msg, const QByteArray &data);

private:
    void loadConfig();
    Q_DISABLE_COPY(QPhoneProfileManager);

    QPhoneProfileManagerPrivate *d;

    void activateProfile(const QPhoneProfile &newProfile,
                         const QPhoneProfile &oldProfile);
    void activateSettings(const QPhoneProfile::Settings &current,
                          const QPhoneProfile::Settings &previous);

};

#endif // _QPHONEPROFILE_H_
