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


#ifdef QTOPIA_DATA_LINKING
#include <qtopia/qdl.h>
#endif

#include "abeditor.h"
#include "addresspicker.h"
#ifdef QTOPIA_PHONE
#include "emaildialogphone.h"
#else
#include "emaildlgimpl.h"
#endif
#include "imagesourcedialog.h"
#include "addressbook.h" // for AB_PDA

#include <qtopia/vscrollview.h>
#include <qtopia/categories.h> // needed to get correct WIN32 exports for QValueList<int>
#include <qtopia/categoryselect.h>
#include <qtopia/qpeapplication.h>
#include <qtopia/qpedialog.h>
#include <qtopia/datetimeedit.h>
#include <qtopia/global.h>
#include <qtopia/resource.h>
#include <qtopia/image.h>
#ifndef QTOPIA_DESKTOP
#include <qtopia/imageselector.h>
#endif
#include <qtopia/pim/private/abtable_p.h>

#ifdef QTOPIA_PHONE
#include <qtopia/contextmenu.h>
#include <qtopia/fileselector.h>
#endif

#include <qlistbox.h>
#include <qpainter.h>
#include <qtextstream.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qradiobutton.h>
#include <qstyle.h>
#include <qaction.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qmultilineedit.h>
#include <qscrollview.h>
#include <qtoolbutton.h>
#include <qpushbutton.h>
#include <qmainwindow.h>
#include <qvbox.h>
#include <qtabwidget.h>
#include <qregexp.h>
#include <qwhatsthis.h>
#include <qmessagebox.h>
#include <qdialog.h>
#include <qlist.h>
#include <qcstring.h>
#include <qstring.h>
#include <qwidgetstack.h>
#include <qbuttongroup.h>
#include <qcolor.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qwmatrix.h>
#include <qdir.h>
#ifdef QTOPIA_DESKTOP
#include <qinputdialog.h>
#include <qfiledialog.h>
#endif

#ifdef QTOPIA_DESKTOP
#include <qtextedit.h>
#endif

#include <stdio.h>

#ifndef QTOPIA_DESKTOP
#include <qtopia/iconselect.h>
#else
#include <qtopia/pixmapdisplay.h>
#endif

#include <time.h>

//make name prefixes globally available and translatable
const QString gNamePrefixStr = QObject::tr( "Mr;Mrs;Ms;Miss;Dr;Prof;" );
const QString gNameSuffixStr = QObject::tr( "Jr;Sr;I;II;III;IV;V" );

// helper functions, convert our comma delimited list to proper
// file format...
void parseEmailFrom( const QString &txt, QString &strDefaultEmail,
		     QStringList &strAll );

// helper convert from file format to comma delimited...
void parseEmailTo( const QString &strDefaultEmail,
		   const QStringList &emails, QString &strBack );


#ifdef QTOPIA_PHONE
// helper to add "Store on SIM" to a ContextMenu.
QAction *addStoreOnSim(ContextMenu *menu, QObject *parent, const char *slot)
{
    QAction *a = new QAction(qApp->translate("ABEditor", "Store on SIM"),
		    QString::null, 0,
		    parent, 0 );
    a->setToggleAction( TRUE );
    QObject::connect( a, SIGNAL(toggled(bool)), parent, slot );
    menu->insertSeparator(0);
    menu->insertAction(a, 0);

    return a;
}
#endif

FileAsCombo::FileAsCombo( QWidget *parent )
    : QComboBox( FALSE, parent, "fileAsCombo" )
{
    setEditable( FALSE );
    // the default template is filled with whatever is available and forms a valid name if other templates don't yield any options
    // in other words, the default template always yields an option if the user has input any name data
    mDefaultTemplate = tr("%prefix %firstname %nickname %middlename %lastname %suffix %firstpro %lastpro",
                            "Defines the default name format for a contact's File As."); 
    mTemplates = QStringList::split( ";",
				     tr(
					"%firstname %lastname;"
					"%lastname, %firstname;"
					"%firstname;"
					"%lastname;"
					"%firstname %middlename %lastname;"
					"%prefix %firstname %lastname %suffix;"
					"%prefix %firstname %lastname;"
					"%prefix %lastname %suffix;"
					"%prefix %firstname %suffix;"
					"%firstname %lastname %suffix;"
					"%prefix %firstname;"
					"%prefix %lastname;"
					"%firstname %suffix;"
					"%lastname %suffix;"
					"%middlename;"
					"%firstname %nickname %lastname;"
					"%nickname %lastname;"
					"%nickname;",
					"Defines locale-specific name formats for a contact's File As. Different formats are separated by ';'"
                                        " Provide new sequences if it is necessary for the localisation otherwise don't provide a translation at all."
				       )
				   );
    mTemplateVars["%prefix"] = 
    mTemplateVars["%suffix"] = 
    mTemplateVars["%firstname"] = 
    mTemplateVars["%middlename"] = 
    mTemplateVars["%lastname"] = 
    mTemplateVars["%nickname"] = 
    mTemplateVars["%firstpro"] = 
    mTemplateVars["%lastpro"] = QString::null;

    connect( this, SIGNAL(activated(int)), this, SLOT(customFileAs(int)) );

    update();
}

void FileAsCombo::setSelected( const QString &fileAs )
{
    if( fileAs == QString::null )
	setCurrentItem( 0 );
    else
    {
	for( int i = 1 ; i < (count()-1) ; ++i )
	{
	    if( text( i ) == fileAs )
	    {
		setCurrentItem( i );
		return;
	    }
	}
	// item not available - custom. add it
	insertItem( fileAs, 1 );
	setCurrentItem( 1 );
	mCustomEntries.prepend( fileAs );
    }
}

QString FileAsCombo::selected() const
{
    if( currentItem() == 0 || currentItem() == (count()-1) )
	return QString::null;
    else
	return currentText();
}

void FileAsCombo::customFileAs( int idx )
{
    if( idx == (count()-1) )
    {
#ifdef QTOPIA_DESKTOP
        bool ok = FALSE;
        QString text = QInputDialog::getText( tr("Add Custom File As"), QString::null, QLineEdit::Normal, QString::null, &ok, this );
        if( ok && !text.isEmpty() )
            setSelected( text );
        else
            setSelected( QString::null ); 
#else
	QDialog *dlg = new QDialog( this, "customFileAs", TRUE );
	dlg->setCaption( tr("Add Custom File As") );
	QVBoxLayout *l = new QVBoxLayout( dlg );
	l->setAutoAdd( TRUE );
	QLineEdit *le = new QLineEdit( dlg );
	le->setFocus();
	dlg->setMaximumHeight( le->height() );
	if( QPEApplication::execDialog( dlg ) == QDialog::Accepted && !le->text().isEmpty() )
	    setSelected( le->text() );
	else 
	    setSelected( QString::null );
	delete dlg;
#endif
    }
}

QString FileAsCombo::fillTemplate( const QString &t, bool allowEmpty )
{
    QString cur = t;
    QMap<QString,QString>::ConstIterator jit;
    for( jit = mTemplateVars.begin() ; jit != mTemplateVars.end() ; ++jit )
    {
	int f;
	while( (f = cur.find( jit.key() )) != -1 )
	{
	    if( jit.data().simplifyWhiteSpace().isEmpty() && !allowEmpty )
	    {
		//template is empty if some data for a variable is this template is empty
		return QString::null;
	    }
	    else
	    {
		cur.replace( f, jit.key().length(), jit.data() );
	    }
	}
    }
    return cur.simplifyWhiteSpace();
}

void FileAsCombo::update()
{
    int current = currentItem();
    clear();
    QStringList options;
    options += tr( "(default)" ); 
    options += mCustomEntries;
    for( QStringList::ConstIterator iit = mTemplates.begin() ; iit != mTemplates.end() ; ++iit )
    {
	QString cur = fillTemplate( *iit );
	if( !cur.isEmpty() && !options.contains( cur ) )
	    options += cur;
    }

    if( !options.count() ) // no options, put together a default if possible
    {
	QString def = fillTemplate( mDefaultTemplate, TRUE );
	if( def.length() )
	    options += def;
    }

    options += tr("...");
    insertStringList( options );
    setCurrentItem( current );
}

void FileAsCombo::setPrefix( const QString &prefix )
{
    mTemplateVars["%prefix"] = prefix;
    update();
}

void FileAsCombo::setSuffix( const QString &suffix )
{
    mTemplateVars["%suffix"] = suffix;
    update();
}

void FileAsCombo::setFirstName( const QString &firstName )
{
    mTemplateVars["%firstname"] = firstName;
    update();
}

void FileAsCombo::setMiddleName( const QString &middleName )
{
    mTemplateVars["%middlename"] = middleName;
    update();
}

void FileAsCombo::setLastName( const QString &lastName )
{
    mTemplateVars["%lastname"] = lastName;
    update();
}

void FileAsCombo::setNickName( const QString &nickName )
{
    mTemplateVars["%nickname"] = nickName;
    update();
}

void FileAsCombo::setFirstNamePro( const QString &firstNamePro )
{
    mTemplateVars["%firstpro"] = firstNamePro;
    update();
}

void FileAsCombo::setLastNamePro( const QString &lastNamePro )
{
    mTemplateVars["%lastpro"] = lastNamePro;
    update();
}

//-----------------------------------------------------------------------

PhoneFieldType::PhoneFieldType()
{
}

PhoneFieldType::PhoneFieldType( const QString& type_id, const QString &str )
{
    id = type_id;
    pixmap = Resource::loadPixmap(id);
    name = str;
}

PhoneFieldType::PhoneFieldType( const PhoneFieldType &other )
{
    *this = other;
}

PhoneFieldType &PhoneFieldType::operator=( const PhoneFieldType &other )
{
    id = other.id;
    pixmap = other.pixmap;
    name = other.name;
    return *this;
}

bool  PhoneFieldType::operator==( const PhoneFieldType &other ) const
{
    return id == other.id;
}

bool PhoneFieldType::operator!=( const PhoneFieldType &other ) const
{
    return !(*this == other);
}

PhoneFieldManager::PhoneFieldManager( QWidget *parent, QGridLayout *layout, int rc )
    : QObject( parent, "phoneFieldManager" ), parLayout(layout), rowCount(rc)
{
    mEmitFieldChanged = TRUE;
}

PhoneFieldManager::~PhoneFieldManager()
{
    phoneFields.setAutoDelete( TRUE );
    phoneFields.clear();  
}

#ifdef QTOPIA_PHONE
void PhoneFieldManager::setFieldOnSim( const PhoneFieldType &type, bool onSim )
{
    QListIterator<PhoneField> it(phoneFields);
    for( ; *it ; ++it )
    {
	if( (*it)->type() == type )
	{
	    if( (*it)->onSim() != onSim )
	    {
		(*it)->setOnSim( onSim );
	    }
	    else
	    {
		break;
	    }
	}
    }
}

bool PhoneFieldManager::fieldOnSim( const PhoneFieldType &type ) const
{
    QListIterator<PhoneField> it(phoneFields);
    for( ; *it ; ++it )
    {
	if( (*it)->type() == type )
	{
	    return (*it)->onSim();
	}
    }
    return FALSE;
}
#endif

bool PhoneFieldManager::isFull() const
{
    bool full = FALSE;
    if ( phoneFields.count() && (phoneFields.count() >= mTypes.count()) ) {
	// if there are no blank fields, we're full
	full = TRUE;
	QListIterator<PhoneField> it(phoneFields);
	for( ; *it ; ++it )
	    if( (*it)->isEmpty() )
		full = FALSE;
    }
    return full;
}

void PhoneFieldManager::add( const QString &number, const PhoneFieldType &type )
{
    if ( number.isEmpty() || isFull() )
	return;

    PhoneField *eField = 0;
    QListIterator<PhoneField> it(phoneFields);
    for( ; *it ; ++it )
    {
	PhoneFieldType t( (*it)->type() ); 
	if( t == type )
	{
	    eField = (*it);
	    break;
	}
    }
    if( eField )
    {
	eField->setNumber( number );
    }
    else
    {
	for( it.toFirst() ; *it ; ++it )
	{
	    if( (*it)->isEmpty() )
	    {
		(*it)->setType( type );
		(*it)->setNumber( number );
		break;
	    }
	}
    }
}

void PhoneFieldManager::addEmpty()
{
    if ( isFull() )
	return;

    QValueList<PhoneFieldType> availTypes = mTypes;

    QListIterator<PhoneField> it(phoneFields);
    for( ; *it ; ++it )
    {
	PhoneFieldType t( (*it)->type() );
	availTypes.remove( t );
    }
    PhoneField *nf = new PhoneField( parLayout, rowCount, (QWidget *)parent() );
    phoneFields.append( nf );
    nf->setTypes( mTypes );
    nf->setType( availTypes.first() );

    connect( nf, SIGNAL(userChangedType(const PhoneFieldType&)),
	this, SLOT(updateTypes(const PhoneFieldType &)) );
    connect( nf, SIGNAL(numberChanged(const QString&)),
	this, SLOT(checkForAdd()) );
    connect( nf, SIGNAL(fieldChanged(const QString&,const PhoneFieldType&)),
	this, SLOT(emitFieldChanged(const QString&,const PhoneFieldType&)) );
#ifdef QTOPIA_PHONE
    connect( nf, SIGNAL(fieldOnSim(bool,const PhoneFieldType&)), this, SIGNAL(fieldOnSim(bool,const PhoneFieldType&)) );
#endif
}

void PhoneFieldManager::emitFieldChanged( const QString &str, const PhoneFieldType &type )
{
    if( mEmitFieldChanged )
	emit fieldChanged( str, type );
}

void PhoneFieldManager::setTypes( const QValueList<PhoneFieldType> &newTypes )
{
    if( phoneFields.count() > newTypes.count() )
    {
	qDebug("BUG : phoneFields.count() > newTypes.count()\n");
	return;
    }

    mTypes = newTypes;

    QValueList<PhoneFieldType> availTypes = newTypes;

    QListIterator<PhoneField> it(phoneFields);
    for( ; *it ; ++it )
    {
	(*it)->setTypes( mTypes );
	if( !availTypes.contains( (*it)->type() ) ) 
	{
	    if( !availTypes.count() )
		qDebug("BUG : Not enough types for phone fields.");
	    PhoneFieldType nt = availTypes.first();
	    (*it)->setType( nt );
	    availTypes.remove( nt );
	}
    }
}

