/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
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
#ifndef ABEDITOR_H
#define ABEDITOR_H

#include <qtopia/pim/qcontact.h>
#include <qtopia/pim/qappointment.h>
#include <qtopia/pim/qcontactmodel.h>
#include <qtopia/qcontent.h>

#include <QDialog>
#include <QList>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QPixmap>
#include <QLineEdit>

class QIconSelector;
class QVBoxLayout;
class QShowEvent;
class QKeyEvent;
class QCheckBox;
class QLabel;
class QComboBox;
class QHBox;
class QTabWidget;
class QToolButton;
class QPushButton;
class QCategorySelector;
class QDateEdit;
class QGroupBox;
class QTextEdit;
class QGridLayout;
class QAction;
class QScrollArea;
class QDLEditClient;
class RingToneButton;
class ReminderPicker;
class GroupView;
class QCategoryManager;
class AbFullEditor;
class PhoneFieldManager;

//-----------------------------------------------------------------------

class PhoneFieldType
{
public:
    PhoneFieldType();
    PhoneFieldType( const QString &id, const QString &str );
    PhoneFieldType( const PhoneFieldType &other );
    PhoneFieldType &operator=( const PhoneFieldType &other );
    bool operator==( const PhoneFieldType &other ) const;
    bool operator!=( const PhoneFieldType &other ) const;
    bool isNull() const { return id.isEmpty(); }

    QString id;
    QIcon icon;
    QString name;
};

//-----------------------------------------------------------------------

class PhoneField : public QObject
{
    Q_OBJECT
public:
    PhoneField( QGridLayout *l, int &rowCount, QWidget *parent = 0 );
    ~PhoneField();

    void setTypes( const QList<PhoneFieldType> &newTypes );

    void setType( const PhoneFieldType &newType );
    PhoneFieldType type();

    QString number() const;
    bool isEmpty() const;
    void setNumber( const QString &newNumber );

    void remove();
protected slots:
    void emitFieldChanged();
    void userChangedType( int idx );
signals:
    void typeChanged(const PhoneFieldType& newType);
    void userChangedType(const PhoneFieldType& newType);
    void internalChangedType(const PhoneFieldType& newType);
    void numberChanged(const QString&);
    void fieldChanged(const QString&,const PhoneFieldType&);
protected:
    friend class PhoneFieldManager;
    QLineEdit *numberLE;

    QIconSelector *typeIS;

private:
    QList<PhoneFieldType> mTypes;
};

//-----------------------------------------------------------------------

// manages the creation of PhoneField children and provides an interface to access them
class PhoneFieldManager : public QObject
{
    Q_OBJECT

public:
    PhoneFieldManager( QWidget *parent, QGridLayout *layout, int rc, AbFullEditor *editor);
    ~PhoneFieldManager();

    //add field. use existing empty field if available, otherwise, addBlank
    void add( const QString &number, const PhoneFieldType &type );
    void addEmpty();
    bool isFull() const;
    bool isEmpty() const;

    bool removeNumber(QWidget *victim);
    bool removeNumber(PhoneFieldType type);

    void setTypes( const QList<PhoneFieldType> &newTypes );
    QList<PhoneFieldType> types() const;

    void setNumberFromType( const PhoneFieldType &type, const QString &newNumber );
    QString numberFromType( const PhoneFieldType &type );

    void clear();

protected slots:
    void emitFieldChanged( const QString &number, const PhoneFieldType &type );

    //  controls the available types phonefields have when the user changes
    //  the type of one of them
    void updateTypes( const PhoneFieldType &newType );
    void checkForAdd(); //  a field has changed, check to see if we need to add()

signals:
    void fieldChanged(const QString&, const PhoneFieldType&);

protected:
    QGridLayout *parLayout;
    int rowCount;
    int firstRow;
    QList<PhoneField*> phoneFields;
    QList<PhoneFieldType> mTypes;
    AbFullEditor *mEditor;

private:
    bool mEmitFieldChanged;
};

//-----------------------------------------------------------------------

// detail editor ; constructs a dialog to edit fields specified by a key => value map
class AbDetailEditor : public QDialog
{
    Q_OBJECT
public:
    AbDetailEditor( QWidget *parent = 0, Qt::WFlags fl = 0 );
    ~AbDetailEditor();

