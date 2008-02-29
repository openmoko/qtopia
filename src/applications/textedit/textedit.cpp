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

#include "textedit.h"

#include <qtopia/global.h>
#include <qtopia/fileselector.h>
#include <qtopia/applnk.h>
#include <qtopia/resource.h>
#include <qtopia/config.h>
#include <qtopia/qpeapplication.h>
#include <qtopia/qpemenubar.h>
#include <qtopia/qpetoolbar.h>
#include <qtopia/docproperties.h>
#include <qtopia/fontdatabase.h>

#include <qaction.h>
#include <qcolordialog.h>
#include <qfileinfo.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qobjectlist.h>
#include <qpopupmenu.h>
#include <qspinbox.h>
#include <qtextcodec.h>
#include <qtoolbutton.h>
#include <qwidgetstack.h>
#include <qclipboard.h>
#include <qwhatsthis.h>

#include <stdlib.h> //getenv



#if QT_VERSION < 0x030000

class QpeEditor : public QMultiLineEdit
{
    Q_OBJECT
public:
    QpeEditor( QWidget *parent, const char * name = 0 )
	: QMultiLineEdit( parent, name )
        {
            clearTableFlags();
            setTableFlags( Tbl_vScrollBar | Tbl_autoHScrollBar );
        }

    void find( const QString &txt, bool caseSensitive );

signals:
    void findNotFound();
    void findFound();
    void findWrapped();

private:
    
};


void QpeEditor::find ( const QString &txt, bool caseSensitive )
{
    static bool wrap = FALSE;
    static QString lastTxt;
    int line = 0;
    int col = 0;
    if ( lastTxt != txt )
	wrap = FALSE;
    if ( lastTxt != txt.left(lastTxt.length()) ) {
	line = col = 0;
    } else {
	getCursorPosition( &line, &col );
	if ( lastTxt != txt )
	    col -= lastTxt.length();
    }
    for (;;) {
	if ( line >= numLines() ) {
	    setCursorPosition( 0, 0, FALSE );
	    line = col = 0;
	    if ( wrap ) 
		emit findWrapped();
	    else
		emit findNotFound();
	    break;
	}
	int findCol = getString( line )->find( txt, col, caseSensitive );
	if ( findCol >= 0 ) {
	    col = findCol;
	    setCursorPosition( line, col, FALSE );
	    setCursorPosition( line, col+txt.length(), TRUE );
	    emit findFound();
	    wrap = TRUE;
	    break;
	}
	line++;
	col = 0;
    }
    lastTxt = txt;
}


#else

#error "Must make a QpeEditor that inherits QTextEdit"

#endif




static int u_id = 1;
static int get_unique_id()
{
    return u_id++;
}

static int nfontsizes;
static int *fontsize;

