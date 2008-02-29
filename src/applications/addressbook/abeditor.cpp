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


#include <qtopia/pim/qcontactmodel.h>

#include "abeditor.h"
#include "addresspicker.h"
#include "imagesourcedialog.h"
#include "addressbook.h" // for AB_PDA

#include <qcategorymanager.h> // needed to get correct WIN32 exports for QList<int>
#include <qcategoryselector.h>
#include <qtopiaapplication.h>

#include <qthumbnail.h>

#include <time.h>
#include <stdio.h>

#include <QDebug>
#include <QTextStream>
#include <QKeyEvent>
#include <QTextEdit>
#include <QTabWidget>
#include <QTextStream>
#include <QComboBox>
#include <QCheckBox>
#include <QLabel>
#include <QRadioButton>
#include <QStyle>
#include <QAction>
#include <QLayout>
#include <QLineEdit>
#include <QToolButton>
#include <QDateTimeEdit>
#include <QGroupBox>
#include <QPushButton>
#include <QMainWindow>
#include <QRegExp>
#include <QWhatsThis>
#include <QMessageBox>
#include <QDialog>
#include <QList>
#include <QString>
#include <QButtonGroup>
#include <QColor>
#include <QFile>
#include <QFileInfo>
#include <QWMatrix>
#include <QDir>
#include <QScrollArea>
#include <QMenu>

#include <QDL>
#include <QDLEditClient>

#ifdef QTOPIA_PHONE
#   include <qtopia/qsoftmenubar.h>
#   include <qtopia/qdocumentselector.h>
#   include "emaildialogphone.h"
#else
#   include "emaildlgimpl.h"
#endif

#include <qiconselector.h>



// helper functions, convert our comma delimited list to proper
// file format...
void parseEmailFrom( const QString &txt, QString &strDefaultEmail,
                     QStringList &strAll );

// helper convert from file format to comma delimited...
void parseEmailTo( const QString &strDefaultEmail,
                   const QStringList &emails, QString &strBack );



//-----------------------------------------------------------------------

PhoneFieldType::PhoneFieldType()
{
}

PhoneFieldType::PhoneFieldType( const QString& type_id, const QString &str )
{
    id = type_id;
    icon = QIcon(":icon/"+id);
    name = str;
}

PhoneFieldType::PhoneFieldType( const PhoneFieldType &other )
{
    *this = other;
}

PhoneFieldType &PhoneFieldType::operator=( const PhoneFieldType &other )
{
    id = other.id;
    icon = other.icon;
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

//-----------------------------------------------------------------------

PhoneFieldManager::PhoneFieldManager( QWidget *parent, QGridLayout *layout, int rc )
    : QObject( parent ), parLayout(layout), rowCount(rc), firstRow(rc)
{
    setObjectName("phoneFieldManager");
    mEmitFieldChanged = true;
}

PhoneFieldManager::~PhoneFieldManager()
{
    phoneFields.clear();
}

bool PhoneFieldManager::isFull() const
{
    bool full = false;
    if ( phoneFields.count() && (phoneFields.count() >= mTypes.count()) ) {
        // if there are no blank fields, we're full
        full = true;
        QListIterator<PhoneField*> it(phoneFields);
        while(it.hasNext()) {
            PhoneField *f = it.next();
            if( f->isEmpty() ) {
                full = false;
                break;
            }
        }

    }
    return full;
}

void PhoneFieldManager::add( const QString &number, const PhoneFieldType &type )
{
    if(number.isEmpty() || isFull())
        return;

    PhoneField *eField = 0;
    QListIterator<PhoneField *> it(phoneFields);
    while(it.hasNext())
    {
        PhoneField *f = it.next();
        if( f->type() == type )
        {
            eField = f;
            break;
        }
    }

    if( eField )
        eField->setNumber( number );
    else
    {
        // masks earlier it and f.
        QListIterator<PhoneField *> it(phoneFields);
        while(it.hasNext())
        {
            PhoneField *f = it.next();
            if( f->isEmpty() )
            {
                f->setType( type );
                f->setNumber( number );
                break;
            }
        }
    }
}

void PhoneFieldManager::addEmpty()
{
    if( isFull() )
        return;

    QList<PhoneFieldType> availTypes = mTypes;

    QListIterator<PhoneField *> it(phoneFields);

    while(it.hasNext())
    {
        PhoneField *f = it.next();
        availTypes.removeAll( f->type() );
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
}

void PhoneFieldManager::clear()
{
    while(phoneFields.count() > 0)
    {
        PhoneField *f = phoneFields.takeAt(0);
        // have to be separate as together will
        // double delete on object deletion,
        // field does not own the widgets deleted
        // in the remove function
        f->remove(); // remove from widget();
        delete f; // stop poluting.
    }

    rowCount = firstRow;

    addEmpty();
}

void PhoneFieldManager::emitFieldChanged( const QString &str, const PhoneFieldType &type )
{
    if( mEmitFieldChanged )
        emit fieldChanged( str, type );
}

void PhoneFieldManager::setTypes( const QList<PhoneFieldType> &newTypes )
{
    if( phoneFields.count() > newTypes.count() )
    {
        qWarning("PhoneFieldManager::setTypes: phoneFields.count() > newTypes.count()\n");
        return;
    }

    mTypes = newTypes;

    QList<PhoneFieldType> availTypes = newTypes;

    QListIterator<PhoneField *> it(phoneFields);
    while(it.hasNext())
    {
        PhoneField *f = it.next();
        f->setTypes(mTypes);
        if( !availTypes.contains(f->type()))
        {
            if(!availTypes.count())
                qWarning("PhoneFieldManager::setTypes: Not enough types for phone fields.");

            PhoneFieldType nt = availTypes.first();
            f->setType(nt);
            availTypes.removeAll(nt);
        }
    }
}

QList<PhoneFieldType> PhoneFieldManager::types() const
{
    return mTypes;
}

void PhoneFieldManager::updateTypes( const PhoneFieldType &newType )
{
    const QObject *s = sender(); // the phone field whose type has changed

    QList<PhoneFieldType> availTypes = mTypes;
    QList<PhoneField *> changedFields;

    foreach(PhoneField *r, phoneFields)
        availTypes.removeAll(r->type());

    mEmitFieldChanged = false;
    QListIterator<PhoneField *> it(phoneFields);
    while(it.hasNext())
    {
        PhoneField *f = it.next();
        if( (QObject *)f == s ) continue;

        if( f->type() == newType )
        {
            if( !availTypes.count() )
                qWarning("PhoneFieldManager::updateTypes: Not enough types for phone fields.");

            PhoneFieldType nt = availTypes.first();
            f->setType( nt );
            availTypes.removeAll( nt );
            changedFields.append( f );
        }
    }
    mEmitFieldChanged = true;
    //emit field changed signals only after we've rearranged

    foreach (PhoneField *cit, changedFields)
        emitFieldChanged( cit->number(), cit->type() );
}

void PhoneFieldManager::checkForAdd()
{
    QListIterator<PhoneField *> it(phoneFields);
    while(it.hasNext())
    {
        PhoneField *f = it.next();
        if( f->isEmpty() )
            return;
    }
    addEmpty();
}

void PhoneFieldManager::setNumberFromType(const PhoneFieldType &type, const QString &newNumber)
{
    QListIterator<PhoneField *> it(phoneFields);
    while(it.hasNext())
    {
        PhoneField *f = it.next();
        if( f->type() == type )
        {
            f->setNumber(newNumber);
            break;
        }
    }
    //no fields matching, add a field
    add( newNumber, type );
}

QString PhoneFieldManager::numberFromType( const PhoneFieldType &type )
{
    QListIterator<PhoneField *> it(phoneFields);
    while(it.hasNext())
    {
        PhoneField *f = it.next();
        if( f->type() == type )
            return f->number();
    }
    return QString();
}

PhoneFieldLineEdit::PhoneFieldLineEdit( QWidget *typeSibling, QWidget *parent )
    : QLineEdit( parent ), mTypeSibling( typeSibling )
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
            return true;
        }
    }
    return false;
}

void PhoneFieldLineEdit::appendText( const QString &txt )
{
#ifdef QTOPIA_PHONE
    if( !Qtopia::mousePreferred() )
    {
        if( !hasEditFocus() )
            setEditFocus( true );
    }
#endif
    setText( text()+txt );
}

//-----------------------------------------------------------------------

