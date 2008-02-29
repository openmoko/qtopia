/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
** 
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
** 
** A copy of the GNU GPL license version 2 is included in this package as 
** LICENSE.GPL.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
** See the GNU General Public License for more details.
**
** In addition, as a special exception Trolltech gives permission to link
** the code of this program with Qtopia applications copyrighted, developed
** and distributed by Trolltech under the terms of the Qtopia Personal Use
** License Agreement. You must comply with the GNU General Public License
** in all respects for all of the code used other than the applications
** licensed under the Qtopia Personal Use License Agreement. If you modify
** this file, you may extend this exception to your version of the file,
** but you are not obligated to do so. If you do not wish to do so, delete
** this exception statement from your version.
** 
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#ifndef ABEDITOR_H
#define ABEDITOR_H

#include <qtopia/pim/contact.h>

#include <qdialog.h>
#include <qlist.h>
#include <qmap.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qwidgetstack.h>
#include <qcombobox.h>
#include <qpixmap.h>
#include <qpushbutton.h>
#include <qlineedit.h>

#if defined(QTOPIA_PHONE) && !defined(QTOPIA_DESKTOP)
#include "../../settings/ringprofile/ringtoneeditor.h"
#endif

class IconSelect;
class PixmapDisplay;
class QVBoxLayout;
class QShowEvent;
class QKeyEvent;
class QPaintEvent;
class QScrollView;
class QMultiLineEdit;
class QLineEdit;
class QCheckBox;
class QLabel;
class QComboBox;
class QRadioButton;
class QButtonGroup;
class QHBox;
class QTabWidget;
class CategorySelect;
class QPEDateEdit;
class QTextEdit;
class AbDetailEditor;
class FileAsCombo;
class VScrollView;
class AbDetailEditor;
class QGridLayout;
class QAction;
#ifdef QTOPIA_PHONE
class ContextMenu;
#endif

QPixmap scalePixmapToMax( QPixmap &, const double &, const double & );

class FileAsCombo : public QComboBox
{
    Q_OBJECT
public:
    FileAsCombo( QWidget *parent );
    void setSelected( const QString &fileAs );
    QString selected() const;

public slots:
    void setPrefix( const QString &prefix );
    void setSuffix( const QString &suffix );
    void setFirstName( const QString &firstName );
    void setMiddleName( const QString &middleName );
    void setLastName( const QString &lastName );
    void setNickName( const QString &nickName );
    void setFirstNamePro( const QString &firstNamePro );
    void setLastNamePro( const QString &lastNamePro );
    void update();

private slots:
    void customFileAs( int );

private:
    QString fillTemplate( const QString &t, bool allowEmpty = FALSE );
    QMap<QString,QString> mTemplateVars;
    QStringList mTemplates;
    QString mDefaultTemplate;
    QStringList mCustomEntries;
};

//--------------------------------------------------------------------------------

class PhoneFieldType
{
public:
    PhoneFieldType();
    PhoneFieldType( const QString &id, const QString &str );
    PhoneFieldType( const PhoneFieldType &other );
    PhoneFieldType &operator=( const PhoneFieldType &other );
    bool  operator==( const PhoneFieldType &other ) const;
    bool operator!=( const PhoneFieldType &other ) const;
    bool isNull() const { return id.isEmpty(); }

    QString id;
    QPixmap pixmap;
    QString name;
};

// PhoneField comprising a lineedit for the number and a combobox for the type
class PhoneField : public QObject 
{
    Q_OBJECT
public:
    PhoneField( QGridLayout *l, int &rowCount, QWidget *parent = 0, const char *name = 0 );
    ~PhoneField();

    void setTypes( const QValueList<PhoneFieldType> &newTypes );

    void setType( const PhoneFieldType &newType );
    PhoneFieldType type();

    QString number() const;
    bool isEmpty() const;
    void setNumber( const QString &newNumber );

#ifdef QTOPIA_PHONE
    void setOnSim( bool onSim );
    bool onSim() const;
    void setHaveSim( bool b );
#endif

protected slots:
    void emitFieldOnSim( bool f );
    void emitFieldChanged();
    void userChangedType( int idx );
signals:
    void typeChanged(const PhoneFieldType&);
    void userChangedType(const PhoneFieldType&);
    void internalChangedType(const PhoneFieldType&);
    void numberChanged(const QString&);
    void fieldChanged(const QString&,const PhoneFieldType&);
    void fieldOnSim(bool,const PhoneFieldType&); 
protected:
//    QLabel *label;
   QLineEdit *numberLE;
#ifdef QTOPIA_DESKTOP
    QComboBox 
#else
    IconSelect 
#endif
    *typeIS;
private:
#ifdef QTOPIA_PHONE
    QAction *simAction;
#endif

