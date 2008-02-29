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

#include "qimpenwidget.h"
#include "qimpensetup.h"
#include "qimpeninput.h"
#include "qimpencombining.h"
#include "qimpenwordpick.h"
#include "qimpenmatch.h"
#include "qimpenhelp.h"

#include <qtopia/qpeapplication.h>
#include <qtopia/qdawg.h>
#include <qtopia/config.h>
#include <qtopia/global.h>

#include <qlayout.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qtimer.h>
#include <qdir.h>

#include <limits.h>

// We'll use little pixmaps for the buttons to save screen space.

/* XPM */
static const char * const pen_xpm[] = {
"12 12 4 1",
" 	c None",
".	c #000000",
"+	c #FFFFFF",
"@	c #808080",
"         .  ",
"        .+. ",
"       ..@@.",
"      .+@.. ",
"     .+@@.  ",
"    .+@@.   ",
"   .+@@.    ",
"  .@.@.     ",
"  .@@.      ",
" ....       ",
" ..         ",
"            "};


/* XPM */
static const char * const bs_xpm[] = {
"12 12 5 1",
" 	c None",
".	c #333333",
"+	c #000000",
"@	c #FFFFFF",
"#	c #666666",
"            ",
"            ",
"            ",
"    .       ",
"   ++       ",
"  +@#+++++. ",
" +@@@@@@@@+ ",
"  +@#+++++. ",
"   ++       ",
"    .       ",
"            ",
"            "};


/* XPM */
static const char * const enter_xpm[] = {
"12 12 5 1",
" 	c None",
".	c #333333",
"+	c #000000",
"@	c #FFFFFF",
"#	c #666666",
"            ",
"        .+. ",
"        +@+ ",
"    .   +@+ ",
"   ++   +@+ ",
"  +@#++++@+ ",
" +@@@@@@@@+ ",
"  +@#+++++. ",
"   ++       ",
"    .       ",
"            ",
"            "};



/* XPM */
static const char * const help_xpm[] = {
"12 12 5 1",
" 	c None",
".	c #000000",
"+	c #FFFFFF",
"@	c #666666",
"#	c #333333",
"            ",
"    ...     ",
"   .+++.    ",
"  .+..@+.   ",
"  #.# .+.   ",
"      .+.   ",
"     .+.    ",
"    .+.     ",
"    .+.     ",
"    #.#     ",
"    .+.     ",
"    #.#     "};