QValueList<PhoneFieldType> PhoneFieldManager::types() const
{
    return mTypes;
}

void PhoneFieldManager::updateTypes( const PhoneFieldType &newType )
{
    const QObject *s = sender(); // the phone field whose type has changed

    QValueList<PhoneFieldType> availTypes = mTypes;
    QList<PhoneField> changedFields;

    QListIterator<PhoneField> it(phoneFields);
    for( ; *it ; ++it )
	availTypes.remove( (*it)->type() );

    mEmitFieldChanged = FALSE;
    for( it.toFirst() ; *it ; ++it )
    {
	if( (QObject *)(*it) == s ) continue;

	if( (*it)->type() == newType )
	{
	    if( !availTypes.count() )
		qDebug("BUG : Not enough types for phone fields.");
    
	    PhoneFieldType nt = availTypes.first();
	    (*it)->setType( nt );
	    availTypes.remove( nt ); 
	    changedFields.append( *it );
	}
    }
    mEmitFieldChanged = TRUE;
    //emit field changed signals only after we've rearranged
    QListIterator<PhoneField> cit( changedFields );
    for( ; *cit ; ++cit )
	emitFieldChanged( (*cit)->number(), (*cit)->type() );
}

#ifdef QTOPIA_PHONE
void PhoneFieldManager::setHaveSim( bool b )
{
    QListIterator<PhoneField> it(phoneFields);
    for( ; *it ; ++it )
	(*it)->setHaveSim( b );
}
#endif

void PhoneFieldManager::checkForAdd()
{
    QListIterator<PhoneField> it(phoneFields);
    for( ; *it ; ++it )
	if( (*it)->isEmpty() )
	    return;
    addEmpty();
}

void PhoneFieldManager::setNumberFromType( const PhoneFieldType &type, const QString &newNumber )
{
    QListIterator<PhoneField> it(phoneFields);
    for( ; *it ; ++it )
    {
	if( (*it)->type() == type )
	{
	    (*it)->setNumber( newNumber );
	    break;
	}
    }
    //no fields matching, add a field
    add( newNumber, type );
}

QString PhoneFieldManager::numberFromType( const PhoneFieldType &type )
{
    QListIterator<PhoneField> it(phoneFields);
    for( ; *it ; ++it )
	if( (*it)->type() == type )
	    return (*it)->number();
    return QString("");
}

PhoneFieldLineEdit::PhoneFieldLineEdit( QWidget *typeSibling, QWidget *parent, const char *name )
    : QLineEdit( parent, name ), mTypeSibling( typeSibling )
{
    mTypeSibling->installEventFilter( this );
}

bool PhoneFieldLineEdit::eventFilter( QObject *o, QEvent *e )
{
    if( o == mTypeSibling && e->type() == QEvent::KeyPress )
    {
	QKeyEvent *ke = (QKeyEvent *)e;
	if( !ke->text().isEmpty() )
	{
	    setFocus();
	    appendText( ke->text() );
	    return TRUE;
	}
    }
    return FALSE;
}

void PhoneFieldLineEdit::appendText( const QString &txt )
{
#ifdef QTOPIA_PHONE
    if( !Global::mousePreferred() )
    {
	if( !isModalEditing() )
	    setModalEditing( TRUE );
    }
#endif
    setText( text()+txt );
}


PhoneField::PhoneField( QGridLayout *l, int &rowCount, QWidget *parent, const char *name )
    : QObject( parent, name )
{
//    label = new QLabel( parent );
//   label->show();

    /*
    QHBox *hBox = new QHBox( parent );
    hBox->setSizePolicy( QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred) );
    hBox->setBackgroundColor( QColor( 0, 0, 255 ) );
    */

#ifndef QTOPIA_DESKTOP
    typeIS = new IconSelect( parent );
    typeIS->setDisplaySize( QSize( 24, 16) );
#else
    typeIS = new QComboBox( FALSE, parent );
#endif
    connect( typeIS, SIGNAL(activated(int)), this, SLOT(userChangedType(int)) );

    numberLE = new PhoneFieldLineEdit( typeIS, parent );
    connect( numberLE, SIGNAL(textChanged(const QString&)), this, SIGNAL(numberChanged(const QString&)) );
    QPEApplication::setInputMethodHint(numberLE,QPEApplication::PhoneNumber);

#ifdef QTOPIA_PHONE
    ContextMenu *menu = ContextMenu::menuFor( numberLE );
    simAction = addStoreOnSim(menu, this, SLOT(emitFieldOnSim(bool)));
#endif
/*
    l->addWidget( label, rowCount, 0 );
    l->addWidget( hBox, rowCount, 2 );
*/
    l->addWidget( typeIS, rowCount, 0, AlignHCenter );
    l->addWidget( numberLE, rowCount, 2 );

//    l->addMultiCellWidget( hBox, rowCount, rowCount, 0, 2 );
    ++rowCount;

//    hBox->show();
    numberLE->show();
    typeIS->show();

    connect( this, SIGNAL(numberChanged(const QString&)), this, SLOT(emitFieldChanged()) );
    connect( this, SIGNAL(internalChangedType(const PhoneFieldType&)), this, SIGNAL(typeChanged(const PhoneFieldType&)) );
    connect( this, SIGNAL(userChangedType(const PhoneFieldType&)), this, SIGNAL(typeChanged(const PhoneFieldType&)) );
    connect( this, SIGNAL(typeChanged(const PhoneFieldType&)), this, SLOT(emitFieldChanged()) );
}

#ifdef QTOPIA_PHONE
void PhoneField::setHaveSim( bool b )
{
    simAction->setEnabled( b );
}
#endif

void PhoneField::emitFieldOnSim( bool f )
{
#ifdef QTOPIA_PHONE
    simAction->setOn( f );
    emit fieldOnSim( f, type() );
#else
    Q_UNUSED( f );
#endif
}

#ifdef QTOPIA_PHONE
void PhoneField::setOnSim( bool onSim )
{
    simAction->setOn( onSim );
}

bool PhoneField::onSim() const
{
    return simAction->isOn();
}
#endif

PhoneField::~PhoneField()
{
    delete typeIS;
    delete numberLE;
}

void PhoneField::userChangedType( int /* idx */)
{
    emit userChangedType( mTypes[typeIS->currentItem()] );
}

void PhoneField::setTypes( const QValueList<PhoneFieldType> &types )
{
    typeIS->clear();

    mTypes = types;
    QValueList<PhoneFieldType>::ConstIterator it;
    for( it = mTypes.begin() ; it != mTypes.end() ; ++it )
    {
	QPixmap p = (*it).pixmap;
	typeIS->insertItem( p, (*it).name );
    }

    emit internalChangedType( mTypes[typeIS->currentItem()] );
}

void PhoneField::setType( const PhoneFieldType &newType )
{
    PhoneFieldType t(newType);
    int idx = mTypes.findIndex( t );
    if( idx != -1 )
	typeIS->setCurrentItem( idx );
	
}

PhoneFieldType PhoneField::type()
{
    return mTypes[typeIS->currentItem()];
}

QString PhoneField::number() const
{
    return numberLE->text();
}

bool PhoneField::isEmpty() const
{
    return number().stripWhiteSpace().isEmpty();
}

void PhoneField::setNumber( const QString &newNumber )
{
    numberLE->setText( newNumber );
}

void PhoneField::emitFieldChanged()
{
    emit fieldChanged( numberLE->text(), mTypes[typeIS->currentItem()] );
}

//-----------------------------------------------------------------------

//FIXME : This is crap iterative widget creation that has survived from the rewrite. 
//now that address parsing has gone, make this into AbNameDetails dialog
AbDetailEditor::AbDetailEditor( QWidget *parent, const char *name, WFlags fl )
    : QDialog( parent, name, TRUE, fl ), fileAsCombo(0), suffixCombo(0), titleCombo(0)
{
    mView = new VScrollView( this );
    editorLayout = new QVBoxLayout( this );
    editorLayout->addWidget( mView );
#ifdef QTOPIA_DESKTOP
    editorLayout->setResizeMode( QLayout::FreeResize );
    setMinimumSize( sizeHint().width()*2, sizeHint().height()*3 );
    setMaximumSize( sizeHint().width()*4, sizeHint().height()*6 );
    QHBoxLayout *buttonLayout = new QHBoxLayout( editorLayout );
    QPushButton *okButton = new QPushButton( tr("OK"), this );
    QPushButton *cancelButton = new QPushButton( tr("Cancel"), this );
    connect( okButton, SIGNAL(clicked()), this, SLOT(accept()) );
    connect( cancelButton, SIGNAL(clicked()), this, SLOT(reject()) );
    buttonLayout->addStretch();
    buttonLayout->addWidget( okButton );
    buttonLayout->addWidget( cancelButton );
#endif
}

AbDetailEditor::~AbDetailEditor()
{
}

void AbDetailEditor::accept()
{
    QMap<PimContact::ContactFields, QString> newFields;
    for ( QMap<PimContact::ContactFields, QString>::Iterator it = myFields.begin();
	  it != myFields.end(); ++it ) 
    {
	switch ( it.key() ) 
	{
	    case PimContact::Suffix:
		    myFields[PimContact::Suffix]  = suffixCombo->currentText();
		break;
	    case PimContact::NameTitle:
		    myFields[PimContact::NameTitle] = titleCombo->currentText();
		break;
	    case PimContact::FileAs:
	    {
		myFields[PimContact::FileAs] = fileAsCombo->selected();
		break;
	    }
	    default:
	    {
		myFields[ it.key() ] = lineEdits[ it.key() ]->text();
		break;
	    }
	}
    }
    QDialog::accept();
}

/* Determines the type of field we're editing and returns a valuelist which is used to create the GUI elements */
QValueList<PimContact::ContactFields> AbDetailEditor::guiValueList( const QMap<PimContact::ContactFields, QString> &f ) const
{
    QValueList<PimContact::ContactFields> gvl;
    if( f.contains( PimContact::NameTitle ) ) // editing a name field
    {
	gvl += PimContact::NameTitle;
	gvl += PimContact::FirstName;
	gvl += PimContact::FirstNamePronunciation;
	gvl += PimContact::MiddleName;
	gvl += PimContact::LastName;
	gvl += PimContact::LastNamePronunciation;
	gvl += PimContact::Suffix;
	gvl += PimContact::Nickname;
	gvl += PimContact::FileAs;
    }

    return gvl;
}

static void adjustPronWidgets(QLabel* label, QLineEdit* le)
{
    label->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    QFont sm = label->font();
#ifndef QTOPIA_DESKTOP
    sm.setPointSize(sm.pointSize()/2);
    le->setFont(sm);
#else
    Q_UNUSED( le ); 
    sm.setItalic( TRUE );
#endif
    label->setFont(sm);
}

void AbDetailEditor::setFields( const QMap<PimContact::ContactFields, QString> &f )
{
    //FIXME : This loop is pointless - make AbDetailEditor an adt and derive address and name editors from that
    myFields = f;
    QWidget *container = mView->widget();

    bool haveFileAs = FALSE;
    if ( ! fileAsCombo )
	fileAsCombo = new FileAsCombo( container );

    const QMap<PimContact::ContactFields, QString> dn = displayNames();
    QGridLayout *gl = new QGridLayout( container, myFields.count()+1, 3, 3, 0 );
#ifdef QTOPIA_DESKTOP
    gl->setResizeMode( QLayout::FreeResize );
#endif
#ifdef QTOPIA_DESKTOP
    gl->setSpacing( 4 );
#else
    gl->addColSpacing(1,4);
#endif

    QWidget *lastEditor = 0, *editor;
    int fieldInTabNum=0;

    //Map may differ in ordering to what we want ; get a value list for the fields we're editing
    QValueList<PimContact::ContactFields> guiList = guiValueList( f );

    for ( QValueList<PimContact::ContactFields>::ConstIterator fieldKey = guiList.begin() ;
          fieldKey != guiList.end() ; ++fieldKey ) {

	QLabel *label = new QLabel( dn[ *fieldKey ], container );
	label->setAlignment(AlignRight|AlignVCenter);

	// do a switch on the field type; most are line edits, the exceptions get a case, the default
	// is a linedit
	editor = 0;
	int cLen = 0;
	int i;
	switch ( *fieldKey ) {
	case PimContact::Suffix:
	    suffixCombo = new QComboBox( FALSE, container );
	    suffixCombo->setEditable( FALSE );
	    suffixCombo->setDuplicatesEnabled( FALSE );
	    suffixCombo->setAutoCompletion( TRUE );

	    connect(suffixCombo, SIGNAL(textChanged(const QString&)),
		    fileAsCombo, SLOT(setSuffix(const QString&)));

	    suffixCombo->insertItem( "", 0 );
	    suffixCombo->insertStringList( QStringList::split( ';', gNameSuffixStr ) );

	    cLen = suffixCombo->count();
	    for( i = 0; i < cLen ; ++i )
	    {
		if( myFields[ *fieldKey ]  == suffixCombo->text( i ) )
		{
		    suffixCombo->setCurrentItem( i );
		    break;
		}
	    }

	    editor = suffixCombo;
	    break;

	case PimContact::NameTitle:
	    titleCombo = new QComboBox( FALSE, container );
	    titleCombo->setEditable( FALSE );
	    titleCombo->setDuplicatesEnabled( FALSE );
	    titleCombo->setAutoCompletion( TRUE );

	    connect(titleCombo, SIGNAL(textChanged(const QString&)),
		    fileAsCombo, SLOT(setPrefix(const QString&)));

	    titleCombo->insertItem( "", 0 );
	    titleCombo->insertStringList( QStringList::split( ';', gNamePrefixStr ) );

	    cLen = titleCombo->count();
	    for( i = 0; i < cLen ; ++i )
	    {
		if( myFields[ *fieldKey ] == titleCombo->text( i ) )
		{
		    titleCombo->setCurrentItem( i );
		    break;
		}
	    }

	    editor = titleCombo;
	    break;
	case PimContact::FileAs:
	    haveFileAs = TRUE;
	    editor = fileAsCombo;
	    fileAsCombo->setSelected( myFields[ *fieldKey ] );
	    break;
	default:
	    {
		//FIXME : ugly temporary solution
		QLineEdit *lineedit = new QLineEdit( container );
		switch( *fieldKey )
		{
		    case PimContact::FirstNamePronunciation:
			connect( lineedit, SIGNAL(textChanged(const QString&)),
				fileAsCombo, SLOT(setFirstNamePro(const QString&)) );
			break;
		    case PimContact::LastNamePronunciation:
			connect( lineedit, SIGNAL(textChanged(const QString&)),
				fileAsCombo, SLOT(setLastNamePro(const QString&)) );
			break;
		    case PimContact::FirstName:
			connect( lineedit, SIGNAL(textChanged(const QString&)),
				fileAsCombo, SLOT(setFirstName(const QString&)) );
			break;
		    case PimContact::MiddleName:
			connect( lineedit, SIGNAL(textChanged(const QString&)),
				fileAsCombo, SLOT(setMiddleName(const QString&)) );
			break;
		    case PimContact::LastName:
			connect( lineedit, SIGNAL(textChanged(const QString&)),
				fileAsCombo, SLOT(setLastName(const QString&)) );
			break;
		    case PimContact::Nickname:
			connect( lineedit, SIGNAL(textChanged(const QString&)),
				fileAsCombo, SLOT(setNickName(const QString&)) );
			break;
                    default:
                        break;
		}

		if ( *fieldKey == PimContact::FirstNamePronunciation
		  || *fieldKey == PimContact::LastNamePronunciation )
		    adjustPronWidgets(label,lineedit);
		else if( *fieldKey == PimContact::BusinessZip ||
			 *fieldKey == PimContact::HomeZip )
		    QPEApplication::setInputMethodHint( lineedit, QPEApplication::Number );
		lineedit->setText( myFields[ *fieldKey ] );
		lineEdits.insert( *fieldKey, lineedit );
		editor = lineedit;
	    }
	    break;
	};

	if ( editor ) 
	{
	    // add the editor to the layout
	    gl->addWidget( label, fieldInTabNum, 0 );
	    gl->addWidget( editor, fieldInTabNum, 2 );
	    if ( lastEditor ) {
		setTabOrder( lastEditor, editor );
	    }
	    lastEditor = editor;

	    // increment the field number for this tab
	    fieldInTabNum++;
	} else {
	    delete label;
	    label = 0;
	}
    }
    QSpacerItem *verticleSpacer = new QSpacerItem( 1, 1, QSizePolicy::Minimum,
						   QSizePolicy::Expanding );
    gl->addItem( verticleSpacer, fieldInTabNum, 2 );

    if( !haveFileAs ) // not editing name, don't need file as combo
    {
	delete fileAsCombo;
	fileAsCombo = 0;
    }
}

