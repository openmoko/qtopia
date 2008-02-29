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

#define QTOPIA_INTERNAL_FS_SEL
#include "global.h"

#include "fileselector.h"
#include "fileselector_p.h"

#include "resource.h"
QIconSet qtopia_internal_loadIconSet( const QString &pix );
#include "config.h"
#include "applnk.h"
#include "storage.h"
#include "qpemenubar.h"
#ifdef Q_WS_QWS
#include "qcopchannel_qws.h"
#endif
#include "applnk.h"
#include "qpeapplication.h"
#include "categorymenu.h"
#include "categoryselect.h"
#include "mimetype.h"
#include "categories.h"
#include "qpemessagebox.h"
#ifdef QTOPIA_PHONE
# include <qtopia/categorydialog.h>
#endif

#include <stdlib.h>

#include <qwidgetstack.h>
#include <qtimer.h>
#include <qdir.h>
#include <qwidget.h>
#include <qregexp.h>
#include <qpopupmenu.h>
#include <qtoolbutton.h>
#include <qpushbutton.h>
#include <qheader.h>
#include <qpainter.h>
#include <qtooltip.h>
#include <qlistbox.h>
#include <qwhatsthis.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qaction.h>

QIconSet qtopia_internal_loadIconSet( const QString &pix );

class TypeFilter : public QHBox
{
    Q_OBJECT
public:
    enum WidgetType { ComboBox, ListBox };

    TypeFilter( WidgetType t, QWidget *parent, const char *name=0 )
	: QHBox( parent, name ), type(t)
    {
	if (type == ComboBox) {
	    combo = new QComboBox(this);
	    combo->setSizePolicy( QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred) );
	    connect( combo, SIGNAL(activated(int)), this, SLOT(selectType(int)) );
	} else {
	    list = new QListBox(this);
	    connect( list, SIGNAL(selected(int)), this, SLOT(selectType(int)) );
	}
    }

    void reread( DocLnkSet &files, const QString &filter );

    void clear()
    {
	if (type == ComboBox)
	    combo->clear();
	else
	    list->clear();
    }
    uint count()
    {
	if (type == ComboBox)
	    return combo->count();
	else
	    return list->count();
    }
    void setCurrentItem(int i)
    {
	if (type == ComboBox)
	    combo->setCurrentItem(i);
	else
	    list->setCurrentItem(i);
    }
    void insertItem( const QString &s )
    {
	if (type == ComboBox)
	    combo->insertItem(s);
	else
	    list->insertItem(s);
    }
    void insertStringList( const QStringList &s )
    {
	if (type == ComboBox)
	    combo->insertStringList(s);
	else
	    list->insertStringList(s);
    }
    QString currentText() const
    {
	if (type == ComboBox)
	    return combo->currentText();
	else
	    return list->currentText();
    }
    QString text(int i) const
    {
	if (type == ComboBox)
	    return combo->text(i);
	else
	    return list->text(i);
    }

signals:
    void selected( const QString & );

protected slots:
    void selectType( int idx ) {
	emit selected( typelist[idx] );
    }

protected:
    QStringList typelist;
    QString prev;
    QComboBox *combo;
    QListBox *list;
    WidgetType type;
};

