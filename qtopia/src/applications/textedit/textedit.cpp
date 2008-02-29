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

#include "textedit.h"

#include <qtopia/global.h>
#include <qtopia/applnk.h>
#include <qtopia/resource.h>
#include <qtopia/config.h>
#include <qtopia/qpeapplication.h>
#include <qtopia/docproperties.h>
#include <qtopia/fontdatabase.h>
#include <qtopia/contextbar.h>
#include <qtopia/services.h>

#include <qmenubar.h>
#include <qcolordialog.h>
#include <qfileinfo.h>
#include <qmessagebox.h>
#include <qtextcodec.h>
#include <qclipboard.h>
#include <qwhatsthis.h>
#include <qtimer.h>

#include <stdlib.h> //getenv



#if QT_VERSION < 0x030000

class QpeEditor : public QMultiLineEdit
{
    Q_OBJECT
public:
    QpeEditor( QWidget *parent, const char * name = 0 )
	: QMultiLineEdit( parent, name ), wrap(FALSE)
        {
            clearTableFlags();
            setTableFlags( Tbl_vScrollBar | Tbl_autoHScrollBar );
        }

    void find( const QString &txt, bool caseSensitive );

protected:
    virtual void mousePressEvent(QMouseEvent *);

signals:
    void findNotFound();
    void findFound();
    void findWrapped();

private:
    QString lastTxt;
    bool wrap;
    
};