    QValueList<PhoneFieldType> mTypes;
};

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

    void setTypes( const QValueList<PhoneFieldType> &newTypes );
    QValueList<PhoneFieldType> types() const;

    void setNumberFromType( const PhoneFieldType &type, const QString &newNumber );
    QString numberFromType( const PhoneFieldType &type );

#ifdef QTOPIA_PHONE
    void setFieldOnSim( const PhoneFieldType &type, bool onSim );
    bool fieldOnSim( const PhoneFieldType &type ) const;
    void setHaveSim( bool b );
#endif
public slots:
protected slots:
    void emitFieldChanged( const QString &number, const PhoneFieldType &type );

    //controls the available types phonefields have when the user changes the type of one of them
    void updateTypes( const PhoneFieldType &newType );
    void checkForAdd(); //a field has changed, check to see if we need to add()
signals:
    void fieldChanged(const QString&, const PhoneFieldType&);
    void fieldOnSim(bool,const PhoneFieldType&); 
protected:
    QGridLayout *parLayout;
    int rowCount;
    QList<PhoneField> phoneFields;
    QValueList<PhoneFieldType> mTypes;

private:
    bool mEmitFieldChanged;
};

// detail editor ; constructs a dialog to edit fields specified by a key => value map
class AbDetailEditor : public QDialog
{
    friend class FileAsCombo;
    Q_OBJECT
public:
    AbDetailEditor( QWidget *parent = 0, const char *name = 0, WFlags fl = 
#ifdef QTOPIA_DESKTOP
    WStyle_Customize | WStyle_DialogBorder | WStyle_Title 
#else
    0
#endif    
    );
    ~AbDetailEditor();

    QValueList<PimContact::ContactFields> guiValueList( const QMap<PimContact::ContactFields, QString> &f ) const;
    void setFields( const QMap<PimContact::ContactFields, QString> &f );
    QMap<PimContact::ContactFields, QString> fields() const;
    
public slots:
protected slots:
    void accept();
protected:
    virtual const QMap<PimContact::ContactFields, QString> displayNames() const;

    QMap<PimContact::ContactFields, QString> myFields;
    FileAsCombo *fileAsCombo;
    QComboBox *suffixCombo;
    QComboBox *titleCombo;

    QMap<PimContact::ContactFields, QLineEdit *> lineEdits;
private:
    VScrollView *mView;
    QVBoxLayout *editorLayout;
};

class AbstractField : public QWidget
{
    Q_OBJECT
public:
    AbstractField( QWidget *parent = 0, const char *name = 0 );
    ~AbstractField();

    void setFields( const QMap<PimContact::ContactFields, QString> &fields );
    QMap<PimContact::ContactFields, QString> fields() const;

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
    QMap<PimContact::ContactFields, QString> myFields;
    bool mModified;

private:
    AbDetailEditor *detailEditor;
};

// AbstractName field handles parsing of user input and calls subdialog to handle details
class AbstractName : public AbstractField 
{
    Q_OBJECT
public:
    AbstractName( QWidget *parent = 0, const char *name = 0 );
    ~AbstractName();

    bool isQuote( QChar c ) const;
    QString stripQuotes( const QString & );
    QString formattedAffix( const QString &str, QStringList &affixes );

    QString fieldName() const;

    bool isEmpty() const;

public slots:
    void parse();
    void fieldsChanged();

private slots:
    void textChanged();
protected:
    QStringList prefixes, suffixes;
private:
    QLineEdit *mainWidget;
};

//--------------------------------------------------------------------------------

class AbEditor : public QDialog
{
    Q_OBJECT
public:
    AbEditor( QWidget* parent = 0, const char* name = 0,  WFlags fl = 
#ifdef QTOPIA_DESKTOP
    WStyle_Customize | WStyle_DialogBorder | WStyle_Title 
#else
    0
#endif    
    );
    ~AbEditor();

    void setCategory(int);
    void setNameFocus();
    PimContact entry() const { return ent; }

#ifdef QTOPIA_DESKTOP
    void updateCategories();
    CategorySelect *categorySelect() { return cmbCat; }
#endif

#ifdef QTOPIA_PHONE
    bool hasNonSimFields() const;
    void setHaveSim( bool b );
#endif

//    static QString createContactImage( QPixmap pix );

    bool isEmpty();
    