void TypeFilter::reread( DocLnkSet &files, const QString &filter )
{
    typelist.clear();
    QStringList filters = QStringList::split( ';', filter );
    int pos = filter.find( '/' );
    //### do for each filter
    if ( filters.count() == 1 && pos >= 0 && filter[pos+1] != '*' ) {
	typelist.append( filter );
	QString minor = filter.mid( pos+1 );
	minor[0] = minor[0].upper();
	clear();
	insertItem( tr("%1 files").arg(minor) );
	setCurrentItem(0);
	setEnabled( FALSE );
    } else {

    QListIterator<DocLnk> dit( files.children() );
    for ( ; dit.current(); ++dit ) {
	if ( !typelist.contains( (*dit)->type() ) )
	    typelist.append(  (*dit)->type() );
    }

    QStringList types;
    QStringList::ConstIterator it;
    for (it = typelist.begin(); it!=typelist.end(); ++it) {
	QString t = *it;
	if ( t.left(12) == "application/" ) {
	    MimeType mt(t);
	    const AppLnk* app = mt.application();
	    if ( app )
		t = app->name();
	    else
		t = t.mid(12);
	} else {
	    QString major, minor;
	    int pos = t.find( '/' );
	    if ( pos >= 0 ) {
		major = t.left( pos );
		minor = t.mid( pos+1 );
	    }
	    if ( minor.find( "x-" ) == 0 )
		minor = minor.mid( 2 );
	    minor[0] = minor[0].upper();
	    major[0] = major[0].upper();
	    if ( filters.count() > 1 )
		t = tr("%1 %2", "minor mimetype / major mimetype").arg(minor).arg(major);
	    else
		t = minor;
	}
	types += tr("%1 files").arg(t);
    }
    for (it = filters.begin(); it!=filters.end(); ++it) {
	typelist.append( *it );
	int pos = (*it).find( '/' );
	if ( pos >= 0 ) {
	    QString maj = (*it).left( pos );
	    maj[0] = maj[0].upper();
	    types << tr("All %1 files").arg(maj);
	}
    }
    if ( filters.count() > 1 ) {
	typelist.append( filter );
	types << tr("All files");
    }
    prev = currentText();
    clear();
    insertStringList(types);
    for (int i=0; i<(int)count(); i++) {
	if ( text(i) == prev ) {
	    setCurrentItem(i);
	    break;
	}
    }
    if ( prev.isNull() )
	setCurrentItem(count()-1);
    setEnabled( TRUE );

    }

#ifndef QTOPIA_DESKTOP
    if( type == ComboBox ) {
        // Invalidate to recalculate size hint
        combo->setFont( combo->font() );
        combo->updateGeometry();
    }
#endif
}

//===========================================================================

static QColor mixColors(const QColor& a, const QColor& b, int pcth)
{
    int pcthb = 100 - pcth;
    return QColor(
	    (a.red() * pcth + b.red() * pcthb) / 100,
	    (a.green() * pcth + b.green() * pcthb) / 100,
	    (a.blue() * pcth + b.blue() * pcthb) / 100);
}

FileSelectorItem::FileSelectorItem( QListView *parent, const DocLnk &f )
    : QListViewItem( parent ), fl( f )
{
    setText( 0, f.name() );
    setPixmap( 0, f.pixmap() );
}

FileSelectorItem::~FileSelectorItem()
{
}

void FileSelectorItem::paintCell( QPainter *p, const QColorGroup &cg,
                               int column, int width, int align )
{
    QColorGroup mycg(cg);
    if ((itemPos() / height()) & 1) {
	QBrush sb = ((FileSelectorView*)listView())->altBrush();
	mycg.setBrush(QColorGroup::Base, sb);
    }
	
    QListViewItem::paintCell(p, mycg, column, width, align);
}

FileSelectorView::FileSelectorView( QWidget *parent, const char *name )
    : QListView( parent, name )
{
    setAllColumnsShowFocus( TRUE );
    addColumn( tr( "Name" ) );
    setColumnWidthMode( 0, QListView::Manual );
    header()->hide();
    paletteChange(palette());
}

FileSelectorView::~FileSelectorView()
{
}

void FileSelectorView::keyPressEvent( QKeyEvent *e )
{
    QString txt = e->text();
    if (e->key() == Key_Space)
	emit returnPressed( currentItem() );
    else if ( !txt.isNull() && txt[0] > ' ' && e->key() < 0x1000 )
	e->ignore();
    else
	QListView::keyPressEvent(e);
}

QBrush FileSelectorView::altBrush() const
{
    return stripebrush;
}

void FileSelectorView::paletteChange( const QPalette &p )
{
#ifndef QTOPIA_DESKTOP
    stripebrush = style().extendedBrush(QStyle::AlternateBase);
#else
    stripebrush = mixColors(colorGroup().base(), colorGroup().highlight(), 90);
#endif
    QListView::paletteChange(p);
}