TextEdit::TextEdit( QWidget *parent, const char *name, WFlags f )
    : QMainWindow( parent, name, f )
{
    connect(qApp, SIGNAL(appMessage(const QCString&, const QByteArray&)),
	    this, SLOT(message(const QCString&, const QByteArray&)));

    doc = 0;

    setToolBarsMovable( FALSE );
    setBackgroundMode( PaletteButton );

    setIcon( Resource::loadPixmap( "TextEditor" ) );

    QPEToolBar *bar = new QPEToolBar( this );
    bar->setHorizontalStretchable( TRUE );
    menu = bar;

    QPEMenuBar *mb = new QPEMenuBar( bar );
    QPopupMenu *file = new QPopupMenu( this );
    QPopupMenu *edit = new QPopupMenu( this );
    QPopupMenu *font = new QPopupMenu( this );

    mb->insertItem( tr( "File" ), file );
    mb->insertItem( tr( "Edit" ), edit );
    mb->insertItem( tr( "View" ), font );

    bar = new QPEToolBar( this );
    editBar = bar;

    QAction *a = new QAction( tr( "New" ), Resource::loadIconSet( "new" ), QString::null, 0, this, 0 );
    connect( a, SIGNAL( activated() ), this, SLOT( fileNew() ) );
    a->setWhatsThis( tr("Create a new text document.") );
    a->addTo( bar );
    a->addTo( file );

    a = new QAction( tr( "Open" ), Resource::loadIconSet( "fileopen" ), QString::null, 0, this, 0 );
    connect( a, SIGNAL( activated() ), this, SLOT( fileOpen() ) );
    a->setWhatsThis( tr("Open a document.") );
    a->addTo( bar );
    a->addTo( file );

    a = new QAction( tr( "Properties" ), Resource::loadIconSet( "mediaplayer/info" ), QString::null, 0, this, 0 );
    connect( a, SIGNAL( activated() ), this, SLOT( fileName() ) );
    a->setWhatsThis( tr("Change properties") );
    a->addTo( bar );
    a->addTo( file );

    QAction *cutAction = new QAction( tr( "Cut" ), Resource::loadIconSet( "cut" ), QString::null, 0, this, 0 );
    connect( cutAction, SIGNAL( activated() ), this, SLOT( editCut() ) );
    cutAction->setWhatsThis( tr("Cut the currently selected text and move it to the clipboard.") );
    cutAction->setEnabled( FALSE );
    cutAction->addTo( editBar );
    cutAction->addTo( edit );

    QAction *copyAction = new QAction( tr( "Copy" ), Resource::loadIconSet( "copy" ), QString::null, 0, this, 0 );
    connect( copyAction, SIGNAL( activated() ), this, SLOT( editCopy() ) );
    copyAction->setWhatsThis( tr("Copy the currently selected text to the clipboard.") );
    copyAction->setEnabled( FALSE );
    copyAction->addTo( editBar );
    copyAction->addTo( edit );

    pasteAction = new QAction( tr( "Paste" ), Resource::loadIconSet( "paste" ), QString::null, 0, this, 0 );
    connect( pasteAction, SIGNAL( activated() ), this, SLOT( editPaste() ) );
    pasteAction->setWhatsThis( tr("Paste the text in the clipboard at the cursor position.") );
    pasteAction->addTo( editBar );
    pasteAction->addTo( edit );
    clipboardChanged();

    a = new QAction( tr( "Find" ), Resource::loadIconSet( "find" ), QString::null, 0, this, 0 );
    a->setToggleAction( TRUE );
    connect( a, SIGNAL(toggled(bool)), this, SLOT(editFind(bool)) );
    a->setWhatsThis( tr("Click to find text in the document.\nClick again to hide the search bar.") );
    edit->insertSeparator();
    a->addTo( bar );
    a->addTo( edit );

    int defsize;
    bool defb, defi, wrap;
    {
	Config cfg("TextEdit");
	cfg.setGroup("View");
	defsize = cfg.readNumEntry("FontSize",10);
	defb = cfg.readBoolEntry("Bold",FALSE);
	defi = cfg.readBoolEntry("Italic",FALSE);
	wrap = cfg.readBoolEntry("Wrap",TRUE);
    }

    editorStack = new QWidgetStack( this );
    setCentralWidget( editorStack );

    editor = new QpeEditor( editorStack );
    editor->setFrameStyle( QFrame::NoFrame );
    editorStack->addWidget( editor, get_unique_id() );
    connect( editor, SIGNAL(findWrapped()), this, SLOT(findWrapped()) );
    connect( editor, SIGNAL(findNotFound()), this, SLOT(findNotFound()) );
    connect( editor, SIGNAL(findFound()), this, SLOT(findFound()) );
    connect( editor, SIGNAL(copyAvailable(bool)), cutAction, SLOT(setEnabled(bool)) );
    connect( editor, SIGNAL(copyAvailable(bool)), copyAction, SLOT(setEnabled(bool)) );


    setupFontSizes();

    zin = new QAction( tr("Zoom in"), QString::null, 0, this, 0 );
    connect( zin, SIGNAL( activated() ), this, SLOT( zoomIn() ) );
    zin->setWhatsThis( tr("Increases the font size.") );
    zin->addTo( font );

    zout = new QAction( tr("Zoom out"), QString::null, 0, this, 0 );
    connect( zout, SIGNAL( activated() ), this, SLOT( zoomOut() ) );
    zout->setWhatsThis( tr("Decreases the font size.") );
    zout->addTo( font );

    font->insertSeparator();

#if 0    
    QAction *ba = new QAction( tr("Bold"), QString::null, 0, this, 0 );
    connect( ba, SIGNAL( toggled(bool) ), this, SLOT( setBold(bool) ) );
    ba->setToggleAction(TRUE);
    ba->addTo( font );

    QAction *ia = new QAction( tr("Italic"), QString::null, 0, this, 0 );
    connect( ia, SIGNAL( toggled(bool) ), this, SLOT( setItalic(bool) ) );
    ia->setToggleAction(TRUE);
    ia->addTo( font );

    ba->setOn(defb);
    ia->setOn(defi);
    
    font->insertSeparator();
#endif

    QAction *wa = new QAction( tr("Wrap lines"), QString::null, 0, this, 0 );
    connect( wa, SIGNAL( toggled(bool) ), this, SLOT( setWordWrap(bool) ) );
    wa->setWhatsThis( tr("Break long lines into two or more lines.") );
    wa->setToggleAction(TRUE);
    wa->addTo( font );

    QAction *fixed = new QAction( tr("Fixed-width"), QString::null,0,this,0);
    connect( fixed, SIGNAL(toggled(bool) ), this, SLOT( setFixedWidth(bool)));
    fixed->setWhatsThis( tr("Fixed-width fonts make some documents more readable.") );
    fixed->setToggleAction(TRUE);
    fixed->addTo( font );

    searchBar = new QPEToolBar(this);
    addToolBar( searchBar,  tr("Search"), QMainWindow::Top, TRUE );

    searchBar->setHorizontalStretchable( TRUE );

    searchEdit = new QLineEdit( searchBar, "searchEdit" );
    searchBar->setStretchableWidget( searchEdit );
    connect( searchEdit, SIGNAL( textChanged( const QString & ) ),
			 this, SLOT( search() ) );
    connect( searchEdit, SIGNAL(returnPressed()), this, SLOT(search()) );

    a = new QAction( tr( "Find Next" ), Resource::loadIconSet( "next" ), QString::null, 0, this, 0 );
    connect( a, SIGNAL(activated()), this, SLOT(search()) );
    a->setWhatsThis( tr("Find the next occurrence of the search text.") );
    a->addTo( searchBar );

    searchBar->hide();
				
    searchVisible = FALSE;

    fileSelector = new FileSelector( "text/*", editorStack, "fileselector" ,
	    TRUE, FALSE );
    connect( fileSelector, SIGNAL( closeMe() ), this, SLOT( showEditTools() ) );
    connect( fileSelector, SIGNAL( newSelected( const DocLnk &) ), this, SLOT( newFile( const DocLnk & ) ) );
    connect( fileSelector, SIGNAL( fileSelected( const DocLnk &) ), this, SLOT( openFile( const DocLnk & ) ) );
    fileOpen();

    connect( qApp->clipboard(), SIGNAL(dataChanged()), this, SLOT(clipboardChanged()) );
    connect( qApp, SIGNAL(linkChanged(const QString&)), this, SLOT(linkChanged(const QString&)) );

    resize( 200, 300 );

    variableFontSize = defsize;
    zoomOutLast = TRUE;
    setFontSize(defsize,zoomOutLast);
    wa->setOn(wrap);
}