    bool imageModified() const;
 signals:
    void categoriesChanged(); // for Qtopia Desktop only


public slots:
    void setEntry( const PimContact &entry, bool newEntry = FALSE );

protected slots:

    void editPhoto();

    void showSpecWidgets( bool s );
    void catCheckBoxChanged( bool b );
    void categorySelectChanged( int catid );

    //Communication between Contact tab and details on other tabs

    void specFieldsFilter( const QString &newValue );
    void phoneFieldsToDetailsFilter( const QString &newNumber, const PhoneFieldType &newType );
    void detailsToPhoneFieldsFilter( const QString &newNumber );
    void detailSimFilter( bool onSim );
    void phoneManagerSimFilter( bool onSim, const PhoneFieldType &type );

    void accept();
    void reject();
    void tabClicked( QWidget *tab );
    void editEmails();

protected:
    void closeEvent(QCloseEvent *e);
    void showEvent( QShowEvent *e );
    
private:
    void init();
    void setupTabs();
    void setupPhoneFields( QWidget *parent = 0 );
    void setupSpecWidgets( QWidget *parent );
    void buildLineEditList();

    void contactFromFields(PimContact &);
    void setTabOrders(void);

private:
    bool mImageModified;
#ifdef QTOPIA_PHONE
    QMap<int,QAction *> simActions;
    bool mHaveSim;
#endif
    bool mNewEntry;

    QPixmap mContactImage;

    PimContact ent;
    QMultiLineEdit *txtNote;
    QTabWidget *tabs;
    VScrollView *contactTab, *businessTab, *personalTab, *otherTab;
    QWidget *summaryTab;
    QTextEdit *summary;

#if 0
    QPixmap *hpPM, *hfPM, *hmPM, *bpPM, *bfPM, *bmPM, *bpaPM;
#endif
    PhoneFieldType mHPType, mHMType, mHFType, mBPType, mBMType, mBFType, mBPAType;


    bool lastUpdateInternal;

    QValueList<PhoneFieldType> phoneTypes;

    QMap<PimContact::ContactFields, QLineEdit *> lineEdits;

    //Contact tab 
    QGridLayout *mainGL;
    AbstractName *abName;
    QLineEdit *phoneLE, *mobileLE;
    CategorySelect *cmbCat;
    QComboBox *genderCombo;
    QCheckBox *categoryCB;
    QLineEdit *emailLE;
    
    //voip
    QLineEdit *voipIdLE;
    QString strVoipStatus;
    
    QPushButton *emailBtn;
    QPEDateEdit *bdayButton;
    QPEDateEdit *anniversaryButton;
    QHBox *ehb;
    bool quitExplicitly;

    // widgets specific to the contact type
    QLineEdit *specCompanyLE, *specJobTitleLE;
    QLabel *specCompanyLA, *specJobTitleLA;
    int specRow;

    //Phone Manager
    int phoneManRow;
    bool phoneManOld;

    //Business tab widgets
    QLineEdit *companyLE, *companyProLE, *jobTitleLE, *busPhoneLE, *busFaxLE, 
	      *busMobileLE, *busPagerLE, *busWebPageLE, *deptLE, *officeLE,
	      *professionLE, *managerLE, *assistantLE;

    //Home tab widgets
    QLineEdit *homePhoneLE, *homeFaxLE, *homeMobileLE, *homeWebPageLE, 
	      *spouseLE, *anniversaryLE, *childrenLE;

    QMultiLineEdit *busStreetME, *homeStreetME;
    QLineEdit *busCityLE, *busStateLE, *busZipLE, *busCountryLE,
	      *homeCityLE, *homeStateLE, *homeZipLE, *homeCountryLE;

#ifdef QTOPIA_PHONE
    QAction *homePhoneOnSim, *homeFaxOnSim, *homeMobileOnSim,
	    *busPhoneOnSim, *busMobileOnSim, *busFaxOnSim, *busPagerOnSim;
#endif

    PhoneFieldManager *phoneMan;

#ifdef QTOPIA_PHONE
    ContextMenu *emailContextMenu;
    QAction *actionEmailDetails;
#endif
#if defined(QTOPIA_PHONE) && !defined(QTOPIA_DESKTOP)
    RingToneButton *editTonePB;
#endif
    PixmapDisplay *photoPB;
};

class IconSelect;
class PhoneFieldLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    PhoneFieldLineEdit( QWidget *typeSibling, QWidget *parent, const char *name = 0 );
    bool eventFilter( QObject *o, QEvent *e );
public slots:
    void appendText( const QString &txt );
private:
    QWidget *mTypeSibling;
};

#endif