/*!
  \class QIMPenInput qimpeninput.h
  \brief The QIMPenInput class is the pen input widget.
*/
QIMPenInput::QIMPenInput( QWidget *parent, const char *name, WFlags f )
    : QFrame( parent, name, f ), helpDlg(0), profile(0)
{
    setFrameStyle( Box | Plain );

    profileList.setAutoDelete( true );

    matcher = new QIMPenMatch( this );
    connect( matcher, SIGNAL(keypress(uint)), this, SLOT(keypress(uint)) );
    connect( matcher, SIGNAL(erase()), this, SLOT(erase()) );

    QGridLayout *gl = new QGridLayout( this, 5, 2, 1, 0 );
    gl->setColStretch( 0, 1 );

    wordPicker = new QIMPenWordPick( this );
    connect( wordPicker, SIGNAL(wordClicked(const QString &)),
	     this, SLOT(wordPicked(const QString &)) );
    connect( matcher, SIGNAL(matchedCharacters(const QIMPenCharMatchList &)),
	     this, SLOT(matchedCharacters(const QIMPenCharMatchList &)) );
    connect( matcher, SIGNAL(matchedWords(const QIMPenMatch::MatchWordList&)),
	     wordPicker, SLOT(setWords(const QIMPenMatch::MatchWordList&)) );
    QFont smoothFont("smallsmooth",9);
    QFontInfo fi( smoothFont );
    wordPicker->setFont( smoothFont );
    wordPicker->setBackgroundColor( white );
    gl->addMultiCellWidget( wordPicker, 0, 0, 0, 1 );
    if ( !Global::fixedDawg().root() || !matcher->isWordMatchingEnabled() )
	wordPicker->hide();

    pw = new QIMPenWidget( this );
    gl->addMultiCellWidget( pw, 1, 4, 0, 0 );

    int bh = pw->sizeHint().height()/4;

    QPushButton *b = new QPushButton( this );
    b->setFocusPolicy( NoFocus );
    b->setPixmap( QPixmap( (const char **)bs_xpm ) );
    b->setFixedHeight(pw->sizeHint().height()-3*bh); // left-over space goes here
    b->setAutoRepeat( TRUE );
    gl->addWidget( b, 1, 1 );
    connect( b, SIGNAL(clicked()), SLOT(backspace()));

    b = new QPushButton( this );
    b->setFocusPolicy( NoFocus );
    b->setPixmap( QPixmap( (const char **)enter_xpm ) );
    b->setFixedHeight(bh);
    b->setAutoRepeat( TRUE );
    gl->addWidget( b, 2, 1 );
    connect( b, SIGNAL(clicked()), SLOT(enter()));

    helpBtn = new QPushButton( this );
    helpBtn->setFocusPolicy( NoFocus );
    helpBtn->setPixmap( QPixmap( (const char **)help_xpm ) );
    helpBtn->setFixedHeight(bh);
    gl->addWidget( helpBtn, 3, 1 );
    connect( helpBtn, SIGNAL(clicked()), SLOT(help()));

    QPixmap pm( (const char **)pen_xpm );
    setupBtn = new QPushButton( this );
    setupBtn->setFocusPolicy( NoFocus );
    setupBtn->setPixmap( pm );
    setupBtn->setFixedHeight(bh);
    gl->addWidget( setupBtn, 4, 1 );
    connect( setupBtn, SIGNAL(clicked()), SLOT(setup()));

    connect( matcher, SIGNAL(removeStroke()), pw, SLOT(removeStroke()) );
    connect( pw, SIGNAL(changeCharSet( QIMPenCharSet * )),
             matcher, SLOT(setCharSet( QIMPenCharSet * )) );
    connect( pw, SIGNAL(changeCharSet( int )),
             this, SLOT(selectCharSet( int )) );
    connect( pw, SIGNAL(beginStroke()),
	     matcher, SLOT(beginStroke()) );
    connect( pw, SIGNAL(stroke( QIMPenStroke * )),
             this, SLOT(strokeEntered( QIMPenStroke * )) );
    connect( pw, SIGNAL(stroke( QIMPenStroke * )),
             matcher, SLOT(strokeEntered( QIMPenStroke * )) );

    shortcutCharSet = 0;
    currCharSet = 0;
    setupDlg = 0;
    profile = 0;
    mode = Normal;

    loadProfiles();
}

QIMPenInput::~QIMPenInput()
{
    delete shortcutCharSet;
    delete (HandwritingHelp*) helpDlg;
    delete setupDlg;
}

QSize QIMPenInput::sizeHint() const
{
    int fw = frameWidth();
    int ps = wordPicker->isHidden() ? 0 : wordPicker->sizeHint().height();
    return pw->sizeHint() + QSize( fw*2, fw*2+ps );
}

void QIMPenInput::loadProfiles()
{
    profileList.clear();
    profile = 0;
    delete shortcutCharSet;
    shortcutCharSet = new QIMPenCharSet();
    shortcutCharSet->setTitle( tr("Shortcut") );
    QString path = QPEApplication::qpeDir() + "etc/qimpen";
    QDir dir( path, "*.conf" );
    QStringList list = dir.entryList();
    QStringList::Iterator it;
    for ( it = list.begin(); it != list.end(); ++it ) {
	QIMPenProfile *p = new QIMPenProfile( path + "/" + *it );
	profileList.append( p );
	if ( p->shortcut() ) {
	    QIMPenCharIterator it( p->shortcut()->characters() );
	    for ( ; it.current(); ++it ) {
		shortcutCharSet->addChar( new QIMPenChar(*it.current()) );
	    }
	}
    }

    Config config( "handwriting" );
    config.setGroup( "Settings" );
    QString prof = config.readEntry( "Profile", "Default" );
    selectProfile( prof );
}