const QMap<PimContact::ContactFields, QString> AbDetailEditor::displayNames() const
{
    QMap<PimContact::ContactFields, QString> shortNames;
    const QMap<int, QString> &longNames= PimContact::trFieldsMap(); // use field names from PimContact

    //names for name fields
    shortNames[PimContact::NameTitle] = longNames[PimContact::NameTitle];
    shortNames[PimContact::FirstName] = longNames[PimContact::FirstName];
    shortNames[PimContact::FirstNamePronunciation] = longNames[PimContact::FirstNamePronunciation];
    shortNames[PimContact::MiddleName] = longNames[PimContact::MiddleName];
    shortNames[PimContact::LastName] = longNames[PimContact::LastName];
    shortNames[PimContact::LastNamePronunciation] = longNames[PimContact::LastNamePronunciation];
    shortNames[PimContact::Suffix] = longNames[PimContact::Suffix];
    shortNames[PimContact::Nickname] = longNames[PimContact::Nickname];
    shortNames[PimContact::FileAs] = longNames[PimContact::FileAs];

    QString street = tr("Street"),
	    city = tr("City"),
	    state = tr("State"),
	    zip = tr("Zip"),
	    country = tr("Country")
	    ;

    shortNames[PimContact::BusinessStreet] = street;
    shortNames[PimContact::BusinessCity] = city;
    shortNames[PimContact::BusinessState] = state;
    shortNames[PimContact::BusinessZip] = zip;
    shortNames[PimContact::BusinessCountry] = country;

    shortNames[PimContact::HomeStreet] = street;
    shortNames[PimContact::HomeCity] = city;
    shortNames[PimContact::HomeState] = state;
    shortNames[PimContact::HomeZip] = zip;
    shortNames[PimContact::HomeCountry] = country;

    return shortNames;
}

QMap<PimContact::ContactFields, QString> AbDetailEditor::fields() const
{
    return myFields;
}
    
//------------------------------------------------------------------------

AbstractField::AbstractField( QWidget *parent , const char *name )
    : QWidget( parent, name ), mModified( FALSE ), detailEditor(0)
{
}

AbstractField::~AbstractField()
{
}

bool AbstractField::modified() const
{
    return mModified;
}

void AbstractField::setModified( bool b )
{
    mModified = b;
}

void AbstractField::setFields( const QMap<PimContact::ContactFields, QString> &f )
{
    myFields = f;

    fieldsChanged();
}

QMap<PimContact::ContactFields, QString> AbstractField::fields() const
{
    return myFields;
}

bool AbstractField::isEmpty() const
{
    for( QMap<PimContact::ContactFields, QString>::ConstIterator it = myFields.begin() ; it != myFields.end() ; ++it )
    {
	QString str = it.data().stripWhiteSpace();
	if( !str.isEmpty() )
	    return FALSE;
    }
    return TRUE;
}

void AbstractField::details()
{
    parse();

    detailEditor = new AbDetailEditor( (QWidget *)parent(), "detailEditor" );
    detailEditor->setCaption( tr("Edit %1").arg( fieldName() ) );

    detailEditor->setFields( fields() );

#ifdef QTOPIA_DESKTOP
    if( detailEditor->exec() )
#else
    if( QPEApplication::execDialog( detailEditor ) )
#endif
    {
	setFields( detailEditor->fields() );
	fieldsChanged();
    }

    delete detailEditor;
    detailEditor = 0;
}

QStringList AbstractField::tokenize( const QString &input ) const
{
    QStringList tokens;
    const QString &in = input;
    int len = input.length();

    int i = 0;
    while( i < len )
    {
	if( in[i] == '\n' ) // new line is a valid token
	{
	    tokens += QString( QChar('\n') );
	    ++i;
	    //skip any following newlines
	    //while( i < len && in[i] == '\n' ) ++i;
	    //make newlines count for address field

	}
	else if( in[i].isSpace() ) // skip whitespace
	{
	    while( i < len && in[i].isSpace() ) ++i;
	}
	else if( in[i] == ',' ) // other meaningful tokens
	{
	    tokens += QString( QChar(in[i]) );
	    ++i;
	}
	else if( in[i] == '('  ) // nickname
	{
	    tokens += "(";
	    QString nick;
	    ++i;
	    while( i < len && in[i] != ')' ) nick += in[i++];
	    tokens += nick;
	    tokens += ")";
	    ++i;
	}
	else if( in[i] == '\"' || in[i] == '\'' ) //string literal
	{
	    //include quotes in the token so we know it's a string literal
	    char quote = in[i];
	    QString literal;
	    literal += quote;
	    ++i;
	    while( i < len && in[i] != quote ) literal += in[i++];
	    literal += quote;
	    ++i;
	    tokens += literal;
	}
	else // identifier
	{
	    QString identifier;
	    while( i < len && !in[i].isSpace() && in[i] != ','  ) identifier += in[i++];
	    tokens += identifier;
	}
	
    }
    /*
    qDebug("Printing tokens.\n");
    QStringList::Iterator it;
    for( it = tokens.begin() ; it != tokens.end() ; ++it )
	qDebug("%s\n", (*it).latin1());
	*/
    return tokens;
}

//------------------------------------------------------------------------

AbstractName::AbstractName( QWidget *parent , const char *name )
    : AbstractField( parent, name ), mainWidget(0)
{
    QHBoxLayout *layout = new QHBoxLayout( this );
    mainWidget = new QLineEdit( this, name );
    setFocusProxy(mainWidget);

#ifdef QTOPIA_PHONE
    ContextMenu *contextMenu = ContextMenu::menuFor(mainWidget);

    QAction *actionDetails = new QAction( tr( "Details" ), Resource::loadIconSet( "field-details" ), QString::null,
			      0, this, 0 );
    connect( actionDetails, SIGNAL(activated()), this, SLOT(details()) );
    contextMenu->insertSeparator(0);
    contextMenu->insertAction(actionDetails, 0);
#endif

//    setSizePolicy( mainWidget->sizePolicy() );
//   mainWidget->setSizePolicy( QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding) );

    layout->addWidget( mainWidget );

    prefixes = QStringList::split( ';', gNamePrefixStr );
    suffixes = QStringList::split( ';', gNameSuffixStr );

    connect( mainWidget, SIGNAL(textChanged(const QString&)), this, SLOT(textChanged()) );
}

AbstractName::~AbstractName()
{
}

bool AbstractName::isEmpty() const
{
    return mainWidget->text().isEmpty();
}

QString AbstractName::fieldName() const
{
    return tr("Name");
}

void AbstractName::parse()
{
/*
1: name = [known-prefix] given-name [nick] {middle-name} family-name [known-suffix] [pron] 
2: name = known-prefix family-name [known-suffix] [pron] 
3: name = given-name [known-suffix] [pron] 
4: name = family-name ',' [known-prefix] given-name [nick] {middle-name} [known-suffix] [pron]
*/
    QString prefix, suffix, firstName, nickName, middleName, lastName, firstNamePro, lastNamePro;
    QString newText = mainWidget->text();
    if( newText.isEmpty() )
    {
	myFields[PimContact::NameTitle] = 
	myFields[PimContact::FirstName] = 
	myFields[PimContact::FirstNamePronunciation] =
	myFields[PimContact::MiddleName] = 
	myFields[PimContact::LastName] = 
	myFields[PimContact::LastNamePronunciation] = 
	myFields[PimContact::Suffix] =
	myFields[PimContact::Nickname] = QString("");
	return;
    }

    QStringList tokens = tokenize( newText );
    int tokCount = tokens.count();
    int i = 0;
    //eliminate different possibilities based on the tokens
    if( tokens[1] == "," )
    {
	// rule 4
	lastName = tokens[i++];
	++i;
	QString fAffix = formattedAffix( tokens[i], prefixes );
	if( fAffix.length() )
	{
	    prefix = fAffix;
	    ++i;
	}
	firstName = tokens[i++];
	if( i < tokCount && tokens[i] == "(" )
	{
	    ++i;
	    nickName = tokens[i++];
	    ++i; // skip closing )
	}
	if( i < tokCount && !isQuote( tokens[i][0] ) )
	    middleName = tokens[i++];
	fAffix = formattedAffix( tokens[i], suffixes );
	if( i < tokCount && !isQuote( tokens[i][0] ) && fAffix.length() )
	{
	    suffix = fAffix;
	    ++i;
	}
	if( i < tokCount && isQuote( tokens[i][0] ) )
	{
	    QString proStr = stripQuotes( tokens[i++] );
	    QStringList proTok = QStringList::split( " ", proStr );
	    firstNamePro = proTok[1]; // pro specified in name order of the rule
	    lastNamePro = proTok[0];
	}
    }
    else if( tokCount == 1 || (tokCount >= 2 && (formattedAffix( tokens[1], suffixes ).length() || isQuote( tokens[1][0] ))) )
    {
	// rule 3
	firstName = tokens[i++];
	QString fAffix = formattedAffix( tokens[i], suffixes );
	if( i < tokCount && fAffix.length() )
	{
	    suffix = fAffix;
	    ++i;
	}
	if( i < tokCount && isQuote( tokens[i][0] ) )
	{
	    QString proStr = stripQuotes( tokens[i++] );
	    QStringList proTok = QStringList::split( " ", proStr );
	    firstNamePro = proTok[0];
	}
    }
    else if( formattedAffix( tokens[0], prefixes ).length() && 
	    (tokCount == 2 || (tokCount >= 3 && (formattedAffix( tokens[2], suffixes ) || isQuote( tokens[2][0] ))))
	   )
    {
	// rule 2
	prefix = formattedAffix(tokens[i++], prefixes);
	lastName = tokens[i++];
	QString fAffix = formattedAffix( tokens[i], suffixes );
	if( i < tokCount && fAffix.length() )
	{
	    suffix = fAffix;
	    ++i;
	}
	if( i < tokCount && isQuote( tokens[i][0] ) )
	{
	    QString proStr = stripQuotes( tokens[i++] );
	    QStringList proTok = QStringList::split( " ", proStr );
	    lastNamePro = proTok[0];
	}
    }
    else
    {
	// rule 1

	// because this is the default rule, check to make sure we have
	// tokens for everything we process (i < tokCount)
	// so if the user enters Mr. Marc ("The Dude") it doesn't require or 
	// assume there's a Britton token remaining
	if( i < tokCount )
	{
	    QString fAffix = formattedAffix( tokens[i], prefixes );
	    if( fAffix.length() )
	    {
		prefix = fAffix;
		++i;
	    }
	}
	if( i < tokCount )
	    firstName = tokens[i++];
	if( i < tokCount && tokens[i] == "(" )
	{
	    ++i;
	    nickName =  tokens[i++];
	    ++i; // skip closing )
	}
	if( (i+1) < tokCount && !formattedAffix( tokens[i+1], suffixes ).length() && !isQuote( tokens[i+1][0] ) )
	   middleName = tokens[i++]; 
	if( i < tokCount )
	    lastName = tokens[i++];
	if( i < tokCount )
	{
	    QString fAffix = formattedAffix( tokens[i], suffixes );
	    if( fAffix.length() )
	    {
		suffix = fAffix;
		++i;
	    }
	}
	if( i < tokCount )
        {
            if( isQuote( tokens[i][0] ) )
	    {
                QString proStr = stripQuotes( tokens[i++] );
                QStringList proTok = QStringList::split( " ", proStr );
                firstNamePro = proTok[0];
                lastNamePro = proTok[1];
            }
            else
            {
                //  There is a bunch of stuff on the end. Just make it part of the last name.
                //  Just throwing this away is Bad (tm).
                while( i < tokCount)
                    lastName += " " + tokens[i++];
                
            }
	}
    }

//    QString prefix, suffix, firstName, nickName, middleName, lastName, firstNamePro, lastNamePro;
    QMap<PimContact::ContactFields, QString> newFields;
    newFields[PimContact::NameTitle] = prefix;
    newFields[PimContact::FirstName] = firstName;
    newFields[PimContact::FirstNamePronunciation] = firstNamePro;
    newFields[PimContact::MiddleName] = middleName;
    newFields[PimContact::LastName] = lastName;
    newFields[PimContact::LastNamePronunciation] = lastNamePro;
    newFields[PimContact::Suffix] = suffix;
    newFields[PimContact::Nickname] = nickName;

    newFields[PimContact::FileAs] = myFields[PimContact::FileAs]; //copy this value across 

    myFields = newFields;
    /*
    const QMap<int, QString> &displayNames = PimContact::trFieldsMap(); // use field names from PimContact
    QMap<PimContact::ContactFields, QString>::Iterator it;
    for( it = myFields.begin(); it != myFields.end() ; ++it )
	if( !(*it).isEmpty() )
	    qDebug("%s : %s\n", displayNames[ it.key() ].latin1(), (*it).latin1());
    qDebug("LastNamePronunciation = %s\n", myFields[PimContact::LastNamePronunciation].latin1());
    */
}

