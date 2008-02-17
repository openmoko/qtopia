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


#include <qtopia/pim/qcontactmodel.h>

#include "abeditor.h"
#include "addressbook.h"
#include "emaildialogphone.h"
#include "../todo/qdelayedscrollarea.h"
#include "groupview.h"


#include <qcategorymanager.h>
#include <qcategoryselector.h>
#include <qiconselector.h>
#include <qtopiaapplication.h>
#include <qtopia/qsoftmenubar.h>
#include <qthumbnail.h>
#include <qimagesourceselector.h>

#include <time.h>
#include <stdio.h>

#include <QTextStream>
#include <QKeyEvent>
#include <QTextEdit>
#include <QTabWidget>
#include <QComboBox>
#include <QCheckBox>
#include <QLabel>
#include <QStyle>
#include <QAction>
#include <QLayout>
#include <QLineEdit>
#include <QToolButton>
#include <QDateTimeEdit>
#include <QGroupBox>
#include <QPushButton>
#include <QRegExp>
#include <QMessageBox>
#include <QDialog>
#include <QList>
#include <QString>
#include <QScrollArea>
#include <QMenu>
#include <QImageReader>
#include <QFormLayout>
#include <QDL>
#include <QDLEditClient>

#include "../todo/reminderpicker.h"
#include "qappointmentmodel.h"

#include "qmailaddress.h"

#if defined(QTOPIA_TELEPHONY)
#include "../../settings/ringprofile/ringtoneeditor.h"
#endif

// helper functions, convert our comma delimited list to proper
// file format...
static void parseEmailFrom( const QString &txt, QString &strDefaultEmail,
                     QStringList &strAll );

