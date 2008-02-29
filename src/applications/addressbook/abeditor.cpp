/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "abeditor.h"
#include "addresspicker.h"
#include "emaildlgimpl.h"

#include <qtopia/categories.h> // needed to get correct WIN32 exports for QValueList<int>
#include <qtopia/categoryselect.h>
#include <qtopia/qpeapplication.h>
#include <qtopia/qpedialog.h>
#include <qtopia/datetimeedit.h>

#include <qcombobox.h>
#include <qlabel.h>
#include <qstyle.h>
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

#ifdef QTOPIA_DESKTOP
#include <qtextedit.h>
#endif

// Make QScrollView in AutoOneFit mode use the minimum horizontal size
// instead of the sizeHint() so that the widgets fit horizontally.
class VScrollBox : public QVBox
{
public:
    VScrollBox( QWidget *parent, const char *name=0 )
	: QVBox( parent, name ) {}
    QSize sizeHint() const {
	int width = QVBox::sizeHint().width();
	if ( width > qApp->desktop()->width()-style().scrollBarExtent().width() )
	    width = qApp->desktop()->width()-style().scrollBarExtent().width();
	width = QMAX(width, QVBox::minimumSize().width());
	return QSize( width, QVBox::sizeHint().height());
    }
};


// helper functions, convert our comma delimited list to proper
// file format...
void parseEmailFrom( const QString &txt, QString &strDefaultEmail,
		     QStringList &strAll );

// helper convert from file format to comma delimited...
void parseEmailTo( const QString &strDefaultEmail,
		   const QStringList &emails, QString &strBack );

class FileAsCombo : public QComboBox
{
    Q_OBJECT
public:
    FileAsCombo( AbEditor *editor, QWidget *container );
    void reload();
    void generateFileAsOptions();

    bool userOverride;
    bool programaticUpdate;
    bool reloading;

public slots:
    void autoUpdate();
    void userChanged( const QString &);

protected:

private:
    enum ChoiceGroup { LastFirst=0, FirstLast,
		       Business, Other };
    void addOption( int index, const QString &str, ChoiceGroup group );

    AbEditor *e;
    QStringList options;
    QMap<int, QStringList::Iterator > indexedOptions;
    QMap<QString, int> indexLookup;
    int curIndex;
    ChoiceGroup curGroup;
    int curNumGroupChoices;
    int lastNumGroupChoices;
    QMap<ChoiceGroup, int> bestChoiceIndex;
    QMap<int, ChoiceGroup> indexedGroups;
};

FileAsCombo::FileAsCombo( AbEditor *editor, QWidget *parent ) :
    QComboBox( parent, "FileAsCombo" ),
    userOverride( FALSE ), programaticUpdate( FALSE ), reloading( FALSE ),
    e( editor ),
    options(), indexedOptions(), curIndex( -1 ), curGroup( LastFirst ),
    curNumGroupChoices(-1), lastNumGroupChoices(-1), bestChoiceIndex()
{
    setInsertionPolicy( QComboBox::AtTop );
    setAutoCompletion( TRUE );
    setEditable( TRUE );
    setDuplicatesEnabled( FALSE );

    connect( this, SIGNAL( textChanged( const QString &) ),
	     SLOT( userChanged( const QString &) ) );

}


void FileAsCombo::userChanged( const QString &newText )
{
    if ( programaticUpdate || reloading )
	return;

    QMap<QString,int>::Iterator found = indexLookup.find( newText );
    //qDebug( "FileAsCombo::userChanged newText = %s curIndex = %d found = %d",
    //    newText.latin1(), curIndex, *found );

    if ( found == indexLookup.end() ) {
	userOverride = TRUE;
	curIndex = -1;
	lastNumGroupChoices = -1;
    }
    else {
	userOverride = FALSE;
	curIndex = *found;
	//qDebug( " userChanged indexedGroups.contains(curIndex) == %d",
//		indexedGroups.contains(curIndex) );
	if ( curGroup != indexedGroups[curIndex] ) {
	    curGroup = indexedGroups[curIndex];
	    curNumGroupChoices = 0;
	    for ( QMap<int, ChoiceGroup>::Iterator it = indexedGroups.begin();
		  it != indexedGroups.end(); ++it ) {
		if ( *it == curGroup )
		    curNumGroupChoices++;
	    }
	}
    }

//qDebug( " curIndex = %d cur group = %d curNumGroupChoices = %d", curIndex, curGroup, curNumGroupChoices );

}