class NewDocItem : public FileSelectorItem
{
public:
    NewDocItem( QListView *parent, const DocLnk &f )
	: FileSelectorItem( parent, f ) {
	setText( 0, FileSelector::tr("New") );
	QImage img( qtopia_internal_loadIconSet( "new" ).pixmap().convertToImage() );
	QPixmap pm;
	pm = img.smoothScale( AppLnk::smallIconSize(), AppLnk::smallIconSize() );
	setPixmap( 0, pm );
    }
    QString key ( int, bool ) const {
	return QString("\n");
    }

    void paintCell( QPainter *p, const QColorGroup &cg, int column, int width, int alignment ) {
	QFont oldFont = p->font();
	QFont newFont = p->font();
	newFont.setWeight( QFont::Bold );
	p->setFont( newFont );
	FileSelectorItem::paintCell( p, cg, column, width, alignment );
	p->setFont( oldFont );
    }

    int width( const QFontMetrics &fm, const QListView *v, int c ) const {
	return FileSelectorItem::width( fm, v, c )*4/3; // allow for bold font
    }
};

//===========================================================================

/*!
  \class FileSelector fileselector.h
  \brief The FileSelector widget allows the user to select DocLnk objects.

  This class presents a file selection dialog to the user. This widget
  is usually the first widget seen in a \link docwidget.html
  document-oriented application\endlink. The developer will most often
  create this widget in combination with a <a
  href="qwidgetstack.html"> QWidgetStack</a> and the appropriate
  editor and/or viewer widget for their application. This widget
  should be shown first and the user can the select which document
  they wish to operate on. Please refer to the implementation of
  texteditor for an example of how to tie these classes together.

  Use setNewVisible() depending on whether the application can be used
  to create new files or not. Use setCloseVisible() depending on
  whether the user may leave the dialog without creating or selecting
  a document or not. The number of files in the view is available from
  fileCount(). To force the view to be updated call reread().

  If the user presses the 'New' button the newSelected()
  signal is emitted. If the user selects an existing file the
  fileSelected() signal is emitted. The selected file's \link
  doclnk.html DocLnk\endlink is available from the selected()
  function. If the file selector is no longer necessary the closeMe()
  signal is emitted.

  The typeChanged() and categoryChanged() signals are emitted when
  either a different file type or category are selected. Note that
  in Qtopia 1.5.0, there were no selectors for this, so these signals
  are never emitted on that platform.

  \ingroup qtopiaemb
  \sa FileManager
*/

/*! \fn void FileSelector::typeChanged(void)
  This signal is emitted when a different file type is selected.
*/

/*! \fn void FileSelector::categoryChanged(void)
  This signal is emitted when a different category is selected.
*/