PhoneField::PhoneField( QGridLayout *l, int &rowCount, QWidget *parent )
    : QObject( parent )
{
    typeIS = new QIconSelector(parent);

    connect( typeIS, SIGNAL(activated(int)), this, SLOT(userChangedType(int)) );
    numberLE = new PhoneFieldLineEdit( typeIS, parent );
    connect( numberLE, SIGNAL(textChanged(const QString&)),
        this, SIGNAL(numberChanged(const QString&)) );
    QtopiaApplication::setInputMethodHint(numberLE,QtopiaApplication::PhoneNumber);

    l->addWidget( typeIS, rowCount, 0, Qt::AlignHCenter );
    l->addWidget( numberLE, rowCount, 2 );
    ++rowCount;

    parent->setTabOrder(typeIS, numberLE);

    numberLE->show();
    typeIS->show();

    connect( this, SIGNAL(numberChanged(const QString&)), this, SLOT(emitFieldChanged()) );
    connect( this, SIGNAL(internalChangedType(const PhoneFieldType&)),
        this, SIGNAL(typeChanged(const PhoneFieldType&)) );
    connect( this, SIGNAL(userChangedType(const PhoneFieldType&)),
        this, SIGNAL(typeChanged(const PhoneFieldType&)) );
    connect( this, SIGNAL(typeChanged(const PhoneFieldType&)), this, SLOT(emitFieldChanged()) );
}

PhoneField::~PhoneField()
{
}

void PhoneField::remove()
{
    delete typeIS;
    delete numberLE;
}

void PhoneField::userChangedType( int /* idx */)
{
    emit userChangedType( mTypes[typeIS->currentIndex()] );
}

void PhoneField::setTypes( const QList<PhoneFieldType> &types )
{
    typeIS->clear();
    bool first = true;
    mTypes = types;
    QList<PhoneFieldType>::ConstIterator it;
    for( it = mTypes.begin() ; it != mTypes.end() ; ++it )
    {
        QIcon icon = (*it).icon;
        typeIS->insertItem( icon, (*it).name );
        if(first)
            typeIS->setIcon(icon);
        first = false;
    }



    emit internalChangedType( mTypes[typeIS->currentIndex()] );
}

void PhoneField::setType( const PhoneFieldType &newType )
{
    PhoneFieldType t(newType);
    int idx = mTypes.indexOf( t );
    if( idx != -1 )
        typeIS->setCurrentIndex( idx );

}

PhoneFieldType PhoneField::type()
{
    return mTypes[typeIS->currentIndex()];
}

QString PhoneField::number() const
{
    return numberLE->text();
}

bool PhoneField::isEmpty() const
{
    return number().trimmed().isEmpty();
}

void PhoneField::setNumber( const QString &newNumber )
{
    numberLE->setText( newNumber );
}

void PhoneField::emitFieldChanged()
{
    emit fieldChanged( numberLE->text(), mTypes[typeIS->currentIndex()] );
}

//-----------------------------------------------------------------------

static void adjustPronWidgets(QLabel* label, QLineEdit* le)
{
    label->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    QFont sm = label->font();

    sm.setPointSize(sm.pointSize()/2);
    le->setFont(sm);

    label->setFont(sm);
}

//-----------------------------------------------------------------------

//
//FIXME : This is crap iterative widget creation that has survived from the rewrite.
//now that address parsing has gone, make this into AbNameDetails dialog
//

AbDetailEditor::AbDetailEditor( QWidget *parent, Qt::WFlags fl )
    : QDialog( parent, fl ), suffixCombo(0), titleCombo(0)
{
    mView = new QScrollArea( this );
    mView->setFocusPolicy(Qt::NoFocus);
    mView->setFrameStyle(QFrame::NoFrame);
    mView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mView->viewport()->installEventFilter( this );
    editorLayout = new QVBoxLayout( this );
    editorLayout->addWidget( mView );
}

AbDetailEditor::~AbDetailEditor()
{
}