void FileAsCombo::addOption( int index, const QString &str, ChoiceGroup group )
{
    QStringList::Iterator it = options.append( str );
    indexedOptions.insert( index, it );
    indexLookup.insert( *it, index );

    indexedGroups.insert( index, group );
    //qDebug("addOption %s index = %d group = %d", str.latin1(), index, group );
    if ( !bestChoiceIndex.contains( group ) )
	bestChoiceIndex.insert( group, index );
    if ( group == curGroup )
	curNumGroupChoices++;
}

void FileAsCombo::generateFileAsOptions()
{
    options.clear();
    indexedOptions.clear();
    bestChoiceIndex.clear();
    indexedGroups.clear();
    indexLookup.clear();
    curNumGroupChoices = 0;

    QString nickName = e->lineEdits[ PimContact::Nickname ]->text();
    QString firstName = e->lineEdits[ PimContact::FirstName ]->text();
    QString middleName = e->lineEdits[ PimContact::MiddleName ]->text();
    QString lastName = e->lineEdits[PimContact::LastName ]->text();
    QString suffix = e->suffixCombo->currentText();
    QString company = e->lineEdits[PimContact::Company]->text();

    bool bfirstName = !firstName.isEmpty();
    bool bnickName = !nickName.isEmpty();
    bool bmiddleName = !middleName.isEmpty();
    bool blastName = !lastName.isEmpty();
    bool bsuffix = !suffix.isEmpty();

    int i = 0;
    // last, first middle
    ChoiceGroup g = LastFirst;
    if ( bfirstName && bmiddleName && blastName )
	addOption( i, lastName + ", " + firstName + " " + middleName, g );

    // last, first
    ++i;
    if ( bfirstName && blastName )
	addOption( i, lastName + ", " + firstName, g );

    // last, first suffix
    ++i;
    if ( bfirstName && blastName && bsuffix )
	addOption( i, lastName + ", " + firstName + " " + suffix, g );

    // last, nick
    ++i;
    if ( bnickName && blastName )
	addOption( i, lastName + ", " + nickName, g );

    g = FirstLast;
    // first middle last suffix
    ++i;
    if ( bfirstName && bmiddleName && blastName && bsuffix )
	addOption( i,  firstName + " " + middleName + " " + lastName + " " + suffix, g );

    // first middle last
    ++i;
    if ( bfirstName && bmiddleName && blastName )
	addOption( i, firstName + " " + middleName + " " + lastName, g );


    // first last
    ++i;
    if ( bfirstName && blastName )
	addOption( i, firstName + " " + lastName, g );

    // first last suffix
    ++i;
    if ( bfirstName && blastName && bsuffix )
	addOption( i, firstName + " " + lastName + " " + suffix, g );


    // nick last
    ++i;
    if ( bnickName && blastName )
	addOption(i,  bnickName + " " + lastName, g );

    g = LastFirst;
    ++i;
    if ( bfirstName )
	addOption( i, firstName, g);

    ++i;
    if ( bnickName )
	addOption( i, nickName, g );

    ++i;
    if ( blastName )
	addOption( i, lastName, g );

    // company
    g = Business;
    ++i;
    if ( !company.isEmpty() )
	addOption( i, company, g );

    // okay, no names so starting to get desperate
    if ( !options.count() ) {
	// let's find out what fields we have that are not empty and choose the
	// most unique one from our uniqueness map
	QString mostUnique;
	int mostUniqueVal = 0;
	for ( QMap<int, QLineEdit *>::ConstIterator lit = e->lineEdits.begin();
	      lit != e->lineEdits.end(); ++lit ) {
	    if ( !(*lit)->text().isEmpty() && PimContact::uniquenessMap()[ lit.key() ] > mostUniqueVal  ) {
		mostUnique = (*lit)->text();
		mostUniqueVal = PimContact::uniquenessMap()[ lit.key() ];
	    }
	}

	if ( !mostUnique.isEmpty() ) {
	    ++i;
	    addOption( i,  mostUnique, Other );
	}
    }

}