/*!
  Constructs a FileSelector with mime filter \a f.
  The standard Qt \a parent and \a name parameters are passed to the
  parent widget.

  If \a newVisible is TRUE, the widget has a button to allow the user
  the create "new" documents; this is useful for applications that can
  create and edit documents but not suitable for applications that
  only provide viewing.

  \a closeVisible is deprecated

  \sa DocLnkSet::DocLnkSet()
*/
FileSelector::FileSelector( const QString &f, QWidget *parent, const char *name, bool newVisible, bool closeVisible )
    : QVBox( parent, name ), filter( f )
{
#define VIEW_ID 1
#define MESSAGE_ID 2

    setMargin( 0 );
    setSpacing( 0 );

    d = new FileSelectorPrivate();
    d->showNew = newVisible;
    d->catId = -2; // All files
    d->files = 0;

    d->rereadTimer = new QTimer( this );
    connect( d->rereadTimer, SIGNAL(timeout()), this, SLOT(slotReread()) );

    d->needReread = TRUE;

#ifndef QTOPIA_PHONE
    d->toolbar = new QHBox( this );
    d->toolbar->setBackgroundMode( PaletteButton );   // same colour as toolbars
    d->toolbar->setSpacing( 0 );
    d->toolbar->hide();

    QWidget *spacer = new QWidget( d->toolbar );
    spacer->setBackgroundMode( PaletteButton );

    QToolButton *tb = new QToolButton( d->toolbar );
    tb->setIconSet( qtopia_internal_loadIconSet( "close" ) );
    connect( tb, SIGNAL( clicked() ), this, SIGNAL( closeMe() ) );
    buttonClose = tb;
    tb->setFixedSize( 18, 20 ); // tb->sizeHint() );
    tb->setAutoRaise( TRUE );
    QToolTip::add( tb, tr( "Close the File Selector" ) );
    QPEMenuToolFocusManager::manager()->addWidget( tb );
#endif

#ifdef QTOPIA_PHONE
    d->widgetStack = new QWidgetStack( this );
    d->widgetStack->addWidget( view = new FileSelectorView( d->widgetStack, "fileview" ), VIEW_ID );
#else
    view = new FileSelectorView( this, "fileview" );
#endif
    view->setSorting(-1);
    view->setFrameStyle( QFrame::NoFrame );
    QPEApplication::setStylusOperation( view->viewport(), QPEApplication::RightOnHold );
    connect( view, SIGNAL( mouseButtonClicked(int,QListViewItem*,const QPoint&,int) ),
	     this, SLOT( fileClicked(int,QListViewItem*,const QPoint&,int) ) );
    connect( view, SIGNAL( mouseButtonPressed(int,QListViewItem*,const QPoint&,int) ),
	     this, SLOT( filePressed(int,QListViewItem*,const QPoint&,int) ) );
    connect( view, SIGNAL( returnPressed(QListViewItem*) ),
	     this, SLOT( fileClicked(QListViewItem*) ) );
    connect( view, SIGNAL( currentChanged(QListViewItem*) ),
	     this, SLOT( currentChanged(QListViewItem*) ) );

#ifndef QTOPIA_PHONE
    setFocusProxy( view );
#endif
    
#ifdef QTOPIA_PHONE
   d->widgetStack->addWidget( d->message = new QLabel( d->widgetStack ), MESSAGE_ID );
#endif

#ifndef QTOPIA_PHONE
    QHBox *hb = new QHBox( this );
    d->typeFilter = new TypeFilter( TypeFilter::ComboBox, hb );
    connect( d->typeFilter, SIGNAL(selected(const QString&)),
	    this, SLOT(typeSelected(const QString&)) );
    QWhatsThis::add( d->typeFilter, tr("Show documents of this type") );

    Categories c;
    c.load(categoryFileName());
    QArray<int> vl( 0 );
    d->catSelect = new CategorySelect( hb );
    d->catSelect->setRemoveCategoryEdit( TRUE );
    d->catSelect->setCategories( vl, "Document View", // No tr
	tr("Document View") );
    d->catSelect->setAllCategories( TRUE );
    connect( d->catSelect, SIGNAL(signalSelected(int)), this, SLOT(catSelected(int)) );
    QWhatsThis::add( d->catSelect, tr("Show documents in this category") );

    setCloseVisible( closeVisible );
#else
    Q_UNUSED(closeVisible);
    d->filterDlg = new CategorySelectDialog("Document View", this, 0, TRUE);
    d->filterDlg->setAllCategories(TRUE);
    connect(d->filterDlg, SIGNAL(selected(int)), this, SLOT(catSelected(int)));

    d->categoryLabel = new QLabel(d->filterDlg->currentCategoryText(), this);
    d->categoryLabel->hide();

    d->haveContextMenu = FALSE;
#endif

    d->storage = new StorageInfo( this );
    connect( d->storage, SIGNAL( disksChanged() ), SLOT( cardChanged() ) );

    connect( qApp, SIGNAL(linkChanged(const QString&)), this, SLOT(linkChanged(const QString&)) );

    updateWhatsThis();
}

/*!
  Destroys the widget.
*/
FileSelector::~FileSelector()
{
    delete d;
}

/*!
  Returns the number of files in the view. If this is zero, an editor
  application might bypass the selector and immediately start with
  a "new" document.
*/
int FileSelector::fileCount()
{
    if ( !d->files )
	reread();
    return d->files->children().count();
}

/*!
  Calling this function is the programmatic equivalent of the user
  pressing the "new" button.

  \sa newSelected(), closeMe()
*/
void FileSelector::createNew()
{
    DocLnk f;
    emit newSelected( f );
    emit closeMe();
}

void FileSelector::fileClicked( int button, QListViewItem *i, const QPoint &, int )
{
    if ( !i )
	return;
    if ( button == Qt::LeftButton ) {
	fileClicked( i );
    }
}