QString AbstractName::formattedAffix( const QString &str, QStringList &affixes )
{
    QString ia = str.lower();
    if( ia.isEmpty() )
	return QString::null;

    bool lastCharPeriod = ia[(int)ia.length()-1] == '.';
    for( QStringList::Iterator it = affixes.begin() ; it != affixes.end() ; ++it )
    {
	QString curAffix = (*it).lower();
	if( ia == curAffix || lastCharPeriod && ia == (curAffix+".") )
	    return *it;
    }
    return QString::null;
}

QString AbstractName::stripQuotes( const QString &pro )
{
    QString str = pro;
    str = str.left( str.length()-1 );
    return str.right( str.length()-1 );
}

bool AbstractName::isQuote( QChar c ) const
{
    if( c == '\"' || c == '\'' ) return TRUE;
    return FALSE;
}


void AbstractName::fieldsChanged()
{
    bool firstNameEmpty = myFields[PimContact::FirstName].isEmpty(),
         lastNameEmpty = myFields[PimContact::LastName].isEmpty(),
	 nickEmpty = myFields[PimContact::Nickname].isEmpty(),
	 firstProEmpty = myFields[PimContact::FirstNamePronunciation].isEmpty(),
	 lastProEmpty = myFields[PimContact::LastNamePronunciation].isEmpty();

    QString displayString;
    QTextOStream os(&displayString);
    if( firstNameEmpty )
    {
	os << myFields[PimContact::NameTitle] << " " << myFields[PimContact::LastName] << " " <<
	      myFields[PimContact::Suffix] << " ";
	if( !lastProEmpty )
	    os << "\"" << myFields[PimContact::LastNamePronunciation] << "\"";
    }
    else if( lastNameEmpty )
    {
	os << myFields[PimContact::FirstName] << " " << myFields[PimContact::Suffix] << " ";
	if( !firstProEmpty )
	    os << "\"" << myFields[PimContact::FirstNamePronunciation] << "\"";
    }
    else
    {
	os << myFields[PimContact::NameTitle] << " " << myFields[PimContact::FirstName] << " ";
	if( !nickEmpty )
	    os << "(" <<  myFields[PimContact::Nickname] << ") "; 
	os << myFields[PimContact::MiddleName] << " " <<
	      myFields[PimContact::LastName] << " " << myFields[PimContact::Suffix] << " ";
	if( !firstProEmpty && !lastProEmpty )
	{
	    os << "\"" << myFields[PimContact::FirstNamePronunciation] << " " 
	       << myFields[PimContact::LastNamePronunciation] << "\"";
		
	}
    }

    displayString = displayString.simplifyWhiteSpace();
    mainWidget->setText( displayString );
}

void AbstractName::textChanged()
{
    mModified = TRUE;
}

//----------------------------------------------------------------------

AbEditor::AbEditor( QWidget *parent, const char *name, WFlags fl ) 
    : QDialog( parent, name, TRUE, fl )
#ifdef QTOPIA_PHONE
    , mHaveSim( FALSE )
#endif
{
    setName( "edit" );

    mNewEntry = FALSE;

    phoneMan = 0;

    specCompanyLA = 0;

    lastUpdateInternal = FALSE;
    init();
    resize( 400, 500 );
}

AbEditor::~AbEditor()
{
}

void AbEditor::setCategory(int id)
{
    cmbCat->setCurrentCategory( id );
}

void AbEditor::init()
{
    quitExplicitly = FALSE;

    // NOTE : If you change these, you also need to change
   // AbEditor::phoneFieldsToDetailsFilter() and
    // AbEditor::detailsToPhoneFieldsFilter()
    mHPType = PhoneFieldType( "homephone", tr("Home Phone") );
    mHMType = PhoneFieldType( "homemobile", tr("Home Mobile") );
    mHFType = PhoneFieldType( "homefax", tr("Home Fax") );
    mBPType = PhoneFieldType( "businessphone", tr("Business Phone") );
    mBMType = PhoneFieldType( "businessmobile", tr("Business Mobile") );
    mBFType = PhoneFieldType( "businessfax", tr("Business Fax") );
    mBPAType = PhoneFieldType( "businesspager", tr("Business Pager") );
    phoneTypes.append( mHPType );
    phoneTypes.append( mHMType );
    phoneTypes.append( mHFType );
    phoneTypes.append( mBPType );
    phoneTypes.append( mBMType );
    phoneTypes.append( mBFType );
    phoneTypes.append( mBPAType );

    setupTabs();

    setTabOrders();

    buildLineEditList(); // create a map of QLineEdit pointers for iterating through.
			// used in AbEditor::isEmpty()

    QWhatsThis::add(spouseLE, tr("e.g. Husband or Wife."));
    QWhatsThis::add(professionLE, tr("Occupation or job description."));
    /* MB : now belongs to abstract editor */
#if 0
    QWhatsThis::add(lineEdits[PimContact::FirstNamePronunciation],
	tr("Describes the spoken name phonetically."));
    QWhatsThis::add(fileAsCombo,
	tr("Preferred format for the contact name."));
#endif 

#ifdef QTOPIA_DESKTOP
    tabs->setSizePolicy( QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum) );
    setMinimumSize( tabs->sizeHint().width(), tabs->sizeHint().height() );
    setMaximumSize( tabs->sizeHint().width()*2, tabs->sizeHint().height()*2 );
#endif

    new QPEDialogListener(this);
}

void AbEditor::setTabOrders(void)
{
#if 0
    setTabOrder(fileAsCombo, cmbCat);
    setTabOrder(cmbCat, emailLE);
    
//    voip
    setTabOrder(emailLE, voipIdLE);        
    setTabOrder(voipIdLE, emailBtn);
    
    setTabOrder(emailBtn, bdayButton);
    setTabOrder(bdayButton, genderCombo);
#endif
    // TODO : set tab orders
}

void AbEditor::tabClicked( QWidget *tab )
{
#ifdef QTOPIA_DESKTOP
    if ( summaryTab && tab == summaryTab ) {
	PimContact c;
	contactFromFields(c);
	summary->setText(c.toRichText());
    }
#else
    Q_UNUSED(tab);
    
    //  Force it to reconsider the layout of the main tab;
    //  fixes layout problems caused by business category
    //  selection from notes tab.
    if(tab == contactTab)
        mainGL->activate();
#endif
}

void AbEditor::editEmails()
{
    EmailDialog *ed = new EmailDialog(this, "Emails", TRUE);

    QString strDefaultEmail;
    QStringList emails;
    parseEmailFrom( emailLE->text(), strDefaultEmail,
	    emails );
    //ent.setDefaultEmail( strDefaultEmail );
    //ent.setEmailList( emails  );

    ed->setEmails( strDefaultEmail, emails );

#ifdef QTOPIA_DESKTOP
    if (ed->exec())
#else
    if (QPEApplication::execDialog(ed))
#endif
    {
	ent.setDefaultEmail(ed->defaultEmail());
	ent.setEmailList(ed->emails());
	QString str;
	parseEmailTo( ed->defaultEmail(), ed->emails(), str );

	emailLE->setText( str );
	emailLE->home( false );
    }

    delete ed;
}

#ifdef QTOPIA_PHONE
void AbEditor::setHaveSim( bool b )
{
    mHaveSim = b;
    homePhoneOnSim->setEnabled( b );
    homeFaxOnSim->setEnabled( b );
    homeMobileOnSim->setEnabled( b );
    busPhoneOnSim->setEnabled( b );
    busFaxOnSim->setEnabled( b );
    busMobileOnSim->setEnabled( b );
    busPagerOnSim->setEnabled( b );
    phoneMan->setHaveSim( b );
    if( b )
    {
	bool hpos = (ent.customField( AbUtil::SIM_HP ).isEmpty() ? FALSE : TRUE),
	    hfos = (ent.customField( AbUtil::SIM_HF ).isEmpty() ? FALSE : TRUE),
	    hmos = (ent.customField( AbUtil::SIM_HM ).isEmpty() ? FALSE : TRUE),
	    bpos = (ent.customField( AbUtil::SIM_BP ).isEmpty() ? FALSE : TRUE),
	    bfos = (ent.customField( AbUtil::SIM_BF ).isEmpty() ? FALSE : TRUE),
	    bmos = (ent.customField( AbUtil::SIM_BM ).isEmpty() ? FALSE : TRUE),
	    bpaos = (ent.customField( AbUtil::SIM_BPA ).isEmpty() ? FALSE : TRUE);

	    //set the toggle actions for both the phone manager and details, because toggled() doesn't already get emitted
	phoneMan->setFieldOnSim( mHPType, hpos );
	phoneMan->setFieldOnSim( mHFType, hfos );
	phoneMan->setFieldOnSim( mHMType, hmos );
	phoneMan->setFieldOnSim( mBPType, bpos );
	phoneMan->setFieldOnSim( mBFType, bfos );
	phoneMan->setFieldOnSim( mBMType, bmos );
	phoneMan->setFieldOnSim( mBPAType, bpaos );
	homePhoneOnSim->setOn( hpos );
	homeFaxOnSim->setOn( hfos );
	homeMobileOnSim->setOn( hmos );
	busPhoneOnSim->setOn( bpos );
	busFaxOnSim->setOn( bfos );
	busMobileOnSim->setOn( bmos );
	busPagerOnSim->setOn( bpaos );
    }
}
#endif