void FileAsCombo::autoUpdate()
{
    if ( programaticUpdate && !reloading )
	return;
    //qDebug(" userOverride = %d programaticUpdate =%d reload = %d", userOverride, programaticUpdate, reloading );

    // set programaticUpdate so userChanged doesn't get called; the user
    // isn't changing it, we are
    programaticUpdate = TRUE;

    // remember what was selected
    QString curText = currentText();
    lastNumGroupChoices = curNumGroupChoices;
    // generate a new list of options
    generateFileAsOptions();

    // make an intelligent choice of what should be shown in the
    // combo box
    //qDebug("\t before new index; last = %d current = %d", lastNumGroupChoices, curNumGroupChoices );
    if ( !userOverride && options.count() ) {
	if (  lastNumGroupChoices != curNumGroupChoices
	     || curIndex == -1 || !indexedOptions.contains( curIndex ) ) {
	    if ( bestChoiceIndex.contains( curGroup ) )
		curIndex = bestChoiceIndex[ curGroup ];
	    else curIndex = indexedOptions.begin().key();
	    curGroup = indexedGroups[curIndex];

	    //qDebug("\tpicking smart choice for index; new index = %d group = %d", curIndex, curGroup);
	}

	curText = *(indexedOptions[curIndex]);
	//qDebug( "setting curText = %s using index = %d", curText.latin1(), curIndex);
    }

    clear();
    insertStringList( options );
    setEditText( curText );

    programaticUpdate = FALSE;
}

void FileAsCombo::reload()
{
    // inialize the file as for the first time for editing
    userOverride = FALSE;

    curGroup = LastFirst;
    reloading = TRUE;
    curIndex = -1;
    lastNumGroupChoices = -1;
    curNumGroupChoices = -1;
    setEditText( "" );
    autoUpdate();
    reloading = FALSE;
    userChanged( e->ent.fileAs() );
    autoUpdate();
}