    QList<QContactModel::Field> guiList( const QMap<QContactModel::Field, QString> &f ) const;
    void setFields( const QMap<QContactModel::Field, QString> &f );
    QMap<QContactModel::Field, QString> fields() const;

protected slots:
    void accept();
protected:
    virtual const QMap<QContactModel::Field, QString> displayNames() const;

    QMap<QContactModel::Field, QString> myFields;
    QComboBox *suffixCombo;
    QComboBox *titleCombo;

    QMap<QContactModel::Field, QLineEdit *> lineEdits;
private:
    QScrollArea *mView;
    QVBoxLayout *editorLayout;
};

//-----------------------------------------------------------------------

class AbstractField : public QLineEdit
{
    Q_OBJECT
public:
    AbstractField( QWidget *parent = 0 );
    ~AbstractField();

    void setFields( const QMap<QContactModel::Field, QString> &fields );
    QMap<QContactModel::Field, QString> fields() const;

    virtual bool isEmpty() const;

    QStringList tokenize( const QString &newText ) const;

    virtual QString fieldName() const = 0;

    bool modified() const;
    void setModified( bool b );

public slots:
    virtual void parse() = 0;
    virtual void fieldsChanged() = 0;
    void details();

protected:
    QMap<QContactModel::Field, QString> myFields;
    bool mModified;

private:
    AbDetailEditor *detailEditor;
};

//-----------------------------------------------------------------------

// AbstractName field handles parsing of user input and calls subdialog to handle details
class AbstractName : public AbstractField
{
    Q_OBJECT
public:
    AbstractName( QWidget *parent = 0 );
    ~AbstractName();

    QString fieldName() const;

    bool isEmpty() const;

public slots:
    void parse();
    void fieldsChanged();

private slots:
    void textChanged();
private:
    bool m_preventModified;
};

//-----------------------------------------------------------------------

class AbEditor : public QDialog
{
    Q_OBJECT
public:
    AbEditor( QWidget* parent = 0, Qt::WFlags fl = 0 )
        : QDialog(parent, fl)
    {}
    ~AbEditor()
    {}

    virtual QContact entry() const = 0;

    virtual bool isEmpty() const = 0;

    virtual void setEntry(const QContact &entry, bool newEntry) = 0;

    virtual bool imageModified() const { return false; }

    virtual QAppointment::AlarmFlags anniversaryReminder() {return QAppointment::NoAlarm;}
    virtual int anniversaryReminderDelay() {return 0;}

    virtual QAppointment::AlarmFlags birthdayReminder() {return QAppointment::NoAlarm;}
    virtual int birthdayReminderDelay() {return 0;}
};

class QDelayedScrollArea;
class AbFullEditor : public AbEditor
{
    Q_OBJECT
public:
    AbFullEditor( QWidget* parent = 0, Qt::WFlags fl = 0 );
    ~AbFullEditor();

    void setCategory(int);
    void setNameFocus();
    QContact entry() const { return ent; }

    bool isEmpty() const;

    bool imageModified() const;

    void setEntry(const QContact &entry, bool newEntry);

    virtual QAppointment::AlarmFlags anniversaryReminder() {return anniversaryAppt.alarm();}
    virtual int anniversaryReminderDelay() {return anniversaryAppt.alarmDelay();}

    virtual QAppointment::AlarmFlags birthdayReminder() {return birthdayAppt.alarm();}
    virtual int birthdayReminderDelay() {return birthdayAppt.alarmDelay();}

    void addRemoveNumberMenu(QWidget *field);

protected slots:
    void editPhoto();

    void editGroups();

    void updateContextMenu();

    void showSpecWidgets( bool s );
    void catCheckBoxChanged( bool b );

    //Communication between Contact tab and details on other tabs

    void specFieldsFilter( const QString &newValue );
    void phoneFieldsToDetailsFilter( const QString &newNumber, const PhoneFieldType &newType );
    void detailsToPhoneFieldsFilter( const QString &newNumber );

    PhoneFieldType findPhoneField(QLineEdit *detail);

    void accept();
    void reject();
    void tabClicked( QWidget *tab );
    void editEmails();
    void prepareTab(int);

    void removeNumber();

    void toneSelected( const QContent &tone );

protected:
    void closeEvent(QCloseEvent *e);
    void showEvent( QShowEvent *e );

private:
    void init();
    void initMainUI();

    void setupTabs();

    void setupTabCommon();
    void setupTabWork();
    void setupTabHome();
    void setupTabOther();

    void setupPhoneFields( QWidget *parent = 0 );