void AbEditor::setupTabs()
{
    QVBoxLayout *vb = new QVBoxLayout( this );

    // setup tab groups
    tabs = new QTabWidget( this );
    vb->addWidget( tabs );

    contactTab = new VScrollView( tabs );
    businessTab = new VScrollView( tabs );
    personalTab = new VScrollView( tabs );
    otherTab = new VScrollView( tabs );

    tabs->addTab( contactTab, Resource::loadIconSet( "addressbook/contactdetails" ), 
#ifdef QTOPIA_DESKTOP
	    tr("Contact")
#else
    ""
#endif
	    );
    tabs->addTab( businessTab, Resource::loadIconSet( "business" ), 
#ifdef QTOPIA_DESKTOP
	    tr("Business")
#else
	    "" 
#endif
	    );
    tabs->addTab( personalTab, Resource::loadIconSet( "home" ), 
#ifdef QTOPIA_DESKTOP
	    tr("Personal")
#else
	    "" 
#endif
	    );
    tabs->addTab( otherTab, Resource::loadIconSet( "addressbook/notes" ), 
#ifdef QTOPIA_DESKTOP
	    tr("Other")
#else
	    "" 
#endif
	    );

    connect( tabs, SIGNAL( currentChanged(QWidget*) ), SLOT( tabClicked(QWidget*) ) );

#ifdef QTOPIA_DESKTOP
    // ### Should use summary tab if has room, not
    // just compiled for the desktop
    summaryTab = new QWidget( tabs, "summaryTab" );
    tabs->addTab( summaryTab, tr("Summary") );

    QVBoxLayout *vbSummary = new QVBoxLayout( summaryTab, 5, 5 );
    summary = new QTextEdit( summaryTab );
    summary->setReadOnly( TRUE );
    vbSummary->addWidget( summary );
#else
    summaryTab = 0;
    summary = 0;
#endif

    // Create widgets on tabs
    QWidget *wContactTab = contactTab->widget();
    QWidget *wBusinessTab = businessTab->widget();
    QWidget *wPersonalTab = personalTab->widget();
    QWidget *wOtherTab = otherTab->widget();

    const QMap<int, QString> &displayNames = PimContact::trFieldsMap(); // use field names from PimContact
    QLabel *label;
    int rowCount = 0;

    // Contact tab
    QGridLayout *gl = new QGridLayout( wContactTab, 8, 3, 4, 1 );
#ifdef QTOPIA_DESKTOP
    gl->setResizeMode( QLayout::FreeResize);
#endif
    mainGL = gl;
#ifdef QTOPIA_DESKTOP
    gl->setSpacing( 2 );
    gl->setMargin( 2 );
#else
    gl->addColSpacing(1,4);
#endif

    // FIXME : Temporary fix to stop spacing from changing
    //gl->addItem( new QSpacerItem( 50, 1, QSizePolicy::Fixed, QSizePolicy::Fixed ), rowCount, 0 );
    //++rowCount;


    //   Abstract Name
    label = new QLabel( tr("Name"), wContactTab );
    label->setAlignment(AlignRight|AlignVCenter);
    gl->addWidget( label, rowCount, 0 );
#ifdef QTOPIA_PHONE
    abName = new AbstractName( wContactTab );
    gl->addWidget( abName, rowCount, 2 );
#else
    QHBox *nb = new QHBox( wContactTab );
    abName = new AbstractName( nb );
    QPushButton *ncm = new QPushButton( "...", nb );
    ncm->setMaximumWidth( ncm->height() );
    connect( ncm, SIGNAL(clicked()), abName, SLOT(details()) );
    gl->addWidget( nb, rowCount, 2 );
#endif
    ++rowCount;

    //   Emails
    label = new QLabel( displayNames[ PimContact::Emails ], wContactTab );
    label->setAlignment(AlignRight|AlignVCenter);
    gl->addWidget( label, rowCount, 0 );
#ifdef QTOPIA_PHONE
    emailLE = new QLineEdit( wContactTab );
    QPEApplication::setInputMethodHint(emailLE,QPEApplication::Named,"email");
    emailContextMenu = ContextMenu::menuFor( emailLE );
    //TODO Icon for this action
    actionEmailDetails = new QAction( tr( "Details" ), Resource::loadIconSet( 
		"field-details" ), QString::null, 0, this, 0 );
    emailContextMenu->insertSeparator(0);
    emailContextMenu->insertAction(actionEmailDetails, 0);
    connect( actionEmailDetails, SIGNAL(activated()), this, SLOT(editEmails()) );
    gl->addWidget( emailLE, rowCount, 2 );

#ifdef QTOPIA_VOIP
    ++rowCount;
    
    label = new QLabel( tr("VoIPid"), wContactTab );
    label->setAlignment(AlignRight|AlignVCenter);
    gl->addWidget( label, rowCount, 0 );

    //VOIP 
    voipIdLE = new QLineEdit( wContactTab );
    QPEApplication::setInputMethodHint(voipIdLE,QPEApplication::Text,"voipId");
    gl->addWidget( voipIdLE, rowCount, 2 );
#endif
    
#else
    QHBox *ehb = new QHBox( wContactTab );
    emailLE = new QLineEdit( ehb );
    QPushButton *ecm = new QPushButton( "...", ehb );
    ecm->setMaximumWidth( ecm->height() );
    connect( ecm, SIGNAL(clicked()), this, SLOT(editEmails()) );
    gl->addWidget( ehb, rowCount, 2 );
#endif
    ++rowCount;

    //   Category Checkbox
    categoryCB = new QCheckBox( tr("Business contact"), wContactTab );
    connect( categoryCB, SIGNAL(toggled(bool)), this, SLOT(catCheckBoxChanged(bool)) );
    gl->addMultiCellWidget( categoryCB, rowCount, rowCount, 0, 2, AlignHCenter );
    ++rowCount;

    //   Specific widgets : show up depending on contact type
    specRow = rowCount;
    setupSpecWidgets( wContactTab );
    ++rowCount; ++rowCount;
    
    //    Phone fields
    int spacerRow = rowCount + phoneTypes.count();
    phoneManRow = rowCount;
    setupPhoneFields( wContactTab );
    int t = rowCount;
    rowCount = phoneManRow;
    phoneManRow = t;

    QSpacerItem *verticalSpacer = new QSpacerItem( 1, 1, QSizePolicy::Minimum,
						   QSizePolicy::Expanding );
    gl->addItem( verticalSpacer, spacerRow, 2 );

    // Business Tab
    gl = new QGridLayout( wBusinessTab, 16, 3, 1, 1 );
#ifdef QTOPIA_DESKTOP
    gl->setResizeMode( QLayout::FreeResize );
#endif
    rowCount = 0;

#ifdef QTOPIA_DESKTOP
    gl->setSpacing( 4 );
#else
    gl->addColSpacing(1,4);
#endif
    //    Company
    label = new QLabel( 
#if defined(QTOPIA_PHONE) && !defined(QTOPIA_DESKTOP)
	    tr("Cmpny", "short for company")
#else
	   tr("Company")
#endif
		, wBusinessTab );

    label->setAlignment(AlignRight|AlignVCenter);
    companyLE = new QLineEdit( wBusinessTab );
    connect( companyLE, SIGNAL(textChanged(const QString&)), this, 
					SLOT(specFieldsFilter(const QString&)) );
    gl->addWidget( label, rowCount, 0 );
    gl->addWidget( companyLE, rowCount, 2 );
    ++rowCount;
    //    Company pronounciation
    label = new QLabel( displayNames[ PimContact::CompanyPronunciation ], 
								wBusinessTab );
    companyProLE = new QLineEdit( wBusinessTab );
    adjustPronWidgets(label,companyProLE);
    gl->addWidget( label, rowCount, 0 );
    gl->addWidget( companyProLE, rowCount, 2 );
    ++rowCount;
    //    Job title
    label = new QLabel( tr("Title"), wBusinessTab );
    label->setAlignment(AlignRight|AlignVCenter);
    jobTitleLE = new QLineEdit(  wBusinessTab );
    connect( jobTitleLE, SIGNAL(textChanged(const QString&)), this, 
					    SLOT(specFieldsFilter(const QString&)) );
    gl->addWidget( label, rowCount, 0 );
    gl->addWidget( jobTitleLE, rowCount, 2 );
    ++rowCount;
    //    Phone
    label = new QLabel( tr("Phone"), wBusinessTab );
    label->setAlignment(AlignRight|AlignVCenter);
    busPhoneLE = new QLineEdit( wBusinessTab );
    QPEApplication::setInputMethodHint(busPhoneLE,QPEApplication::PhoneNumber);
    connect( busPhoneLE, SIGNAL(textChanged(const QString&)), this,
	    SLOT(detailsToPhoneFieldsFilter(const QString&)) );
#ifdef QTOPIA_PHONE
    ContextMenu *menu = ContextMenu::menuFor( busPhoneLE );
    busPhoneOnSim = addStoreOnSim(menu, this, SLOT(detailSimFilter(bool)));
    simActions[PimContact::BusinessPhone] = busPhoneOnSim;
#endif
    gl->addWidget( label, rowCount, 0 );
    gl->addWidget( busPhoneLE, rowCount, 2 );
    ++rowCount;
    //    Mobile
    label = new QLabel( tr("Mobile"), wBusinessTab );
    label->setAlignment(AlignRight|AlignVCenter);
    busMobileLE = new QLineEdit( wBusinessTab );
    QPEApplication::setInputMethodHint(busMobileLE,QPEApplication::PhoneNumber);
    connect( busMobileLE, SIGNAL(textChanged(const QString&)), this,
	    SLOT(detailsToPhoneFieldsFilter(const QString&)) );
#ifdef QTOPIA_PHONE
    menu = ContextMenu::menuFor( busMobileLE );
    busMobileOnSim = addStoreOnSim(menu, this, SLOT(detailSimFilter(bool)));
    simActions[PimContact::BusinessMobile] = busMobileOnSim;
#endif
    gl->addWidget( label, rowCount, 0 );
    gl->addWidget( busMobileLE, rowCount, 2 );
    ++rowCount;
    //    Fax
    label = new QLabel( tr("Fax"), wBusinessTab );
    label->setAlignment(AlignRight|AlignVCenter);
    busFaxLE = new QLineEdit( wBusinessTab );
    QPEApplication::setInputMethodHint(busFaxLE,QPEApplication::PhoneNumber);
    connect( busFaxLE, SIGNAL(textChanged(const QString&)), this,
	    SLOT(detailsToPhoneFieldsFilter(const QString&)) );
#ifdef QTOPIA_PHONE
    menu = ContextMenu::menuFor( busFaxLE );
    busFaxOnSim = addStoreOnSim(menu, this, SLOT(detailSimFilter(bool)));
    simActions[PimContact::BusinessFax] = busFaxOnSim;
#endif
    gl->addWidget( label, rowCount, 0 );
    gl->addWidget( busFaxLE, rowCount, 2 );
    ++rowCount;
    //    Pager
    label = new QLabel( tr("Pager"), wBusinessTab );
    label->setAlignment(AlignRight|AlignVCenter);
    busPagerLE = new QLineEdit( wBusinessTab );
    QPEApplication::setInputMethodHint(busPagerLE,QPEApplication::PhoneNumber);
    connect( busPagerLE, SIGNAL(textChanged(const QString&)), this,
	    SLOT(detailsToPhoneFieldsFilter(const QString&)) );
#ifdef QTOPIA_PHONE
    menu = ContextMenu::menuFor( busPagerLE );
    busPagerOnSim = addStoreOnSim(menu, this, SLOT(detailSimFilter(bool)));
    simActions[PimContact::BusinessPager] = busPagerOnSim;
#endif
    gl->addWidget( label, rowCount, 0 );
    gl->addWidget( busPagerLE, rowCount, 2 );
    ++rowCount;
    //    Business Address
    label = new QLabel( tr("Street"), wBusinessTab );
    label->setAlignment(AlignRight|AlignVCenter);
    busStreetME = new QMultiLineEdit( wBusinessTab );
#ifdef QTOPIA_DESKTOP
    busStreetME->setMaximumHeight( fontMetrics().lineSpacing() * 4 );
#endif
    gl->addWidget( label, rowCount, 0 );
    gl->addWidget( busStreetME, rowCount, 2 );
    ++rowCount;

    label = new QLabel( tr("City"), wBusinessTab );
    label->setAlignment(AlignRight|AlignVCenter);
    busCityLE = new QLineEdit( wBusinessTab );
    gl->addWidget( label, rowCount, 0 );
    gl->addWidget( busCityLE, rowCount, 2 );
    ++rowCount;

    label = new QLabel( tr("State"), wBusinessTab );
    label->setAlignment(AlignRight|AlignVCenter);
    busStateLE = new QLineEdit( wBusinessTab );
    gl->addWidget( label, rowCount, 0 );
    gl->addWidget( busStateLE, rowCount, 2 );
    ++rowCount;

    label = new QLabel( tr("Zip"), wBusinessTab );
    label->setAlignment(AlignRight|AlignVCenter);
    busZipLE = new QLineEdit( wBusinessTab );
    QPEApplication::setInputMethodHint( busZipLE, QPEApplication::Number );
    gl->addWidget( label, rowCount, 0 );
    gl->addWidget( busZipLE, rowCount, 2 );
    ++rowCount;

    label = new QLabel( tr("Country"), wBusinessTab );
    label->setAlignment(AlignRight|AlignVCenter);
    busCountryLE = new QLineEdit( wBusinessTab );
    gl->addWidget( label, rowCount, 0 );
    gl->addWidget( busCountryLE, rowCount, 2 );
    ++rowCount;

    //    Web page
    label = new QLabel( tr("URL"), wBusinessTab );
    label->setAlignment(AlignRight|AlignVCenter);
    busWebPageLE = new QLineEdit( wBusinessTab );
    gl->addWidget( label, rowCount, 0 );
    gl->addWidget( busWebPageLE, rowCount, 2 );
    ++rowCount;
    //    Department
    label = new QLabel( displayNames[ PimContact::Department ], wBusinessTab );
    label->setAlignment(AlignRight|AlignVCenter);
    deptLE = new QLineEdit( wBusinessTab );
    gl->addWidget( label, rowCount, 0 );
    gl->addWidget( deptLE, rowCount, 2 );
    ++rowCount;
    //    Office
    label = new QLabel( displayNames[ PimContact::Office ], wBusinessTab );
    label->setAlignment(AlignRight|AlignVCenter);
    officeLE = new QLineEdit( wBusinessTab );
    gl->addWidget( label, rowCount, 0 );
    gl->addWidget( officeLE, rowCount, 2 );
    ++rowCount;
    //    Profession
    label = new QLabel( displayNames[ PimContact::Profession ], wBusinessTab );
    label->setAlignment(AlignRight|AlignVCenter);
    professionLE = new QLineEdit( wBusinessTab );
    gl->addWidget( label, rowCount, 0 );
    gl->addWidget( professionLE, rowCount, 2 );
    ++rowCount;
    //    Manager
    label = new QLabel( displayNames[ PimContact::Manager ], wBusinessTab );
    label->setAlignment(AlignRight|AlignVCenter);
    managerLE = new QLineEdit( wBusinessTab );
    gl->addWidget( label, rowCount, 0 );
    gl->addWidget( managerLE, rowCount, 2 );
    ++rowCount;
    //    Assistant
    label = new QLabel( displayNames[ PimContact::Assistant ], wBusinessTab );
    label->setAlignment(AlignRight|AlignVCenter);
    assistantLE = new QLineEdit( wBusinessTab );
    gl->addWidget( label, rowCount, 0 );
    gl->addWidget( assistantLE, rowCount, 2 );
    ++rowCount;
    verticalSpacer = new QSpacerItem( 1, 1, QSizePolicy::Minimum,
						   QSizePolicy::Expanding );
    gl->addItem( verticalSpacer, rowCount, 0 );

    // Home Tab
    gl = new QGridLayout( wPersonalTab, 10, 3, 1, 1 );
#ifdef QTOPIA_DESKTOP
    gl->setResizeMode( QLayout::FreeResize );
#endif
#ifdef QTOPIA_DESKTOP
    gl->setSpacing( 4 );
#else
    gl->addColSpacing(1,4);
#endif
    rowCount = 0;
    //    Home phone
    label = new QLabel( tr("Phone"), wPersonalTab );
    label->setAlignment(AlignRight|AlignVCenter);
    homePhoneLE = new QLineEdit( wPersonalTab );
    QPEApplication::setInputMethodHint(homePhoneLE,QPEApplication::PhoneNumber);
    connect( homePhoneLE, SIGNAL(textChanged(const QString&)), this,
	    SLOT(detailsToPhoneFieldsFilter(const QString&)) );
#ifdef QTOPIA_PHONE
    menu = ContextMenu::menuFor( homePhoneLE );
    homePhoneOnSim = addStoreOnSim(menu, this, SLOT(detailSimFilter(bool)));
    simActions[PimContact::HomePhone] = homePhoneOnSim;
#endif
    gl->addWidget( label, rowCount, 0 );
    gl->addWidget( homePhoneLE, rowCount, 2 );
    ++rowCount;
    //    Home mobile
    label = new QLabel( tr("Mobile"), wPersonalTab );
    label->setAlignment(AlignRight|AlignVCenter);
    homeMobileLE = new QLineEdit( wPersonalTab );
    QPEApplication::setInputMethodHint(homeMobileLE,QPEApplication::PhoneNumber);
    connect( homeMobileLE, SIGNAL(textChanged(const QString&)), this,
	    SLOT(detailsToPhoneFieldsFilter(const QString&)) );
#ifdef QTOPIA_PHONE
    menu = ContextMenu::menuFor( homeMobileLE );
    homeMobileOnSim = addStoreOnSim(menu, this, SLOT(detailSimFilter(bool)));
    simActions[PimContact::HomeMobile] = homeMobileOnSim;
#endif
    gl->addWidget( label, rowCount, 0 );
    gl->addWidget( homeMobileLE, rowCount, 2 );
    ++rowCount;
    //    Home fax
    label = new QLabel( tr("Fax"), wPersonalTab );
    label->setAlignment(AlignRight|AlignVCenter);
    homeFaxLE = new QLineEdit( wPersonalTab );
    QPEApplication::setInputMethodHint(homeFaxLE,QPEApplication::PhoneNumber);
    connect( homeFaxLE, SIGNAL(textChanged(const QString&)), this,
	    SLOT(detailsToPhoneFieldsFilter(const QString&)) );
#ifdef QTOPIA_PHONE
    menu = ContextMenu::menuFor( homeFaxLE );
    homeFaxOnSim = addStoreOnSim(menu, this, SLOT(detailSimFilter(bool)));
    simActions[PimContact::HomeFax] = homeFaxOnSim;
#endif
    gl->addWidget( label, rowCount, 0 );
    gl->addWidget( homeFaxLE, rowCount, 2 );
    ++rowCount;
    //    Home address
    label = new QLabel( tr("Street"), wPersonalTab );
    label->setAlignment(AlignRight|AlignVCenter);
    homeStreetME = new QMultiLineEdit( wPersonalTab );
#ifdef QTOPIA_DESKTOP
    homeStreetME->setMaximumHeight( fontMetrics().lineSpacing() * 4 );
#endif
    gl->addWidget( label, rowCount, 0 );
    gl->addWidget( homeStreetME, rowCount, 2 );
    ++rowCount;

    label = new QLabel( tr("City"), wPersonalTab );
    label->setAlignment(AlignRight|AlignVCenter);
    homeCityLE = new QLineEdit( wPersonalTab );
    gl->addWidget( label, rowCount, 0 );
    gl->addWidget( homeCityLE, rowCount, 2 );
    ++rowCount;

    label = new QLabel( tr("State"), wPersonalTab );
    label->setAlignment(AlignRight|AlignVCenter);
    homeStateLE = new QLineEdit( wPersonalTab );
    gl->addWidget( label, rowCount, 0 );
    gl->addWidget( homeStateLE, rowCount, 2 );
    ++rowCount;

    label = new QLabel( tr("Zip"), wPersonalTab );
    label->setAlignment(AlignRight|AlignVCenter);
    homeZipLE = new QLineEdit( wPersonalTab );
    QPEApplication::setInputMethodHint( homeZipLE, QPEApplication::Number );
    gl->addWidget( label, rowCount, 0 );
    gl->addWidget( homeZipLE, rowCount, 2 );
    ++rowCount;

    label = new QLabel( tr("Country"), wPersonalTab );
    label->setAlignment(AlignRight|AlignVCenter);
    homeCountryLE = new QLineEdit( wPersonalTab );
    gl->addWidget( label, rowCount, 0 );
    gl->addWidget( homeCountryLE, rowCount, 2 );
    ++rowCount;

    //    Home web page
    label = new QLabel( tr("URL"), wPersonalTab );
    label->setAlignment(AlignRight|AlignVCenter);
    homeWebPageLE = new QLineEdit( wPersonalTab );
    gl->addWidget( label, rowCount, 0 );
    gl->addWidget( homeWebPageLE, rowCount, 2 );
    ++rowCount;
    //    Spouse
    label = new QLabel( displayNames[ PimContact::Spouse ], wPersonalTab );
    label->setAlignment(AlignRight|AlignVCenter);
    spouseLE = new QLineEdit( wPersonalTab );
    gl->addWidget( label, rowCount, 0 );
    gl->addWidget( spouseLE, rowCount, 2 );
    ++rowCount;
    //    Children
    label = new QLabel( displayNames[ PimContact::Children ], wPersonalTab );
    label->setAlignment(AlignRight|AlignVCenter);
    childrenLE = new QLineEdit( wPersonalTab );
    gl->addWidget( label, rowCount, 0 );
    gl->addWidget( childrenLE, rowCount, 2 );
    ++rowCount;
    //    Anniversary
    label = new QLabel( tr("Anniv."), wPersonalTab );
    label->setAlignment(AlignRight|AlignVCenter);
    anniversaryButton = new QPEDateEdit( wPersonalTab, "", FALSE, TRUE );
    gl->addWidget( label, rowCount, 0 );
    gl->addWidget( anniversaryButton, rowCount, 2 );
    ++rowCount;
    //    Gender
    label = new QLabel( displayNames[ PimContact::Gender ], wPersonalTab );
    label->setAlignment(AlignRight|AlignVCenter);
    genderCombo = new QComboBox( FALSE, wPersonalTab );
    genderCombo->insertItem( "", 0 );
    genderCombo->insertItem( tr( "Male" ), 1 );
    genderCombo->insertItem( tr( "Female" ), 2 );
    gl->addWidget( label, rowCount, 0 );
    gl->addWidget( genderCombo, rowCount, 2 );
    ++rowCount;
    //    Birthday
    label = new QLabel( displayNames[ PimContact::Birthday ], wPersonalTab );
    label->setAlignment(AlignRight|AlignVCenter);
    bdayButton = new QPEDateEdit( wPersonalTab, "", FALSE, TRUE );
    gl->addWidget( label,  rowCount, 0 );
    gl->addWidget( bdayButton, rowCount, 2 );
    ++rowCount;
    verticalSpacer = new QSpacerItem( 1, 1, QSizePolicy::Minimum,
						   QSizePolicy::Expanding );
    gl->addItem( verticalSpacer, rowCount, 1 );

    // Other Tab
    rowCount = 0;
    gl = new QGridLayout( wOtherTab, 4, 3, 1, 1 );
#ifdef QTOPIA_DESKTOP
    gl->setResizeMode( QLayout::FreeResize );
#endif
#ifdef QTOPIA_DESKTOP
    gl->setSpacing( 4 );
#else
    gl->addColSpacing(1,4);
#endif

#ifndef QTOPIA_DESKTOP // not supported on QD yet
    // Photo
    label = new QLabel( tr("Photo"), wOtherTab );
    photoPB = new PixmapDisplay( wOtherTab );
    photoPB->setDisplaySize( QSize(80, 96) );
    connect( photoPB, SIGNAL(clicked()), this, SLOT(editPhoto()) );
    gl->addWidget( label, rowCount, 0 );
    gl->addWidget( photoPB, rowCount, 2, AlignHCenter );
    ++rowCount;
#endif

#if defined(QTOPIA_PHONE) && !defined(QTOPIA_DESKTOP)
    // ringtone selection
    label = new QLabel( tr("Ringtone"), wOtherTab );
    editTonePB = new RingToneButton( wOtherTab );
    editTonePB->setAllowNone(TRUE);
    gl->addWidget( label, rowCount, 0 );
    gl->addWidget( editTonePB, rowCount, 2 );
    ++rowCount;
#endif

    //    Notes
    label = new QLabel( tr("Notes"), wOtherTab );
    txtNote = new QMultiLineEdit( wOtherTab );
#ifdef QTOPIA_DATA_LINKING
    QDLWidgetClient *txtNoteQC = new QDLWidgetClient( txtNote, "contactNotes" );
#ifdef QTOPIA_PHONE
    txtNoteQC->setupStandardContextMenu();
#else
    PixmapDisplay *linkButton = new PixmapDisplay( wOtherTab );
    linkButton->setPixmap( Resource::loadIconSet( "qdllink" ).pixmap( QIconSet::Small, TRUE ) );
    connect( linkButton, SIGNAL(clicked()), txtNoteQC, SLOT(requestLink()) );
    linkButton->setFocusPolicy( NoFocus );
#endif
#endif
    txtNote->setWordWrap(QMultiLineEdit::WidgetWidth);
#if defined(QTOPIA_PHONE) || !defined(QTOPIA_DATA_LINKING)
    gl->addMultiCellWidget( label, rowCount, rowCount, 0, 2 );
#else
    // non phone with QDL enabled
    gl->addWidget( label, rowCount, 0 );
    gl->addWidget( linkButton, rowCount, 2, Qt::AlignRight );
#endif
    ++rowCount;
    gl->addMultiCellWidget( txtNote, rowCount, rowCount, 0, 2 );
    ++rowCount;

    gl->addItem( new QSpacerItem( 1, 5, QSizePolicy::Fixed, QSizePolicy::Fixed ), 
								    rowCount, 1 );
    ++rowCount;

    //    Categories
    QArray<int> emptyCat;
    cmbCat = new CategorySelect( emptyCat, "Address Book", // No tr()
	    tr("Contacts"), wOtherTab );
    connect(cmbCat, SIGNAL(signalSelected(int)), this, 
						SLOT(categorySelectChanged(int)));
    /*
    if ( cmbCat->widgetType() == CategorySelect::ListView )
    {
    */
	label = new QLabel ( tr("Category")/*displayNames[ PimContact::Categories ]*/
		, wOtherTab );
#ifdef QTOPIA_DESKTOP
	gl->addMultiCellWidget( label, rowCount, rowCount, 0, 2 );
	++rowCount;
	gl->addMultiCellWidget( cmbCat, rowCount, rowCount, 0, 2 );
#else
	label->setAlignment(AlignRight|AlignVCenter);
	gl->addWidget( label, rowCount, 0 );
	gl->addWidget( cmbCat, rowCount, 2 );
#endif
    /*
    }
    else
   {
	gl->addMultiCellWidget( cmbCat, rowCount, rowCount, 0, 2 );
    }  
    */
    ++rowCount;

    verticalSpacer = new QSpacerItem( 1, 1, QSizePolicy::Minimum,
						   QSizePolicy::Expanding );
    gl->addItem( verticalSpacer, rowCount, 1 );

#ifdef QTOPIA_DESKTOP
    QHBoxLayout *bottomBox = new QHBoxLayout( vb );
    bottomBox->addStretch();
    QPushButton *okButton = new QPushButton( tr("OK"), this );
    okButton->setDefault( TRUE );
    connect( okButton, SIGNAL(clicked()), this, SLOT(accept()) );
    bottomBox->addWidget( okButton );
    QPushButton *cancelButton = new QPushButton( tr("Cancel"), this );
    cancelButton->setAutoDefault( TRUE );
    connect( cancelButton, SIGNAL(clicked()), this, SLOT(reject()) );
    bottomBox->addWidget( cancelButton );
#endif

#ifdef QTOPIA_PHONE
    //context menus for sim phone numbers
#endif
}

