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
#ifndef ABEDITOR_H
#define ABEDITOR_H

#include <qtopia/pim/qcontact.h>
#include <qtopia/pim/qcontactmodel.h>

#include <QDialog>
#include <QList>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QComboBox>
#include <QPixmap>
#include <QPushButton>
#include <QLineEdit>

#ifdef QTOPIA_CELL
#include "../../settings/ringprofile/ringtoneeditor.h"
#endif

class QIconSelector;
class QVBoxLayout;
class QShowEvent;
class QKeyEvent;
class QPaintEvent;
class QLineEdit;
class QCheckBox;
class QLabel;
class QComboBox;
class QRadioButton;
class QButtonGroup;
class QHBox;
class QTabWidget;
class QToolButton;
class QCategorySelector;
class QDateEdit;
class QGroupBox;
class QTextEdit;
class AbDetailEditor;
class QGridLayout;
class QAction;
class QScrollArea;
#ifdef QTOPIA_PHONE
class QMenu;
#endif
class QDLEditClient;



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
    void typeChanged(const PhoneFieldType&);
    void userChangedType(const PhoneFieldType&);
    void internalChangedType(const PhoneFieldType&);
    void numberChanged(const QString&);
    void fieldChanged(const QString&,const PhoneFieldType&);
protected:
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
    PhoneFieldManager( QWidget *parent, QGridLayout *layout, int rc );
    ~PhoneFieldManager();

    //add field. use existing empty field if available, otherwise, addBlank
    void add( const QString &number, const PhoneFieldType &type );
    void addEmpty();
    bool isFull() const;

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

public slots:
protected slots:
    void accept();
protected:
    bool eventFilter( QObject *receiver, QEvent *event );

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
    AbEditor( QWidget* parent = 0, Qt::WFlags fl = 0 );
    ~AbEditor();

    void setCategory(int);
    void setNameFocus();
    QContact entry() const { return ent; }

    bool isEmpty();

    bool imageModified() const;

signals:
    void categoriesChanged(); // for Qtopia Desktop only

public slots:
    void setEntry(const QContact &entry, bool newEntry = false
#ifdef QTOPIA_PHONE
        , bool simEntry = false
#endif
        );

protected slots:
    void editPhoto();

    void showSpecWidgets( bool s );
    void catCheckBoxChanged( bool b );
    void categorySelectChanged(const QList<QString>& cats);

    //Communication between Contact tab and details on other tabs

    void specFieldsFilter( const QString &newValue );
    void phoneFieldsToDetailsFilter( const QString &newNumber, const PhoneFieldType &newType );
    void detailsToPhoneFieldsFilter( const QString &newNumber );

    void accept();
    void reject();
    void tabClicked( QWidget *tab );
    void editEmails();
    void tabChanged(int);

protected:
    void closeEvent(QCloseEvent *e);
    void showEvent( QShowEvent *e );

#ifdef QTOPIA_PHONE
    void keyPressEvent(QKeyEvent *e);
#endif

private:
    void init();
    void initMainUI();

#if QTOPIA_PHONE
    void initSimUI();
#endif

    void setupTabs();
    void setupTabOther();
    void setupPhoneFields( QWidget *parent = 0 );
    void buildLineEditList();
    void setEntryOther();

    void contactFromFields(QContact &);

private:
    bool mImageModified;
    bool mNewEntry;

    QPixmap mContactImage;

    QContact ent;
    QTextEdit *txtNote;
    QDLEditClient *txtNoteQC;
    QTabWidget *tabs;
    QScrollArea *contactTab, *businessTab, *personalTab, *otherTab;
    QWidget *summaryTab;
    QTextEdit *summary;

    bool mainUIInit;
#ifdef QTOPIA_PHONE
    bool simUIInit;
#endif

    PhoneFieldType  mHPType, mHMType, mHFType, mBPType, mBMType,
                    mBFType, mBPAType;

    bool lastUpdateInternal;

    QList<PhoneFieldType> phoneTypes;

    QMap<QContactModel::Field, QLineEdit *> lineEdits;

    QVBoxLayout* mainVBox;

    //
    //  SIM-plified contact dialog
    //

#ifdef QTOPIA_PHONE
    QWidget *simEditor;
    QLineEdit *simName;
    QLineEdit *simNumber;
    bool editingSim;
#endif

    //
    //  Contact Tab
    //

    AbstractName *abName;
    QLineEdit *phoneLE, *mobileLE;
    QCategorySelector *cmbCat;
    QComboBox *genderCombo;
    QCheckBox *categoryCB;
    QLineEdit *emailLE;
    QPushButton *emailBtn;
    QDateEdit *bdayEdit;
    QDateEdit *anniversaryEdit;
    QGroupBox *bdayCheck;
    QGroupBox *anniversaryCheck;
    QHBox *ehb;
    bool quitExplicitly;

#ifdef QTOPIA_VOIP
    QLineEdit *voipIdLE;
#endif

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

    QTextEdit *busStreetME, *homeStreetME;
    QLineEdit *busCityLE, *busStateLE, *busZipLE, *busCountryLE,
              *homeCityLE, *homeStateLE, *homeZipLE, *homeCountryLE;

    QToolButton *photoPB;
    PhoneFieldManager *phoneMan;

#ifdef QTOPIA_PHONE
    QAction *actionEmailDetails;
#endif
#ifdef QTOPIA_CELL
    RingToneButton *editTonePB;
#endif
    bool tabOtherInit;
    QWidget *wOtherTab;
    QWidget *wBusinessTab;
    QWidget *wPersonalTab;
};

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