void QIMPenInput::selectProfile( const QString &name )
{
    QListIterator<QIMPenProfile> it( profileList );
    for ( ; it.current(); ++it ) {
	if ( it.current()->name() == name ) {
	    profile = it.current();
	    break;
	}
    }

    if ( !it.current() )
	return;

    pw->clearCharSets();
    baseSets.clear();

    matcher->setMultiStrokeTimeout( profile->multiStrokeTimeout() );
    matcher->setWordMatchingEnabled( profile->matchWords() );

    if ( !Global::fixedDawg().root() || !matcher->isWordMatchingEnabled() )
	wordPicker->hide();
    else
	wordPicker->show();

    if ( profile->uppercase() && profile->style() == QIMPenProfile::BothCases ) {
	baseSets.append( profile->uppercase() );
	pw->insertCharSet( profile->uppercase() );
    }

    if ( profile->lowercase() ) {
	baseSets.append( profile->lowercase() );
	pw->insertCharSet( profile->lowercase(), profile->style() == QIMPenProfile::BothCases ? 1 : 2 );
    }

    if ( profile->numeric() ) {
	baseSets.append( profile->numeric() );
	pw->insertCharSet( profile->numeric() );
    }

    if ( helpDlg )
	delete (HandwritingHelp*) helpDlg;
}

void QIMPenInput::wordPicked( const QString &w )
{
    int bs = matcher->word().length();
    int i;
    for ( i = 0; i < bs; i++ )
	keypress( Qt::Key_Backspace << 16 );

    for ( i = 0; i < (int)w.length(); i++ )
	keypress( w[i].unicode() );

    matcher->resetState();
    wordPicker->clear();
}

void QIMPenInput::selectCharSet( int idx )
{
    if ( mode == Switch ) {
	//qDebug( "Switch back to normal" );
	pw->changeCharSet( baseSets.at(currCharSet), currCharSet );
	mode = Normal;
    }
    currCharSet = idx;
}

void QIMPenInput::beginStroke()
{
}

void QIMPenInput::strokeEntered( QIMPenStroke * )
{
    pw->greyStroke();
}

void QIMPenInput::erase()
{
    keypress( Qt::Key_Backspace << 16 );
}

void QIMPenInput::matchedCharacters( const QIMPenCharMatchList &cl )
{
    const QIMPenChar *ch = cl.first().penChar;
    int scan = ch->character() >> 16;

    if ( scan < QIMPenChar::ModeBase )
	return;

    // We matched a special character...

    switch ( scan ) {
	case QIMPenChar::Caps:
	    if ( profile->style() == QIMPenProfile::ToggleCases ) {
//		qDebug( "Caps" );
		if ( mode == SwitchLock ) {
//		    qDebug( "Switch to normal" );
		    pw->changeCharSet( profile->lowercase(), currCharSet );
		    mode = Switch;
		} else {
//		    qDebug( "Switch to upper" );
		    pw->changeCharSet( profile->uppercase(), currCharSet );
		    mode = Switch;
		}
	    }
	    break;
	case QIMPenChar::CapsLock:
	    if ( profile->style() == QIMPenProfile::ToggleCases ) {
//		qDebug( "CapsLock" );
		if ( mode == Switch &&
		     baseSets.at(currCharSet) == profile->uppercase() ) {
//		    qDebug( "Switch to normal" );
		    pw->changeCharSet( profile->lowercase(), currCharSet );
		    // change our base set back to lower.
		    baseSets.remove( currCharSet );
		    baseSets.insert( currCharSet, profile->lowercase() );
		    mode = Normal;
		} else {		
//		    qDebug( "Switch to caps lock" );
		    pw->changeCharSet( profile->uppercase(), currCharSet );
		    // change our base set to upper.
		    baseSets.remove( currCharSet );
		    baseSets.insert( currCharSet, profile->uppercase() );
		    mode = SwitchLock;
		}
	    }
	    break;
	case QIMPenChar::Punctuation:
	    if ( profile->punctuation() ) {
		//qDebug( "Switch to punctuation" );
		pw->changeCharSet( profile->punctuation(), currCharSet );
		mode = Switch;
	    }
	    break;
	case QIMPenChar::Symbol:
	    if ( profile->symbol() ) {
		//qDebug( "Switch to symbol" );
		pw->changeCharSet( profile->symbol(), currCharSet );
		mode = Switch;
	    }
	    break;
	case QIMPenChar::Shortcut:
	    if ( shortcutCharSet ) {
		pw->changeCharSet( shortcutCharSet, currCharSet );
		mode = Switch;
	    }
	    break;
	case QIMPenChar::Extended:
	    handleExtended( ch->data() );
	    break;
    }
}