// helper convert from file format to comma delimited...
static void parseEmailTo( const QString &strDefaultEmail,
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

PhoneFieldManager::PhoneFieldManager( QWidget *parent, QGridLayout *layout, int rc, AbFullEditor *editor )
    : QObject( parent ), parLayout(layout), rowCount(rc), firstRow(rc), mEditor(editor)
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

bool PhoneFieldManager::isEmpty() const
{
    QListIterator<PhoneField*> it(phoneFields);
    while(it.hasNext()) {
        PhoneField *f = it.next();
        if( !f->isEmpty() )
            return false;
    }
    return true;
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

bool PhoneFieldManager::removeNumber(PhoneFieldType type)
{
    QMutableListIterator<PhoneField *> it(phoneFields);
    QWidget *focusee = 0;

    while(it.hasNext())
    {
        PhoneField *f = it.next();

        if (f->type() == type) {

            emitFieldChanged(QString(), f->type());

            f->remove();
            it.remove();
            delete f;

            if (it.hasNext()) {
                PhoneField *next = it.next();
                focusee = next->numberLE;
            }

            if (focusee)
                focusee->setFocus();
            else {
                // Presumably no fields left
                checkForAdd();
                phoneFields.first()->numberLE->setFocus();
            }
            return true;
        } else
            focusee = f->numberLE;
    }

    return false;
}

bool PhoneFieldManager::removeNumber(QWidget *w)
{
    // The supplied widget is the line edit in a phone field
    QMutableListIterator<PhoneField *> it(phoneFields);
    QWidget *focusee = 0;

    while(it.hasNext())
    {
        PhoneField *f = it.next();

        if (f->numberLE == w) {

            emitFieldChanged(QString(), f->type());

            f->remove();
            it.remove();
            delete f;

            if (it.hasNext()) {
                PhoneField *next = it.next();
                focusee = next->numberLE;
            }

            if (focusee)
                focusee->setFocus();
            else {
                // Presumably no fields left
                checkForAdd();
                phoneFields.first()->numberLE->setFocus();
            }

            return true;
        } else
            focusee = f->numberLE;
    }

    return false;
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

    PhoneField *nf = new PhoneField( parLayout, rowCount, (QWidget *)parent());
    phoneFields.append( nf );
    mEditor->addRemoveNumberMenu(nf->numberLE);
    nf->setTypes( mTypes );
    nf->setType( availTypes.first() );

    connect( nf, SIGNAL(userChangedType(PhoneFieldType)),
        this, SLOT(updateTypes(PhoneFieldType)) );
    connect( nf, SIGNAL(numberChanged(QString)),
        this, SLOT(checkForAdd()) );
    connect( nf, SIGNAL(fieldChanged(QString,PhoneFieldType)),
        this, SLOT(emitFieldChanged(QString,PhoneFieldType)) );
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
    if( !Qtopia::mousePreferred() )
    {
        if( !hasEditFocus() )
            setEditFocus( true );
    }
    setText( text()+txt );
}

//-----------------------------------------------------------------------

PhoneField::PhoneField( QGridLayout *l, int &rowCount, QWidget *parent )
    : QObject( parent )
{
    typeIS = new QIconSelector(parent);
    typeIS->setFocusPolicy(Qt::NoFocus);


    connect( typeIS, SIGNAL(activated(int)), this, SLOT(userChangedType(int)) );
    numberLE = new PhoneFieldLineEdit( typeIS, parent );

    QMenu *contextMenu = QSoftMenuBar::menuFor( numberLE, QSoftMenuBar::AnyFocus );
    contextMenu->addAction( tr("Type"), typeIS, SIGNAL(clicked()));

    connect( numberLE, SIGNAL(textChanged(QString)), this, SIGNAL(numberChanged(QString)) );
    QtopiaApplication::setInputMethodHint(numberLE,QtopiaApplication::PhoneNumber);

    l->addWidget( typeIS, rowCount, 0, Qt::AlignHCenter );
    l->addWidget( numberLE, rowCount, 2 );
    ++rowCount;

    parent->setTabOrder(typeIS, numberLE);

    numberLE->show();
    typeIS->show();

    connect( this, SIGNAL(numberChanged(QString)), this, SLOT(emitFieldChanged()) );
    connect( this, SIGNAL(internalChangedType(PhoneFieldType)),
        this, SIGNAL(typeChanged(PhoneFieldType)) );
    connect( this, SIGNAL(userChangedType(PhoneFieldType)),
        this, SIGNAL(typeChanged(PhoneFieldType)) );
    connect( this, SIGNAL(typeChanged(PhoneFieldType)), this, SLOT(emitFieldChanged()) );
}

PhoneField::~PhoneField()
{
}

void PhoneField::remove()
{
    disconnect( typeIS, SIGNAL(activated(int)), this, SLOT(userChangedType(int)) );
    disconnect( numberLE, SIGNAL(textChanged(QString)), this, SIGNAL(numberChanged(QString)) );
    typeIS->hide();
    numberLE->hide();
    typeIS->deleteLater();
    numberLE->deleteLater();
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

    sm.setPointSize(sm.pointSize() - 1);
    le->setFont(sm);

    label->setFont(sm);
}

//-----------------------------------------------------------------------

//
//FIXME : This is suboptimal iterative widget creation that has survived from the rewrite.
//Now that address parsing has gone, make this into AbNameDetails dialog
//

AbDetailEditor::AbDetailEditor( QWidget *parent, Qt::WFlags fl )
    : QDialog( parent, fl ), suffixCombo(0), titleCombo(0)
{
    mView = new QScrollArea( this );
    mView->setFocusPolicy(Qt::NoFocus);
    mView->setFrameStyle(QFrame::NoFrame);
    mView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
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

void AbDetailEditor::setFields( const QMap<QContactModel::Field, QString> &f )
{
    // FIXME : This loop is pointless - make AbDetailEditor an
    // adt and derive address and name editors from that
    myFields = f;
    QWidget *container = new QWidget;

    const QMap<QContactModel::Field, QString> dn = displayNames();
    QFormLayout *gl = new QFormLayout( container ); //, myFields.count()+1, 3, 3, 0 );

    QWidget *lastEditor = 0, *editor;
    int fieldInTabNum=0;

    //Map may differ in ordering to what we want ; get a value list for the fields we're editing
    QList<QContactModel::Field> g = guiList( f );

    for ( QList<QContactModel::Field>::ConstIterator fieldKey = g.begin() ;
          fieldKey != g.end() ; ++fieldKey ) {

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

                if( *fieldKey == QContactModel::BusinessZip ||
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
            gl->addRow( dn[ *fieldKey], editor);
            if ( *fieldKey == QContactModel::FirstNamePronunciation
                    || *fieldKey == QContactModel::LastNamePronunciation ) {
                QLabel* label = qobject_cast<QLabel*>(gl->labelForField(editor));
                adjustPronWidgets(label, qobject_cast<QLineEdit*>(editor));
            }

            if ( lastEditor )
            {
                setTabOrder( lastEditor, editor );
            }
            lastEditor = editor;

            // increment the field number for this tab
            fieldInTabNum++;
        }
    }
    mView->setWidget(container);
    mView->setWidgetResizable(true);
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
    QMenu *contextMenu;

    contextMenu = QSoftMenuBar::menuFor( this, QSoftMenuBar::EditFocus );
    contextMenu->addSeparator();
    contextMenu->addAction( QIcon(":icon/details"), tr("Details"), this, SLOT(details()) );

    contextMenu = QSoftMenuBar::menuFor( this, QSoftMenuBar::NavigationFocus );
    contextMenu->addSeparator();
    contextMenu->addAction( QIcon(":icon/details"), tr("Details"), this, SLOT(details()) );

    connect( this, SIGNAL(textChanged(QString)), this, SLOT(textChanged()) );
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

AbFullEditor::AbFullEditor(QWidget *parent, Qt::WFlags fl)
    : AbEditor(parent, fl),
    abName(0)
{
    setObjectName("edit");
    setModal(true);

    setWindowState(windowState() | Qt::WindowMaximized);

    mImageModified = false;
    mNewEntry = false;
    phoneMan = 0;
    specCompanyLA = 0;
    lastUpdateInternal = false;
    wBusinessTab = 0;
    wPersonalTab = 0;
    wOtherTab = 0;
    companyLE = 0;
    jobTitleLE = 0;
    homePhoneLE = 0;
    homeMobileLE = 0;
    homeFaxLE = 0;
    busPhoneLE = 0;
    busMobileLE = 0;
    busFaxLE = 0;
    busPagerLE = 0;
    birthdayRP = 0;
    anniversaryRP = 0;
#if defined(QTOPIA_VOIP)
    homeVoipLE = 0;
    busVoipLE = 0;
#endif
    catPB = 0;
    mGroupPicker = 0;
    mCatMan = 0;
    init();
}

AbFullEditor::~AbFullEditor()
{
}

void AbFullEditor::init()
{
    //
    // NOTE : If you change these, you also need to change
    // AbFullEditor::phoneFieldsToDetailsFilter() and
    // AbFullEditor::detailsToPhoneFieldsFilter()
    //

    mHPType = PhoneFieldType("homephone", tr("Home Phone"));
    mHMType = PhoneFieldType("homemobile", tr("Home Mobile"));
    mHFType = PhoneFieldType("homefax", tr("Home Fax"));
    mBPType = PhoneFieldType("businessphone", tr("Business Phone"));
    mBMType = PhoneFieldType("businessmobile", tr("Business Mobile"));
    mBFType = PhoneFieldType("businessfax", tr("Business Fax"));
    mBPAType = PhoneFieldType("businesspager", tr("Business Pager"));
    mHVType = PhoneFieldType("homevoip", tr("Home VOIP"));
    mBVType = PhoneFieldType("businessvoip", tr("Business VOIP"));

    phoneTypes.append(mHPType);
    phoneTypes.append(mHMType);
    phoneTypes.append(mHVType);
    phoneTypes.append(mHFType);
    phoneTypes.append(mBPType);
    phoneTypes.append(mBMType);
    phoneTypes.append(mBVType);
    phoneTypes.append(mBFType);
    phoneTypes.append(mBPAType);

    mainVBox = new QVBoxLayout(this);
    mainVBox->setSpacing(0);
    mainVBox->setContentsMargins(0, 0, 0, 0);

    initMainUI();
}


void AbFullEditor::initMainUI()
{
    actionRemoveNumber = new QAction(QIcon(":icon/trash"), tr("Remove number"), this);
    connect(actionRemoveNumber, SIGNAL(triggered()), this, SLOT(removeNumber()));
    setupTabs();
}

void AbFullEditor::tabClicked( QWidget * )
{
}

void AbFullEditor::editEmails()
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

void AbFullEditor::prepareTab(int tab)
{
    switch (tab) {
    case 1:
        setupTabWork();
        break;
    case 2:
        setupTabHome();
        break;
    case 3:
        setupTabOther();
    default:
        break;
    }
}

void AbFullEditor::addRemoveNumberMenu(QWidget *w)
{
    Q_ASSERT(w);
    QMenu* contextMenu = QSoftMenuBar::menuFor(w, QSoftMenuBar::AnyFocus );
    contextMenu->addAction(actionRemoveNumber);
    connect(contextMenu, SIGNAL(aboutToShow()), this, SLOT(updateContextMenu()));
}

#include "abeditor.moc"

void AbFullEditor::setupTabs()
{
    //
    //  Set up the tabs.
    //

    tabs = new QTabWidget(this);

    mainVBox->addWidget(tabs);

    contactTab = new QDelayedScrollArea(0);
    businessTab = new QDelayedScrollArea(1);
    personalTab = new QDelayedScrollArea(2);
    otherTab = new QDelayedScrollArea(3);

    connect(businessTab, SIGNAL(aboutToShow(int)), this, SLOT(prepareTab(int)));
    connect(personalTab, SIGNAL(aboutToShow(int)), this, SLOT(prepareTab(int)));
    connect(otherTab, SIGNAL(aboutToShow(int)), this, SLOT(prepareTab(int)));

    tabs->addTab(contactTab, QIcon( ":icon/contactdetails" ), tr("Contact"));
    tabs->addTab(businessTab, QIcon( ":icon/business" ), tr("Business"));
    tabs->addTab(personalTab, QIcon( ":icon/home" ), tr("Personal"));
    tabs->addTab(otherTab, QIcon( ":icon/notes" ), tr("Media"));

    //
    //  Add summary information, if on the desktop.
    //

    summaryTab = 0;
    summary = 0;

    //
    //  Create widgets in the scrollable area of the tabs.
    //

    // set up first tab.
    setupTabCommon();
}

void AbFullEditor::setupTabCommon()
{
    //
    //  Contact Tab
    //
    int rowCount = 0;

    QWidget *wContactTab = new QWidget;
    contactTab->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    contactTab->viewport()->setAutoFillBackground(false); // transparent window color
    wContactTab->setAutoFillBackground(false); // transparent window color

    QFormLayout *formLayout = new QFormLayout(wContactTab);

    //
    //  Abstract Name
    //

    abName = new AbstractName(0);
    formLayout->addRow(tr("Name"), abName);
    QtopiaApplication::setInputMethodHint(abName, QtopiaApplication::ProperNouns);

    //
    //   Emails
    //

    emailLE = new QLineEdit(wContactTab);
    lineEdits[QContactModel::Emails] = emailLE ;
    QtopiaApplication::setInputMethodHint(emailLE,"email");
    formLayout->addRow(tr("Emails"), emailLE);

    QMenu *emailContextMenu;
    emailContextMenu = QSoftMenuBar::menuFor( emailLE, QSoftMenuBar::EditFocus );
    emailContextMenu->addSeparator();
    emailContextMenu->addAction( QIcon(":icon/details"), tr("Details"), this, SLOT(editEmails()) );
    emailContextMenu = QSoftMenuBar::menuFor( emailLE, QSoftMenuBar::NavigationFocus );
    emailContextMenu->addSeparator();
    emailContextMenu->addAction( QIcon(":icon/details"), tr("Details"), this, SLOT(editEmails()) );

    //
    //   Category Checkbox
    //

    categoryCB = new QCheckBox(tr("Business contact"));
    connect(categoryCB, SIGNAL(toggled(bool)), this, SLOT(catCheckBoxChanged(bool)));
    QHBoxLayout *hBox = new QHBoxLayout;
    hBox->addStretch(1);
    hBox->addWidget(categoryCB, 0, Qt::AlignHCenter);
    hBox->addStretch(1);
    formLayout->addRow(hBox);

    //
    //   Specific widgets : Depend on whether or not this is a business contact.
    //

    specCompanyLE = new QLineEdit();
    connect(specCompanyLE, SIGNAL(textChanged(QString)),
        this, SLOT(specFieldsFilter(QString)));
    formLayout->addRow(tr("Company"), specCompanyLE);
    specCompanyLA = qobject_cast<QLabel*>(formLayout->labelForField(specCompanyLE));
    QtopiaApplication::setInputMethodHint(specCompanyLE, QtopiaApplication::ProperNouns);

    specJobTitleLE = new QLineEdit();
    connect(specJobTitleLE, SIGNAL(textChanged(QString)),
        this, SLOT(specFieldsFilter(QString)));
    formLayout->addRow(tr("Title"), specJobTitleLE);
    specJobTitleLA = qobject_cast<QLabel*>(formLayout->labelForField(specJobTitleLE));

    //
    //    Phone fields
    //

    QGridLayout *gridLayout = new QGridLayout();
    formLayout->addRow(gridLayout);
    phoneMan = new PhoneFieldManager(wContactTab, gridLayout, rowCount, this);
    phoneMan->setTypes(phoneTypes);
    phoneMan->addEmpty();

    // We try to make sure the fields have at least some useable width,
    // since QLineEdit::minimumWidth isn't really that useable.
    emailLE->setMinimumWidth(100);

    connect( phoneMan, SIGNAL(fieldChanged(QString,PhoneFieldType)),
            this, SLOT(phoneFieldsToDetailsFilter(QString,PhoneFieldType)) );

    contactTab->setWidget(wContactTab);
    contactTab->setWidgetResizable(true);
}

void AbFullEditor::setupTabWork()
{
    if (wBusinessTab)
        return;
    //
    //  Business Tab
    //

    wBusinessTab = new QWidget;
    businessTab->setWidgetResizable(true);
    businessTab->viewport()->setAutoFillBackground(false); // transparent window color
    wBusinessTab->setAutoFillBackground(false); // transparent window color

    QFormLayout *formLayout = new QFormLayout(wBusinessTab);

    //
    //  Company
    //

    companyLE = new QLineEdit();
    lineEdits[QContactModel::Company] = companyLE ;
    QtopiaApplication::setInputMethodHint(companyLE, QtopiaApplication::ProperNouns);
    connect( companyLE, SIGNAL(textChanged(QString)),
        this, SLOT(specFieldsFilter(QString)) );
    formLayout->addRow(tr("Company"), companyLE);

    //
    //  Company pronunciation
    //

    companyProLE = new QLineEdit();
    formLayout->addRow(tr("Pronunciation"), companyProLE);
    QLabel* label = qobject_cast<QLabel*>(formLayout->labelForField(companyProLE));
    adjustPronWidgets(label, companyProLE);

    //
    //  Job title
    //

    jobTitleLE = new QLineEdit();
    lineEdits[QContactModel::JobTitle] = jobTitleLE;
    connect( jobTitleLE, SIGNAL(textChanged(QString)), this, SLOT(specFieldsFilter(QString)) );
    QtopiaApplication::setInputMethodHint(jobTitleLE, QtopiaApplication::ProperNouns);
    formLayout->addRow(tr("Title"), jobTitleLE);

    //
    //    Phone
    //

    busPhoneLE = new QLineEdit();
    addRemoveNumberMenu(busPhoneLE);
    lineEdits[QContactModel::BusinessPhone] = busPhoneLE;
    QtopiaApplication::setInputMethodHint(busPhoneLE,QtopiaApplication::PhoneNumber);
    connect( busPhoneLE, SIGNAL(textChanged(QString)), this,
            SLOT(detailsToPhoneFieldsFilter(QString)) );
    formLayout->addRow(tr("Phone"), busPhoneLE);

    //
    //    Mobile
    //

    busMobileLE = new QLineEdit();
    addRemoveNumberMenu(busMobileLE);
    lineEdits[QContactModel::BusinessMobile] = busMobileLE;
    QtopiaApplication::setInputMethodHint(busMobileLE,QtopiaApplication::PhoneNumber);
    connect( busMobileLE, SIGNAL(textChanged(QString)), this,
            SLOT(detailsToPhoneFieldsFilter(QString)) );
    formLayout->addRow(tr("Mobile"), busMobileLE);

#if defined(QTOPIA_VOIP)
    //
    //    Business e voip
    //

    busVoipLE = new QLineEdit();
    addRemoveNumberMenu(busVoipLE);
    lineEdits[QContactModel::BusinessVOIP] = busVoipLE;
    QtopiaApplication::setInputMethodHint(busVoipLE, "email");
    connect( busVoipLE, SIGNAL(textChanged(QString)), this,
            SLOT(detailsToPhoneFieldsFilter(QString)) );
    formLayout->addRow(tr("VOIP"), busVoipLE);
#endif
    //
    //    Fax
    //

    busFaxLE = new QLineEdit();
    addRemoveNumberMenu(busFaxLE);
    lineEdits[QContactModel::BusinessFax] = busFaxLE;
    QtopiaApplication::setInputMethodHint(busFaxLE,QtopiaApplication::PhoneNumber);
    connect( busFaxLE, SIGNAL(textChanged(QString)), this,
            SLOT(detailsToPhoneFieldsFilter(QString)) );
    formLayout->addRow(tr("Fax"), busFaxLE);

    //
    //    Pager
    //

    busPagerLE = new QLineEdit();
    addRemoveNumberMenu(busPagerLE);
    lineEdits[QContactModel::BusinessPager] = busPagerLE;
    QtopiaApplication::setInputMethodHint(busPagerLE,QtopiaApplication::PhoneNumber);
    connect( busPagerLE, SIGNAL(textChanged(QString)), this,
            SLOT(detailsToPhoneFieldsFilter(QString)) );
    formLayout->addRow(tr("Pager"), busPagerLE);

    //
    //    Business Address
    //

    busStreetME = new QTextEdit();
    QFontMetrics fmBusStreet(busStreetME->font());
    busStreetME->setFixedHeight( fmBusStreet.height() * 3 );
    formLayout->addRow(tr("Address"), busStreetME);

    busCityLE = new QLineEdit();
    lineEdits[QContactModel::BusinessCity] = busCityLE;
    formLayout->addRow(tr("City"), busCityLE);

    busStateLE = new QLineEdit();
    lineEdits[QContactModel::BusinessState] = busStateLE;
    formLayout->addRow(tr("State"), busStateLE);

    busZipLE = new QLineEdit();
    lineEdits[QContactModel::BusinessZip] = busZipLE;
    QtopiaApplication::setInputMethodHint( busZipLE, QtopiaApplication::Number );
    formLayout->addRow(tr("Zip"), busZipLE);

    busCountryLE = new QLineEdit();
    lineEdits[QContactModel::BusinessCountry] = busCountryLE;
    formLayout->addRow(tr("Country"), busCountryLE);

    //
    //    Web page
    //

    busWebPageLE = new QLineEdit();
    lineEdits[QContactModel::BusinessWebPage] = busWebPageLE;
    QtopiaApplication::setInputMethodHint(busWebPageLE, "url");
    formLayout->addRow(tr("URL"), busWebPageLE);

    //
    //    Department
    //

    deptLE = new QLineEdit();
    lineEdits[QContactModel::Department] = deptLE;
    formLayout->addRow(tr("Department"), deptLE);
    label = qobject_cast<QLabel*>(formLayout->labelForField(deptLE));
    adjustPronWidgets(label, deptLE);

    //
    //    Office
    //

    officeLE = new QLineEdit();
    lineEdits[QContactModel::Office] = officeLE;
    formLayout->addRow(tr("Office"), officeLE);

    //
    //    Profession
    //

    professionLE = new QLineEdit();
    professionLE->setWhatsThis(tr("Occupation or job description."));
    lineEdits[QContactModel::Profession] = professionLE;
    formLayout->addRow(tr("Profession"), professionLE);

    //
    //    Manager
    //

    managerLE = new QLineEdit();
    lineEdits[QContactModel::Manager] = managerLE;
    formLayout->addRow(tr("Manager"), managerLE);

    //
    //    Assistant
    //

    assistantLE = new QLineEdit();
    lineEdits[QContactModel::Assistant] = assistantLE;
    formLayout->addRow(tr("Assistant"), assistantLE);

    businessTab->setWidget(wBusinessTab);
    setEntryWork();
}

void AbFullEditor::setupTabHome()
{
    if (wPersonalTab)
        return;
    //
    //  Home Tab
    //

    wPersonalTab = new QWidget;
    personalTab->setWidgetResizable(true);
    personalTab->viewport()->setAutoFillBackground(false); // transparent window color
    wPersonalTab->setAutoFillBackground(false); // transparent window color
    QFormLayout *formLayout = new QFormLayout(wPersonalTab);

    //
    //    Home phone
    //

    homePhoneLE = new QLineEdit();
    addRemoveNumberMenu(homePhoneLE);
    lineEdits[QContactModel::HomePhone] = homePhoneLE;
    QtopiaApplication::setInputMethodHint(homePhoneLE,QtopiaApplication::PhoneNumber);
    connect( homePhoneLE, SIGNAL(textChanged(QString)), this,
            SLOT(detailsToPhoneFieldsFilter(QString)) );
    formLayout->addRow(tr("Phone"), homePhoneLE);

    //
    //    Home mobile
    //

    homeMobileLE = new QLineEdit();
    addRemoveNumberMenu(homeMobileLE);
    lineEdits[QContactModel::HomeMobile] = homeMobileLE;
    QtopiaApplication::setInputMethodHint(homeMobileLE,QtopiaApplication::PhoneNumber);
    connect( homeMobileLE, SIGNAL(textChanged(QString)), this,
            SLOT(detailsToPhoneFieldsFilter(QString)) );
    formLayout->addRow(tr("Mobile"), homeMobileLE);

#if defined(QTOPIA_VOIP)
    //
    //    Home voip
    //

    homeVoipLE = new QLineEdit();
    addRemoveNumberMenu(homeVoipLE);
    lineEdits[QContactModel::HomeVOIP] = homeVoipLE;
    QtopiaApplication::setInputMethodHint(homeVoipLE, "email");
    connect( homeVoipLE, SIGNAL(textChanged(QString)), this,
            SLOT(detailsToPhoneFieldsFilter(QString)) );
    formLayout->addRow(tr("VOIP"), homeVoipLE);
#endif

    //
    //    Home fax
    //

    homeFaxLE = new QLineEdit();
    addRemoveNumberMenu(homeFaxLE);
    lineEdits[QContactModel::HomeFax] = homeFaxLE;
    QtopiaApplication::setInputMethodHint(homeFaxLE,QtopiaApplication::PhoneNumber);
    connect( homeFaxLE, SIGNAL(textChanged(QString)), this,
            SLOT(detailsToPhoneFieldsFilter(QString)) );
    formLayout->addRow(tr("Fax"), homeFaxLE);

    //
    //    Home address
    //

    homeStreetME = new QTextEdit();
    QFontMetrics fmHomeStreet(homeStreetME->font());
    homeStreetME->setFixedHeight( fmHomeStreet.height() * 3 );
    formLayout->addRow(tr("Street"), homeStreetME);

    homeCityLE = new QLineEdit();
    lineEdits[QContactModel::HomeCity] = homeCityLE;
    formLayout->addRow(tr("City"), homeCityLE);

    homeStateLE = new QLineEdit();
    lineEdits[QContactModel::HomeState] = homeStateLE;
    formLayout->addRow(tr("State"), homeStateLE);

    homeZipLE = new QLineEdit();
    lineEdits[QContactModel::HomeZip] = homeZipLE;
    QtopiaApplication::setInputMethodHint( homeZipLE, QtopiaApplication::Number );
    formLayout->addRow(tr("Zip"), homeZipLE);

    homeCountryLE = new QLineEdit();
    lineEdits[QContactModel::HomeCountry] = homeCountryLE;
    formLayout->addRow(tr("Country"), homeCountryLE);

    //
    //    Home web page
    //

    homeWebPageLE = new QLineEdit();
    lineEdits[QContactModel::HomeWebPage] = homeWebPageLE;
    QtopiaApplication::setInputMethodHint(homeWebPageLE, "url");
    formLayout->addRow(tr("URL"), homeWebPageLE);

    //
    //    Spouse
    //

    spouseLE = new QLineEdit();
    spouseLE->setWhatsThis(tr("e.g. Husband or Wife."));
    lineEdits[QContactModel::Spouse] = spouseLE;
    QtopiaApplication::setInputMethodHint(spouseLE, QtopiaApplication::ProperNouns);
    formLayout->addRow(tr("Spouse"), spouseLE);

    //
    //    Children
    //

    childrenLE = new QLineEdit();
    lineEdits[QContactModel::Children] = childrenLE;
    QtopiaApplication::setInputMethodHint(childrenLE, QtopiaApplication::ProperNouns);
    formLayout->addRow(tr("Children"), childrenLE);

    //
    //    Gender
    //

    genderCombo = new QComboBox();
    genderCombo->setEditable( false );
    genderCombo->insertItem( 0, "" );
    genderCombo->insertItem( 1, tr( "Male" ) );
    genderCombo->insertItem( 2, tr( "Female" ) );
    formLayout->addRow(tr("Gender"), genderCombo);

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

    QFormLayout *fl = new QFormLayout(anniversaryCheck);
    fl->addRow(anniversaryEdit);
    anniversaryRP = new ReminderPicker(this, fl, anniversaryAppt);
    formLayout->addRow(anniversaryCheck);

    connect(anniversaryCheck, SIGNAL(clicked(bool)), anniversaryRP, SLOT(updateUI(bool)));

    //
    //    Birthday
    //

    bdayCheck = new QGroupBox();
    bdayCheck->setCheckable(true);
    bdayCheck->setChecked(false);
    bdayCheck->setTitle( tr("Birthday") );

    bdayEdit = new QDateEdit( );
    bdayEdit->setDate(QDate::currentDate());

    fl = new QFormLayout(bdayCheck);
    fl->addRow(bdayEdit);
    birthdayRP = new ReminderPicker(this, fl, birthdayAppt);
    formLayout->addRow(bdayCheck);

    connect(bdayCheck, SIGNAL(clicked(bool)), birthdayRP, SLOT(updateUI(bool)));

    personalTab->setWidget(wPersonalTab);
    setEntryHome();
}

//
//  Other Tab
//
void AbFullEditor::setupTabOther()
{
    if (wOtherTab)
        return;

    wOtherTab = new QWidget;
    otherTab->setWidgetResizable(true);

    QFormLayout *formLayout = new QFormLayout(wOtherTab);

    //
    //      Photo
    //

    photoPB = new QToolButton( 0 );
    photoPB->setIconSize( QContact::portraitSize() );
    photoPB->setFixedSize( QContact::portraitSize() + QSize(8,8));
    connect( photoPB, SIGNAL(clicked()), this, SLOT(editPhoto()) );
    formLayout->addRow(tr("Photo"), photoPB);

#if defined(QTOPIA_TELEPHONY)
    //
    //      Ringtone selection
    //

    editTonePB = new RingToneButton(0);
    editTonePB->setAllowNone(true);
    formLayout->addRow(tr("Ringtone"), editTonePB);
    connect( editTonePB, SIGNAL(selected(QContent)),
            this, SLOT(toneSelected(QContent)) );

    //
    //      Video Ringtone selection

    editVideoTonePB = new RingToneButton(0);
    editVideoTonePB->setAllowNone(true);
    editVideoTonePB->setVideoSelector( true );
    formLayout->addRow(tr("Video Tone"), editVideoTonePB);
    connect( editVideoTonePB, SIGNAL(selected(QContent)),
            this, SLOT(toneSelected(QContent)) );
#endif

    //
    //    Notes
    //

    txtNote = new QTextEdit();
    txtNote->setLineWrapMode( QTextEdit::WidgetWidth );

    QFontMetrics fmTxtNote( txtNote->font() );
    txtNote->setFixedHeight( fmTxtNote.height() * 5 );
    formLayout->addRow(tr("Notes"), txtNote);

    txtNoteQC = new QDLEditClient( txtNote, "contactnotes" );
    txtNoteQC->setupStandardContextMenu();

    //
    //    Groups
    //

    catPB = new QPushButton();
    connect(catPB, SIGNAL(clicked()), this, SLOT(editGroups()));

    formLayout->addRow(tr("Groups"), catPB);

    otherTab->setWidget(wOtherTab);
    setEntryOther();
}

void AbFullEditor::editPhoto()
{
    QImageSourceSelectorDialog *iface = new QImageSourceSelectorDialog(this);
    iface->setMaximumImageSize(QContact::portraitSize());
    iface->setContent(mContactImage);
    iface->setWindowTitle(tr("Contact Portrait"));
    if( QtopiaApplication::execDialog( iface ) == QDialog::Accepted ) {
        mImageModified = true;
        mContactImage = iface->content();
        photoPB->setIcon( QIcon(mContactImage.fileName()) );
    }
    delete iface;
}

void AbFullEditor::removeNumber()
{
    // Find out what has focus
    QWidget *w = focusWidget();

    if (!phoneMan->removeNumber(w)) {
        // rely on the contentsChanged signal to update everything
        QLineEdit *le = qobject_cast<QLineEdit*>(w);
        if (le) {
            le->clear();
            if (le->hasEditFocus())
                le->setEditFocus(false);
        }

        // Now try to remove the corresponding phoneman field
        PhoneFieldType type = findPhoneField(le);
        if (!type.icon.isNull())
            phoneMan->removeNumber(type);
    }
}

void AbFullEditor::catCheckBoxChanged( bool  b )
{
    QString bcatid = QLatin1String("Business"); // no tr
    if( b ) {
        if (!mCatMan)
            mCatMan = new QCategoryManager("Address Book", this); // no tr
        // Ensure the Business system categoy exists
        // For new code a more unique id should be used instead of using the untranslated text
        // eg. ensureSystemCategory("com.mycompany.myapp.mycategory", "My Category");
        mCatMan->ensureSystemCategory(bcatid, bcatid);
        mGroupList.append(bcatid);
    } else {
        mGroupList.removeAll(bcatid);
    }
    updateGroupButton();
    showSpecWidgets( b );
}

void AbFullEditor::showSpecWidgets( bool s )
{
    if( s ) {
        specCompanyLA->setMaximumHeight(QWIDGETSIZE_MAX);
        specCompanyLE->show();

        specJobTitleLA->setMaximumHeight(QWIDGETSIZE_MAX);
        specJobTitleLE->show();
    } else {
        specCompanyLA->setMaximumHeight(0);
        specCompanyLE->hide();

        specJobTitleLA->setMaximumHeight(0);
        specJobTitleLE->hide();
    }
}

void AbFullEditor::specFieldsFilter( const QString &newValue )
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
void AbFullEditor::phoneFieldsToDetailsFilter( const QString &newNumber,
                                                    const PhoneFieldType &newType )
{
    QLineEdit *detail = 0;
    if (wPersonalTab) {
        if( newType == mHPType ) //phone
            detail = homePhoneLE;
        else if( newType == mHMType )//mobile
            detail = homeMobileLE;
#if defined(QTOPIA_VOIP)
        else if( newType == mHVType )//voip
            detail = homeVoipLE;
#endif
        else if( newType == mHFType )//fax
            detail = homeFaxLE;
    }

    if (!detail && wBusinessTab) {
        if( newType == mBPType )//phone
            detail = busPhoneLE;
        else if( newType == mBMType )//mobile
            detail = busMobileLE;
#if defined(QTOPIA_VOIP)
        else if( newType == mBVType )//voip
            detail = busVoipLE;
#endif
        else if( newType == mBFType )//fax
            detail = busFaxLE;
        else if( newType == mBPAType )//pager
            detail = busPagerLE;
    }

    if( detail )
    {
        //avoid recursion
        if( newNumber == detail->text() )
            return;

        lastUpdateInternal = true;
        detail->setText( newNumber );

    }
}

PhoneFieldType AbFullEditor::findPhoneField(QLineEdit *detail)
{
    PhoneFieldType type;

    if( detail == busPhoneLE )
        type = mBPType ;
    else if( detail == busMobileLE )
        type = mBMType ;
#if defined(QTOPIA_VOIP)
    else if( detail == busVoipLE )
        type = mBVType ;
    else if( detail == homeVoipLE )
        type = mHVType;
#endif
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

    return type;
}

// when details of phone fields changed, this updates the phone field manager
void AbFullEditor::detailsToPhoneFieldsFilter( const QString &newNumber )
{
    PhoneFieldType type = findPhoneField((QLineEdit*) sender());

    if( !type.icon.isNull() && !lastUpdateInternal )
        phoneMan->setNumberFromType( type, newNumber );

    lastUpdateInternal = false;
}

void AbFullEditor::editGroups()
{
    if (!mGroupPicker) {
        mGroupDialog = new QDialog();
        QVBoxLayout *vl = new QVBoxLayout();
        mGroupPicker = new GroupView(true, this);
        vl->addWidget(mGroupPicker);
        mGroupDialog->setLayout(vl);
        mGroupDialog->setWindowTitle(tr("Groups"));
        connect(mGroupPicker, SIGNAL(backClicked()), mGroupDialog, SLOT(accept()));

        // Actions!
        actionAddGroup = new QAction(QIcon(":icon/new"), QApplication::translate("AddressbookWindow", "New group"), this);
        actionAddGroup->setWhatsThis(QApplication::translate("AddressbookWindow", "Add new contact group."));

#if defined(QTOPIA_TELEPHONY)
        actionSetRingTone = new QAction(QIcon(), QApplication::translate("AddressbookWindow", "Set group ringtone...", "Set ringtone to current contact group"), this);
        actionSetRingTone->setWhatsThis(QApplication::translate("AddressbookWindow", "Set a ringtone that is played when an incoming call comes in from this group members."));
#endif

        actionRemoveGroup = new QAction(QIcon(":icon/trash"), QApplication::translate("AddressbookWindow", "Delete", "Delete current contact group"), this);
        actionRemoveGroup->setWhatsThis(QApplication::translate("AddressbookWindow", "Delete highlighted contact group."));

        actionRenameGroup = new QAction(QIcon(":icon/edit"), QApplication::translate("AddressbookWindow", "Rename", "Rename current contact group"), this);
        actionRenameGroup->setWhatsThis(QApplication::translate("AddressbookWindow", "Rename highlighted contact group."));

#if defined(QTOPIA_TELEPHONY)
        connect(actionSetRingTone, SIGNAL(triggered()), mGroupPicker, SLOT(setGroupRingTone()));
#endif
        connect(actionAddGroup, SIGNAL(triggered()), mGroupPicker, SLOT(addGroup()));
        connect(actionRemoveGroup, SIGNAL(triggered()), mGroupPicker, SLOT(removeCurrentGroup()));
        connect(actionRenameGroup, SIGNAL(triggered()), mGroupPicker, SLOT(renameCurrentGroup()));

        QMenu* contextMenu = QSoftMenuBar::menuFor(mGroupDialog);

        contextMenu->addAction(actionAddGroup);
#if defined(QTOPIA_TELEPHONY)
        contextMenu->addAction(actionSetRingTone);
#endif
        contextMenu->addAction(actionRemoveGroup);
        contextMenu->addAction(actionRenameGroup);

        connect(contextMenu, SIGNAL(aboutToShow()), this, SLOT(updateContextMenu()));
    }

    mGroupPicker->setSelectedGroups(mGroupList);

    if (QtopiaApplication::execDialog(mGroupDialog)) {
        mGroupList = mGroupPicker->selectedGroups();
        if(mGroupList.indexOf("Business") > -1) // no tr
            categoryCB->setChecked( true );
        else
            categoryCB->setChecked( false );
        updateGroupButton();
    }
}

void AbFullEditor::updateContextMenu()
{
    if (mGroupPicker && mGroupPicker->isVisible()) {
        bool groupSelected = mGroupPicker->currentIndex().isValid();
        bool groupSystem = groupSelected && mGroupPicker->isCurrentSystemGroup();

#if defined(QTOPIA_TELEPHONY)
        actionSetRingTone->setVisible( groupSelected );
#endif
        actionRemoveGroup->setVisible( groupSelected && !groupSystem);
        actionRenameGroup->setVisible( groupSelected && !groupSystem);
    }

    // Note that actionRemoveNumber is only present on phone number fields,
    // but we don't need to be that specific to update this
    QLineEdit *le = qobject_cast<QLineEdit *>(focusWidget());
    if (le)
        actionRemoveNumber->setVisible(!le->text().isEmpty());
}

void AbFullEditor::setEntry( const QContact &entry, bool newEntry)
{
    ent = entry;
    mNewEntry = newEntry;

    QMap<QContactModel::Field, QString> abNameMap;

    phoneMan->clear();

    if( newEntry )
        setWindowTitle(tr("New Contact"));
    else
        setWindowTitle(tr("Edit Contact"));

    mImageModified = false;

    mGroupList = ent.categories();

    QString business = QLatin1String("Business");
    bool busCat = mGroupList.contains(business);

    /*
       force an update to category select, even if the checkbox is already
       checked the right way (wouldn't emit toggled() signal)
       saves wasting an extra Categories load when we don't need it
     */

    if(busCat == categoryCB->isChecked())
        catCheckBoxChanged(busCat);
    categoryCB->setChecked(busCat);


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

    // set phoneMan numbers
    phoneMan->add( ent.homePhone(), mHPType);
    phoneMan->add( ent.homeMobile(), mHMType );
    phoneMan->add( ent.homeVOIP(), mHVType );
    phoneMan->add( ent.homeFax(), mHFType );
    phoneMan->add( ent.businessPhone(), mBPType );
    phoneMan->add( ent.businessMobile(), mBMType );
    phoneMan->add( ent.businessVOIP(), mBVType );
    phoneMan->add( ent.businessFax(), mBFType );
    phoneMan->add( ent.businessPager(), mBPAType );

    // Spec fields on the common tab
    specCompanyLE->setText(ent.company());
    specJobTitleLE->setText(ent.jobTitle());

    // Grab the appointments for the reminders
    QUniqueId annivId = ent.dependentChildrenOfType("anniversary").value(0); // no tr
    QUniqueId birthdayId = ent.dependentChildrenOfType("birthday").value(0); // no tr

    anniversaryAppt = QAppointment();
    birthdayAppt = QAppointment();

    if (!annivId.isNull() || !birthdayId.isNull()) {
        QAppointmentModel am;

        if (!annivId.isNull())
            anniversaryAppt = am.appointment(annivId);
        if (!birthdayId.isNull())
            birthdayAppt = am.appointment(birthdayId);
    }

    bool setAlarm;
    QTime defaultReminderTime;

    {
        QSettings config("Trolltech","DateBook");
        config.beginGroup("Main");
        defaultReminderTime = QTime(config.value("startviewtime", 8).toInt(), 0);
        setAlarm = config.value("alarmpreset").toBool();
    }

    int alarmMinutes = -(defaultReminderTime.hour() * 60 + defaultReminderTime.minute());

    if (anniversaryAppt.uid().isNull()) {
        anniversaryAppt.setAlarm(alarmMinutes, setAlarm ? QAppointment::Audible : QAppointment::NoAlarm);
        anniversaryAppt.setAllDay();
    } else if (anniversaryAppt.alarm() == QAppointment::NoAlarm) {
        anniversaryAppt.setAlarm(alarmMinutes, QAppointment::NoAlarm);
    }

    if (birthdayAppt.uid().isNull()) {
        birthdayAppt.setAlarm(alarmMinutes, setAlarm ? QAppointment::Audible : QAppointment::NoAlarm);
        birthdayAppt.setAllDay();
    } else if (birthdayAppt.alarm() == QAppointment::NoAlarm) {
        birthdayAppt.setAlarm(alarmMinutes, QAppointment::NoAlarm);
    }

    if (wOtherTab)
        setEntryOther();
    if (wBusinessTab)
        setEntryWork();
    if (wPersonalTab)
        setEntryHome();

    setNameFocus();
}

void AbFullEditor::setEntryHome()
{
    QDate bday = ent.birthday();

    if (bday.isNull()) {
        bdayCheck->setChecked(false);
    } else {
        bdayCheck->setChecked(true);
        bdayEdit->setDate( bday );
    }

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

    // mirrored one 'common' screen, may have changed
    // so take from there rather than entry
    homePhoneLE->setText( phoneMan->numberFromType( mHPType ) );
    homeFaxLE->setText( phoneMan->numberFromType( mHFType ) );
    homeMobileLE->setText( phoneMan->numberFromType( mHMType ) );
#if defined(QTOPIA_VOIP)
    homeVoipLE->setText( phoneMan->numberFromType( mHVType ) );
#endif

    homeWebPageLE->setText( ent.homeWebpage() );

    birthdayRP->updateUI(!bday.isNull());
    anniversaryRP->updateUI(!aday.isNull());
}

void AbFullEditor::setEntryWork()
{
    //
    //  Business
    //

    busStreetME->setPlainText( ent.businessStreet() );
    busCityLE->setText( ent.businessCity() );
    busStateLE->setText( ent.businessState() );
    busZipLE->setText( ent.businessZip() );
    busCountryLE->setText( ent.businessCountry() );
    companyProLE->setText( ent.companyPronunciation() );
    busWebPageLE->setText( ent.businessWebpage() );
    deptLE->setText( ent.department() );
    officeLE->setText( ent.office() );

    // mirrored on 'common' screen, may have changed
    // so take from there rather than entry
    busPhoneLE->setText( phoneMan->numberFromType( mBPType ) );
    busFaxLE->setText( phoneMan->numberFromType( mBFType ) );
    busMobileLE->setText( phoneMan->numberFromType( mBMType ) );
#if defined(QTOPIA_VOIP)
    busVoipLE->setText( phoneMan->numberFromType( mBVType) );
#endif
    busPagerLE->setText( phoneMan->numberFromType( mBPAType ) );
    companyLE->setText( specCompanyLE->text() );
    jobTitleLE->setText( specJobTitleLE->text() );

    professionLE->setText( ent.profession() );
    assistantLE->setText( ent.assistant() );
    managerLE->setText( ent.manager() );
}

void AbFullEditor::setEntryOther()
{
    QString photoFile = ent.portraitFile();
    if( !photoFile.isEmpty() )
    {
        QPixmap myPixmap;

        QString baseDirStr = Qtopia::applicationFileName( "addressbook", "contactimages/" );
        QString portraitFilename = photoFile.startsWith(QChar(':')) ? photoFile : baseDirStr + photoFile;
        mContactImage = QContent(portraitFilename);
        photoPB->setIcon( QIcon(portraitFilename) );
    }
    else
    {
        mContactImage = QContent();
        photoPB->setIcon( QIcon() );
    }

    txtNote->setHtml(ent.notes());
    if ( !ent.customField( QDL::CLIENT_DATA_KEY ).isEmpty() )
        QDL::loadLinks( ent.customField( QDL::CLIENT_DATA_KEY ), QDL::clients( this ) );
    txtNoteQC->verifyLinks();

#if defined(QTOPIA_TELEPHONY)
    if ( !ent.customField("tone").isEmpty() )
        editTonePB->setTone( QContent( ent.customField("tone") ) ); // No tr()
    if ( !ent.customField("videotone").isEmpty() )
        editVideoTonePB->setTone( QContent( ent.customField("videotone") ) );
#endif

    updateGroupButton();
}

void AbFullEditor::updateGroupButton()
{
    if (catPB) {
        // Figure out what to put on the button...
        if (mGroupList.count() == 0) {
            catPB->setText(QCategoryManager::unfiledLabel()); // XXX sucky label.
        } else if (mGroupList.count() == 1) {
            if (!mCatMan)
                mCatMan = new QCategoryManager("Address Book", this); // no tr
            catPB->setText(mCatMan->label(mGroupList.at(0)));
        } else {
            catPB->setText(QCategoryManager::multiLabel());
        }
    }
}

void AbFullEditor::closeEvent(QCloseEvent *e)
{
    QDialog::closeEvent(e);
}

void AbFullEditor::accept()
{
    if(mNewEntry && isEmpty())
    {
        reject();
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
                abName->setFocus();
                return;
            }
        }

        ent = tmp; // now we can copy the fields over

        if (QDL::clients(this).count()) {
            QString links;
            QDL::saveLinks( links, QDL::clients( this ) );
            ent.setCustomField( QDL::CLIENT_DATA_KEY, links );
        }

        if (wOtherTab) {
            if (mImageModified)
            {
                QIODevice* io = mContactImage.open();
                QImageReader reader( io );
                if ( reader.supportsOption( QImageIOHandler::Size ))
                {
                    QSize imageSize( reader.size() );
                    QSize boundedSize = imageSize.boundedTo( QContact::portraitSize() );

                    if (imageSize != boundedSize )
                    {
                        imageSize.scale( QContact::portraitSize(), Qt::KeepAspectRatio );
                        reader.setScaledSize( imageSize );
                    }
                }

                QPixmap pixmap = QPixmap::fromImage( reader.read() );
                if ( !reader.supportsOption( QImageIOHandler::Size ))
                {
                    pixmap = pixmap.scaled( QContact::portraitSize(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
                }
                delete io;

                ent.changePortrait( pixmap );
            }
        }

        QDialog::accept();
    }
}

bool AbFullEditor::imageModified() const
{
    return mImageModified;
}

void AbFullEditor::reject()
{
    QDialog::reject();
}

bool AbFullEditor::isEmpty() const
{
    if( !(abName->isEmpty()) )
        return false;

    // analyze all the line edits and make sure there is _something_ there
    // that warrants saving...
    QString text;
    QMap<QContactModel::Field, QLineEdit *>::ConstIterator it;
    for ( it = lineEdits.begin() ; it != lineEdits.end() ; ++it )
    {
        text = it.value()->text();
        if ( !text.trimmed().isEmpty() )
            return false;
    }

    if (!phoneMan->isEmpty())
        return false;

    if (wBusinessTab) {
        if (busStreetME->toPlainText().trimmed().length())
            return false;
    } else {
        if(!ent.businessStreet().isEmpty())
            return false;
    }

    if ( wPersonalTab ) {
        if( homeStreetME->toPlainText().trimmed().length())
            return false;
        if( anniversaryCheck->isChecked() )
            return false;
        if( bdayCheck->isChecked() )
            return false;
        if( !genderCombo->currentText().isEmpty() )
            return false;
    } else {
        // otherwise, check existing contact
        if ( !ent.homeStreet().isEmpty() )
            return false;
        if ( !ent.anniversary().isNull() )
            return false;
        if ( !ent.birthday().isNull() )
            return false;
        if ( ent.gender() != QContact::UnspecifiedGender )
            return false;
    }
#if defined(QTOPIA_TELEPHONY)
    if(wOtherTab) {
        if (editTonePB->tone().isValid() )
            return false;
    } else if (!ent.customField("tone").isEmpty()) {
        return false;
    }
    if(wOtherTab) {
        if (editVideoTonePB->tone().isValid() )
            return false;
    } else if (!ent.customField("videotone").isEmpty()) {
        return false;
    }
#endif
    if (wOtherTab) {
        // User may have modified
        text = txtNote->toPlainText();
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

    return true;
}

void AbFullEditor::contactFromFields(QContact &e)
{
    QMap<QContactModel::Field, QString> fields;

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

    QString strDefaultEmail;
    QStringList emails;
    parseEmailFrom( emailLE->text(), strDefaultEmail,
            emails );
    e.setDefaultEmail( strDefaultEmail );
    e.setEmailList( emails  );

    e.setHomePhone( phoneMan->numberFromType( mHPType ) );
    e.setHomeMobile( phoneMan->numberFromType( mHMType ) );
    e.setHomeVOIP( phoneMan->numberFromType( mHVType ) );
    e.setHomeFax( phoneMan->numberFromType( mHFType ) );

    e.setBusinessPhone( phoneMan->numberFromType( mBPType ) );
    e.setBusinessMobile( phoneMan->numberFromType( mBMType ) );
    e.setBusinessVOIP( phoneMan->numberFromType( mBVType ) );
    e.setBusinessFax( phoneMan->numberFromType( mBFType ) );
    e.setBusinessPager( phoneMan->numberFromType( mBPAType ) );

    //
    // Home Tab
    //

    if (wPersonalTab) {
        int gender = genderCombo->currentIndex();
        switch( gender ) {
            case 0: e.setGender( QContact::UnspecifiedGender ); break;
            case 1: e.setGender( QContact::Male ); break;
            case 2: e.setGender( QContact::Female ); break;
        }
        if (bdayCheck->isChecked())
            e.setBirthday( bdayEdit->date() );
        else
            e.setBirthday( QDate() );

        e.setNickname( fields[ QContactModel::Nickname ] );

        if (anniversaryCheck->isChecked())
            e.setAnniversary( anniversaryEdit->date() );
        else
            e.setAnniversary( QDate() );

        e.setHomeStreet( homeStreetME->toPlainText() );
        e.setHomeCity( homeCityLE->text() );
        e.setHomeState( homeStateLE->text() );
        e.setHomeZip( homeZipLE->text() );
        e.setHomeCountry( homeCountryLE->text() );
        e.setHomeWebpage( homeWebPageLE->text() );
        e.setSpouse( spouseLE->text() );
        e.setChildren( childrenLE->text() );
    }

    //
    //  Business Tab
    //

    if (wBusinessTab) {
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
        e.setProfession( professionLE->text() );
        e.setAssistant( assistantLE->text() );
        e.setManager( managerLE->text() );
    } else {
        if (specJobTitleLE)
            e.setJobTitle(specJobTitleLE->text());
        if (specCompanyLE)
            e.setCompany(specCompanyLE->text());
    }

    //
    //  Notes tab
    //

    if (wOtherTab) {
        if (txtNote->toPlainText().simplified().isEmpty())
            e.setNotes(QString());
        else
            e.setNotes( txtNote->toHtml() );
#if defined(QTOPIA_TELEPHONY)
        if ( !editTonePB->tone().isValid() )
            e.removeCustomField("tone"); // No tr()
        else
            e.setCustomField("tone", editTonePB->tone().fileName() ); // No tr()
        if ( !editVideoTonePB->tone().isValid() )
            e.removeCustomField("videotone");
        else
            e.setCustomField("videotone", editVideoTonePB->tone().fileName() );
#endif
    }

    e.setCategories(mGroupList);
}

void AbFullEditor::showEvent( QShowEvent *e )
{
    QDialog::showEvent( e );
    if( mNewEntry )
        abName->setFocus();
}

void AbFullEditor::setNameFocus()
{
    tabs->setCurrentIndex( tabs->indexOf(contactTab) );

    // Also reset the focused widget for the other tabs
    if (wBusinessTab) {
        companyLE->setFocus();
        businessTab->ensureVisible(0,0,0,0);
    }

    if (wPersonalTab) {
        homePhoneLE->setFocus();
        personalTab->ensureVisible(0,0,0,0);
    }

    if (wOtherTab) {
        photoPB->setFocus();
        otherTab->ensureVisible(0,0,0,0);
    }

    // Finally this
    abName->setFocus();
    contactTab->ensureVisible(0,0,0,0);
}

void AbFullEditor::toneSelected( const QContent &tone )
{
#if defined(QTOPIA_TELEPHONY)
    // normal ringtone and video ringtone are mutually exclusive
    if ( sender() == editTonePB && tone.isValid() )
        editVideoTonePB->setTone( QContent() );
    else if ( sender() == editVideoTonePB && tone.isValid() )
        editTonePB->setTone( QContent() );
#endif
}

void parseEmailFrom( const QString &txt, QString &strDefaultEmail,
                     QStringList &all )
{
    all.clear();
    strDefaultEmail = "";

    QString emailStr = txt.trimmed();

    QList<QMailAddress> addresses = QMailAddress::fromStringList(emailStr);
    all = QMailAddress::toStringList(addresses);

    if (all.count() > 0)
        strDefaultEmail = all.first();
}

void parseEmailTo( const QString &strDefaultEmail,
                   const QStringList &allEmails, QString &strBack )
{
    if ( strDefaultEmail.isEmpty() && !allEmails.count() ) {
        strBack = "";
        return;
    }

    QStringList list = allEmails;
    if (allEmails.value(0) != strDefaultEmail) {
        list.removeAll(strDefaultEmail);
        list.prepend(strDefaultEmail);
    }

    QList<QMailAddress> addresses = QMailAddress::fromStringList(list);
    strBack = QMailAddress::toStringList(addresses).join(",");
}

#ifdef QTOPIA_CELL

AbSimEditor::AbSimEditor(QWidget *parent, Qt::WFlags fl)
    : AbEditor(parent, fl)
{
    setObjectName("edit");
    setModal(true);

    setWindowState(windowState() | Qt::WindowMaximized);

    mNewEntry = false;

    initSimUI();
}

AbSimEditor::~AbSimEditor()
{
}

void AbSimEditor::initSimUI()
{
    QVBoxLayout *mainVBox = new QVBoxLayout(this);
    mainVBox->setSpacing(0);
    mainVBox->setContentsMargins(0, 0, 0, 0);

    simEditor = new QWidget(0);
    mainVBox->addWidget(simEditor);

    QGridLayout *gridLayout = new QGridLayout(simEditor);

    gridLayout->addItem(new QSpacerItem(4, 0), 0, 1);
    gridLayout->setSpacing(0);

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
    QtopiaApplication::setInputMethodHint(simNumber,QtopiaApplication::PhoneNumber);
    gridLayout->addWidget(simNumber, rowCount, 2);

    rowCount++;
    gridLayout->addItem(new QSpacerItem(4, 0), rowCount, 1);
}

void AbSimEditor::setEntry( const QContact &entry, bool newEntry)
{
    ent = entry;
    if( newEntry )
        setWindowTitle(tr("New SIM Contact"));
    else
        setWindowTitle(tr("Edit SIM Contact"));

    simName->setText(entry.firstName());
    simNumber->setText(entry.homePhone());

    mNewEntry = newEntry;
}

bool AbSimEditor::isEmpty() const
{
    return simName->text().trimmed().isEmpty() && simNumber->text().trimmed().isEmpty();
}

void AbSimEditor::accept()
{
    if(mNewEntry && isEmpty()) {
        reject();
    }
    else
    {
        QContact tmp(ent); //preserve uid.
        tmp.setFirstName(simName->text());
        tmp.setHomePhone(simNumber->text());

        if (tmp.label().isEmpty()) {
            if (QMessageBox::warning(this, tr("Contacts"),
                    tr("<qt>Name is required. Cancel editing?</qt>"),
                    QMessageBox::Yes,
                    QMessageBox::No) == QMessageBox::Yes) {
                reject();
                return;
            } else {
                simName->setFocus();
                return;
            }
        }

        ent = tmp; // now we can copy the fields over

        QDialog::accept();
    }
}

void AbSimEditor::reject()
{
    QDialog::reject();
}
#endif
