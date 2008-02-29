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

#include <QListWidget>
#include <QDialog>
#include <QMap>
#include <QPhoneProfile>
#include <QPhoneProfileManager>
#include <QtopiaAbstractService>

class QListWidget;
class QDialog;
class QAction;
class QMenu;
class QGroupBox;
class QGridLayout;
class QTabWidget;
class QLabel;
class QLineEdit;
class QVBoxLayout;
class QSpinBox;
class QCheckBox;
class QComboBox;
class RingToneButton;
class QListWidget;
class QHBoxLayout;
class QToolButton;
class QTimeEdit;
class QPushButton;
class QSlider;
class PhoneProfileItem;

class ProfileEditDialog : public QDialog
{
    Q_OBJECT
public:
    ProfileEditDialog(QWidget *parent, bool isnew);
    ~ProfileEditDialog();
    void setProfile(PhoneProfileItem *);
    void setActive( bool b ) { isActive = b; };
    bool isModified() const { return mIsModified; }
    void addSetting( const QPhoneProfile::Setting s );

protected:
    void accept();
    bool event(QEvent *);

private:
    void init();
    void setPhoneProfile();
    void setSettings();
    void setSchedule();
    void initDialog( QDialog *dlg );
    void processSchedule();
#ifdef QTOPIA_PHONE
    bool eventFilter( QObject *o, QEvent *e );
#endif
    bool showSettingList( QStringList &settingList );

private slots:
    void updateState();
    void viewSetting();
    void deleteSetting();
#ifdef QTOPIA_PHONE
    void setSoftMenu(int);
    void tabChanged(int);
#endif
    void showEditScheduleDialog();
    void enableEditSchedule(bool);
    void pullSettingStatus();

private:
    QTabWidget *tabWidget;

    QWidget *infoTab, *toneTab, *settingTab;
    QGridLayout *infoLayout, *ringToneLayout, *messageToneLayout;
    QVBoxLayout *infoTabLayout, *toneTabLayout, *settingLayout, *scheduleVLayout;
    QHBoxLayout *scheduleHLayout1, *scheduleHLayout2;

    QLabel *lblName, *profileLabel;
    QLineEdit *profileName;
    QLabel *lblVolume;
    QSlider *masterVolume;
    QCheckBox *autoAnswer, *vibrateAlert, *autoActivation;
    QPushButton *editSchedule;
    QLabel *lblTime;
    QTimeEdit *time;
    QToolButton **days;

    QGroupBox *ringToneGroup;
    QLabel *lblAlert1, *lblTone1;
    QComboBox *ringType;
    RingToneButton *ringTone;

    QGroupBox *messageToneGroup;
    QLabel *lblAlert2, *lblTone2;
    QComboBox *messageType;
    QSpinBox *messageAlertDuration;
    RingToneButton *messageTone;

    QListWidget *settingListWidget;

    PhoneProfileItem *profile;
    QPhoneProfile::Settings settings;
    int id;
    bool isActive;
    bool mIsModified;
    bool isLoading;
    bool deleteDays;
    bool isNew;
    bool editVolume;
    int mvOrigValue;
    QPhoneProfile::Schedule schedule;

    QMenu *contextMenu;
    QAction *actionView;
    QAction *actionDelete;
    QAction *actionCapture;
};

class PhoneProfileItem : public QListWidgetItem {
public:
    PhoneProfileItem(const QPhoneProfile &pr, QListWidget *l = 0);

    const QPhoneProfile &profile() const { return rpp; }
    QPhoneProfile &profile() { return rpp; }

    int width( const QListWidget* lb ) const;
    int height( const QListWidget* lb ) const;

private:
    QIcon cache;
    QPhoneProfile rpp;
};

class ActiveProfileDisplay;
class ProfileSelect : public QDialog
{
    Q_OBJECT
    friend class ProfilesService;

public:
    ProfileSelect( QWidget *parent = 0, Qt::WFlags f = 0, const char *name = 0);
    ~ProfileSelect();

public slots:
    void appMessage(const QString &msg, const QByteArray &data);
    void activatePlaneMode();

protected:
    void closeEvent(QCloseEvent *);

private slots:
    void activateProfile();
    void activateProfile(int);
    void activateProfile(QListWidgetItem *);
    void activateProfileAndClose(QListWidgetItem *);
    void activateProfileAndClose();
    void editProfile();
    void editProfile(QListWidgetItem *);
    void editProfile(PhoneProfileItem *);
    void createNewProfile();
    void removeCurrentProfile();
    void setPlaneMode(bool);
    void addToSpeedDial();
    void updateIcons();

private:
    void setActiveProfile( PhoneProfileItem *pItem, bool = false );
    void loadConfig();
    void capture();
    QString findUniqueName( const QString &name, int curIndex );
    void makeProfileNameUnique( PhoneProfileItem * );

    QAction *actionNew, *actionEdit, *actionRemove, *actionActivate, *actionSpeedDial;
    QAction *actionPlane;
    bool isLoading;

    QListWidget *itemlist;
    ActiveProfileDisplay *activeDisplay;
    ProfileEditDialog *editDialog;

    PhoneProfileItem *activeProfile;

    QPhoneProfileManager profMan;

    bool origPlaneMode;
    QString appName, appTitle, details;

#ifdef QTOPIA_PHONE
    QMenu *contextMenu;
#endif
};

class ProfilesService : public QtopiaAbstractService
{
    Q_OBJECT
    friend class ProfileSelect;
private:
    ProfilesService( ProfileSelect *parent )
        : QtopiaAbstractService( "Profiles", parent )
        { this->parent = parent; publishAll(); }

public:
    ~ProfilesService();

public slots:
    void activatePlaneMode();
    void showProfiles();
    void setProfile( int id );

private:
    ProfileSelect *parent;
};