void FileSelector::filePressed( int button, QListViewItem *i, const QPoint &, int )
{
    if ( !i || i == d->newDocItem )
	return;
    if ( button == Qt::RightButton ) {
	DocLnk l = ((FileSelectorItem *)i)->file();
	QPopupMenu pop( this );
	pop.insertItem( tr("Delete"), 1 );
	if ( pop.exec(QCursor::pos()) == 1 && 
	     QPEMessageBox::confirmDelete( this, tr("Delete"), l.name() ) ) {
	    l.removeFiles();
	    // We get a linkChanged so rereading happens again
	}
    }
}

void FileSelector::fileClicked( QListViewItem *i )
{
    if ( !i )
	return;
    if ( i == d->newDocItem ) {
	createNew();
    } else {
	emit fileSelected( ( (FileSelectorItem*)i )->file() );
	emit closeMe();
    }
}

void FileSelector::currentChanged(QListViewItem *item)
{
#ifdef QTOPIA_PHONE
    if (d->haveContextMenu && d->newDocItem) 
	d->deleteAction->setEnabled( item != d->newDocItem );
#else
    Q_UNUSED( item );
#endif
}

void FileSelector::typeSelected( const QString &type )
{
    d->mimeFilters.clear();
    QStringList subFilter = QStringList::split(";", type);
    for( QStringList::Iterator it = subFilter.begin(); it != subFilter.end(); ++it )
	d->mimeFilters.append( QRegExp(*it, FALSE, TRUE) );
    updateView();

    emit typeChanged();
}

void FileSelector::catSelected( int c )
{
    d->catId = c;
    updateView();
#ifdef QTOPIA_PHONE
    if (d->filterDlg->currentCategory() == -2) { // All categories
	d->categoryLabel->hide();
    } else {
	d->categoryLabel->setText(tr("Category: %1").arg(d->filterDlg->currentCategoryText()));
	d->categoryLabel->show();
    }
#endif

    emit categoryChanged();
}

void FileSelector::deleteFile()
{
    FileSelectorItem *item = (FileSelectorItem *)view->selectedItem();
    if (item && item != d->newDocItem) {
	DocLnk l = item->file();
	if (QPEMessageBox::confirmDelete(this, tr("Delete"), l.name())) {
	    l.removeFiles();
	}
    }
}

void FileSelector::cardChanged()
{
    if ( isVisible() ) 
	d->initReread( view, 200 );
    else
	d->needReread = TRUE;
}

void FileSelector::linkChanged( const QString & )
{
    if ( isVisible() ) 
	d->initReread( view, 200 );
    else
	d->needReread = TRUE;
}

const DocLnk *FileSelector::selected()
{
    FileSelectorItem *item = (FileSelectorItem *)view->selectedItem();
    if ( item && item != d->newDocItem )
	return new DocLnk( item->file() );
    return NULL;
}

/*!
  \fn void FileSelector::fileSelected( const DocLnk &f )

  This signal is emitted when the user selects a document.
  \a f is the document.
*/

/*!
  \fn void FileSelector::newSelected( const DocLnk &f )

  This signal is emitted when the user selects a "new" document.
  \a f is a DocLnk for the document. You will need to set the type
  of the document after copying it.
*/

/*!
  \fn void FileSelector::closeMe()

  This signal is emitted when the user no longer needs to view the widget.
*/


/*!
  If \a b is TRUE a "new document" entry is visible; if \a b is FALSE
  this entry is not visible and the user is unable to create new
  documents from the dialog.
*/
void FileSelector::setNewVisible( bool b )
{
    if ( d->showNew != b ) {
	d->showNew = b;
	updateView();
	updateWhatsThis();
    }
}

/*!
  If \a b is TRUE a "close" or "no document" button is visible; if \a
  b is FALSE this button is not visible and the user is unable to
  leave the dialog without creating or selecting a document.

  This function is deprecated.
*/
void FileSelector::setCloseVisible( bool b )
{
#ifndef QTOPIA_PHONE
    if (  b )
	d->toolbar->show();
    else
	d->toolbar->hide();
#else
    Q_UNUSED(b);
#endif
}