TextEdit::~TextEdit()
{
    if (fontsize)
	delete [] fontsize;

    save();

    Config cfg("TextEdit");
    cfg.setGroup("View");
    QFont f = editor->font();
    cfg.writeEntry("FontSize",f.pointSize());
    cfg.writeEntry("Bold",f.bold());
    cfg.writeEntry("Italic",f.italic());
    cfg.writeEntry("Wrap",editor->wordWrap() == QMultiLineEdit::WidgetWidth);
}

//
// Figure out how many "zoom" levels there for the given font.
//
void
TextEdit::setupFontSizes(void)
{
    FontDatabase	fd;
    QValueList<int> pointSizes = fd.pointSizes(editor->font().family().lower());
    QValueList<int>::Iterator it;

    nfontsizes = pointSizes.count();
    fontsize = new int[nfontsizes];

    if (fontsize) {
	int i = 0;
	for (it = pointSizes.begin(); it != pointSizes.end();
	    ++it) {
	    int foo = *it;
	    fontsize[i++] = foo;
	}
    } else {
	nfontsizes = 0;
    }
}

void TextEdit::zoomIn()
{
    zoomOutLast = FALSE;
    variableFontSize = editor->font().pointSize()+1;
    setFontSize(variableFontSize,FALSE);
}

void TextEdit::zoomOut()
{
    zoomOutLast = TRUE;
    variableFontSize = editor->font().pointSize()-1;
    setFontSize(variableFontSize,TRUE);
}