    void setEntryWork();
    void setEntryHome();
    void setEntryOther();

    void contactFromFields(QContact &);

    void updateGroupButton();

    void updateAppts();

private:
    bool mImageModified;
    bool mNewEntry;

    QContent mContactImage;

    QContact ent;
    QTextEdit *txtNote;
    QDLEditClient *txtNoteQC;
    QTabWidget *tabs;
    QDelayedScrollArea *contactTab, *businessTab, *personalTab, *otherTab;
    QWidget *summaryTab;
    QTextEdit *summary;

    PhoneFieldType  mHPType, mHMType, mHFType, mBPType, mBMType,
                    mBFType, mBPAType, mHVType, mBVType;

    bool lastUpdateInternal;

    QList<PhoneFieldType> phoneTypes;

    QMap<QContactModel::Field, QLineEdit *> lineEdits;

    QVBoxLayout* mainVBox;

    //
    //  Contact Tab
    //

    AbstractName *abName;
    QLineEdit *phoneLE, *mobileLE;
    QPushButton *catPB;
    QComboBox *genderCombo;
    QCheckBox *categoryCB;
    QLineEdit *emailLE;
    QPushButton *emailBtn;
    QDateEdit *bdayEdit;
    QDateEdit *anniversaryEdit;
    QGroupBox *bdayCheck;
    QGroupBox *anniversaryCheck;
    QHBox *ehb;

    //
    //  Widgets specific to the contact type
    //

    QLineEdit *specCompanyLE, *specJobTitleLE;
    QLabel *specCompanyLA, *specJobTitleLA;

    //
    //  Business tab widgets
    //

    QLineEdit *companyLE, *companyProLE, *jobTitleLE, *busPhoneLE, *busFaxLE,
              *busMobileLE, *busPagerLE, *busWebPageLE, *deptLE, *officeLE,
              *professionLE, *managerLE, *assistantLE;

    //Home tab widgets
    QLineEdit *homePhoneLE, *homeFaxLE, *homeMobileLE, *homeWebPageLE,
              *spouseLE, *anniversaryLE, *childrenLE;

#if defined(QTOPIA_VOIP)
    QLineEdit *busVoipLE, *homeVoipLE;
#endif

    QTextEdit *busStreetME, *homeStreetME;
    QLineEdit *busCityLE, *busStateLE, *busZipLE, *busCountryLE,
              *homeCityLE, *homeStateLE, *homeZipLE, *homeCountryLE;

    ReminderPicker *anniversaryRP, *birthdayRP;
    QAppointment anniversaryAppt, birthdayAppt;

    QToolButton *photoPB;
    PhoneFieldManager *phoneMan;

    QAction *actionEmailDetails;
#if defined(QTOPIA_TELEPHONY)
    RingToneButton *editTonePB, *editVideoTonePB;
#endif
    QWidget *wOtherTab;
    QWidget *wBusinessTab;
    QWidget *wPersonalTab;
    GroupView *mGroupPicker;
    QCategoryManager *mCatMan;
    QStringList mGroupList;
    QDialog *mGroupDialog;

    QAction* actionAddGroup;
    QAction* actionRemoveGroup;
    QAction* actionRenameGroup;
#if defined(QTOPIA_TELEPHONY)
    QAction* actionSetRingTone;
#endif
    QAction *actionRemoveNumber;
};

#ifdef QTOPIA_CELL
class AbSimEditor : public AbEditor
{
    Q_OBJECT
public:
    AbSimEditor( QWidget* parent = 0, Qt::WFlags fl = 0 );
    ~AbSimEditor();

    void setNameFocus();

    QContact entry() const { return ent; }

    bool isEmpty() const;

    void setEntry(const QContact &entry, bool newEntry);

protected slots:
    void accept();
    void reject();
private:
    void initSimUI();

    //
    //  SIM-plified contact dialog
    //

    QWidget *simEditor;
    QLineEdit *simName;
    QLineEdit *simNumber;

    QContact ent;

    bool mNewEntry;
};
#endif

//-----------------------------------------------------------------------

class PhoneFieldLineEdit : public QLineEdit
{
    Q_OBJECT

public:
    PhoneFieldLineEdit( QWidget *typeSibling, QWidget *parent );
    bool eventFilter( QObject *o, QEvent *e );

public slots:
    void appendText( const QString &txt );

private:
    QWidget *mTypeSibling;
};

#endif