/*!
  Rereads the list of documents.
*/
void FileSelector::reread()
{
#ifdef Q_WS_QWS
    delete d->files;
    d->files = new DocLnkSet;
    Global::findDocuments(d->files, filter);
    if (d->typeFilter)
	d->typeFilter->reread( *d->files, filter );
#endif
    d->needReread = FALSE;
    updateView();
}

void FileSelector::slotReread()
{
    reread();
}

void FileSelector::showEvent( QShowEvent *e )
{
    if ( d->needReread )
	d->initReread( view, 0 );
    QScrollBar *sb = view->verticalScrollBar();
    int sbWidth = sb->isVisible() ? sb->width() : 0;
    view->setColumnWidth( 0, view->width() - sbWidth );
    view->triggerUpdate();
    QVBox::showEvent( e );
}

class AppLnkPriv : public AppLnk {
public:
    inline bool linkFileKnown() const
    {
	return !mLinkFile.isNull();
    }
};

static inline bool linkFileKnown(const AppLnk* l)
{
    return ((const AppLnkPriv*)l)->linkFileKnown();
}

static int compareDocLnk(const void* va, const void* vb)
{
    const DocLnk* docB = *(const DocLnk**)va;
    const DocLnk* docA = *(const DocLnk**)vb;
    const QChar *a = docA->name().unicode();
    const QChar *b = docB->name().unicode();
    int alen = docA->name().length();
    int blen = docB->name().length();
    int l = alen < blen ? alen : blen;
    while ( l-- && a->lower() == b->lower() )
	a++,b++;
    if ( l!=-1 ) {
	QChar al = a->lower();
	QChar bl = b->lower();
	return al.unicode() - bl.unicode();
    } else {
	if ( alen == blen ) {
	    QFileInfo fa(linkFileKnown(docA) ? docA->linkFile() : docA->file());
	    QFileInfo fb(linkFileKnown(docB) ? docB->linkFile() : docB->file());
	    return fa.lastModified().secsTo(fb.lastModified());
	} else {
	    return alen - blen; 
	}
    }
}

static int compareDocLnkReverse(const void* va, const void* vb)
{
    return -compareDocLnk(va, vb);
}

static int compareDocLnkChron(const void* va, const void* vb)
{
    const DocLnk* docB = *(const DocLnk**)va;
    const DocLnk* docA = *(const DocLnk**)vb;
    QFileInfo fa(linkFileKnown(docA) ? docA->linkFile() : docA->file());
    QFileInfo fb(linkFileKnown(docB) ? docB->linkFile() : docB->file());
    return fb.lastModified().secsTo(fa.lastModified());
}

static int compareDocLnkChronReverse(const void* va, const void* vb)
{
    const DocLnk* docB = *(const DocLnk**)va;
    const DocLnk* docA = *(const DocLnk**)vb;
    QFileInfo fa(linkFileKnown(docA) ? docA->linkFile() : docA->file());
    QFileInfo fb(linkFileKnown(docB) ? docB->linkFile() : docB->file());
    return fa.lastModified().secsTo(fb.lastModified());
}