void TextEdit::setFontSize(int sz, bool round_down_not_up)
{
    int s=10;
    for (int i=0; i<nfontsizes; i++) {
	if ( fontsize[i] == sz ) {
	    s = sz;
	    break;
	} else if ( round_down_not_up ) {
	    if ( fontsize[i] < sz )
		s = fontsize[i];
	} else {
	    if ( fontsize[i] > sz ) {
		s = fontsize[i];
		break;
	    }
	}
    }

    QFont f = editor->font();
    f.setPointSize(s);
    editor->setFont(f);

    zin->setEnabled(s != fontsize[nfontsizes-1]);
    zout->setEnabled(s != fontsize[0]);
}

void TextEdit::setBold(bool y)
{
    QFont f = editor->font();
    f.setBold(y);
    editor->setFont(f);
}

void TextEdit::setItalic(bool y)
{
    QFont f = editor->font();
    f.setItalic(y);
    editor->setFont(f);
}

void TextEdit::setWordWrap(bool y)
{
    bool state = editor->edited();
    editor->setWordWrap(y ? QMultiLineEdit::WidgetWidth : QMultiLineEdit::NoWrap );
    editor->setEdited( state );
}

void TextEdit::setFixedWidth(bool y)
{
    if (y) {
	editor->setFont(QFont("fixed"));
	zinE = zin->isEnabled();
	zoutE = zout->isEnabled();
	zin->setEnabled(FALSE);
	zout->setEnabled(FALSE);
    } else {
	editor->setFont(QFont());
	setFontSize(variableFontSize,zoomOutLast);
	zin->setEnabled(zinE);
	zout->setEnabled(zoutE);
    }
}

void TextEdit::clipboardChanged()
{
    pasteAction->setEnabled( !qApp->clipboard()->text().isEmpty() );
}

void TextEdit::linkChanged( const QString &linkfile )
{
    if ( doc ) {
	DocLnk dl( linkfile );
	if ( doc->linkFileKnown() && doc->linkFile() == linkfile ||
	     doc->fileKnown() &&
	     (doc->file() == linkfile || dl.isValid() && dl.file() == doc->file()) ) {
	    if ( !QFile::exists(doc->file()) && !QFile::exists(doc->linkFile()) ) {
		// deleted
		fileRevert();
	    } else {
		if ( doc->name() != dl.name() )
		    updateCaption(dl.name());
		*doc = dl;
	    }
	}
    }
}

void TextEdit::fileNew()
{
    save();
    newFile(DocLnk());
}

void TextEdit::fileOpen()
{
    if ( !save() ) {
	if ( QMessageBox::critical( this, tr( "Out of space" ),
				    tr( "Text Editor was unable to\n"
					"save your changes.\n"
					"Free some space and try again.\n"
					"\nContinue anyway?" ),
				    QMessageBox::Yes|QMessageBox::Escape,
				    QMessageBox::No|QMessageBox::Default )
	     != QMessageBox::Yes )
	    return;
	else {
	    delete doc;
	    doc = 0;
	}
    }
    menu->hide();
    editBar->hide();
    searchBar->hide();
    editorStack->raiseWidget( fileSelector );
    fileSelector->reread();
    updateCaption();
}


void TextEdit::fileRevert()
{
    clear();
    fileOpen();
}

void TextEdit::editCut()
{
#ifndef QT_NO_CLIPBOARD
    editor->cut();
#endif
}

void TextEdit::editCopy()
{
#ifndef QT_NO_CLIPBOARD
    editor->copy();
#endif
}

void TextEdit::editPaste()
{
#ifndef QT_NO_CLIPBOARD
    editor->paste();
#endif
}

void TextEdit::editFind(bool s)
{
    if ( s ) {
	searchBar->show();
	searchVisible = TRUE;
	searchEdit->setFocus();
    } else {
	searchVisible = FALSE;
	searchBar->hide();
    }
}

void TextEdit::search()
{
    editor->find( searchEdit->text(), FALSE );
}

void TextEdit::findWrapped()
{
    Global::statusMessage( tr("Find: reached end") );
}

void TextEdit::findNotFound()
{
    Global::statusMessage( tr("Find: not found") );
}

void TextEdit::findFound()
{
    Global::statusMessage( "" );
}

void TextEdit::newFile( const DocLnk &f )
{
    DocLnk nf = f;
    nf.setType("text/plain");
    clear();
    editorStack->raiseWidget( editor );
    editor->setFocus();
    doc = new DocLnk(nf);
    setReadOnly(FALSE);
    updateCaption();
}