void AbEditor::setupSpecWidgets( QWidget *parent )
{
    if( specCompanyLA ) // recreate all specific widgets
    {
	delete specCompanyLA;
	delete specCompanyLE;
	delete specJobTitleLA;
	delete specJobTitleLE;
    }

    QGridLayout *gl = mainGL;
    int rowCount = specRow;
    
    specCompanyLA = new QLabel( 
#if defined(QTOPIA_PHONE) && !defined(QTOPIA_DESKTOP)
    tr("Cmpny", "short for company")
#else
    tr("Company")
#endif
    , parent );
    specCompanyLA->setAlignment(AlignRight);
    specCompanyLE = new QLineEdit( parent );
    connect( specCompanyLE, SIGNAL(textChanged(const QString&)), this, 
					    SLOT(specFieldsFilter(const QString&)) );
    gl->addWidget( specCompanyLA, rowCount, 0 );
    gl->addWidget( specCompanyLE, rowCount, 2 );
    ++rowCount;

    specJobTitleLA = new QLabel( tr("Title"), parent );
    specJobTitleLA->setAlignment(AlignRight);
    specJobTitleLE = new QLineEdit( parent );
    connect( specJobTitleLE, SIGNAL(textChanged(const QString&)), this, 
					    SLOT(specFieldsFilter(const QString&)) );
    gl->addWidget( specJobTitleLA, rowCount, 0 );
    gl->addWidget( specJobTitleLE, rowCount, 2 );
    ++rowCount;
}

void AbEditor::editPhoto()
{
    ImageSourceDialog *iface = new ImageSourceDialog( mContactImage, this );
    
    //  Work with the photo at the maximum size this device's screen can display without scaling
    iface->setMaximumImageSize( qApp->desktop()->size() );
    
#ifdef QTOPIA_DESKTOP
    if( iface->exec() == QDialog::Accepted )
#else
    if( QPEApplication::execDialog( iface ) == QDialog::Accepted )
#endif
    {
	mContactImage = iface->pixmap();
        
        if( !mContactImage.isNull() )
        {
            QPixmap scaledPixmap;
            QSize scaledSize;
            
            scaledSize = Image::aspectScaleSize( mContactImage.width(), 
                mContactImage.height(), 80, 96 );
            
            scaledPixmap.convertFromImage( mContactImage.convertToImage().smoothScale(
                scaledSize.width(), scaledSize.height() ) ); 
            photoPB->setPixmap( scaledPixmap );
        }
        else
            photoPB->setPixmap( QPixmap() );
    }
    delete iface;
}

void AbEditor::catCheckBoxChanged( bool  b  )
{
    Categories c;
    if( !c.load( categoryFileName() ) )
	return;
    int bcatid = c.id( "addressbook", "_Business" );
    QArray<int> curCats = cmbCat->currentCategories();
    if( !bcatid )
	return;
    if( b )
    {
	if( !curCats.contains( bcatid ) )
	{
	    int s = curCats.size();
	    curCats.resize( s+1 );
	    curCats[s] = bcatid;
	    cmbCat->setCategories( curCats, "Address Book", // No tr
		tr("Contacts") );
	}
    }
    else
    {
	int bi = curCats.find(bcatid);
	if( bi >= 0 )
	{
	    const int ns =  curCats.count()-1;
	    int last = curCats[ns];
	    curCats.resize( ns );
	    if ( bi < ns )
		curCats[bi] = last;
	    cmbCat->setCategories( curCats, "Address Book", // No tr
		tr("Contacts") );
	}
    }
    showSpecWidgets( b );
}

void AbEditor::showSpecWidgets( bool s )
{
    if( s ) 
    {
	specCompanyLA->show();
	specCompanyLE->show();

	specJobTitleLA->show();
	specJobTitleLE->show();
    }
    else
    {
	specCompanyLA->hide();
	specCompanyLE->hide();

	specJobTitleLA->hide();
	specJobTitleLE->hide();
    }
}

void AbEditor::categorySelectChanged( int )
{
    Categories c;
    bool loadOK = c.load( categoryFileName() );
    int bcid = c.id( "addressbook", "_Business" );
    if ( loadOK && bcid ) {
	categoryCB->setChecked( cmbCat->currentCategories().contains(bcid) );
    }
}