void FileSelector::updateView()
{
    FileSelectorItem *item = (FileSelectorItem *)view->selectedItem();
    if ( item == d->newDocItem )
	item = 0;
    QString oldFile;
    if ( item )
	oldFile = item->file().file();
    view->clear();
    int ndocs = fileCount();
    QListIterator<DocLnk> dit( d->files->children() );
    DocLnk* *doc = new DocLnk*[ndocs];
    int i=0;
    for ( ; dit.current(); ++dit ) {
	doc[i++] = dit.current();
    }
    switch( d->sortMode ) {
	case FileSelector::Alphabetical:
	default:
	    qsort(doc,ndocs,sizeof(doc[0]),compareDocLnk);
	    break;

	case FileSelector::ReverseAlphabetical:
	    qsort(doc,ndocs,sizeof(doc[0]),compareDocLnkReverse);
	    break;

	case FileSelector::Chronological:
	    qsort(doc,ndocs,sizeof(doc[0]),compareDocLnkChron);
	    break;

	case FileSelector::ReverseChronological:
	    qsort(doc,ndocs,sizeof(doc[0]),compareDocLnkChronReverse);
	    break;
    }
    for ( i=0; i<ndocs; ++i ) {
	bool mimeMatch = FALSE;
	if ( d->mimeFilters.count() ) {
	    QValueList<QRegExp>::Iterator it;
	    for ( it = d->mimeFilters.begin(); it != d->mimeFilters.end(); ++it ) {
		if ( (*it).match(doc[i]->type()) >= 0 ) {
		    mimeMatch = TRUE;
		    break;
		}
	    }
	} else {
	    mimeMatch = TRUE;
	}
	if ( mimeMatch &&
		(d->catId == -2 || doc[i]->categories().contains(d->catId) ||
		 (d->catId == -1 && doc[i]->categories().isEmpty())) ) {
	    item = new FileSelectorItem( view, *doc[i] );
	    if ( oldFile && item->fl.file() == oldFile )
		view->setCurrentItem( item );
	}
    }

    delete [] doc;

    if (d->showNew)
	d->newDocItem = new NewDocItem( view, DocLnk() );
    else 
	d->newDocItem = 0;
        
#ifdef QTOPIA_PHONE
    if( view->childCount() == 0 && !d->newDocItem ) {
        if( d->haveContextMenu ) d->deleteAction->setEnabled( FALSE );
        d->message->setText( tr( "<qt><center><p>No documents found.</p></center></qt>" ) );
        if( d->widgetStack->visibleWidget() != d->message ) d->widgetStack->raiseWidget( d->message );
        d->message->setFocus();
    } else {
        if( d->haveContextMenu ) d->deleteAction->setEnabled( TRUE );
        if( d->widgetStack->visibleWidget() != view ) d->widgetStack->raiseWidget( view );
        view->setFocus();
    }
#endif

    if ( !view->selectedItem() || view->childCount() == 1 ) {
	view->setCurrentItem( view->firstChild() );
	view->setSelected( view->firstChild(), TRUE );
    }
    
    qApp->processEvents();
    QScrollBar *sb = view->verticalScrollBar();
    int sbWidth = sb->isVisible() ? sb->width() : 0;
    view->setColumnWidth( 0, view->width() - sbWidth );
    view->triggerUpdate();
}

void FileSelector::updateWhatsThis()
{
    QWhatsThis::remove( this );
    QString text;
    if (d->showNew)
    {
	text = tr("Click to select a document from the list, or select <b>New</b> to create a new document.  <br><br>Click and hold for document properties");
    } else {
	text = tr("Click to select a document from the list<br><br>Click and hold for document properties");
    }

    QWhatsThis::add( this, text );
}

void FileSelector::showFilterDlg()
{
#ifdef QTOPIA_PHONE
    QPEApplication::execDialog(d->filterDlg);
#endif
}

/*!
  Adds the standard FileSelector options (New, Delete, View Category) to
  the supplied QPopupMenu \a menu.
*/
#ifdef QTOPIA_PHONE
void FileSelector::addOptions(QPopupMenu *menu)
{
    if (!d->filterAction) {
	d->newAction = new QAction(tr("New"), qtopia_internal_loadIconSet("new"), QString::null, 0, this, 0);
	connect(d->newAction, SIGNAL(activated()), this, SLOT(createNew()));
	d->deleteAction = new QAction(tr("Delete"), qtopia_internal_loadIconSet("trash"), QString::null, 0, this, 0);
	connect(d->deleteAction, SIGNAL(activated()), this, SLOT(deleteFile()));
	d->filterAction = new QAction(tr("View Category..."), qtopia_internal_loadIconSet("viewcategory"), QString::null, 0, this, 0);
	connect(d->filterAction, SIGNAL(activated()), this, SLOT(showFilterDlg()));
    }

    if (d->showNew)
	d->newAction->addTo(menu);
    d->deleteAction->addTo(menu);
    menu->insertSeparator();
    d->filterAction->addTo(menu);
    d->haveContextMenu = TRUE;
}
#endif

/*!
    \fn const DocLnk *FileSelector::selected()

    Returns a copy of the selected \link doclnk.html DocLnk\endlink which
    must be deleted by the caller.
    This function is deprecated. It will be removed in Qtopia 2.
    Please switch to using \link selectedDocument() \endlink instead.
*/

#include "fileselector.moc"