void TextEdit::setDocument(const QString& f)
{
    DocLnk nf(f);
    nf.setType("text/plain");
    openFile(nf);
    showEditTools();
    // Show filename in caption
    QString name;
    if ( nf.linkFileKnown() && !nf.name().isEmpty() ) {
	name = nf.name();
    } else {
	name = f;
	int sep = name.findRev( '/' );
	if ( sep > 0 )
	    name = name.mid( sep+1 );
    }
    updateCaption( name );
}

void TextEdit::openFile( const DocLnk &f )
{
    clear();
    FileManager fm;
    QString txt;
    QByteArray ba;
    bool needsave = FALSE;
    if ( fm.loadFile( f, ba ) ) {
	txt = QString::fromUtf8(ba, ba.size());
	if ( txt.utf8().length() != ba.size() ) {
	    // not UTF8
	    QTextCodec* codec = QTextCodec::codecForContent(ba.data(),ba.size());
	    if ( codec ) {
		txt = codec->toUnicode(ba);
		needsave = TRUE;
	    }
	}
    }
    fileNew();
    if ( doc )
	delete doc;
    doc = new DocLnk(f);
    editor->setText(txt);
    editor->setEdited(needsave);
    updateCaption();
}

void TextEdit::showEditTools()
{
    if ( !doc )
	close();
    fileSelector->hide();
    menu->show();
    editBar->show();
    if ( searchVisible )
	searchBar->show();
    updateCaption();
}

bool TextEdit::save()
{
    // case of nothing to save...
    if ( !doc )
	return true;
    if ( !editor->edited() ) {
	delete doc;
	doc = 0;
	return true;
    }

    QString rt = editor->text();

    if ( doc->name().isEmpty() )
	doc->setName(calculateName(rt));
	
    FileManager fm;
    if ( !fm.saveFile( *doc, rt ) ) {
	return false;
    }
    delete doc;
    doc = 0;
    editor->setEdited( false );
    return true;
}

QString TextEdit::calculateName(QString rt) {
    QString pt = rt.simplifyWhiteSpace();
    int i = pt.find( ' ' );
    QString docname = pt;
    if ( i > 0 )
	docname = pt.left( i );
    // remove "." at the beginning
    while( docname.startsWith( "." ) )
	docname = docname.mid( 1 );
    docname.replace( QRegExp("/"), "_" );
    // cut the length. filenames longer than that don't make sense and something goes wrong when they get too long.
    if ( docname.length() > 40 )
	docname = docname.left(40);
    if ( docname.isEmpty() )
	docname = tr("Empty Text");
    return docname;
}

void TextEdit::fileName()
{
    if (!doc)
	newFile(DocLnk());
    if (doc->name().isEmpty())
	doc->setName(calculateName(editor->text()));
    DocPropertiesDialog *lp = new DocPropertiesDialog(doc, this);
    QPEApplication::execDialog( lp );
    delete lp;
    updateCaption(doc->name());
}

void TextEdit::clear()
{
    delete doc;
    doc = 0;
    editor->clear();
}

void TextEdit::updateCaption( const QString &name )
{
    if ( !doc )
	setCaption( tr("Text Editor") );
    else {
	QString s = name;
	if ( s.isNull() )
	    s = doc->name();
	if ( s.isEmpty() )
	    s = tr( "Unnamed" );
	setCaption( s + " - " + tr("Text Editor") );
    }
}

void TextEdit::accept()
{
    fileOpen();
}	

void TextEdit::message(const QCString& msg, const QByteArray& data)
{
    if ( msg == "viewFile(QString)" || msg == "openFile(QString)" ) {
	QDataStream d(data,IO_ReadOnly);
	QString filename;
	d >> filename;

	//
	// .desktop files should _not_ be able to be edited easily,
	// as they are generated by the server.  Force opening the
	// file they refer to, rather than the .desktop file.
	//
	if (!filename.contains(".desktop")) {
	    DocLnk dc;
	    dc.setFile(filename);
	    dc.setType("text/plain");
	    openFile(dc);
	} else {
	    openFile(DocLnk(filename));
	}
	showEditTools();
	updateCaption( filename );
	if ( msg == "viewFile(QString)" )
	    setReadOnly(TRUE);
	QPEApplication::setKeepRunning();
    }
}

void TextEdit::setReadOnly(bool y)
{
    editor->setReadOnly(y);
    if ( y )
	editor->setEdited(FALSE);
}

#include "textedit.moc"