void QpeEditor::find ( const QString &txt, bool caseSensitive )
{
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

//
// Ensure that the we are looking at the end of the line when the
// line length exceeds the viewable area, and the user taps on the
// space underneath the line.
//
void
QpeEditor::mousePressEvent(QMouseEvent *e)
{
    int line;
    int col;

    QMultiLineEdit::mousePressEvent(e);

    getCursorPosition(&line, &col);
    setCursorPosition(line, col);
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
    lastSaveCancelled = FALSE;
    
    connect(qApp, SIGNAL(appMessage(const QCString&,const QByteArray&)),
	    this, SLOT(message(const QCString&,const QByteArray&)));

#ifdef QTOPIA_PHONE
    qCopActivated = canceled = FALSE;
#endif

    doc = 0;

    editorStack = new QWidgetStack( this );
    setCentralWidget( editorStack );

    editor = new QpeEditor( editorStack );
    editor->setFrameStyle( QFrame::NoFrame );
    editorStack->addWidget( editor, get_unique_id() );

    fileSelector = new FileSelector( "text/*", editorStack, "fileselector" , TRUE, FALSE );

    setupFontSizes();

    setToolBarsMovable( FALSE );
    setBackgroundMode( PaletteButton );

    setIcon( Resource::loadPixmap( "TextEditor" ) );

    QAction *newAction = new QAction( tr( "New" ), Resource::loadIconSet( "new" ), QString::null, 0, this, 0 );
    connect( newAction, SIGNAL( activated() ), this, SLOT( fileNew() ) );
    newAction->setWhatsThis( tr( "Create a document." ) );

    QAction *openAction = new QAction( tr( "Open" ), Resource::loadIconSet( "txt" ), QString::null, 0, this, 0 );
    connect( openAction, SIGNAL( activated() ), this, SLOT( fileOpen() ) );
    openAction->setWhatsThis( tr( "Open a document." ) );

    QAction *propAction = new QAction( tr( "Properties" ), Resource::loadIconSet( "info" ), QString::null, 0, this );
    connect( propAction, SIGNAL( activated() ), this, SLOT( fileName() ) );
    propAction->setWhatsThis( tr( "Edit the document properties." ) );

    QAction *cutAction = new QAction( tr( "Cut" ), Resource::loadIconSet( "cut" ), QString::null, 0, this, 0 );
    connect( cutAction, SIGNAL( activated() ), this, SLOT( editCut() ) );
    cutAction->setWhatsThis( tr("Cut the currently selected text and move it to the clipboard.") );
#ifdef QTOPIA_PHONE
    cutAction->setEnabled( FALSE );
#endif

    QAction *copyAction = new QAction( tr( "Copy" ), Resource::loadIconSet( "copy" ), QString::null, 0, this, 0 );
    connect( copyAction, SIGNAL( activated() ), this, SLOT( editCopy() ) );
    copyAction->setWhatsThis( tr("Copy the currently selected text to the clipboard.") );
#ifdef QTOPIA_PHONE
    copyAction->setEnabled( FALSE );
#endif

    pasteAction = new QAction( tr( "Paste" ), Resource::loadIconSet( "paste" ), QString::null, 0, this, 0 );
    connect( pasteAction, SIGNAL( activated() ), this, SLOT( editPaste() ) );
    pasteAction->setWhatsThis( tr("Paste the text in the clipboard at the cursor position.") );
    QTimer::singleShot(0, this, SLOT(clipboardChanged()));

    findAction = new QAction( tr( "Find" ), Resource::loadIconSet( "find" ), QString::null, 0, this, 0 );
    findAction->setToggleAction( TRUE );
    connect( findAction, SIGNAL(toggled(bool)), this, SLOT(editFind(bool)) );
    findAction->setWhatsThis( tr("Click to find text in the document.\nClick again to hide the search bar.") );

    zin = new QAction( tr( "Zoom In" ), QString::null, 0, this, 0 );
    connect( zin, SIGNAL( activated() ), this, SLOT( zoomIn() ) );
    zin->setWhatsThis( tr( "Increase the font size." ) );

    zout = new QAction( tr( "Zoom Out" ), QString::null, 0, this, 0 );
    connect( zout, SIGNAL( activated() ), this, SLOT( zoomOut() ) );
    zout->setWhatsThis( tr( "Decrease the font size." ) );

    QAction *wa = new QAction( tr( "Wrap Lines" ), QString::null, 0, this, 0 );
    connect( wa, SIGNAL( toggled(bool) ), this, SLOT( setWordWrap(bool) ) );
    wa->setWhatsThis( tr("Break long lines into two or more lines.") );
    wa->setToggleAction(TRUE);

    fixedAction = new QAction( tr( "Fixed Width" ), QString::null,0,this,0);
    connect( fixedAction, SIGNAL(toggled(bool) ),
	this, SLOT( setFixedWidth(bool)));
    fixedAction->setWhatsThis( tr( "Use a fixed width font. Useful for preformatted documents." ) );
    fixedAction->setToggleAction(TRUE);

#ifdef QTOPIA_PHONE
    contextMenu = new ContextMenu(editor, 0, ContextBar::ModalAndNonModal);
    contextMenu->setEnableHelp( FALSE );
    
    QPopupMenu *settingsMenu = new QPopupMenu(contextMenu);
    zin->addTo(settingsMenu);
    zout->addTo(settingsMenu);
    wa->addTo(settingsMenu);
    
    propAction->addTo(contextMenu);
    cutAction->addTo(contextMenu);
    copyAction->addTo(contextMenu);
    pasteAction->addTo(contextMenu);
    findAction->addTo(contextMenu);
    contextMenu->insertItem(tr("Settings"), settingsMenu);
    fixedAction->addTo(settingsMenu);
    contextMenu->insertSeparator();
    contextMenu->insertItem( Resource::loadIconSet( "help_icon" ), tr( "Help" ), contextMenu, SLOT( help() ) );
    contextMenu->insertItem( Resource::loadIconSet( "close" ), tr( "Cancel" ), this, SLOT( fileRevert() ) );
    
    fileContextMenu = new ContextMenu(fileSelector);
    fileSelector->addOptions(fileContextMenu);
#else
    QToolBar *bar = new QToolBar( this );
    bar->setHorizontalStretchable( TRUE );
    menu = bar;

    QMenuBar *mb = new QMenuBar( bar );
    QPopupMenu *file = new QPopupMenu( this );
    QPopupMenu *edit = new QPopupMenu( this );
    QPopupMenu *font = new QPopupMenu( this );

    mb->insertItem( tr( "File" ), file );
    mb->insertItem( tr( "Edit" ), edit );
    mb->insertItem( tr( "View" ), font );

    bar = new QToolBar( this );
    editBar = bar;

    newAction->addTo( bar );
    newAction->addTo( file );
    openAction->addTo( bar );
    openAction->addTo( file );
    propAction->addTo( bar );
    propAction->addTo( file );
    cutAction->addTo( editBar );
    cutAction->addTo( edit );
    copyAction->addTo( editBar );
    copyAction->addTo( edit );
    pasteAction->addTo( editBar );
    pasteAction->addTo( edit );
    findAction->addTo( bar );
    findAction->addTo( edit );
    zin->addTo( font );
    zout->addTo( font );
    font->insertSeparator();
    wa->addTo( font );
    fixedAction->addTo( font );
#endif

    int defsize;
    bool wrap, fixedwidth;
    {
	Config cfg("TextEdit");
	cfg.setGroup("View");
	defsize = cfg.readNumEntry("FontSize",10);
	wrap = cfg.readBoolEntry("Wrap",TRUE);
	fixedwidth = cfg.readBoolEntry("Fixed-width", FALSE);
    }

    connect( editor, SIGNAL(findWrapped()), this, SLOT(findWrapped()) );
    connect( editor, SIGNAL(findNotFound()), this, SLOT(findNotFound()) );
    connect( editor, SIGNAL(findFound()), this, SLOT(findFound()) );
#ifdef QTOPIA_PHONE
    connect( editor, SIGNAL(copyAvailable(bool)), cutAction, SLOT(setEnabled(bool)) );
    connect( editor, SIGNAL(copyAvailable(bool)), copyAction, SLOT(setEnabled(bool)) );
#endif

    // create search bar on demand
    searchBar = 0;
    searchEdit = 0;
    searchVisible = FALSE;

    connect( fileSelector, SIGNAL( closeMe() ), this, SLOT( showEditTools() ) );
    connect( fileSelector, SIGNAL( newSelected(const DocLnk&) ), this, SLOT( newFile(const DocLnk&) ) );
    connect( fileSelector, SIGNAL( fileSelected(const DocLnk&) ), this, SLOT( openFile(const DocLnk&) ) );
    fileOpen();

    connect( qApp->clipboard(), SIGNAL(dataChanged()), this, SLOT(clipboardChanged()) );
    connect( qApp, SIGNAL(linkChanged(const QString&)), this, SLOT(linkChanged(const QString&)) );

    resize( 200, 300 );

    variableFontSize = defsize;
    zoomOutLast = TRUE;
    setFontSize(defsize,zoomOutLast);
    wa->setOn(wrap);

    fixedAction->setOn(fixedwidth);
    setFixedWidth(fixedAction->isOn());

    if ( qApp->argc() == 3 && qApp->argv()[1] == QCString("-f") )
	setDocument(qApp->argv()[2]);
}

TextEdit::~TextEdit()
{
    if (fontsize)
	delete [] fontsize;

    Config cfg("TextEdit");
    cfg.setGroup("View");
    QFont f = editor->font();
    cfg.writeEntry("FontSize",f.pointSize());
    cfg.writeEntry("Wrap",editor->wordWrap() == QMultiLineEdit::WidgetWidth);
    cfg.writeEntry("Fixed-width", fixedAction->isOn() ? "1" : "0");
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

    //
    // Zooming only makes sense if we have more than one font size.
    //
    if (nfontsizes > 1) {
	zin->setEnabled(s != fontsize[nfontsizes-1]);
	zout->setEnabled(s != fontsize[0]);

	zinE = zin->isEnabled();
	zoutE = zout->isEnabled();
    }
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
    newFile( DocLnk() );
}

void TextEdit::fileOpen()
{
    
    if ( !save() ) {
       QMessageBox box( tr( "Out of space"),
			tr( "<qt>Text Editor was unable to "
			    "save your changes. "
			    "Free some space and try again."
			    "<br>Continue anyway?</qt>" ),
                        QMessageBox::Critical,
			QMessageBox::Yes|QMessageBox::Escape,
			QMessageBox::No|QMessageBox::Default,
                        QMessageBox::Cancel, this);
        box.setButtonText(QMessageBox::Cancel, tr("Cleanup"));
        switch(box.exec()){
            case QMessageBox::Yes:
                delete doc;
                doc = 0;
                break;
            case QMessageBox::No:
                return;
                break;
            case QMessageBox::Cancel:
                ServiceRequest req( "CleanupWizard", "showCleanupWizard()");
                req.send();
                return;
                break;
        }
    }
#ifndef QTOPIA_PHONE
    menu->hide();
    editBar->hide();
#endif
    if (searchBar)
	searchBar->hide();
    editorStack->raiseWidget( fileSelector );
    updateCaption();
}

void TextEdit::fileRevert()
{
#ifdef QTOPIA_PHONE
    if( wasCreated ) doc->removeFiles();
    else {
        if( editor->edited() && saved ) {
            FileManager fm;
            fm.saveFile( *doc, backup );
        }
    }
#endif
    clear();
#ifdef QTOPIA_PHONE
    if( qCopActivated ) {
        close();
        canceled = TRUE;
    } else fileOpen();
#else
    fileOpen();
#endif
}

void TextEdit::editCut()
{
#ifndef QT_NO_CLIPBOARD
    if( qApp->focusWidget() == editor )
        editor->cut();
    else if( qApp->focusWidget() == searchEdit )
        searchEdit->cut();
#endif
}

void TextEdit::editCopy()
{
#ifndef QT_NO_CLIPBOARD
    if( qApp->focusWidget() == editor )
        editor->copy();
    else if( qApp->focusWidget() == searchEdit )
        searchEdit->copy();
#endif
}

void TextEdit::editPaste()
{
#ifndef QT_NO_CLIPBOARD
    if( qApp->focusWidget() == editor )
        editor->paste();
    else if( qApp->focusWidget() == searchEdit )
        searchEdit->paste();
#endif
}

void TextEdit::editFind(bool s)
{
    if ( !searchBar ) {
	searchBar = new QToolBar(this);

	addToolBar( searchBar,  tr("Search"), QMainWindow::Top, TRUE );

	searchBar->setHorizontalStretchable( TRUE );

	searchEdit = new QLineEdit( searchBar, "searchEdit" );
	searchBar->setStretchableWidget( searchEdit );
	connect( searchEdit, SIGNAL( textChanged(const QString&) ),
		this, SLOT( search() ) );
	connect( searchEdit, SIGNAL(returnPressed()), this, SLOT(searchNext()));

#ifndef QTOPIA_PHONE
	QAction *a = new QAction( tr( "Find Next" ), Resource::loadIconSet( "next" ), QString::null, 0, this, 0 );
	connect( a, SIGNAL(activated()), this, SLOT(search()) );
	a->setWhatsThis( tr("Find the next occurrence of the search text.") );
	a->addTo( searchBar );
#else
	findTb = new QToolButton(Resource::loadIconSet( "next" ), tr( "Find Next" ), QString::null, 
				    this, SLOT(search()), searchBar);
#endif
    }
    if ( s ) {
	searchBar->show();
	searchVisible = TRUE;
        searchEdit->setFocus();
#ifdef QTOPIA_PHONE
	if( !Global::mousePreferred() ) {
	    if (!searchEdit->isModalEditing())
		searchEdit->setModalEditing(TRUE);
	}
#endif
    } else {
	searchVisible = FALSE;
#ifdef QTOPIA_PHONE
	if( !Global::mousePreferred() ) {
	    if (searchEdit->isModalEditing())
		searchEdit->setModalEditing(FALSE);
	}
#endif
	editor->setFocus();
	searchBar->hide();
#ifdef QTOPIA_PHONE
	if( !Global::mousePreferred() ) {
	    if (!editor->isModalEditing())
		editor->setModalEditing(TRUE);
	}
#endif
    }
}

void TextEdit::search()
{
    editor->find( searchEdit->text(), FALSE );
}

void TextEdit::searchNext()
{
#ifdef QTOPIA_PHONE
    if (findTb)
	findTb->setFocus();
#endif
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
    clear();
    doc = new DocLnk(f);
    doc->setType("text/plain");
    editorStack->raiseWidget( editor );
    editor->setFocus();
    editor->setEdited(FALSE);
    setReadOnly(FALSE);
    updateCaption();
}

void TextEdit::setDocument(const QString& f)
{
#ifdef QTOPIA_PHONE
    qCopActivated = TRUE;
#endif
    save();
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
#ifdef QTOPIA_PHONE
    backup = txt;
#endif
    editor->setText(txt);
    editor->setEdited(needsave);
    updateCaption();
}

void TextEdit::showEditTools()
{
    if ( !doc )
	close();
    fileSelector->hide();
#ifndef QTOPIA_PHONE
    menu->show();
    editBar->show();
#endif
    if ( searchBar && searchVisible )
	searchBar->show();
    updateCaption();
}

bool TextEdit::save()
{
    // case of nothing to save...
    if ( !doc )
	return true;
    if ( !editor->edited() ) {
        if( wasCreated ) doc->removeFiles();
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
    if (doc->name().isEmpty())
	doc->setName(calculateName(editor->text()));

    //
    // Document properties operations depend on the file being
    // up-to-date.  Force a write before changing properties.
    //
    wasCreated = wasCreated || !doc->fileKnown();
    
    FileManager fm;
    if ( fm.saveFile( *doc, editor->text() ) ) {
#ifdef QTOPIA_PHONE
        saved = TRUE;
#endif
    } else return;

    DocPropertiesDialog *lp = new DocPropertiesDialog(doc, this);
    if (QPEApplication::execDialog(lp)) {
	updateCaption(doc->name());
    }
    delete lp;
}

void TextEdit::clear()
{
    //  Flush out any pending input method gunk
    editor->setReadOnly( TRUE );
    editor->setReadOnly( FALSE );
    
    delete doc;
    doc = 0;
    editor->clear();
#ifdef QTOPIA_PHONE
    saved = FALSE;
#endif
    wasCreated = FALSE;
}

void TextEdit::updateCaption( const QString &name )
{
    if ( !doc )
	setCaption( tr("Notes") );
#ifndef QTOPIA_PHONE
    else {
	QString s = name;
	if ( s.isNull() )
	    s = doc->name();
	if ( s.isEmpty() )
	    s = tr( "Unnamed" );
	setCaption( s + " - " + tr("Notes") );
    }
#endif
}

void TextEdit::accept()
{
    fileOpen();
}	

void TextEdit::message(const QCString& msg, const QByteArray& data)
{
    if ( msg == "viewFile(QString)" || msg == "openFile(QString)" ) {
        save();
#ifdef QTOPIA_PHONE
        qCopActivated = TRUE;
#endif
	QDataStream d(data,IO_ReadOnly);
	QString filename;
	d >> filename;

	//
	// .desktop files should _not_ be able to be edited easily,
	// as they are generated by the server.  Force opening the
	// file they refer to, rather than the .desktop file.
	//
	if (!filename.contains(".desktop")) {
	    if (filename.stripWhiteSpace().isEmpty()){
		newFile(DocLnk());
	    }else{
		DocLnk dc;
		dc.setFile(filename);
		dc.setType("text/plain");
		openFile(dc);
	    }
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

void TextEdit::closeEvent( QCloseEvent* e )
{
#ifdef QTOPIA_PHONE
    if( searchVisible )
        findAction->setOn( FALSE );
    else {
        if( editorStack->visibleWidget() == editor ) { 
            if( qCopActivated ) {
                if( !canceled ) save();
                e->accept();
            } else fileOpen();
        } else e->accept();
    }
#else
    if( editorStack->visibleWidget() == editor ) fileOpen();
    e->accept();
#endif
}

#include "textedit.moc"