AbEditor::AbEditor( QWidget *parent, const char *name, WFlags fl)
    : QDialog( parent, name, TRUE, fl )
{
    fileAsCombo = 0;
    init();
    resize( 400, 500 );
    setCaption( tr("Edit Contact Details") );
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
    // setup which fields go in which tabs and in which order; this is done first since it is
    // most likely to change and we can calculate the QGridLayout num rows for each tab

    // personal tab keys
    QValueList<int> personalTabKeys;
    personalTabKeys.append( PimContact::NameTitle );
    personalTabKeys.append( PimContact::FirstName );
    personalTabKeys.append( PimContact::FirstNamePronunciation );
    personalTabKeys.append( PimContact::MiddleName );
    personalTabKeys.append( PimContact::LastName );
    personalTabKeys.append( PimContact::LastNamePronunciation );
    personalTabKeys.append( PimContact::Suffix );
    personalTabKeys.append( PimContact::Nickname );
    personalTabKeys.append( PimContact::FileAs );
    personalTabKeys.append( PimContact::Categories );
    personalTabKeys.append( PimContact::Emails );
    personalTabKeys.append( PimContact::Birthday );
    personalTabKeys.append( PimContact::Gender );

    // business tab keys
    QValueList<int> businessTabKeys;
    businessTabKeys.append( PimContact::Company );
    businessTabKeys.append( PimContact::CompanyPronunciation );
    businessTabKeys.append( PimContact::JobTitle );
    businessTabKeys.append( PimContact::BusinessPhone );
    businessTabKeys.append( PimContact::BusinessFax );
    businessTabKeys.append( PimContact::BusinessMobile );
    businessTabKeys.append( PimContact::BusinessPager );

    businessTabKeys.append( PimContact::BusinessStreet );
    businessTabKeys.append( PimContact::BusinessCity );
    businessTabKeys.append( PimContact::BusinessState );
    businessTabKeys.append( PimContact::BusinessZip );
    businessTabKeys.append( PimContact::BusinessCountry );

    businessTabKeys.append( PimContact::BusinessWebPage );
    businessTabKeys.append( PimContact::Department );
    businessTabKeys.append( PimContact::Office );
    businessTabKeys.append( PimContact::Profession );
    businessTabKeys.append( PimContact::Manager );
    businessTabKeys.append( PimContact::Assistant );

    // home tab keys
    QValueList<int> homeTabKeys;
    homeTabKeys.append( PimContact::Spouse );
    homeTabKeys.append( PimContact::Anniversary );
    homeTabKeys.append( PimContact::Children );
    homeTabKeys.append( PimContact::HomePhone );
    homeTabKeys.append( PimContact::HomeFax );
    homeTabKeys.append( PimContact::HomeMobile );
    homeTabKeys.append( PimContact::HomeStreet );
    homeTabKeys.append( PimContact::HomeCity );
    homeTabKeys.append( PimContact::HomeState );
    homeTabKeys.append( PimContact::HomeZip );
    homeTabKeys.append( PimContact::HomeCountry );
    homeTabKeys.append( PimContact::HomeWebPage );

    // notes has it's own tab currently; if you want to change this
    // 1. remove the code at the bottom of this section
    // 2. add PimContact::Notes to one section and
    // 3. modify the addFields method to make notes a multi line edit

    QVBoxLayout *vb = new QVBoxLayout( this );

    // setup tab groups
    tabs = new QTabWidget( this );
    vb->addWidget( tabs );

    personalTab = new QWidget( tabs );
    QWidget *businessTab = new QWidget( tabs );
    QWidget *homeTab = new QWidget( tabs );
    QWidget *notesTab = new QWidget( tabs );

#ifdef QTOPIA_DESKTOP
    // ### Should use summary tab if has room, not
    // just compiled for the desktop
    summaryTab = new QWidget( tabs, "summaryTab" );
    tabs->addTab( summaryTab, tr("Summary") );
    connect( tabs, SIGNAL( currentChanged( QWidget *) ), SLOT( tabClicked(QWidget *) ) );

    QVBoxLayout *vbSummary = new QVBoxLayout( summaryTab, 5, 5 );
    summary = new QTextEdit( summaryTab );
    summary->setReadOnly( TRUE );
    vbSummary->addWidget( summary );
#else
    summaryTab = 0;
    summary = 0;
#endif

    tabs->addTab( personalTab, tr("Personal") );
    tabs->addTab( businessTab, tr("Business") );
    tabs->addTab( homeTab, tr("Home") );
    tabs->addTab( notesTab, tr("Notes") );

    // add the key fields to the personal tab
    addFields( addScrollBars( personalTab ), personalTabKeys );
    addFields( addScrollBars( businessTab ), businessTabKeys );
    addFields( addScrollBars( homeTab ), homeTabKeys );

    QWidget *notesContainer = addScrollBars( notesTab );
    QVBoxLayout *vbNote = new QVBoxLayout( notesContainer );
    txtNote = new QMultiLineEdit( notesContainer );
    txtNote->setWordWrap(QMultiLineEdit::WidgetWidth);
    vbNote->addWidget( txtNote );

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

    QWhatsThis::add(lineEdits[PimContact::FirstNamePronunciation],
	tr("Describes the spoken name phonetically."));
    QWhatsThis::add(lineEdits[PimContact::Spouse],
	tr("e.g. Husband or Wife."));
    QWhatsThis::add(fileAsCombo,
	tr("Preferred format for the contact name."));
    QWhatsThis::add(lineEdits[PimContact::Profession],
	tr("Occupation or job description."));

    new QPEDialogListener(this);
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

QWidget *AbEditor::addScrollBars( QWidget *tab )
{
    QScrollView *svPage = new QScrollView( tab );
    QVBoxLayout *vb = new QVBoxLayout( tab );
    vb->addWidget( svPage );
    svPage->setHScrollBarMode( QScrollView::AlwaysOff );
    svPage->setResizePolicy( QScrollView::AutoOneFit );
    svPage->setFrameStyle( QFrame::NoFrame );

    VScrollBox *vsb = new VScrollBox( svPage->viewport() );
    svPage->addChild( vsb );

    QWidget *container = new QWidget ( vsb );
    return container;
}

void AbEditor::addFields( QWidget *container, const QValueList<int> &keys )
{

    // init this combo here, since addFields assumes is already exists
    if ( ! fileAsCombo )
	fileAsCombo = new FileAsCombo( this, container );

    const QMap<int, QString> & displayNames = PimContact::trFieldsMap();
    QGridLayout *gl = new QGridLayout( container, keys.count()+1, 2, 4, 2 );
    gl->setSpacing(1);

    QWidget *lastEditor = 0, *editor;
    int fieldInTabNum=0;
    for ( QValueList<int>::ConstIterator fieldKey = keys.begin();
	  fieldKey != keys.end(); ++fieldKey ) {

	QLabel *label = new QLabel( displayNames[ *fieldKey ], container );

	// do a switch on the field type; most are line edits, the exceptions get a case, the default
	// is a linedit
	editor = 0;
	switch ( *fieldKey ) {
	case PimContact::Suffix:
	    suffixCombo = new QComboBox( container );
	    suffixCombo->setEditable( TRUE );
	    suffixCombo->setDuplicatesEnabled( FALSE );
	    suffixCombo->setAutoCompletion( TRUE );

	    suffixCombo->insertItem( "", 0 );
	    suffixCombo->insertItem( tr( "Jr.", "Person's name suffix" )  );
	    suffixCombo->insertItem( tr( "Sr." )  );
	    suffixCombo->insertItem( tr( "I", "Person's name suffix" )  );
	    suffixCombo->insertItem( tr( "II" )  );
	    suffixCombo->insertItem( tr( "III" )  );
	    suffixCombo->insertItem( tr( "IV" )  );
	    suffixCombo->insertItem( tr( "V" )  );

	    editor = suffixCombo;
	    break;

	case PimContact::NameTitle:
	    titleCombo = new QComboBox( container );
	    titleCombo->setEditable( TRUE );
	    titleCombo->setDuplicatesEnabled( FALSE );
	    titleCombo->setAutoCompletion( TRUE );

	    titleCombo->insertItem( "", 0 );
	    titleCombo->insertItem( tr( "Mr." )  );
	    titleCombo->insertItem( tr( "Mrs." )  );
	    titleCombo->insertItem( tr( "Ms." )  );
	    titleCombo->insertItem( tr( "Miss" )  );
	    titleCombo->insertItem( tr( "Dr." )  );
	    titleCombo->insertItem( tr( "Prof." )  );

	    editor = titleCombo;
	    break;
	case PimContact::Gender:
	    genderCombo = new QComboBox( container );
	    genderCombo->insertItem( "", 0 );
	    genderCombo->insertItem( tr( "Male" ), 1 );
	    genderCombo->insertItem( tr( "Female" ), 2 );
	    editor = genderCombo;
	    break;
	case PimContact::FileAs:
	    editor = fileAsCombo;
	    break;
	case PimContact::Birthday:
	    bdayButton = new QPEDateEdit( container, "", FALSE, TRUE );
	    editor = bdayButton;
	    break;
	case PimContact::DefaultEmail:
	case PimContact::Emails:
	    ehb = new QHBox(container);
	    emailLE = new QLineEdit( ehb );
	    emailBtn = new QPushButton("...", ehb);
	    connect(emailBtn, SIGNAL(clicked()), this, SLOT(editEmails()));
	    editor = ehb;
	    break;
	case PimContact::Anniversary:
	    anniversaryButton = new QPEDateEdit( container, "", FALSE, TRUE );
	    editor = anniversaryButton;
	    break;
	case PimContact::Categories:
	    {
		QArray<int> emptyCat;
		cmbCat = new CategorySelect( emptyCat, "Address Book", // No tr()
			tr("Contacts"), container );
		editor = cmbCat;
		if ( cmbCat->widgetType() == CategorySelect::ListView ) {
		    delete label;
		    label = 0;
		}
	    }
	    break;
	case PimContact::Notes:
	    // this is taken care of later; don't want to create a line edit for it
	    editor = 0;
	    break;
	case PimContact::HomeStreet:
	    {
	    homeStreetEdit = new QMultiLineEdit( container );
	    homeStreetEdit->setWordWrap(QMultiLineEdit::WidgetWidth);
	    int fontSize = homeStreetEdit->font().pixelSize();
	    if (fontSize <= 1)
		fontSize = 10;
	    homeStreetEdit->setMaximumHeight(fontSize*4);
	    editor = homeStreetEdit;
	    }
	    break;
	case PimContact::BusinessStreet:
	    {
	    businessStreetEdit = new QMultiLineEdit( container );
	    businessStreetEdit->setWordWrap(QMultiLineEdit::WidgetWidth);
	    int fontSize = businessStreetEdit->font().pixelSize();
	    if (fontSize <= 1)
		fontSize = 10;
	    businessStreetEdit->setMaximumHeight(fontSize*4);
	    editor = businessStreetEdit;
	    }
	    break;
	default:
	{
	    QLineEdit *lineedit = new QLineEdit( container );
	    lineEdits.insert( *fieldKey, lineedit );
	    connect( lineedit, SIGNAL ( textChanged( const QString & ) ),
		     fileAsCombo, SLOT( autoUpdate() ) );
	    editor = lineedit;
	}
	    break;
	};

	if ( editor ) {

	    // add the editor to the layout
	    if ( label ) {
		gl->addWidget( label, fieldInTabNum, 0 );
		labels.insert( *fieldKey, label );
		gl->addWidget( editor, fieldInTabNum, 1 );
	    }
	    else
		gl->addMultiCellWidget( editor, fieldInTabNum, fieldInTabNum, 0, 1 );
	    if ( lastEditor ) {
		setTabOrder( lastEditor, editor );
	    }
	    lastEditor = editor;

	    // increment the field number for this tab
	    fieldInTabNum++;
	} else delete label;
    }
    QSpacerItem *verticleSpacer = new QSpacerItem( 1, 1, QSizePolicy::Minimum,
						   QSizePolicy::Expanding );
    gl->addItem( verticleSpacer, fieldInTabNum, 1 );

}

void AbEditor::setEntry( const PimContact &entry )
{
    quitExplicitly = FALSE;
    ent = entry;

    // do this first, so then we can ignore the rest of the line edits change events
    fileAsCombo->reload();

    fileAsCombo->programaticUpdate = TRUE;

    // categories not part of map
    cmbCat->setCategories( ent.categories(), "Address Book", // No tr()
	tr("Contacts") );

    // iterate through all display fields for contact
    QMap<int, QCString> i2keyMap = PimContact::keyToIdentifierMap();
    QMap<int, QCString>::Iterator it;
    for( it = i2keyMap.begin(); it != i2keyMap.end(); ++it ) {
	int keyIt = it.key();
	if (keyIt <= PimRecord::PrivateFieldsEnd)
	    continue;

	// put the non-lineedit sets here
	bool isLineEdit = FALSE;
	switch( keyIt ) {
	case PimContact::FileAs:
	    break;
	case PimContact::Gender:
	{
	    switch ( ent.gender() ) {
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
	}
	    break;
	case PimContact::NameTitle:
	    if ( ent.nameTitle().isEmpty() )
		titleCombo->setCurrentItem( 0 );
	    else
		titleCombo->setEditText( ent.nameTitle() );
	    break;
	case PimContact::Suffix:
	    if ( ent.suffix().isEmpty() )
		suffixCombo->setCurrentItem( 0 );
	    else
	    suffixCombo->setEditText( ent.suffix() );
	    break;
	case PimContact::Notes:
	    txtNote->setText( ent.notes() );
	    break;
	case PimContact::HomeStreet:
	    homeStreetEdit->setText( ent.homeStreet() );
	    break;
	case PimContact::BusinessStreet:
	    businessStreetEdit->setText( ent.businessStreet() );
	    break;
	case PimContact::Birthday:
	    bdayButton->setDate( ent.birthday() );
	    break;
	case PimContact::Anniversary:
	    anniversaryButton->setDate( ent.anniversary() );
	    break;
	    // email
	case PimContact::DefaultEmail:
	case PimContact::Emails:
	    {
		QString strDefEmail = ent.defaultEmail();
		QStringList strAllEmail = ent.emailList();
		QString strFinal;
		parseEmailTo( strDefEmail, strAllEmail, strFinal );
		emailLE->setText( strFinal );
		// make sure we see the "default"
		emailLE->home( false );
		break;
	    }

	default:
	    isLineEdit = TRUE;
	}

	// handle all of the line edits together since we
	// can extract the QLineEdit once for all rather than duplicate
	// the extraction
	if ( isLineEdit ) {
	    if ( !lineEdits.contains( keyIt ) ) {
		qWarning("AbEditor Possible Bug: %s has no editor",
			 PimContact::trFieldsMap()[keyIt].latin1() );
		continue;
	    }
	    QLineEdit *le = lineEdits[ keyIt ];
	    switch ( keyIt ) {
	    case PimContact::FirstName:
		le->setText( ent.firstName() );
		break;
	    case PimContact::MiddleName:
		le->setText( ent.middleName() );
		break;
	    case PimContact::LastName:
		le->setText( ent.lastName() );
		break;
	    case PimContact::Suffix:
		le->setText( ent.suffix() );
		break;
	    case PimContact::FirstNamePronunciation:
		le->setText( ent.firstNamePronunciation() );
		break;
	    case PimContact::LastNamePronunciation:
		le->setText( ent.lastNamePronunciation() );
		break;
            // home
	    case PimContact::HomeCity:
		le->setText( ent.homeCity() );
		break;
	    case PimContact::HomeState:
		le->setText( ent.homeState() );
		break;
	    case PimContact::HomeZip:
		le->setText( ent.homeZip() );
		break;
	    case PimContact::HomeCountry:
		le->setText( ent.homeCountry() );
		break;
	    case PimContact::HomePhone:
		le->setText( ent.homePhone() );
		break;
	    case PimContact::HomeFax:
		le->setText( ent.homeFax() );
		break;
	    case PimContact::HomeMobile:
		le->setText( ent.homeMobile() );
		break;
	    case PimContact::HomeWebPage:
		le->setText( ent.homeWebpage() );
		break;

		// business
	    case PimContact::Company:
		le->setText( ent.company() );
		break;
	    case PimContact::CompanyPronunciation:
		le->setText( ent.companyPronunciation() );
		break;
	    case PimContact::BusinessCity:
		le->setText( ent.businessCity() );
		break;
	    case PimContact::BusinessState:
		le->setText( ent.businessState() );
		break;
	    case PimContact::BusinessZip:
		le->setText( ent.businessZip() );
		break;
	    case PimContact::BusinessCountry:
		le->setText( ent.businessCountry() );
		break;
	    case PimContact::BusinessWebPage:
		le->setText( ent.businessWebpage() );
		break;
	    case PimContact::JobTitle:
		le->setText( ent.jobTitle() );
		break;
	    case PimContact::Department:
		le->setText( ent.department() );
		break;
	    case PimContact::Office:
		le->setText( ent.office() );
		break;
	    case PimContact::BusinessPhone:
		le->setText( ent.businessPhone() );
		break;
	    case PimContact::BusinessFax:
		le->setText( ent.businessFax() );
		break;
	    case PimContact::BusinessMobile:
		le->setText( ent.businessMobile() );
		break;
	    case PimContact::BusinessPager:
		le->setText( ent.businessPager() );
		break;
	    case PimContact::Profession:
		le->setText( ent.profession() );
		break;
	    case PimContact::Assistant:
		le->setText( ent.assistant() );
		break;
	    case PimContact::Manager:
		le->setText( ent.manager() );
		break;

		// personal
	    case PimContact::Spouse:
		le->setText( ent.spouse() );
		break;
	    case PimContact::Children:
		le->setText( ent.children() );
		break;
	    case PimContact::Nickname:
		le->setText( ent.nickname() );
		break;
	    }
	}
    }
    fileAsCombo->programaticUpdate = FALSE;
    fileAsCombo->reload();
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
    if ( isEmpty() ) {
	reject();
    } else {
	contactFromFields(ent);
	QDialog::accept();
    }
}

void AbEditor::reject()
{
    quitExplicitly = TRUE;    
    QDialog::reject();
}


bool AbEditor::isEmpty()
{
    // analyze all the fields and make sure there is _something_ there
    // that warrants saving...
    QString text;
    for ( QMap<int, QLineEdit *>::ConstIterator it = lineEdits.begin(); it != lineEdits.end();
	  ++it ) {
	text = (*it)->text();
	if ( !text.stripWhiteSpace().isEmpty() )
	    return FALSE;
    }

    text = txtNote->text();
    if ( !text.stripWhiteSpace().isEmpty() )
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

    e.setNameTitle( titleCombo->currentText() );
    e.setSuffix( suffixCombo->currentText() );
    QString tmpFileAs = fileAsCombo->currentText();
    if (tmpFileAs.isEmpty()) {
	tmpFileAs = fileAsCombo->text(0);
    }
    e.setFileAs( tmpFileAs );
    e.setBirthday( bdayButton->date() );
    e.setAnniversary( anniversaryButton->date() );
    e.setCategories( cmbCat->currentCategories() );
    e.setNotes( txtNote->text() );
    e.setHomeStreet (homeStreetEdit->text());
    e.setBusinessStreet (businessStreetEdit->text());

    QString strDefaultEmail;
    QStringList emails;
    parseEmailFrom( emailLE->text(), strDefaultEmail,
	    emails );
    e.setDefaultEmail( strDefaultEmail );
    e.setEmailList( emails  );

    QString str;
    for ( QMap<int, QLineEdit *>::ConstIterator it = lineEdits.begin(); it != lineEdits.end();
	  ++it ) {
	str = (*it)->text();
	switch ( it.key() ) {
	case PimContact::FirstName:
	    e.setFirstName( str );
	    break;
	case PimContact::MiddleName:
	    e.setMiddleName( str );
	    break;
	case PimContact::LastName:
	    e.setLastName( str );
	    break;
	case PimContact::FirstNamePronunciation:
	    e.setFirstNamePronunciation( str );
	    break;
	case PimContact::LastNamePronunciation:
	    e.setLastNamePronunciation( str );
	    break;
	    // home
	case PimContact::HomeCity:
	    e.setHomeCity( str );
	    break;
	case PimContact::HomeState:
	    e.setHomeState( str );
	    break;
	case PimContact::HomeZip:
	    e.setHomeZip( str );
	    break;
	case PimContact::HomeCountry:
	    e.setHomeCountry( str );
	    break;
	case PimContact::HomePhone:
	    e.setHomePhone( str );
	    break;
	case PimContact::HomeFax:
	    e.setHomeFax( str );
	    break;
	case PimContact::HomeMobile:
	    e.setHomeMobile( str );
	    break;
	case PimContact::HomeWebPage:
	    e.setHomeWebpage( str );
	    break;

	    // business
	case PimContact::Company:
	    e.setCompany( str );
	    break;
	case PimContact::CompanyPronunciation:
	    e.setCompanyPronunciation( str );
	    break;
	case PimContact::BusinessCity:
	    e.setBusinessCity( str );
	    break;
	case PimContact::BusinessState:
	    e.setBusinessState( str );
	    break;
	case PimContact::BusinessZip:
	    e.setBusinessZip( str );
	    break;
	case PimContact::BusinessCountry:
	    e.setBusinessCountry( str );
	    break;
	case PimContact::BusinessWebPage:
	    e.setBusinessWebpage( str );
	    break;
	case PimContact::JobTitle:
	    e.setJobTitle( str );
	    break;
	case PimContact::Department:
	    e.setDepartment( str );
	    break;
	case PimContact::Office:
	    e.setOffice( str );
	    break;
	case PimContact::BusinessPhone:
	    e.setBusinessPhone( str );
	    break;
	case PimContact::BusinessFax:
	    e.setBusinessFax( str );
	    break;
	case PimContact::BusinessMobile:
	    e.setBusinessMobile( str );
	    break;
	case PimContact::BusinessPager:
	    e.setBusinessPager( str );
	    break;
	case PimContact::Profession:
	    e.setProfession( str );
	    break;
	case PimContact::Assistant:
	    e.setAssistant( str );
	    break;
	case PimContact::Manager:
	    e.setManager( str );
	    break;

	    // personal
	case PimContact::Spouse:
	    e.setSpouse( str );
	    break;
	case PimContact::Children:
	    e.setChildren( str );
	    break;
	case PimContact::Nickname:
	    e.setNickname( str );
	    break;
	default:
	    qWarning("AbEditor Possible Bug: %s is not being saved",
		     PimContact::trFieldsMap()[it.key()].latin1() );
	    break;
	}
    }
}

void AbEditor::setNameFocus()
{
    tabs->showPage( personalTab );
    QLineEdit *firstEdit = lineEdits[ PimContact::FirstName ];
    firstEdit->setFocus();
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

#include "abeditor.moc"