void AbEditor::buildLineEditList()
{
    lineEdits[PimContact::Emails] = emailLE ;

    lineEdits[PimContact::Company] = companyLE ;
    lineEdits[PimContact::CompanyPronunciation] = companyProLE;
    lineEdits[PimContact::JobTitle] = jobTitleLE;
    lineEdits[PimContact::BusinessPhone] = busPhoneLE;
    lineEdits[PimContact::BusinessFax] = busFaxLE;
    lineEdits[PimContact::BusinessMobile] = busMobileLE;
    lineEdits[PimContact::BusinessPager] = busPagerLE;
    lineEdits[PimContact::BusinessWebPage] = busWebPageLE;
    lineEdits[PimContact::Department] = deptLE;
    lineEdits[PimContact::Office] = officeLE;
    lineEdits[PimContact::Profession] = professionLE;
    lineEdits[PimContact::Manager] = managerLE;
    lineEdits[PimContact::Assistant] = assistantLE;

    lineEdits[PimContact::Spouse] = spouseLE;
    lineEdits[PimContact::Children] = childrenLE;
    lineEdits[PimContact::HomePhone] = homePhoneLE;
    lineEdits[PimContact::HomeFax] = homeFaxLE;
    lineEdits[PimContact::HomeMobile] = homeMobileLE;
    lineEdits[PimContact::HomeWebPage] = homeWebPageLE;

    lineEdits[PimContact::HomeCity] = homeCityLE;
    lineEdits[PimContact::HomeState] = homeStateLE;
    lineEdits[PimContact::HomeZip] = homeZipLE;
    lineEdits[PimContact::HomeCountry] = homeCountryLE;
    lineEdits[PimContact::BusinessCity] = busCityLE;
    lineEdits[PimContact::BusinessState] = busStateLE;
    lineEdits[PimContact::BusinessZip] = busZipLE;
    lineEdits[PimContact::BusinessCountry] = busCountryLE;
}

void AbEditor::setupPhoneFields( QWidget *parent )
{
    if( phoneMan ) delete phoneMan;
    QWidget *phoneContainer = parent;
//    QWidget *mb = new QWidget( parent );
//    QVBoxLayout *mbl = new QVBoxLayout( mb );
//    phoneContainer->setBackgroundColor( QColor( 255, 0, 0 ) );
    int &rowCount = phoneManRow;

    QGridLayout *phoneLayout = mainGL;
//    phoneLayout->addColSpacing(1,4);
    //phoneLayout->setColStretch(0,1);
    //phoneLayout->setColStretch(1,10);

    /*
    phoneLayout->addMultiCellWidget( new QLabel( tr("Contact Numbers"), 
		    phoneContainer ), rowCount, rowCount, 0, 2, AlignHCenter );
    ++rowCount;
    */

    phoneMan = new PhoneFieldManager( phoneContainer, phoneLayout, rowCount );
    phoneMan->setTypes( phoneTypes );
    phoneMan->addEmpty();
    connect( phoneMan, SIGNAL(fieldChanged(const QString&,const PhoneFieldType&)),
	    this, SLOT(phoneFieldsToDetailsFilter(const QString&,const PhoneFieldType&)) );
    connect( phoneMan, SIGNAL(fieldOnSim(bool,const PhoneFieldType&)),
	    this, SLOT(phoneManagerSimFilter(bool,const PhoneFieldType&)) );
    //phoneLayout->addItem( new QSpacerItem( 1, 1,
		       // QSizePolicy::Minimum, QSizePolicy::Expanding ), 9, 1 );

    phoneManOld = FALSE;
}

void AbEditor::phoneManagerSimFilter( bool onSim, const PhoneFieldType &type )
{
#ifdef QTOPIA_PHONE
    QAction *simAction = 0;
    if( type.id == "homephone" )
	simAction = homePhoneOnSim;
    else if( type.id == "businessphone" )
	simAction = busPhoneOnSim;
    else if( type.id == "businessmobile" )
	simAction = busMobileOnSim;
    else if( type.id == "homemobile" )
	simAction = homeMobileOnSim;
    else if( type.id == "businessfax" )
	simAction = busFaxOnSim;
    else if( type.id == "homefax" )
	simAction = homeFaxOnSim;
    else if( type.id == "businesspager" )
	simAction = busPagerOnSim;
    if( simAction )
    {
	simAction->setOn( onSim );
    }
#else
    Q_UNUSED( onSim );
    //Q_UNUSED( type ); This gives a compile error
    // So we do this:
    if( type.id == "homephone" ) {
        // nothing
    }
#endif
}

void AbEditor::detailSimFilter( bool onSim )
{
#ifdef QTOPIA_PHONE
    const QObject *detail = sender();

    PhoneFieldType fieldType;
    if( detail == busPhoneOnSim )
	fieldType.id = "businessphone";
    else if( detail == homePhoneOnSim )
	fieldType.id = "homephone";
    else if( detail == busMobileOnSim )
	fieldType.id = "businessmobile";
    else if( detail == homeMobileOnSim )
	fieldType.id = "homemobile";
    else if( detail == busFaxOnSim )
	fieldType.id = "businessfax";
    else if( detail == homeFaxOnSim )
	fieldType.id = "homefax";
    else if( detail == busPagerOnSim )
	fieldType.id = "businesspager";
    if( !fieldType.isNull() )
    {
	phoneMan->setFieldOnSim( fieldType, onSim );
    }
    else
    {
	qDebug("BUG : fieldType is NULL");
    }
#else
    Q_UNUSED( onSim );
#endif
}

void AbEditor::specFieldsFilter( const QString &newValue )
{
    QLineEdit *s = (QLineEdit *)sender();
    QLineEdit *r = 0;

    if( s == specJobTitleLE )
	r = jobTitleLE;
    else if( s == specCompanyLE )
	r = companyLE;
    else if( s == jobTitleLE )
	r = specJobTitleLE;
    else if( s == companyLE )
	r = specCompanyLE;

    if( r )
    {
	if( s->text() == r->text() )
	    return;

	r->setText( newValue );
    }
}

// when phone fields changed, this slot gets called and sets the corresponding 
// phone widgets in other tabs
void AbEditor::phoneFieldsToDetailsFilter( const QString &newNumber, 
						    const PhoneFieldType &newType )
{
    QLineEdit *detail = 0;
    if( newType == mHPType ) //phone
	detail = homePhoneLE;
    else if( newType == mHMType )//mobile
	detail = homeMobileLE;
    else if( newType == mHFType )//fax
	detail = homeFaxLE;

    else if( newType == mBPType )//phone
	detail = busPhoneLE;
    else if( newType == mBMType )//mobile
	detail = busMobileLE;
    else if( newType == mBFType )//fax
	detail = busFaxLE;
    else if( newType == mBPAType )//pager
	detail = busPagerLE;

    if( detail )
    {
	//avoid recursion
	if( newNumber == detail->text() )
	    return;

	lastUpdateInternal = TRUE;
	detail->setText( newNumber );

    }
}

// when details of phone fields changed, this updates the phone field manager
void AbEditor::detailsToPhoneFieldsFilter( const QString &newNumber )
{
    QLineEdit *detail = (QLineEdit *)sender();
    PhoneFieldType type;

    if( detail == busPhoneLE )
	type = mBPType ;
    else if( detail == busMobileLE )
	type = mBMType ;
    else if( detail == busFaxLE )
	type = mBFType ;
    else if( detail == busPagerLE )
	type = mBPAType;
    else if( detail == homePhoneLE )
	type = mHPType ;
    else if( detail == homeMobileLE )
	type = mHMType ;
    else if( detail == homeFaxLE )
	type = mHFType ;

    if( !type.pixmap.isNull() && !lastUpdateInternal )
	phoneMan->setNumberFromType( type, newNumber );

    lastUpdateInternal = FALSE;
}

void AbEditor::setEntry( const PimContact &entry, bool newEntry )
{
    if( newEntry )
	setCaption( tr("New Contact") );
    else
	setCaption( tr("Edit Contact") );
    mImageModified = FALSE;
    if( phoneManOld ) // second call of setEntry on the same spec widgets. recreate 
    {
	//recreate specific widgets while we're at it
	//setupSpecWidgets( contactTab->widget() );

	setupPhoneFields( contactTab->widget() );

	//reset all specific widget's detail widgets
	homePhoneLE->setText( "" );
        homeFaxLE->setText( "" );
	homeMobileLE->setText( "" );
	busPhoneLE->setText( "" );
       	busFaxLE->setText( "" );
        busMobileLE->setText( "" );
	busPagerLE->setText( "" );
    }

    QMap<PimContact::ContactFields, QString> abNameMap;
    quitExplicitly = FALSE;
    ent = entry;

#if defined(QTOPIA_PHONE) && !defined(QTOPIA_DESKTOP)
    editTonePB->setTone( ent.customField("tone") ); // No tr()
#endif

    txtNote->setText( ent.notes() );

    cmbCat->setCategories( ent.categories(), "Address Book", // No tr()
	tr("Contacts") );
    Categories c;
    bool loadOK = c.load( categoryFileName() );
    bool busCat = FALSE;
    if( loadOK )
    {
	int bcatid = c.id( "addressbook", "_Business");
	if( bcatid )
	    busCat = ent.categories().contains( bcatid );
    }

    /*
       force an update to category select, even if the checkbox is already 
       checked the right way (wouldn't emit toggled() signal)
       saves wasting an extra Categories load when we don't need it
   */
    if( busCat == categoryCB->isChecked() ) 	
	catCheckBoxChanged( busCat ); 
    categoryCB->setChecked( busCat );

    switch ( ent.gender() ) 
    {
    case PimContact::UnspecifiedGender :
	genderCombo->setCurrentItem( 0 );
	break;
    case PimContact::Male:
	genderCombo->setCurrentItem( 1 );
	break;
    case PimContact::Female:
	genderCombo->setCurrentItem( 2 );
	break;
    }

    abNameMap.insert( PimContact::Nickname, ent.nickname() );
    abNameMap.insert( PimContact::NameTitle, ent.nameTitle() );
    abNameMap.insert( PimContact::FirstName, ent.firstName() );
    abNameMap.insert( PimContact::MiddleName, ent.middleName() );
    abNameMap.insert( PimContact::LastName, ent.lastName() );
    abNameMap.insert( PimContact::FirstNamePronunciation, 
						ent.firstNamePronunciation() );
    abNameMap.insert( PimContact::LastNamePronunciation, 
						ent.lastNamePronunciation() );
    abNameMap.insert( PimContact::Suffix, ent.suffix() );
    QString fa;
    if( ent.fileAs() != ent.fullName() )
	fa = ent.fileAs();
    abNameMap.insert( PimContact::FileAs, fa ); //set a null file as if it's the system default
    abName->setFields( abNameMap );
    abName->setModified( FALSE );
    
    QString strDefEmail = ent.defaultEmail();
    QStringList strAllEmail = ent.emailList();
    QString strFinal;
    parseEmailTo( strDefEmail, strAllEmail, strFinal );
    emailLE->setText( strFinal );
    // make sure we see the "default"
    emailLE->home( false );

    //voip
#ifdef QTOPIA_VOIP
    QString strVoipId = ent.customField("VOIP_ID");
    voipIdLE->setText( strVoipId );
    // make sure we see the "default"
    voipIdLE->home( false );
    
    if(newEntry)
        ent.setCustomField("VOIP_STATUS", "OFFLINE");
#endif
            
    bdayButton->setDate( ent.birthday() );

    //home
    spouseLE->setText( ent.spouse() );
    anniversaryButton->setDate( ent.anniversary() );
    childrenLE->setText( ent.children() );

    homeStreetME->setText( ent.homeStreet() );
    homeCityLE->setText( ent.homeCity() );
    homeStateLE->setText(  ent.homeState() );
    homeZipLE->setText( ent.homeZip() );
    homeCountryLE->setText( ent.homeCountry() );
    QString homePhone = ent.homePhone(), homeFax = ent.homeFax(), 
						    homeMobile = ent.homeMobile();
    phoneMan->add( homePhone, mHPType );
    phoneMan->add( homeMobile, mHMType );
    phoneMan->add( homeFax, mHFType );
    homeWebPageLE->setText( ent.homeWebpage() );

    // business
    busStreetME->setText( ent.businessStreet() );
    busCityLE->setText( ent.businessCity() );
    busStateLE->setText( ent.businessState() );
    busZipLE->setText( ent.businessZip() );
    busCountryLE->setText( ent.businessCountry() );
    specCompanyLE->setText( ent.company() );
    companyProLE->setText( ent.companyPronunciation() );
    busWebPageLE->setText( ent.businessWebpage() );
    specJobTitleLE->setText( ent.jobTitle() );
    deptLE->setText( ent.department() );
    officeLE->setText( ent.office() );

    QString busPhone = ent.businessPhone(), busFax = ent.businessFax(), 
	    busMobile = ent.businessMobile(), busPager = ent.businessPager();
    phoneMan->add( busPhone, mBPType );
    phoneMan->add( busMobile, mBMType );
    phoneMan->add( busFax, mBFType );
    phoneMan->add( busPager, mBPAType );

    professionLE->setText( ent.profession() );
    assistantLE->setText( ent.assistant() );
    managerLE->setText( ent.manager() );
    
#ifndef QTOPIA_DESKTOP // not supported on QD yet
    QString photoFile = ent.customField( "photofile" );
    if( !photoFile.isEmpty() )
    {
	QPixmap myPixmap;

	QString baseDirStr = Global::applicationFileName( "addressbook", 
								"contactimages/" );
	myPixmap.convertFromImage(Image::loadScaled(baseDirStr+photoFile, 80, 96));
	if (!myPixmap.isNull()) {
	    mContactImage = myPixmap;
	    photoPB->setPixmap( myPixmap );
	} else {
	    qDebug("AbEditor::setEntry - Unable to get image for contact");
	}
    }
    else
    {
	QPixmap e;
	mContactImage = e;
	photoPB->setPixmap( e );
    }
#endif

#ifdef QTOPIA_DATA_LINKING
    QDL::loadLinks( ent.customField( QDL::DATA_KEY ), QDL::clients( this ) );
#endif

    phoneManOld = TRUE;
    lastUpdateInternal = FALSE;

    mNewEntry = newEntry;

    tabs->setCurrentPage( 0 );
}