void AbDetailEditor::accept()
{
    QMap<QContactModel::Field, QString> newFields;
    for ( QMap<QContactModel::Field, QString>::Iterator it = myFields.begin();
          it != myFields.end(); ++it )
    {
        switch ( it.key() )
        {
            case QContactModel::Suffix:
                    myFields[QContactModel::Suffix]  = suffixCombo->currentText();
                break;
            case QContactModel::NameTitle:
                    myFields[QContactModel::NameTitle] = titleCombo->currentText();
                break;
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
QList<QContactModel::Field> AbDetailEditor::guiList( const QMap<QContactModel::Field, QString> &f ) const
{
    QList<QContactModel::Field> gvl;
    if( f.contains( QContactModel::NameTitle ) ) // editing a name field
    {
        gvl += QContactModel::NameTitle;
        gvl += QContactModel::FirstName;
        gvl += QContactModel::FirstNamePronunciation;
        gvl += QContactModel::MiddleName;
        gvl += QContactModel::LastName;
        gvl += QContactModel::LastNamePronunciation;
        gvl += QContactModel::Suffix;
        gvl += QContactModel::Nickname;
    }

    return gvl;
}

bool AbDetailEditor::eventFilter( QObject *receiver, QEvent *event )
{
    //  Force the contents of the scroll area to conform to the width of the scroll area's viewport.
    if( mView && receiver == mView->viewport() && mView->widget() && event->type() == QEvent::Resize )
        mView->widget()->setFixedWidth( mView->viewport()->width() );

    return false;
}

void AbDetailEditor::setFields( const QMap<QContactModel::Field, QString> &f )
{
    // FIXME : This loop is pointless - make AbDetailEditor an
    // adt and derive address and name editors from that
    myFields = f;
    QWidget *container = new QWidget;
    mView->setWidget(container);
    mView->setWidgetResizable(true);
    container->setFixedWidth( mView->viewport()->width() );

    const QMap<QContactModel::Field, QString> dn = displayNames();
    QGridLayout *gl = new QGridLayout( container ); //, myFields.count()+1, 3, 3, 0 );
    gl->addItem(new QSpacerItem(4, 0), 0, 1);

    QWidget *lastEditor = 0, *editor;
    int fieldInTabNum=0;

    //Map may differ in ordering to what we want ; get a value list for the fields we're editing
    QList<QContactModel::Field> g = guiList( f );

    for ( QList<QContactModel::Field>::ConstIterator fieldKey = g.begin() ;
          fieldKey != g.end() ; ++fieldKey ) {

        QLabel *label = new QLabel( dn[ *fieldKey ], container );
        label->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

        // do a switch on the field type; most are line edits,
        // the exceptions get a case, the default is a linedit
        editor = 0;
        int cLen = 0;
        int i;
        switch ( *fieldKey )
        {
        case QContactModel::Suffix:
            suffixCombo = new QComboBox( container );
            suffixCombo->setEditable( false );
            suffixCombo->setDuplicatesEnabled( false );
            suffixCombo->setAutoCompletion( true );

            suffixCombo->insertItem( 0, "" );
            suffixCombo->addItems( QContactModel::localeSuffixes() );

            cLen = suffixCombo->count();
            for( i = 0; i < cLen ; ++i )
            {
                if( myFields[ *fieldKey ]  == suffixCombo->itemText( i ) )
                {
                    suffixCombo->setCurrentIndex( i );
                    break;
                }
            }

            editor = suffixCombo;
            break;

        case QContactModel::NameTitle:
            titleCombo = new QComboBox( container );
            titleCombo->setEditable( false );
            titleCombo->setDuplicatesEnabled( false );
            titleCombo->setAutoCompletion( true );

            titleCombo->insertItem( 0, "" );
            titleCombo->addItems( QContactModel::localeNameTitles() );

            cLen = titleCombo->count();
            for( i = 0; i < cLen ; ++i )
            {
                if( myFields[ *fieldKey ] == titleCombo->itemText( i ) )
                {
                    titleCombo->setCurrentIndex( i );
                    break;
                }
            }

            editor = titleCombo;
            break;
        default:
            {
                //FIXME : ugly temporary solution
                QLineEdit *lineedit = new QLineEdit( container );

                if ( *fieldKey == QContactModel::FirstNamePronunciation
                  || *fieldKey == QContactModel::LastNamePronunciation )
                    adjustPronWidgets(label,lineedit);
                else if( *fieldKey == QContactModel::BusinessZip ||
                         *fieldKey == QContactModel::HomeZip )
                    QtopiaApplication::setInputMethodHint( lineedit, QtopiaApplication::Number );
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
            if ( lastEditor )
            {
                setTabOrder( lastEditor, editor );
            }
            lastEditor = editor;

            // increment the field number for this tab
            fieldInTabNum++;
        }
        else
        {
            delete label;
            label = 0;
        }
    }
    QSpacerItem *verticleSpacer = new QSpacerItem( 1, 1, QSizePolicy::Minimum,
                                                   QSizePolicy::Expanding );
    gl->addItem( verticleSpacer, fieldInTabNum, 2 );
}

const QMap<QContactModel::Field, QString> AbDetailEditor::displayNames() const
{
    QMap<QContactModel::Field, QString> shortNames;

    //names for name fields
    shortNames[QContactModel::NameTitle] = QContactModel::fieldLabel(QContactModel::NameTitle);
    shortNames[QContactModel::FirstName] = QContactModel::fieldLabel(QContactModel::FirstName);
    shortNames[QContactModel::FirstNamePronunciation] = QContactModel::fieldLabel(QContactModel::FirstNamePronunciation);
    shortNames[QContactModel::MiddleName] = QContactModel::fieldLabel(QContactModel::MiddleName);
    shortNames[QContactModel::LastName] = QContactModel::fieldLabel(QContactModel::LastName);
    shortNames[QContactModel::LastNamePronunciation] = QContactModel::fieldLabel(QContactModel::LastNamePronunciation);
    shortNames[QContactModel::Suffix] = QContactModel::fieldLabel(QContactModel::Suffix);
    shortNames[QContactModel::Nickname] = QContactModel::fieldLabel(QContactModel::Nickname);

    QString street = tr("Street"),
            city = tr("City"),
            state = tr("State"),
            zip = tr("Zip"),
            country = tr("Country")
            ;

    shortNames[QContactModel::BusinessStreet] = street;
    shortNames[QContactModel::BusinessCity] = city;
    shortNames[QContactModel::BusinessState] = state;
    shortNames[QContactModel::BusinessZip] = zip;
    shortNames[QContactModel::BusinessCountry] = country;

    shortNames[QContactModel::HomeStreet] = street;
    shortNames[QContactModel::HomeCity] = city;
    shortNames[QContactModel::HomeState] = state;
    shortNames[QContactModel::HomeZip] = zip;
    shortNames[QContactModel::HomeCountry] = country;

    return shortNames;
}

QMap<QContactModel::Field, QString> AbDetailEditor::fields() const
{
    return myFields;
}

//------------------------------------------------------------------------

AbstractField::AbstractField( QWidget *parent )
    : QLineEdit( parent ), mModified( false ), detailEditor(0)
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

void AbstractField::setFields( const QMap<QContactModel::Field, QString> &f )
{
    myFields = f;
    fieldsChanged();
}

QMap<QContactModel::Field, QString> AbstractField::fields() const
{
    return myFields;
}

bool AbstractField::isEmpty() const
{
    for( QMap<QContactModel::Field, QString>::ConstIterator it = myFields.begin() ; it != myFields.end() ; ++it )
    {
        QString str = it.value().trimmed();
        if( !str.isEmpty() )
            return false;
    }
    return true;
}

void AbstractField::details()
{
    parse();

    detailEditor = new AbDetailEditor( (QWidget *)parent() );
    detailEditor->setObjectName( "detailEditor" );
    detailEditor->setWindowTitle( tr("Edit %1").arg( fieldName() ) );

    detailEditor->setFields(fields());

    if( QtopiaApplication::execDialog( detailEditor ) )
        setFields(detailEditor->fields());

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
            QChar quote = in[i];
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
    return tokens;
}

//------------------------------------------------------------------------

AbstractName::AbstractName( QWidget *parent )
    : AbstractField( parent ), m_preventModified(false)
{
#ifdef QTOPIA_PHONE
    QMenu *contextMenu;

    contextMenu = QSoftMenuBar::menuFor( this, QSoftMenuBar::EditFocus );
    contextMenu->addSeparator();
    contextMenu->addAction( QIcon(":icon/details"), tr("Details"), this, SLOT(details()) );

    contextMenu = QSoftMenuBar::menuFor( this, QSoftMenuBar::NavigationFocus );
    contextMenu->addSeparator();
    contextMenu->addAction( QIcon(":icon/details"), tr("Details"), this, SLOT(details()) );
#endif

    connect( this, SIGNAL(textChanged(const QString&)), this, SLOT(textChanged()) );
}

AbstractName::~AbstractName()
{
}

bool AbstractName::isEmpty() const
{
    return text().trimmed().isEmpty();
}

QString AbstractName::fieldName() const
{
    return tr("Name");
}

void AbstractName::parse()
{
    if(!isModified()) return;

    QContact c = QContact::parseLabel(text());

//    QString prefix, suffix, firstName, nickName, middleName, lastName, firstNamePro, lastNamePro;
    QMap<QContactModel::Field, QString> newFields;
    newFields[QContactModel::NameTitle] = c.nameTitle();
    newFields[QContactModel::FirstName] = c.firstName();
    newFields[QContactModel::FirstNamePronunciation] = c.firstNamePronunciation();
    newFields[QContactModel::MiddleName] = c.middleName();
    newFields[QContactModel::LastName] = c.lastName();
    newFields[QContactModel::LastNamePronunciation] = c.lastNamePronunciation();
    newFields[QContactModel::Suffix] = c.suffix();
    newFields[QContactModel::Nickname] = c.nickname();

    myFields = newFields;
}

void AbstractName::fieldsChanged()
{
    bool firstNameEmpty = myFields[QContactModel::FirstName].isEmpty(),
         lastNameEmpty = myFields[QContactModel::LastName].isEmpty(),
         nickEmpty = myFields[QContactModel::Nickname].isEmpty(),
         firstProEmpty = myFields[QContactModel::FirstNamePronunciation].isEmpty(),
         lastProEmpty = myFields[QContactModel::LastNamePronunciation].isEmpty();

    QString displayString;
    QTextStream os(&displayString);
    if( firstNameEmpty )
    {
        os << myFields[QContactModel::NameTitle] << " " << myFields[QContactModel::LastName] << " " <<
              myFields[QContactModel::Suffix] << " ";
        if( !lastProEmpty )
            os << "(" << myFields[QContactModel::LastNamePronunciation] << ")";
    }
    else if( lastNameEmpty )
    {
        os << myFields[QContactModel::FirstName] << " " << myFields[QContactModel::Suffix] << " ";
        if( !firstProEmpty )
            os << "(" << myFields[QContactModel::FirstNamePronunciation] << ")";
    }
    else
    {
        os << myFields[QContactModel::NameTitle] << " " << myFields[QContactModel::FirstName] << " ";
        if( !nickEmpty )
            os << "\"" <<  myFields[QContactModel::Nickname] << "\" ";
        os << myFields[QContactModel::MiddleName] << " " <<
              myFields[QContactModel::LastName] << " " << myFields[QContactModel::Suffix] << " ";
        if( !firstProEmpty && !lastProEmpty )
        {
            os << "(" << myFields[QContactModel::FirstNamePronunciation] << " "
               << myFields[QContactModel::LastNamePronunciation] << ")";

        }
    }

    displayString = displayString.simplified();
    m_preventModified = true;
    setText( displayString );
    m_preventModified = false;
}

void AbstractName::textChanged()
{
    if(!m_preventModified)
        mModified = true;
}

//----------------------------------------------------------------------

AbEditor::AbEditor(QWidget *parent, Qt::WFlags fl)
    : QDialog(parent, fl),
#ifdef QTOPIA_PHONE
    simName(0),
#endif
    abName(0)
{
    setObjectName("edit");

    setWindowState(windowState() | Qt::WindowMaximized);

    mImageModified = false;
    mNewEntry = false;
    phoneMan = 0;
    specCompanyLA = 0;
    lastUpdateInternal = false;
    mainUIInit = false;
#ifdef QTOPIA_PHONE
    simUIInit = false;
#endif
    quitExplicitly = false;
    tabOtherInit = false;
    init();
}

AbEditor::~AbEditor()
{
}

void AbEditor::init()
{
    //
    // NOTE : If you change these, you also need to change
    // AbEditor::phoneFieldsToDetailsFilter() and
    // AbEditor::detailsToPhoneFieldsFilter()
    //

    mHPType = PhoneFieldType("homephone", tr("Home Phone"));
    mHMType = PhoneFieldType("homemobile", tr("Home Mobile"));
    mHFType = PhoneFieldType("homefax", tr("Home Fax"));
    mBPType = PhoneFieldType("businessphone", tr("Business Phone"));
    mBMType = PhoneFieldType("businessmobile", tr("Business Mobile"));
    mBFType = PhoneFieldType("businessfax", tr("Business Fax"));
    mBPAType = PhoneFieldType("businesspager", tr("Business Pager"));

    phoneTypes.append(mHPType);
    phoneTypes.append(mHMType);
    phoneTypes.append(mHFType);
    phoneTypes.append(mBPType);
    phoneTypes.append(mBMType);
    phoneTypes.append(mBFType);
    phoneTypes.append(mBPAType);

    mainVBox = new QVBoxLayout(this);
    mainVBox->setSpacing(0);
    mainVBox->setMargin(0);

#ifdef QTOPIA_PHONE
    editingSim = false;
#endif
}

#ifdef QTOPIA_PHONE
void AbEditor::initSimUI()
{
    if(mainUIInit)
        tabs->hide();

    editingSim = true;

    if(simUIInit)
        return;

    simUIInit = true;

    simEditor = new QWidget(0);
    mainVBox->addWidget(simEditor);

    QGridLayout *gridLayout = new QGridLayout(simEditor);

    gridLayout->addItem(new QSpacerItem(4, 0), 0, 1);
    gridLayout->setSpacing(0);
    gridLayout->setMargin(2);

    int rowCount = 0;

    //
    //  Name
    //

    QLabel *label = new QLabel(tr("Name"));
    label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    gridLayout->addWidget(label, rowCount, 0);

    simName = new QLineEdit(0);
    gridLayout->addWidget(simName, rowCount, 2);

    rowCount++;

    //
    //  Phone number
    //

    label = new QLabel(tr("Number"));
    label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    gridLayout->addWidget(label, rowCount, 0);

    simNumber = new QLineEdit(0);
    gridLayout->addWidget(simNumber, rowCount, 2);

    rowCount++;
    gridLayout->addItem(new QSpacerItem(4, 0), rowCount, 1);
}
#endif

void AbEditor::initMainUI()
{
#if QTOPIA_PHONE
    if(simUIInit)
        simEditor->hide();

    editingSim = false;
#endif

    if(mainUIInit)
        return;

    mainUIInit = true;

    setupTabs();

    //
    // create a map of QLineEdit pointers for iterating through
    // used in AbEditor::isEmpty()
    //

    buildLineEditList();

    spouseLE->setWhatsThis(tr("e.g. Husband or Wife."));
    professionLE->setWhatsThis(tr("Occupation or job description."));

}

void AbEditor::tabClicked( QWidget * )
{
}

void AbEditor::editEmails()
{
    QString strDefaultEmail;
    QStringList emails;
    EmailDialog *ed;
    QString str;

    ed = new EmailDialog(this);
    ed->setModal(true);

    parseEmailFrom(emailLE->text(), strDefaultEmail, emails);
    ed->setEmails( strDefaultEmail, emails );

    if (QtopiaApplication::execDialog(ed)) {
        ent.setDefaultEmail(ed->defaultEmail());
        ent.setEmailList(ed->emails());

        parseEmailTo( ed->defaultEmail(), ed->emails(), str );

        emailLE->setText( str );
        emailLE->home( false );
    }

    delete ed;
}

void AbEditor::tabChanged(int tab)
{
    switch (tab) {
    case 1:
        // This delays the cost of layout until we actually want to see the tab
        businessTab->setWidget(wBusinessTab);
        businessTab->viewport()->setAutoFillBackground(false); // transparent window color
        wBusinessTab->setAutoFillBackground(false); // transparent window color
        break;
    case 2:
        // This delays the cost of layout until we actually want to see the tab
        personalTab->setWidget(wPersonalTab);
        personalTab->viewport()->setAutoFillBackground(false); // transparent window color
        wPersonalTab->setAutoFillBackground(false); // transparent window color
        break;
    case 3:
        // Wait until we want to use the "Other" tab before initializing.
        setupTabOther();
    default:
        break;
    }
}

void AbEditor::setupTabs()
{
    //
    //  Set up the tabs.
    //

    tabs = new QTabWidget(this);
    connect(tabs, SIGNAL(currentChanged(int)), this, SLOT(tabChanged(int)));
    mainVBox->addWidget(tabs);

    contactTab = new QScrollArea(0);
    contactTab->setFocusPolicy(Qt::NoFocus);
    contactTab->setFrameStyle(QFrame::NoFrame);
    businessTab = new QScrollArea(0);
    businessTab->setFocusPolicy(Qt::NoFocus);
    businessTab->setFrameStyle(QFrame::NoFrame);
    personalTab = new QScrollArea(0);
    personalTab->setFocusPolicy(Qt::NoFocus);
    personalTab->setFrameStyle(QFrame::NoFrame);
    otherTab = new QScrollArea(0);
    otherTab->setFocusPolicy(Qt::NoFocus);
    otherTab->setFrameStyle(QFrame::NoFrame);

    tabs->addTab(contactTab, QIcon( ":icon/contactdetails" ), "");
    tabs->addTab(businessTab, QIcon( ":icon/business" ), "");
    tabs->addTab(personalTab, QIcon( ":icon/home" ), "");
    tabs->addTab(otherTab, QIcon( ":icon/notes" ), "");

    //
    //  Add summary information, if on the desktop.
    //

    summaryTab = 0;
    summary = 0;

    //
    //  Create widgets in the scrollable area of the tabs.
    //

    QWidget *wContactTab = new QWidget;
    contactTab->setWidget(wContactTab);
    contactTab->setWidgetResizable(true);
    contactTab->viewport()->setAutoFillBackground(false); // transparent window color
    wContactTab->setAutoFillBackground(false); // transparent window color
    wBusinessTab = new QWidget;
    businessTab->setWidgetResizable(true);
    wPersonalTab = new QWidget;
    personalTab->setWidgetResizable(true);

    //
    //  Fill in the tabs.
    //

    QLabel *label;
    int rowCount = 0;

    //
    //  Contact Tab
    //

    QGridLayout *gridLayout = new QGridLayout(wContactTab);

    gridLayout->addItem(new QSpacerItem(4, 0), 0, 1);
    gridLayout->setSpacing(0);
    gridLayout->setMargin(2);

    //
    //  Abstract Name
    //

    label = new QLabel(tr("Name"));
    label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    gridLayout->addWidget(label, rowCount, 0);

#ifdef QTOPIA_PHONE
    abName = new AbstractName(0);
    gridLayout->addWidget(abName, rowCount, 2);
    QtopiaApplication::setInputMethodHint(abName, QtopiaApplication::Named,"text");
#else
    QHBoxLayout *nameBoxLayout = new QHBoxLayout();
    gridLayout->addLayout(nameBoxLayout, rowCount, 2);

    abName = new AbstractName(wContactTab);
    nameBoxLayout->addWidget(abName);

    QPushButton *ncm = new QPushButton("...", wContactTab);
    nameBoxLayout->addWidget(ncm);
    ncm->setFixedWidth(ncm->height());
    connect(ncm, SIGNAL(clicked()), abName, SLOT(details()));
#endif
    ++rowCount;

    //
    //   Emails
    //

    label = new QLabel(tr("Emails"));
    label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    gridLayout->addWidget(label, rowCount, 0);

#ifdef QTOPIA_PHONE
    emailLE = new QLineEdit(wContactTab);
    QtopiaApplication::setInputMethodHint(emailLE,QtopiaApplication::Named,"email");
    gridLayout->addWidget(emailLE, rowCount, 2);

    QMenu *emailContextMenu;
    emailContextMenu = QSoftMenuBar::menuFor( emailLE, QSoftMenuBar::EditFocus );
    emailContextMenu->addSeparator();
    emailContextMenu->addAction( QIcon(":icon/details"), tr("Details"), this, SLOT(editEmails()) );
    emailContextMenu = QSoftMenuBar::menuFor( emailLE, QSoftMenuBar::NavigationFocus );
    emailContextMenu->addSeparator();
    emailContextMenu->addAction( QIcon(":icon/details"), tr("Details"), this, SLOT(editEmails()) );
#else
    QHBoxLayout *emailBoxLayout = new QHBoxLayout();
    gridLayout->addLayout(emailBoxLayout, rowCount, 2);

    emailLE = new QLineEdit(wContactTab);
    emailBoxLayout->addWidget(emailLE);

    QPushButton *ecm = new QPushButton("...", wContactTab);
    emailBoxLayout->addWidget(ecm);
    ecm->setFixedWidth(ecm->height());
    connect(ecm, SIGNAL(clicked()), this, SLOT(editEmails()));
#endif
    ++rowCount;

    //
    // VoIP ID
    //

#ifdef QTOPIA_VOIP
    label = new QLabel(tr("VoIPid"), wContactTab);
    label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    gridLayout->addWidget(label, rowCount, 0);

    voipIdLE = new QLineEdit(wContactTab);
    QtopiaApplication::setInputMethodHint(voipIdLE, QtopiaApplication::Text,
            "voipId");
    gridLayout->addWidget(voipIdLE, rowCount, 2);

    ++rowCount;
#endif

    //
    //   Category Checkbox
    //

    categoryCB = new QCheckBox(tr("Business contact"));
    connect(categoryCB, SIGNAL(toggled(bool)), this, SLOT(catCheckBoxChanged(bool)));
    gridLayout->addWidget( categoryCB, rowCount, 0, 1, 3, Qt::AlignHCenter );
    ++rowCount;

    //
    //   Specific widgets : Depend on whether or not this is a business contact.
    //

    specCompanyLA = new QLabel(tr("Company"));
    specCompanyLA->setAlignment(Qt::AlignRight);
    specCompanyLE = new QLineEdit();
    connect(specCompanyLE, SIGNAL(textChanged(const QString&)),
        this, SLOT(specFieldsFilter(const QString&)));
    gridLayout->addWidget(specCompanyLA, rowCount, 0);
    gridLayout->addWidget(specCompanyLE, rowCount, 2);
    ++rowCount;

    specJobTitleLA = new QLabel(tr("Title"));
    specJobTitleLA->setAlignment(Qt::AlignRight);
    specJobTitleLE = new QLineEdit();
    connect(specJobTitleLE, SIGNAL(textChanged(const QString&)),
        this, SLOT(specFieldsFilter(const QString&)));
    gridLayout->addWidget(specJobTitleLA, rowCount, 0);
    gridLayout->addWidget(specJobTitleLE, rowCount, 2);
    ++rowCount;

    //
    //    Phone fields
    //

    phoneMan = new PhoneFieldManager(wContactTab, gridLayout, rowCount);
    phoneMan->setTypes(phoneTypes);
    phoneMan->addEmpty();

    connect( phoneMan, SIGNAL(fieldChanged(const QString&,const PhoneFieldType&)),
            this, SLOT(phoneFieldsToDetailsFilter(const QString&,const PhoneFieldType&)) );

    QSpacerItem* verticalSpacer = new QSpacerItem(1, 1,
        QSizePolicy::Minimum, QSizePolicy::Expanding);
    gridLayout->addItem(verticalSpacer, rowCount + phoneTypes.count(), 2);

    //
    //  Business Tab
    //

    gridLayout = new QGridLayout(wBusinessTab);

    gridLayout->addItem(new QSpacerItem(4, 0), 0, 1);
    gridLayout->setSpacing(0);
    gridLayout->setMargin(2);

    rowCount = 0;

    //
    //  Company
    //

    label = new QLabel( tr("Company"));
    label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    companyLE = new QLineEdit();
    connect( companyLE, SIGNAL(textChanged(const QString&)),
        this, SLOT(specFieldsFilter(const QString&)) );

    gridLayout->addWidget(label, rowCount, 0);
    gridLayout->addWidget(companyLE, rowCount, 2);
    ++rowCount;

    //
    //  Company pronunciation
    //

    label = new QLabel(tr("Pronunciation"));
    label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    companyProLE = new QLineEdit();
    adjustPronWidgets(label, companyProLE);

    gridLayout->addWidget( label, rowCount, 0 );
    gridLayout->addWidget( companyProLE, rowCount, 2 );
    ++rowCount;

    //
    //  Job title
    //

    label = new QLabel(tr("Title"));
    label->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    jobTitleLE = new QLineEdit();
    connect( jobTitleLE, SIGNAL(textChanged(const QString&)), this, SLOT(specFieldsFilter(const QString&)) );

    gridLayout->addWidget( label, rowCount, 0 );
    gridLayout->addWidget( jobTitleLE, rowCount, 2 );
    ++rowCount;

    //
    //    Phone
    //

    label = new QLabel( tr("Phone"));
    label->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    busPhoneLE = new QLineEdit();
    QtopiaApplication::setInputMethodHint(busPhoneLE,QtopiaApplication::PhoneNumber);
    connect( busPhoneLE, SIGNAL(textChanged(const QString&)), this,
            SLOT(detailsToPhoneFieldsFilter(const QString&)) );

    gridLayout->addWidget( label, rowCount, 0 );
    gridLayout->addWidget( busPhoneLE, rowCount, 2 );
    ++rowCount;

    //
    //    Mobile
    //

    label = new QLabel( tr("Mobile"));
    label->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    busMobileLE = new QLineEdit();
    QtopiaApplication::setInputMethodHint(busMobileLE,QtopiaApplication::PhoneNumber);
    connect( busMobileLE, SIGNAL(textChanged(const QString&)), this,
            SLOT(detailsToPhoneFieldsFilter(const QString&)) );

    gridLayout->addWidget( label, rowCount, 0 );
    gridLayout->addWidget( busMobileLE, rowCount, 2 );
    ++rowCount;

    //
    //    Fax
    //

    label = new QLabel( tr("Fax") );
    label->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    busFaxLE = new QLineEdit();
    QtopiaApplication::setInputMethodHint(busFaxLE,QtopiaApplication::PhoneNumber);
    connect( busFaxLE, SIGNAL(textChanged(const QString&)), this,
            SLOT(detailsToPhoneFieldsFilter(const QString&)) );

    gridLayout->addWidget( label, rowCount, 0 );
    gridLayout->addWidget( busFaxLE, rowCount, 2 );
    ++rowCount;

    //
    //    Pager
    //

    label = new QLabel( tr("Pager") );
    label->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    busPagerLE = new QLineEdit();
    QtopiaApplication::setInputMethodHint(busPagerLE,QtopiaApplication::PhoneNumber);
    connect( busPagerLE, SIGNAL(textChanged(const QString&)), this,
            SLOT(detailsToPhoneFieldsFilter(const QString&)) );

    gridLayout->addWidget( label, rowCount, 0 );
    gridLayout->addWidget( busPagerLE, rowCount, 2 );
    ++rowCount;

    //
    //    Business Address
    //

    label = new QLabel( tr("Street") );
    label->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    busStreetME = new QTextEdit();

    QFontMetrics fmBusStreet(busStreetME->font());
    busStreetME->setFixedHeight( fmBusStreet.height() * 3 );

    gridLayout->addWidget( label, rowCount, 0 );
    gridLayout->addWidget( busStreetME, rowCount, 2 );
    ++rowCount;

    label = new QLabel( tr("City") );
    label->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    busCityLE = new QLineEdit();

    gridLayout->addWidget( label, rowCount, 0 );
    gridLayout->addWidget( busCityLE, rowCount, 2 );
    ++rowCount;

    label = new QLabel( tr("State") );
    label->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    busStateLE = new QLineEdit();

    gridLayout->addWidget( label, rowCount, 0 );
    gridLayout->addWidget( busStateLE, rowCount, 2 );
    ++rowCount;

    label = new QLabel( tr("Zip") );
    label->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    busZipLE = new QLineEdit();
    QtopiaApplication::setInputMethodHint( busZipLE, QtopiaApplication::Number );

    gridLayout->addWidget( label, rowCount, 0 );
    gridLayout->addWidget( busZipLE, rowCount, 2 );
    ++rowCount;

    label = new QLabel( tr("Country") );
    label->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    busCountryLE = new QLineEdit();

    gridLayout->addWidget( label, rowCount, 0 );
    gridLayout->addWidget( busCountryLE, rowCount, 2 );
    ++rowCount;

    //
    //    Web page
    //

    label = new QLabel( tr("URL") );
    label->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    busWebPageLE = new QLineEdit();

    gridLayout->addWidget( label, rowCount, 0 );
    gridLayout->addWidget( busWebPageLE, rowCount, 2 );
    ++rowCount;

    //
    //    Department
    //

#ifdef QTOPIA_PHONE
    label = new QLabel( tr("Dept'ment") );
#else
    label = new QLabel( tr("Deptartment") );
#endif
    label->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    deptLE = new QLineEdit();

    gridLayout->addWidget( label, rowCount, 0 );
    gridLayout->addWidget( deptLE, rowCount, 2 );
    ++rowCount;

    //
    //    Office
    //

    label = new QLabel( tr("Office") );
    label->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    officeLE = new QLineEdit();

    gridLayout->addWidget( label, rowCount, 0 );
    gridLayout->addWidget( officeLE, rowCount, 2 );
    ++rowCount;

    //
    //    Profession
    //

    label = new QLabel( tr("Profession") );
    label->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    professionLE = new QLineEdit();

    gridLayout->addWidget( label, rowCount, 0 );
    gridLayout->addWidget( professionLE, rowCount, 2 );
    ++rowCount;

    //
    //    Manager
    //

    label = new QLabel( tr("Manager") );
    label->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    managerLE = new QLineEdit();

    gridLayout->addWidget( label, rowCount, 0 );
    gridLayout->addWidget( managerLE, rowCount, 2 );
    ++rowCount;

    //
    //    Assistant
    //

    label = new QLabel( tr("Assistant") );
    label->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    assistantLE = new QLineEdit();

    gridLayout->addWidget( label, rowCount, 0 );
    gridLayout->addWidget( assistantLE, rowCount, 2 );
    ++rowCount;

    verticalSpacer = new QSpacerItem( 4, 1, QSizePolicy::QSizePolicy::Fixed,
                                                   QSizePolicy::Expanding );
    gridLayout->addItem( verticalSpacer, rowCount, 1 );

    //
    //  Home Tab
    //

    gridLayout = new QGridLayout( wPersonalTab );

    gridLayout->addItem(new QSpacerItem(4, 0), 0, 1);
    gridLayout->setSpacing(0);
    gridLayout->setMargin(2);

    rowCount = 0;

    //
    //    Home phone
    //

    label = new QLabel(tr("Phone"));
    label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    homePhoneLE = new QLineEdit();
    QtopiaApplication::setInputMethodHint(homePhoneLE,QtopiaApplication::PhoneNumber);
    connect( homePhoneLE, SIGNAL(textChanged(const QString&)), this,
            SLOT(detailsToPhoneFieldsFilter(const QString&)) );

    gridLayout->addWidget( label, rowCount, 0 );
    gridLayout->addWidget( homePhoneLE, rowCount, 2 );

    ++rowCount;

    //
    //    Home mobile
    //

    label = new QLabel( tr("Mobile") );
    label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    homeMobileLE = new QLineEdit();
    QtopiaApplication::setInputMethodHint(homeMobileLE,QtopiaApplication::PhoneNumber);
    connect( homeMobileLE, SIGNAL(textChanged(const QString&)), this,
            SLOT(detailsToPhoneFieldsFilter(const QString&)) );

    gridLayout->addWidget( label, rowCount, 0 );
    gridLayout->addWidget( homeMobileLE, rowCount, 2 );

    ++rowCount;

    //
    //    Home fax
    //

    label = new QLabel( tr("Fax") );
    label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    homeFaxLE = new QLineEdit();
    QtopiaApplication::setInputMethodHint(homeFaxLE,QtopiaApplication::PhoneNumber);
    connect( homeFaxLE, SIGNAL(textChanged(const QString&)), this,
            SLOT(detailsToPhoneFieldsFilter(const QString&)) );

    gridLayout->addWidget( label, rowCount, 0 );
    gridLayout->addWidget( homeFaxLE, rowCount, 2 );

    ++rowCount;

    //
    //    Home address
    //

    label = new QLabel( tr("Street") );
    label->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    homeStreetME = new QTextEdit();

    QFontMetrics fmHomeStreet(homeStreetME->font());
    homeStreetME->setFixedHeight( fmHomeStreet.height() * 3 );

    gridLayout->addWidget( label, rowCount, 0 );
    gridLayout->addWidget( homeStreetME, rowCount, 2 );

    ++rowCount;

    label = new QLabel( tr("City") );
    label->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    homeCityLE = new QLineEdit();

    gridLayout->addWidget( label, rowCount, 0 );
    gridLayout->addWidget( homeCityLE, rowCount, 2 );

    ++rowCount;

    label = new QLabel(tr("State"));
    label->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    homeStateLE = new QLineEdit();

    gridLayout->addWidget( label, rowCount, 0 );
    gridLayout->addWidget( homeStateLE, rowCount, 2 );

    ++rowCount;

    label = new QLabel( tr("Zip") );
    label->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    homeZipLE = new QLineEdit();
    QtopiaApplication::setInputMethodHint( homeZipLE, QtopiaApplication::Number );

    gridLayout->addWidget( label, rowCount, 0 );
    gridLayout->addWidget( homeZipLE, rowCount, 2 );

    ++rowCount;

    label = new QLabel( tr("Country") );
    label->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    homeCountryLE = new QLineEdit();

    gridLayout->addWidget( label, rowCount, 0 );
    gridLayout->addWidget( homeCountryLE, rowCount, 2 );

    ++rowCount;

    //
    //    Home web page
    //

    label = new QLabel( tr("URL") );
    label->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    homeWebPageLE = new QLineEdit();

    gridLayout->addWidget( label, rowCount, 0 );
    gridLayout->addWidget( homeWebPageLE, rowCount, 2 );

    ++rowCount;

    //
    //    Spouse
    //

    label = new QLabel( tr("Spouse") );
    label->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    spouseLE = new QLineEdit();

    gridLayout->addWidget( label, rowCount, 0 );
    gridLayout->addWidget( spouseLE, rowCount, 2 );

    ++rowCount;

    //
    //    Children
    //

    label = new QLabel( tr("Children") );
    label->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    childrenLE = new QLineEdit();

    gridLayout->addWidget( label, rowCount, 0 );
    gridLayout->addWidget( childrenLE, rowCount, 2 );

    ++rowCount;

    //
    //    Gender
    //

    label = new QLabel( tr("Gender") );
    label->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    genderCombo = new QComboBox();
    genderCombo->setEditable( false );
    genderCombo->insertItem( 0, "" );
    genderCombo->insertItem( 1, tr( "Male" ) );
    genderCombo->insertItem( 2, tr( "Female" ) );

    gridLayout->addWidget( label, rowCount, 0 );
    gridLayout->addWidget( genderCombo, rowCount, 2 );

    ++rowCount;

    //
    //    Anniversary
    //

    //label = new QLabel( tr("Anniv.") );
    //label->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

    anniversaryCheck = new QGroupBox();
    anniversaryCheck->setCheckable(true);
    anniversaryCheck->setChecked(false);
    anniversaryCheck->setTitle( tr("Anniversary") );

    anniversaryEdit = new QDateEdit( );
    anniversaryEdit->setDate(QDate::currentDate());

    QVBoxLayout *gl = new QVBoxLayout();
    gl->addWidget(anniversaryEdit);
    anniversaryCheck->setLayout(gl);

    gridLayout->addWidget( anniversaryCheck, rowCount, 0, 1, 3);

    ++rowCount;

    //
    //    Birthday
    //

    bdayCheck = new QGroupBox();
    bdayCheck->setCheckable(true);
    bdayCheck->setChecked(false);
    bdayCheck->setTitle( tr("Birthday") );

    bdayEdit = new QDateEdit( );
    bdayEdit->setDate(QDate::currentDate());

    gl = new QVBoxLayout();
    gl->addWidget(bdayEdit);
    bdayCheck->setLayout(gl);

    gridLayout->addWidget( bdayCheck, rowCount, 0, 1, 3);

    ++rowCount;

    verticalSpacer = new QSpacerItem( 1, 1, QSizePolicy::Minimum, QSizePolicy::Expanding );
    gridLayout->addItem( verticalSpacer, rowCount, 1 );


    cmbCat = new QCategorySelector( "Address Book", QCategorySelector::Filter );
    connect(cmbCat, SIGNAL(categoriesSelected(const QList<QString>&)),
        this, SLOT(categorySelectChanged(const QList<QString>&)));

    // The "Other" tab is not setup until the user attempts to view it.
}

//
//  Other Tab
//
void AbEditor::setupTabOther()
{
    if (tabOtherInit)
        return;

    wOtherTab = new QWidget;
    otherTab->setWidgetResizable(true);

    QGridLayout *gridLayout = new QGridLayout( wOtherTab );

//    gridLayout->addItem(new QSpacerItem(4, 0), 0, 1);
    gridLayout->setColumnMinimumWidth(1, 4);    
    gridLayout->setSpacing(0);
    gridLayout->setMargin(0);

    int rowCount = 0;

    //
    //      Photo
    //

    QLabel *label = new QLabel( tr("Photo") );
    photoPB = new QToolButton( 0 );
    photoPB->setIconSize( QSize(80, 96) );
    photoPB->setFixedSize(90, 106);
    QPalette pal = photoPB->palette();
    pal.setBrush( QPalette::Button, wOtherTab->palette().window() );
    photoPB->setPalette( pal );

    connect( photoPB, SIGNAL(clicked()), this, SLOT(editPhoto()) );

    gridLayout->setRowMinimumHeight(rowCount, 106);    
    gridLayout->addWidget( label, rowCount, 0 );
    gridLayout->addWidget( photoPB, rowCount, 3, Qt::AlignHCenter );

    ++rowCount;

#ifdef QTOPIA_CELL
    //
    //      Ringtone selection
    //

    label = new QLabel( tr("Ringtone") );
    editTonePB = new RingToneButton(0);
    editTonePB->setAllowNone(true);

    gridLayout->addWidget( label, rowCount, 0 );
    gridLayout->addWidget( editTonePB, rowCount, 3 );

    ++rowCount;
#endif

    //
    //    Notes
    //

    label = new QLabel( tr("Notes") );
    txtNote = new QTextEdit();
    QFontMetrics fmTxtNote( txtNote->font() );
    txtNote->setFixedHeight( fmTxtNote.height() * 3 );

    txtNoteQC = new QDLEditClient( txtNote, "contactnotes" );
#ifdef QTOPIA_PHONE
    txtNoteQC->setupStandardContextMenu();
#endif

#if defined(QTOPIA_PHONE)
    gridLayout->addWidget( label, rowCount, 0, 1, 4 );
#endif

    ++rowCount;

    gridLayout->addWidget( txtNote, rowCount, 0, 1, 4 );
    ++rowCount;

    gridLayout->addItem( new QSpacerItem( 1, 5, QSizePolicy::Fixed,
        QSizePolicy::Fixed ), rowCount, 1 );
    ++rowCount;

    //
    //    Categories
    //

    label = new QLabel(tr("Category"));

    label->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    gridLayout->addWidget( label, rowCount, 0 );
    ++rowCount;
    gridLayout->addWidget( cmbCat, rowCount, 0, 1, 4 );

    ++rowCount;

    QSpacerItem* verticalSpacer = new QSpacerItem( 1, 1, QSizePolicy::Minimum, QSizePolicy::Expanding );
    gridLayout->addItem( verticalSpacer, rowCount, 1 );

    otherTab->setWidget(wOtherTab);

    tabOtherInit = true;
    setEntryOther();
}

void AbEditor::editPhoto()
{
    ImageSourceDialog *iface = new ImageSourceDialog( mContactImage, this );
    if( QtopiaApplication::execDialog( iface ) == QDialog::Accepted ) {
        mContactImage = iface->pixmap();
        photoPB->setIcon( QIcon(mContactImage) );
    }
    delete iface;
}

void AbEditor::catCheckBoxChanged( bool  b )
{
    QString bcatid = QLatin1String("Business"); // no tr
    QList<QString> curCats = cmbCat->selectedCategories();
    if( b )
    {
        QCategoryManager catman;
        if( !curCats.contains( bcatid ) )
        {
            // Ensure the Business system categoy exists
            catman.addCategory(bcatid, bcatid, QString(), false, true);
            curCats.append(bcatid);
            cmbCat->selectCategories( curCats );
        }
        if(!catman.isSystem( bcatid ))
            catman.setSystem( bcatid );
    }
    else
    {
        if( curCats.contains( bcatid ) )
        {
            curCats.removeAll(bcatid);
            cmbCat->selectCategories( curCats );
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

void AbEditor::categorySelectChanged(const QList<QString>& cats)
{
    if(cats.indexOf("Business") > -1)
        categoryCB->setChecked( true );
    else
        categoryCB->setChecked( false );
}

void AbEditor::buildLineEditList()
{
    lineEdits[QContactModel::Emails] = emailLE ;

    lineEdits[QContactModel::Company] = companyLE ;
    lineEdits[QContactModel::CompanyPronunciation] = companyProLE;
    lineEdits[QContactModel::JobTitle] = jobTitleLE;
    lineEdits[QContactModel::BusinessPhone] = busPhoneLE;
    lineEdits[QContactModel::BusinessFax] = busFaxLE;
    lineEdits[QContactModel::BusinessMobile] = busMobileLE;
    lineEdits[QContactModel::BusinessPager] = busPagerLE;
    lineEdits[QContactModel::BusinessWebPage] = busWebPageLE;
    lineEdits[QContactModel::Department] = deptLE;
    lineEdits[QContactModel::Office] = officeLE;
    lineEdits[QContactModel::Profession] = professionLE;
    lineEdits[QContactModel::Manager] = managerLE;
    lineEdits[QContactModel::Assistant] = assistantLE;

    lineEdits[QContactModel::Spouse] = spouseLE;
    lineEdits[QContactModel::Children] = childrenLE;
    lineEdits[QContactModel::HomePhone] = homePhoneLE;
    lineEdits[QContactModel::HomeFax] = homeFaxLE;
    lineEdits[QContactModel::HomeMobile] = homeMobileLE;
    lineEdits[QContactModel::HomeWebPage] = homeWebPageLE;

    lineEdits[QContactModel::HomeCity] = homeCityLE;
    lineEdits[QContactModel::HomeState] = homeStateLE;
    lineEdits[QContactModel::HomeZip] = homeZipLE;
    lineEdits[QContactModel::HomeCountry] = homeCountryLE;
    lineEdits[QContactModel::BusinessCity] = busCityLE;
    lineEdits[QContactModel::BusinessState] = busStateLE;
    lineEdits[QContactModel::BusinessZip] = busZipLE;
    lineEdits[QContactModel::BusinessCountry] = busCountryLE;
}

#ifdef QTOPIA_PHONE
void AbEditor::keyPressEvent(QKeyEvent *e)
{
    int k = e->key();
    switch( k )
    {
        case Qt::Key_Back:
            //accept();
            //break;
        default:
            QDialog::keyPressEvent( e );
            break;
    }
}
#endif

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

        lastUpdateInternal = true;
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

    if( !type.icon.isNull() && !lastUpdateInternal )
        phoneMan->setNumberFromType( type, newNumber );

    lastUpdateInternal = false;
}

void AbEditor::setEntry( const QContact &entry, bool newEntry
#ifdef QTOPIA_PHONE
    , bool simEntry
#endif
    )
{
    quitExplicitly = false;
    ent = entry;

#ifdef QTOPIA_PHONE
    if(simEntry)
    {
        initSimUI();
        if( newEntry )
            setWindowTitle(tr("New SIM Contact"));
        else
            setWindowTitle(tr("Edit SIM Contact"));

        simName->setText(entry.firstName());
        simNumber->setText(entry.homePhone());
    }
    else
#endif
    {
        QMap<QContactModel::Field, QString> abNameMap;

        initMainUI();
        phoneMan->clear();

        if( newEntry )
            setWindowTitle(tr("New Contact"));
        else
            setWindowTitle(tr("Edit Contact"));

        mImageModified = false;

        cmbCat->selectCategories(ent.categories());
        QString business = QLatin1String("Business");
        bool busCat = ent.categories().contains(business);

        /*
            force an update to category select, even if the checkbox is already
            checked the right way (wouldn't emit toggled() signal)
            saves wasting an extra Categories load when we don't need it
        */

        if(busCat == categoryCB->isChecked())
            catCheckBoxChanged(busCat);
        categoryCB->setChecked(busCat);

        switch( ent.gender() )
        {
        case QContact::UnspecifiedGender:
            genderCombo->setCurrentIndex(0);
            break;
        case QContact::Male:
            genderCombo->setCurrentIndex(1);
            break;
        case QContact::Female:
            genderCombo->setCurrentIndex(2);
            break;
        }

        abName->setModified(false);
        abNameMap.insert(QContactModel::Nickname, ent.nickname());
        abNameMap.insert(QContactModel::NameTitle, ent.nameTitle());
        abNameMap.insert(QContactModel::FirstName, ent.firstName());
        abNameMap.insert(QContactModel::MiddleName, ent.middleName());
        abNameMap.insert(QContactModel::LastName, ent.lastName());
        abNameMap.insert(QContactModel::FirstNamePronunciation,
                                                    ent.firstNamePronunciation());
        abNameMap.insert(QContactModel::LastNamePronunciation,
                                                    ent.lastNamePronunciation());
        abNameMap.insert(QContactModel::Suffix, ent.suffix() );
        abName->setFields(abNameMap);

        QString strDefEmail = ent.defaultEmail();
        QStringList strAllEmail = ent.emailList();
        QString strFinal;
        parseEmailTo(strDefEmail, strAllEmail, strFinal);
        emailLE->setText(strFinal);

        //
        //  Make sure we see the "default"
        //

        emailLE->home( false );

        //
        // VoIP ID
        //
#ifdef QTOPIA_VOIP
        QString strVoipId = ent.customField("VOIP_ID");
        voipIdLE->setText(strVoipId);

        // make sure we see the default
        voipIdLE->home( false );

        if (newEntry)
            ent.setCustomField("VOIP_STATUS", "OFFLINE");
#endif

        QDate bday = ent.birthday();
        if (bday.isNull()) {
            bdayCheck->setChecked(false);
        } else {
            bdayCheck->setChecked(true);
            bdayEdit->setDate( bday );
        }

        //
        //  Home
        //

        spouseLE->setText( ent.spouse() );

        QDate aday = ent.anniversary();
        if (aday.isNull()) {
            anniversaryCheck->setChecked(false);
        } else {
            anniversaryCheck->setChecked(true);
            anniversaryEdit->setDate( aday );
        }

        childrenLE->setText( ent.children() );

        homeStreetME->setPlainText( ent.homeStreet() );
        homeCityLE->setText( ent.homeCity() );
        homeStateLE->setText(  ent.homeState() );
        homeZipLE->setText( ent.homeZip() );
        homeCountryLE->setText( ent.homeCountry() );

        QString homePhone = ent.homePhone();
        QString homeFax = ent.homeFax();
        QString homeMobile = ent.homeMobile();

        homePhoneLE->setText( homePhone );
        homeFaxLE->setText( homeFax );
        homeMobileLE->setText( homeMobile );

        phoneMan->add( homePhone, mHPType );
        phoneMan->add( homeMobile, mHMType );
        phoneMan->add( homeFax, mHFType );
        homeWebPageLE->setText( ent.homeWebpage() );

        //
        //  Business
        //

        busStreetME->setPlainText( ent.businessStreet() );
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

        QString busPhone = ent.businessPhone();
        QString busFax = ent.businessFax();
        QString busMobile = ent.businessMobile();
        QString busPager = ent.businessPager();

        busPhoneLE->setText( busPhone );
        busFaxLE->setText( busFax );
        busMobileLE->setText( busMobile );
        busPagerLE->setText( busPager );

        phoneMan->add( busPhone, mBPType );
        phoneMan->add( busMobile, mBMType );
        phoneMan->add( busFax, mBFType );
        phoneMan->add( busPager, mBPAType );

        professionLE->setText( ent.profession() );
        assistantLE->setText( ent.assistant() );
        managerLE->setText( ent.manager() );

        if (tabOtherInit)
            setEntryOther();

        tabs->setCurrentIndex(0);
    }
    lastUpdateInternal = false;
    mNewEntry = newEntry;
}

void AbEditor::setEntryOther()
{
    QString photoFile = ent.portraitFile();
    if( !photoFile.isEmpty() )
    {
        QPixmap myPixmap;

        QString baseDirStr = Qtopia::applicationFileName( "addressbook", "contactimages/" );
        QThumbnail thumbnail( baseDirStr+photoFile );
        myPixmap = thumbnail.pixmap( QSize( 80, 96 ) );
        if (!myPixmap.isNull()) {
            mContactImage = myPixmap;
            photoPB->setIcon( QIcon(myPixmap) );
        } else {
            qWarning("AbEditor::setEntry - Unable to get image for contact");
        }
    }
    else
    {
        QPixmap e;
        mContactImage = e;
        photoPB->setIcon( QIcon(e) );
    }

    QDL::loadLinks( ent.customField( QDL::CLIENT_DATA_KEY ), QDL::clients( this ) );
    txtNoteQC->verifyLinks();

#ifdef QTOPIA_CELL
    if ( !ent.customField("tone").isEmpty() )
        editTonePB->setTone( QContent( ent.customField("tone") ) ); // No tr()
#endif
    txtNote->setHtml(ent.notes());
}

void AbEditor::closeEvent(QCloseEvent *e)
{
    QDialog::closeEvent(e);
    quitExplicitly = false;
}

void AbEditor::accept()
{
    quitExplicitly = true;
    if(isEmpty())
    {
       if(mNewEntry)
            reject();
        else
            QDialog::accept();
    }
    else
    {
        QContact tmp(ent); //preserve uid.
        contactFromFields(tmp);

        if (tmp.label().isEmpty()) {
            if (QMessageBox::warning(this, tr("Contacts"),
                    tr("<qt>Name is required. Cancel editing?</qt>"),
                    QMessageBox::Yes,
                    QMessageBox::No) == QMessageBox::Yes) {
                reject();
                return;
            } else {
#ifdef QTOPIA_PHONE
                if (simName)
                    simName->setFocus();
                else
#endif
                    abName->setFocus();
                return;
            }
        }

        ent = tmp; // now we can copy the fields over

#ifdef QTOPIA_PHONE
        if(!editingSim)
#endif
        {
            QString links;
            QDL::saveLinks( links, QDL::clients( this ) );
            ent.setCustomField( QDL::CLIENT_DATA_KEY, links );

            if (tabOtherInit) {
                QString pFileName = ent.portraitFile();
                QString baseDirStr = Qtopia::applicationFileName( "addressbook", "contactimages/" );
                mImageModified = !mContactImage.isNull() || !pFileName.isEmpty();
                ent.changePortrait( mContactImage );
            }
        }

        QDialog::accept();
    }
}

bool AbEditor::imageModified() const
{
    return mImageModified;
}

void AbEditor::reject()
{
    quitExplicitly = true;
    QDialog::reject();
}

bool AbEditor::isEmpty()
{
#ifdef QTOPIA_PHONE
    if(editingSim)
    {
        if(!simName->text().trimmed().isEmpty())
            return false;

        if(!simNumber->text().trimmed().isEmpty())
            return false;
    }
    else
    {
#endif
        if( !(abName->isEmpty()) )
            return false;
        if( homeStreetME->toPlainText().trimmed().length() ||
            busStreetME->toPlainText().trimmed().length() )
            return false;

        // analyze all the line edits and make sure there is _something_ there
        // that warrants saving...
        QString text;
        QMap<QContactModel::Field, QLineEdit *>::Iterator it;
        for ( it = lineEdits.begin() ; it != lineEdits.end() ; ++it )
        {
            text = it.value()->text();
            if ( !text.trimmed().isEmpty() )
                return false;
        }

        if( anniversaryCheck->isChecked() )
            return false;
        if( bdayCheck->isChecked() )
            return false;
        if( !genderCombo->currentText().isEmpty() )
            return false;
#ifdef QTOPIA_CELL
        if( tabOtherInit) {
            if (editTonePB->tone().isValid() )
                return false;
        } else if (!ent.customField("tone").isEmpty()) {
            return false;
        }
#endif
#ifdef QTOPIA_VOIP
        text = voipIdLE->text();
        if ( !text.trimmed().isEmpty() )
            return false;
#endif
        if (tabOtherInit) {
            // User may have modified
            text = txtNote->toHtml();
            if ( !text.trimmed().isEmpty() )
                return false;
            if( !mContactImage.isNull() )
                return false;
        } else {
            // otherwise, check existing contact
            if (!ent.notes().isEmpty())
                return false;
            if (!ent.portraitFile().isEmpty())
                return false;
        }
#ifdef QTOPIA_PHONE
    }
#endif

    return true;
}

void AbEditor::contactFromFields(QContact &e)
{
#ifdef QTOPIA_PHONE
    if(editingSim)
    {
        e.setFirstName(simName->text());
        e.setHomePhone(simNumber->text());
    }
    else
#endif
    {
        QMap<QContactModel::Field, QString> fields;
        int gender = genderCombo->currentIndex();
        switch( gender ) {
        case 0: e.setGender( QContact::UnspecifiedGender ); break;
        case 1: e.setGender( QContact::Male ); break;
        case 2: e.setGender( QContact::Female ); break;
        }

        e.setCustomField( "BUSINESS_CONTACT",
            categoryCB->isChecked() ? QString("1") : QString() );

        //
        //  Contact Tab
        //

        if(abName->modified())
            abName->parse();
        fields = abName->fields();
        e.setNameTitle( fields[ QContactModel::NameTitle ] );
        e.setSuffix( fields[ QContactModel::Suffix ]  );
        e.setFirstName( fields[ QContactModel::FirstName ] );
        e.setMiddleName( fields[ QContactModel::MiddleName ] );
        e.setLastName( fields[ QContactModel::LastName ] );
        e.setFirstNamePronunciation( fields[ QContactModel::FirstNamePronunciation ] );
        e.setLastNamePronunciation( fields[ QContactModel::LastNamePronunciation ] );
        if (bdayCheck->isChecked())
            e.setBirthday( bdayEdit->date() );
        else
            e.setBirthday( QDate() );

        e.setNickname( fields[ QContactModel::Nickname ] );

        QString strDefaultEmail;
        QStringList emails;
        parseEmailFrom( emailLE->text(), strDefaultEmail,
                emails );
        e.setDefaultEmail( strDefaultEmail );
        e.setEmailList( emails  );

        //
        // VoIP ID
        //
#ifdef QTOPIA_VOIP
        QString oldVoIP = e.customField("VOIP_ID");
        QString newVoIP = voipIdLE->text();
        if (newVoIP.isEmpty() && !oldVoIP.isEmpty())
            e.removeCustomField("VOIP_ID");
        if (!newVoIP.isEmpty())
            e.setCustomField( "VOIP_ID", newVoIP);
#endif

        //
        // Home Tab
        //

        if (anniversaryCheck->isChecked())
            e.setAnniversary( anniversaryEdit->date() );
        else
            e.setAnniversary( QDate() );

        e.setHomeStreet( homeStreetME->toPlainText() );
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

        //
        //  Business Tab
        //

        e.setCompany( companyLE->text() );
        e.setCompanyPronunciation( companyProLE->text() );
        e.setBusinessStreet( busStreetME->toPlainText() );
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

        //
        //  Notes tab
        //

        if (tabOtherInit) {
            if (txtNote->toPlainText().simplified().isEmpty())
                e.setNotes(QString());
            else
                e.setNotes( txtNote->toHtml() );
#ifdef QTOPIA_CELL
            if ( !editTonePB->tone().isValid() )
                e.removeCustomField("tone"); // No tr()
            else
                e.setCustomField("tone", editTonePB->tone().linkFile() ); // No tr()
#endif
        }
        e.setCategories(cmbCat->selectedCategories());
    }
}

void AbEditor::showEvent( QShowEvent *e )
{
    QDialog::showEvent( e );
    if( mNewEntry )
        abName->setFocus();
}

void AbEditor::setNameFocus()
{
    tabs->setCurrentIndex( tabs->indexOf(contactTab) );
    abName->setFocus();
}

void parseEmailFrom( const QString &txt, QString &strDefaultEmail,
                     QStringList &all )
{
    all.clear();
    strDefaultEmail = "";

    QString emailStr = txt.trimmed();
    if ( emailStr.contains(',') ||
         emailStr.contains(';') || emailStr.contains(' ') ) {
        all = emailStr.split( QRegExp( "[,; ]" ) , QString::SkipEmptyParts );
    }
    else
        all.append(emailStr);
    strDefaultEmail = all.first();
}

void parseEmailTo( const QString &strDefaultEmail,
                   const QStringList &allEmails, QString &strBack )
{
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

    int defaultPos = emails.indexOf( strDefaultEmail );

    if ( defaultPos == -1 ) {
        qWarning("AbEditor::parseEmailTo default email is not found in the email list; bug!");
        strBack = strDefaultEmail;
        strBack += emails.join(", ");
        return;
    }

    if ( defaultPos == 0 ) {
        strBack = emails.join(", ");
        return;
    }

    // reorder the emails to put default first
    emails.removeAt( defaultPos );
    emails.prepend( strDefaultEmail );

    strBack = emails.join(", ");
}