void QIMPenInput::keypress( uint scan_uni )
{
    int scan = scan_uni >> 16;
    if ( !scan ) {
	if ( scan_uni >= 'a' && scan_uni <= 'z' ) {
	    scan = Qt::Key_A + scan_uni - 'a';
	} else if ( scan_uni >= 'A' && scan_uni <= 'Z' ) {
	    scan = Qt::Key_A + scan_uni - 'A';
	} else if ( scan_uni == ' ' ) {
	    scan = Qt::Key_Space;
	}
    }

    switch ( scan ) {
	case Key_Tab:
	    scan_uni = 9;
	    break;
	case Key_Return:
	    scan_uni = 13;
	    break;
	case Key_Backspace:
	    scan_uni = 8;
	    break;
	case Key_Escape:
	    scan_uni = 27;
	    break;
	default:
	    break;
    }

    if ( mode == Switch ) {
//	qDebug( "Switch back to normal" );
	pw->changeCharSet( baseSets.at(currCharSet), currCharSet );
	if ( baseSets.at(currCharSet) == profile->uppercase() )
	    mode = SwitchLock;
	else
	    mode = Normal;
    }

    emit key( scan_uni&0xffff, scan, 0, true, false );
    emit key( scan_uni&0xffff, scan, 0, false, false );
}

void QIMPenInput::handleExtended( const QString &ex )
{
    if ( ex.find( "Select" ) == 0 ) {
	QString set = ex.mid( 7 );
	qDebug( "Select new profile: %s", set.latin1() );
	selectProfile( set );
    }
}

void QIMPenInput::help()
{
    if ( helpDlg )
	delete (HandwritingHelp*) helpDlg;
    helpDlg = new HandwritingHelp( profile, 0, 0, WDestructiveClose );
    helpDlg->showMaximized();
    helpDlg->show();
    helpDlg->raise();
}

/*!
  Open the setup dialog
*/
void QIMPenInput::setup()
{
    if ( !setupDlg ) {
	// We are working with our copy of the char sets here.
	setupDlg = new QIMPenSetup( profile, 0, 0, TRUE );
	if ( qApp->desktop()->width() < 640 )
	    setupDlg->showMaximized();
	Global::hideInputMethod();
	setupDlg->exec();
	loadProfiles();
	delete setupDlg;
	setupDlg = 0;
	Global::showInputMethod();
    } else {
	setupDlg->raise();
    }
}

void QIMPenInput::backspace()
{
    keypress( Qt::Key_Backspace << 16 );
    matcher->backspace();
}

void QIMPenInput::enter()
{
    keypress( Qt::Key_Return << 16 );
    matcher->resetState();
}


void QIMPenInput::resetState()
{
    matcher->resetState();
}