void AbEditor::closeEvent(QCloseEvent *e)
{
#ifdef QTOPIA_DESKTOP
    PimContact current;
    contactFromFields(current);
    if ( !quitExplicitly && ent.toRichText() != current.toRichText() 
	 && QMessageBox::warning(this, tr("Contacts"), 
				 tr("Discard changes?"),
				 QMessageBox::Yes, 
				 QMessageBox::No) == QMessageBox::No )

	e->ignore();
    else
	QDialog::closeEvent(e);
#else 
    QDialog::closeEvent(e);
#endif
    quitExplicitly = FALSE;
}

void AbEditor::accept()
{
    quitExplicitly = TRUE;
    if ( isEmpty() )
    {
        if(mNewEntry)
	   reject();
        else
           QDialog::accept();
    }
    else 
    {
	PimContact tmp( ent ); //preserve uid.
	contactFromFields( tmp );
        
#ifdef QTOPIA_PHONE
	//use tmp so that if the user says no to the following question, the original 
        // contact is unaffected
	if( !tmp.customField("SIM_CONTACT").isEmpty() && (AbUtil::numbersForSimCount(tmp) == 0) &&
            !hasNonSimFields() )
	{
	    // sim contact with no numbers for sim and no other data worth saving 
            int r = QMessageBox::warning( this, tr("Remove SIM Contact"), tr("<qt>Your changes to "
                        "the SIM contact '%1' will cause it to be removed entirely. "
			"Are you sure you want to do this?</qt>").arg(ent.fileAs()),
                        QMessageBox::Yes, QMessageBox::No | QMessageBox::Default );
	    if( r == QMessageBox::No )
		return;
	}
#endif
	ent = tmp; //now we can copy the fields over

#ifdef QTOPIA_DATA_LINKING
	QString links;
	QDL::saveLinks( links, QDL::clients( this ) );
	ent.setCustomField( QDL::DATA_KEY, links );
#endif

#ifndef QTOPIA_DESKTOP // not supported on QD yet
	//save the photo
	QString pFileName = ent.customField( "photofile" );
	QString baseDirStr = Global::applicationFileName( "addressbook", 
								"contactimages/" );
	mImageModified = !mContactImage.isNull() || !pFileName.isEmpty();
        ent.changePhoto( mContactImage, AbUtil::ImageWidth, AbUtil::ImageHeight );
#endif

	QDialog::accept();
    }
}

bool AbEditor::imageModified() const
{
    return mImageModified;
}

// creates a new contact image from pix
/*QString AbEditor::createContactImage( QPixmap pix )
{
    if( !pix.isNull() )
    {
	QString baseDirStr = Global::applicationFileName( "addressbook", 
								"contactimages/" );
	QDir baseDir( baseDirStr );
	if( !baseDir.exists() )
	    baseDir.mkdir( baseDirStr );

	QString uniqueFileName;
	uniqueFileName = "ci-";
	QString stamp; stamp.setNum( time(0) ); stamp += "-";
	uniqueFileName += stamp;
	QString fn;
	QFile photoFile;
	int ui = 0;
	do
	{
	    
	    fn = uniqueFileName + QString("").setNum( ui++ ) + ".jpg";
	    photoFile.setName( baseDirStr+fn );
	} while( photoFile.exists() ); 

	if( !pix.save( baseDirStr+fn, "JPEG", 50 ) )
	    qWarning(QString("AbEditor::createContactImage - Unable to save" 
						    " contact image '%1'").arg(fn));
	else
	    return fn;
    }
    return QString::null;
}*/

void AbEditor::reject()
{
    quitExplicitly = TRUE;    
    QDialog::reject();
}


bool AbEditor::isEmpty()
{
    if( !(abName->isEmpty()) )
	return FALSE;
    if( homeStreetME->text().stripWhiteSpace().length() ||
	busStreetME->text().stripWhiteSpace().length() )
	return FALSE;

    // analyze all the line edits and make sure there is _something_ there
    // that warrants saving...
    QString text;
    QMap<PimContact::ContactFields, QLineEdit *>::Iterator it;
    for ( it = lineEdits.begin() ; it != lineEdits.end() ; ++it ) 
    {
	text = it.data()->text();
	if ( !text.stripWhiteSpace().isEmpty() )
	    return FALSE;
    }

    if( !anniversaryButton->date().isNull() )
	return FALSE;
    if( !bdayButton->date().isNull() )
	return FALSE;
    if( !genderCombo->currentText().isEmpty() )
	return FALSE;
#if defined(QTOPIA_PHONE) && !defined(QTOPIA_DESKTOP)
    if( editTonePB->tone().isValid() )
	return FALSE;
#ifdef QTOPIA_VOIP
    text = voipIdLE->text();
    if ( !text.stripWhiteSpace().isEmpty() )
	return FALSE;
#endif
#endif
    text = txtNote->text();
    if ( !text.stripWhiteSpace().isEmpty() )
	return FALSE;
    if( !mContactImage.isNull() )
	return FALSE;

    return TRUE;
}

void AbEditor::contactFromFields(PimContact &e)
{
    int gender = genderCombo->currentItem();
    switch( gender ) {
    case 0: e.setGender( PimContact::UnspecifiedGender ); break;
    case 1: e.setGender( PimContact::Male ); break;
    case 2: e.setGender( PimContact::Female ); break;
    }

    e.setCustomField( "BUSINESS_CONTACT", categoryCB->isChecked() ? QString("1") : QString::null );

    QMap<PimContact::ContactFields, QString> fields;

    //Contact Tab
    if( abName->modified() )
    abName->parse();
    fields = abName->fields();
    e.setNameTitle( fields[ PimContact::NameTitle ] );
    e.setSuffix( fields[ PimContact::Suffix ]  );
    e.setFirstName( fields[ PimContact::FirstName ] );
    e.setMiddleName( fields[ PimContact::MiddleName ] );
    e.setLastName( fields[ PimContact::LastName ] );
    e.setFirstNamePronunciation( fields[ PimContact::FirstNamePronunciation ] );
    e.setLastNamePronunciation( fields[ PimContact::LastNamePronunciation ] );
    e.setBirthday( bdayButton->date() );

    e.setCategories( cmbCat->currentCategories() );

    e.setNickname( fields[ PimContact::Nickname ] );

    QString strDefaultEmail;
    QStringList emails;
    parseEmailFrom( emailLE->text(), strDefaultEmail,
	    emails );
    e.setDefaultEmail( strDefaultEmail );
    e.setEmailList( emails  );

    //voip
#ifdef QTOPIA_VOIP
    QString str = e.customField("VOIP_ID");
    QString voipStr = voipIdLE->text();
    
    if (voipStr.isEmpty() && !str.isEmpty())
          e.removeCustomField("VOIP_ID");

    if (!voipStr.isEmpty())
          e.setCustomField( "VOIP_ID", voipStr);
#endif

    // Home Tab
    e.setAnniversary( anniversaryButton->date() );
    e.setHomeStreet( homeStreetME->text() );
    e.setHomeCity( homeCityLE->text() );
    e.setHomeState( homeStateLE->text() );
    e.setHomeZip( homeZipLE->text() );
    e.setHomeCountry( homeCountryLE->text() );
    e.setHomePhone( phoneMan->numberFromType( mHPType ) );
    e.setHomeMobile( phoneMan->numberFromType( mHMType ) );
    e.setHomeFax( phoneMan->numberFromType( mHFType ) );
    e.setHomeWebpage( homeWebPageLE->text() );
    e.setSpouse( spouseLE->text() );
    e.setChildren( childrenLE->text() );

    //Business Tab
    e.setCompany( companyLE->text() );
    e.setCompanyPronunciation( companyProLE->text() );
    e.setBusinessStreet( busStreetME->text() );
    e.setBusinessCity( busCityLE->text() );
    e.setBusinessState( busStateLE->text() );
    e.setBusinessZip( busZipLE->text() );
    e.setBusinessCountry( busCountryLE->text() );
    e.setBusinessWebpage( busWebPageLE->text() );
    e.setJobTitle( jobTitleLE->text() );
    e.setDepartment( deptLE->text() );
    e.setOffice( officeLE->text() );
    e.setBusinessPhone( phoneMan->numberFromType( mBPType ) );
    e.setBusinessMobile( phoneMan->numberFromType( mBMType ) );
    e.setBusinessFax( phoneMan->numberFromType( mBFType ) );
    e.setBusinessPager( phoneMan->numberFromType( mBPAType ) );
    e.setProfession( professionLE->text() );
    e.setAssistant( assistantLE->text() );
    e.setManager( managerLE->text() );

    e.setNotes( txtNote->text() );
#if defined(QTOPIA_PHONE) && !defined(QTOPIA_DESKTOP)
    if ( !editTonePB->tone().isValid() )
	e.removeCustomField("tone"); // No tr()
    else
	e.setCustomField("tone", editTonePB->tone().linkFile() ); // No tr()
#endif

    QString fa = fields[ PimContact::FileAs ];
    if( fa.isEmpty() )
	fa = e.fullName();
    e.setFileAs( fa );

#ifdef QTOPIA_PHONE
    //sim flags
    QString val;
    if( mHaveSim )
    {
	for( QMap<int,QAction *>::Iterator it = simActions.begin() ; it != simActions.end() ;
		++it )
	{
	    if( (*it)->isOn() )
		val = "1";
	    else
		val = QString::null;
	    e.setCustomField( AbUtil::simKeyForPhoneType( it.key() ), val );
	}
    }
    // else sim isn't enabled, don't change current values 
#endif
}

void AbEditor::showEvent( QShowEvent *e )
{
    QDialog::showEvent( e );
    if( mNewEntry )
	abName->setFocus();
}

void AbEditor::setNameFocus()
{
    /*
       this doesn't actually work because when 
       show() is called, the tabwidget gets focus
   */
    tabs->showPage( contactTab );
    abName->setFocus();
}

void parseEmailFrom( const QString &txt, QString &strDefaultEmail,
		     QStringList &all )
{
    //qDebug( "parseEmailFrom <%s> def <%s>", txt.latin1(), strDefaultEmail.latin1() );
    all.clear();
    strDefaultEmail = "";

    QString emailStr = txt.stripWhiteSpace();
    if ( emailStr.contains(',') ||
	 emailStr.contains(';') || emailStr.contains(' ') ) {
	all = QStringList::split( QRegExp( "[,; ]" ), emailStr );
    }
    else
	all = emailStr;
    strDefaultEmail = all.first();
}

void parseEmailTo( const QString &strDefaultEmail,
		   const QStringList &allEmails, QString &strBack )
{
    //qDebug("parseEmailTo default email = %s all emails = %s",strDefaultEmail.latin1(), allEmails.join(", ").latin1() );
    if ( strDefaultEmail.isEmpty() && !allEmails.count() ) {
	strBack = "";
	return;
    }

    if ( strDefaultEmail.isEmpty() ) {
	strBack = allEmails.join(", ");
	return ;
    }

    if ( !strDefaultEmail.isEmpty() && strDefaultEmail == allEmails.join( " " ) ) {
	//handle imported single addresses with spaces in them:
	strBack = strDefaultEmail;
	return; 
    }

    QStringList emails = allEmails;
    // make sure the default email is first

    QStringList::Iterator defaultIt = emails.find( strDefaultEmail );
    if ( defaultIt == emails.end() ) {
	qWarning("AbEditor::parseEmailTo default email is not found in the email list; bug!");
	strBack = strDefaultEmail;
	strBack += emails.join(", ");
	return;
    }

    if ( defaultIt == emails.begin() ) {
	strBack = emails.join(", ");
	return;
    }

    // reorder the emails to put default first
    emails.remove( defaultIt );
    emails.prepend( strDefaultEmail );

    strBack = emails.join(", ");
}

#ifdef QTOPIA_DESKTOP
void AbEditor::updateCategories()
{
    if ( !cmbCat )
	return;

    connect( this, SIGNAL( categoriesChanged() ),
	     cmbCat, SLOT( categoriesChanged() ) );
    emit categoriesChanged();
    disconnect( this, SIGNAL( categoriesChanged() ),
		cmbCat, SLOT( categoriesChanged() ) );
}
#endif

#ifdef QTOPIA_PHONE

//check for any data that isn't to do with the sim
// FIXME : this is almost just the inverse of AbEditor::isEmpty
bool AbEditor::hasNonSimFields() const
{
    const QString simKeyBase = "SIM_";
    QString text;
    QMap<PimContact::ContactFields, QLineEdit *>::ConstIterator it;
    for ( it = lineEdits.begin() ; it != lineEdits.end() ; ++it ) 
    {
	QString simKeyType;
	switch( it.key() )
	{
	    case PimContact::HomePhone:
		simKeyType = "HP";
		break;
	    case PimContact::HomeFax:
		simKeyType = "HF";
		break;
	    case PimContact::HomeMobile:
		simKeyType = "HM";
		break;
	    case PimContact::BusinessPhone:
		simKeyType = "BP";
		break;
	    case PimContact::BusinessFax:
		simKeyType = "BF";
		break;
	    case PimContact::BusinessMobile:
		simKeyType = "BM";
		break;
	    case PimContact::BusinessPager:
		simKeyType = "BPA";
		break;
            default:
                break;
	}
	if( !simKeyType.isEmpty() && 
	    !simActions[AbUtil::simTypeTextToPhoneType(simKeyType)]->isOn() && 
	    !it.data()->text().isEmpty() )
	{
	    //have a phone field that isn't on the sim and has a value
	    return TRUE;
	}
	else if( !simKeyType.isEmpty() )
	{
	    continue;
	}

	text = it.data()->text();
	if ( !text.stripWhiteSpace().isEmpty() )
	{
	    return TRUE;
	}
    }

    if( !mContactImage.isNull() )
	return TRUE;

#if defined(QTOPIA_PHONE) && !defined(QTOPIA_DESKTOP)
    if( editTonePB->tone().isValid() )
	return TRUE;
#endif

    if( !anniversaryButton->date().isNull() )
    {
	return TRUE;
    }
    if( !bdayButton->date().isNull() )
    {
	return TRUE;
    }
    if( !genderCombo->currentText().isEmpty() )
    {
	return TRUE;
    }

    text = txtNote->text();
    if ( !text.stripWhiteSpace().isEmpty() )
    {
	return TRUE;
    }

    return FALSE;
}

#endif
