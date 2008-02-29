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

#include "textedit.h"

#include <qcontent.h>
#include <qtopiaapplication.h>
#include <qdocumentproperties.h>
#ifdef QTOPIA_PHONE
#include <qsoftmenubar.h>
#endif
#include <qtopiaservices.h>

#include <QClipboard>
#include <QCloseEvent>
#include <QDebug>
#include <QFontDatabase>
#include <QLineEdit>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QStackedWidget>
#include <QTextCodec>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextEdit>
#include <QTimer>
#include <QToolBar>
#include <QSettings>
#include <QWhatsThis>


class QpeEditor : public QTextEdit
{
    Q_OBJECT
public:
    QpeEditor( QWidget *parent )
        : QTextEdit( parent )
    {
    }

    void findText( const QString &txt, bool caseSensitive )
    {
        QTextDocument::FindFlags flags = 0;
        if ( caseSensitive )
            flags |= QTextDocument::FindCaseSensitively;
        Qt::CaseSensitivity s = Qt::CaseInsensitive;
        if ( caseSensitive )
            s = Qt::CaseSensitive ;

        const bool findNext = txt == lastTxt;
        if ( !findNext &&
                (txt.startsWith( lastTxt, s ) || lastTxt.startsWith( txt, s ) ) ){
            setTextCursor( QTextCursor( document() ) );
        }
        bool found = find( txt, flags );

        if ( !found ) {
            if ( findNext )
                emit findWrapped();
            else
                emit findNotFound();
        }

        lastTxt = txt;
    }

signals:
    void findNotFound();
    void findWrapped();

private:
    QString lastTxt;
};


static int nfontsizes;
static int *fontsize;

TextEdit::TextEdit( QWidget *parent, Qt::WFlags f )
    : QMainWindow( parent, f )
{
    connect(qApp, SIGNAL(appMessage(const QString&,const QByteArray&)),
            this, SLOT(message(const QString&,const QByteArray&)));

#ifdef QTOPIA_PHONE
    qCopActivated = canceled = false;
#endif

    doc = 0;

    editorStack = new QStackedWidget( this );
    setCentralWidget( editorStack );

    editor = new QpeEditor( editorStack );
    editor->setFrameStyle( QFrame::NoFrame );
    editorStack->addWidget( editor );

    fileSelector = new QDocumentSelector( editorStack );
    fileSelector->setFilter( QContentFilter( QContent::Document ) & QContentFilter( QContentFilter::MimeType, "text/*" ) );
    fileSelector->enableOptions( QDocumentSelector::NewDocument );
    fileSelector->setFocus();
    editorStack->addWidget(fileSelector);

    setupFontSizes();

    setBackgroundRole( QPalette::Button );

    QAction *newAction = new QAction(QIcon( ":icon/new" ), tr( "New" ), this);
    connect( newAction, SIGNAL( triggered() ), this, SLOT( fileNew() ) );
    newAction->setWhatsThis( tr( "Create a document." ) );

    QAction *openAction = new QAction(QIcon( ":icon/txt" ), tr( "Open" ), this );
    connect( openAction, SIGNAL( triggered() ), this, SLOT( fileOpen() ) );
    openAction->setWhatsThis( tr( "Open a document." ) );

    QAction *propAction = new QAction(QIcon( ":icon/info" ), tr( "Properties" ), this );
    connect( propAction, SIGNAL( triggered() ), this, SLOT( fileName() ) );
    propAction->setWhatsThis( tr( "Edit the document properties." ) );

    QAction *cutAction = new QAction(QIcon( ":icon/cut" ), tr( "Cut" ), this );
    connect( cutAction, SIGNAL( triggered() ), this, SLOT( editCut() ) );
    cutAction->setWhatsThis( tr("Cut the currently selected text and move it to the clipboard.") );
#ifdef QTOPIA_PHONE
    cutAction->setVisible( false );
#endif

    QAction *copyAction = new QAction(QIcon( ":icon/copy" ), tr( "Copy" ), this );
    connect( copyAction, SIGNAL( triggered() ), this, SLOT( editCopy() ) );
    copyAction->setWhatsThis( tr("Copy the currently selected text to the clipboard.") );
#ifdef QTOPIA_PHONE
    copyAction->setVisible( false );
#endif

    pasteAction = new QAction(QIcon( ":icon/paste" ), tr( "Paste" ), this );
    connect( pasteAction, SIGNAL( triggered() ), this, SLOT( editPaste() ) );
    pasteAction->setWhatsThis( tr("Paste the text in the clipboard at the cursor position.") );
    QTimer::singleShot(0, this, SLOT(clipboardChanged()));

    findAction = new QAction(QIcon( ":icon/find" ), tr( "Find" ), this );
    findAction->setCheckable( true );
    connect( findAction, SIGNAL(toggled(bool)), this, SLOT(editFind(bool)) );
    findAction->setWhatsThis( tr("Click to find text in the document.\nClick again to hide the search bar.") );

    zin = new QAction(tr( "Zoom In" ), this);
    connect( zin, SIGNAL( triggered() ), this, SLOT( zoomIn() ) );
    zin->setWhatsThis( tr( "Increase the font size." ) );

    zout = new QAction(tr( "Zoom Out" ), this);
    connect( zout, SIGNAL( triggered() ), this, SLOT( zoomOut() ) );
    zout->setWhatsThis( tr( "Decrease the font size." ) );

    QAction *wa = new QAction(tr( "Wrap Lines" ), this);
    connect( wa, SIGNAL( toggled(bool) ), this, SLOT( setWordWrap(bool) ) );
    wa->setWhatsThis( tr("Break long lines into two or more lines.") );
    wa->setCheckable(true);

    fixedAction = new QAction(tr( "Fixed Width" ),this);
    connect( fixedAction, SIGNAL(toggled(bool) ),
        this, SLOT( setFixedWidth(bool)));
    fixedAction->setWhatsThis( tr( "Use a fixed width font. Useful for preformatted documents." ) );
    fixedAction->setCheckable(true);

#ifdef QTOPIA_PHONE
    contextMenu = new QMenu(editor);
    QSoftMenuBar::addMenuTo(editor, contextMenu, QSoftMenuBar::AnyFocus);
    QSoftMenuBar::setHelpEnabled(editor, true);

    QMenu *settingsMenu = new QMenu( tr("Settings"), contextMenu);
    settingsMenu->addAction( zin );
    settingsMenu->addAction( zout );
    settingsMenu->addAction( wa );
    settingsMenu->addAction( fixedAction );

    contextMenu->addAction( propAction );
    contextMenu->addAction( cutAction );
    contextMenu->addAction( copyAction );
    contextMenu->addAction( pasteAction );
    contextMenu->addAction( findAction );
    contextMenu->addMenu( settingsMenu );
    contextMenu->addSeparator();

    contextMenu->addAction( tr( "Print" ), this, SLOT(print()) );

    QMenu *menu = (QMenu*)contextMenu;
    menu->addAction( QIcon( ":icon/cancel" ), tr( "Cancel" ), this, SLOT( fileRevert() ) );
#else
    QToolBar *bar = new QToolBar( this );
    addToolBar(bar);
#ifdef QTOPIA4_TODO
    bar->setHorizontalStretchable( true );
#endif
    bar->setMovable(false);
    menu = bar;

    QMenuBar *mb = new QMenuBar( bar );
    bar->addWidget(mb);
    QMenu *file = new QMenu( tr( "File" ), this );
    QMenu *edit = new QMenu( tr( "Edit" ), this );
    QMenu *font = new QMenu( tr( "View" ), this );

    mb->addMenu( file );
    mb->addMenu( edit );
    mb->addMenu( font );

    bar = new QToolBar( this );
    bar->setMovable(false);
    addToolBar(bar);
    editBar = bar;

    bar->addAction(newAction);
    file->addAction(newAction);
    bar->addAction(openAction);
    file->addAction(openAction);
    bar->addAction(propAction);
    file->addAction(propAction);
    bar->addAction(cutAction);
    edit->addAction(cutAction);
    bar->addAction(copyAction);
    edit->addAction(copyAction);
    bar->addAction(pasteAction);
    edit->addAction(pasteAction);
    bar->addAction(findAction);
    edit->addAction(findAction);
    font->addAction(zin);
    font->addAction(zout);
    font->addSeparator();
    font->addAction(wa);
    font->addAction(fixedAction);
#endif

    int defsize;
    bool wrap, fixedwidth;
    {
        QSettings cfg("Trolltech","TextEdit");
        cfg.beginGroup("View");
        defsize = cfg.value("FontSize",10).toInt();
        wrap = cfg.value("Wrap",true).toBool();
        fixedwidth = cfg.value("Fixed-width", false).toBool();
    }

    connect( editor, SIGNAL(findWrapped()), this, SLOT(findWrapped()) );
    connect( editor, SIGNAL(findNotFound()), this, SLOT(findNotFound()) );
#ifdef QTOPIA_PHONE
    connect( editor, SIGNAL(copyAvailable(bool)), cutAction, SLOT(setVisible(bool)) );
    connect( editor, SIGNAL(copyAvailable(bool)), copyAction, SLOT(setVisible(bool)) );
#endif

    // create search bar on demand
    searchBar = 0;
    searchEdit = 0;
    searchVisible = false;

    connect( fileSelector, SIGNAL(newSelected()), this, SLOT(newFile()) );
    connect( fileSelector, SIGNAL(documentSelected(const QContent&)), this, SLOT(openFile(const QContent&)) );
    fileOpen();

    connect( qApp->clipboard(), SIGNAL(dataChanged()), this, SLOT(clipboardChanged()) );
    connect( qApp, SIGNAL(contentChanged(const QContentIdList&,const QContent::ChangeType)),
        this, SLOT(contentChanged(const QContentIdList&,const QContent::ChangeType)));

    resize( 200, 300 );

    variableFontSize = defsize;
    zoomOutLast = true;
    setFontSize(defsize,zoomOutLast);
    wa->setChecked(wrap);

    fixedAction->setChecked(fixedwidth);
    setFixedWidth(fixedAction->isChecked());

    if ( qApp->argc() == 3 && qApp->argv()[1] == QLatin1String("-f") )
        setDocument(qApp->argv()[2]);
}

TextEdit::~TextEdit()
{
    if (fontsize)
        delete [] fontsize;

    QSettings cfg("Trolltech","TextEdit");
    cfg.beginGroup("View");
    QFont f = editor->font();
    cfg.setValue("FontSize",f.pointSize());
    cfg.setValue("Wrap",editor->lineWrapMode() == QTextEdit::WidgetWidth);
    cfg.setValue("Fixed-width", fixedAction->isChecked() ? "1" : "0");
}

//
// Figure out how many "zoom" levels there for the given font.
//
void
TextEdit::setupFontSizes(void)
{
    QFontDatabase       fd;
    QList<int> pointSizes = fd.pointSizes(editor->font().family().toLower());
    QList<int>::Iterator it;

    nfontsizes = pointSizes.count();
    fontsize = new int[nfontsizes];

    if (fontsize) {
        int i = 0;
        for (it = pointSizes.begin(); it != pointSizes.end(); ++it) {
            int foo = *it;
            fontsize[i++] = foo;
        }
    } else {
        nfontsizes = 0;
    }
}

void TextEdit::zoomIn()
{
    zoomOutLast = false;
    variableFontSize = editor->font().pointSize()+1;
    setFontSize(variableFontSize,false);
}

void TextEdit::zoomOut()
{
    zoomOutLast = true;
    variableFontSize = editor->font().pointSize()-1;
    setFontSize(variableFontSize,true);
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
        zin->setVisible(s != fontsize[nfontsizes-1]);
        zout->setVisible(s != fontsize[0]);

        zinE = zin->isVisible();
        zoutE = zout->isVisible();
    }
}

void TextEdit::setWordWrap(bool y)
{
    bool state = editor->document()->isModified();
    editor->setLineWrapMode(y ? QTextEdit::WidgetWidth : QTextEdit::NoWrap );
    editor->document()->setModified( state );
}

void TextEdit::setFixedWidth(bool y)
{
    if (y) {
        editor->setFont(QFont("fixed"));
        zinE = zin->isVisible();
        zoutE = zout->isVisible();
        zin->setVisible(false);
        zout->setVisible(false);
    } else {
        editor->setFont(QFont());
        setFontSize(variableFontSize,zoomOutLast);
        zin->setVisible(zinE);
        zout->setVisible(zoutE);
    }
}

void TextEdit::clipboardChanged()
{
    pasteAction->setVisible( !qApp->clipboard()->text().isEmpty() );
}

void TextEdit::contentChanged( const QContentIdList& idList, const QContent::ChangeType type )
{
    if ( !doc )
        return;

    foreach( QContentId id, idList ) {
        if ( doc->id() != id )
            continue;
         if ( QContent::Removed == type ) {
            fileRevert();
        } else {
            doc = new QContent( id );
            updateCaption(doc->name());
        }
         break;
    }
}


void TextEdit::fileNew()
{
    save();
    newFile();
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
                QtopiaServiceRequest req( "CleanupWizard", "showCleanupWizard()");
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
    editorStack->setCurrentIndex(1);
    updateCaption();
}

void TextEdit::fileRevert()
{
#ifdef QTOPIA_PHONE
    if (wasCreated)
        doc->removeFiles();
    else if (saved)
        doc->save(backup.toUtf8());
#endif
    clear();
#ifdef QTOPIA_PHONE
    if( qCopActivated ) {
        close();
        canceled = true;
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

        addToolBar( Qt::TopToolBarArea, searchBar );

        searchEdit = new QLineEdit( searchBar );
        searchBar->addWidget( searchEdit );
        connect( searchEdit, SIGNAL(textChanged(const QString&)),
                this, SLOT(search()) );
        connect( searchEdit, SIGNAL(editingFinished()/*returnPressed()*/), this, SLOT(searchNext()));


#ifdef QTOPIA_PHONE
        findTb = new QToolButton( searchBar );
        findTb->setIcon( QIcon( ":icon/i18n/next" ) );
        findTb->setText( tr( "Find Next" ) );
        findTb->setWhatsThis( tr("Find the next occurrence of the search text.") );
        searchBar->addWidget( findTb );
        connect( findTb, SIGNAL( clicked() ), this, SLOT( search() ) );
#endif
    }
    if ( s ) {
        searchBar->show();
        searchVisible = true;
        searchEdit->setFocus();
#ifdef QTOPIA_PHONE
        if( !Qtopia::mousePreferred() ) {
            if (!searchEdit->hasEditFocus())
                searchEdit->setEditFocus(true);
        }
#endif
    } else {
        searchVisible = false;
#ifdef QTOPIA_PHONE
        if( !Qtopia::mousePreferred() ) {
            if (searchEdit->hasEditFocus())
                searchEdit->setEditFocus(false);
        }
#endif
        editor->setFocus();
        searchBar->hide();
#ifdef QTOPIA_PHONE
        if( !Qtopia::mousePreferred() ) {
            if (!editor->hasEditFocus())
                editor->setEditFocus(true);
        }
#endif
    }
}

void TextEdit::search()
{
    editor->findText( searchEdit->text(), false );
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
    Qtopia::statusMessage( tr("Find: reached end") );
}

void TextEdit::findNotFound()
{
    Qtopia::statusMessage( tr("Find: not found") );
}

void TextEdit::newFile()
{
    clear();
    doc = new QContent;
    doc->setType("text/plain");
    editorStack->setCurrentIndex(0);
    editor->setFocus();
    editor->document()->setModified(false);
    setReadOnly(false);
    updateCaption();
}

void TextEdit::setDocument(const QString& f)
{
#ifdef QTOPIA_PHONE
    qCopActivated = true;
#endif
    save();
    QContent nf(f);
    nf.setType("text/plain");
    openFile(nf);
    showEditTools();
    // Show filename in caption
    QString name;
    if ( nf.linkFileKnown() && !nf.name().isEmpty() ) {
        name = nf.name();
    } else {
        name = f;
        int sep = name.lastIndexOf( '/' );
        if ( sep > 0 )
            name = name.mid( sep+1 );
    }
    updateCaption( name );
}

void TextEdit::openFile( const QContent &f )
{
    clear();
    QString txt;
    QByteArray ba;
    bool needsave = false;
    if (f.load(ba)) {
        txt = QString::fromUtf8(ba, ba.size());
        if ( strlen(txt.toUtf8()) != (size_t)ba.size() ) {
            // not UTF8
#ifdef QTOPIA4_TODO
            QTextCodec* codec = QTextCodec::codecForContent(ba.data(),ba.size());
            if ( codec ) {
                txt = codec->toUnicode(ba);
                needsave = true;
            }
#else
            //XXX wrong!
            txt = QString::fromLatin1(ba, ba.size());
#endif
        }
    }
    fileNew();
    if ( doc )
        delete doc;
    doc = new QContent(f);
#ifdef QTOPIA_PHONE
    backup = txt;
#endif
    editor->setDocument(new QTextDocument(txt, editor));
    editor->document()->setModified(needsave);
    updateCaption();
}

void TextEdit::showEditTools()
{
    if ( !doc )
        close();
    fileSelector->hide();
    editorStack->setCurrentIndex(0);
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
    if ( !editor->document()->isModified() ) {
        if( wasCreated ) doc->removeFiles();
        delete doc;
        doc = 0;
        return true;
    }

    QString rt = editor->toPlainText();

    if ( doc->name().isEmpty() )
        doc->setName(calculateName(rt));

    if (!doc->save(rt.toUtf8()))
        return false;
    delete doc;
    doc = 0;
    editor->document()->setModified( false );
    return true;
}

QString TextEdit::calculateName(QString rt)
{
    QString pt = rt.simplified();
    int i = pt.indexOf( ' ' );
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
        doc->setName(calculateName(editor->document()->toPlainText()));

    //
    // Document properties operations depend on the file being
    // up-to-date.  Force a write before changing properties.
    //
    wasCreated = wasCreated || !doc->fileKnown();

    if (!doc->save(editor->document()->toPlainText().toUtf8()))
        return;

#ifdef QTOPIA_PHONE
    saved = true;
#endif

    QDocumentPropertiesDialog *lp = new QDocumentPropertiesDialog(*doc, this);
    if (QtopiaApplication::execDialog(lp)) {
        updateCaption(doc->name());
    }
    delete lp;
}

void TextEdit::clear()
{
    delete doc;
    doc = 0;
    editor->clear();
#ifdef QTOPIA_PHONE
    saved = false;
#endif
    wasCreated = false;
}

void TextEdit::updateCaption( const QString &name )
{
    if ( !doc )
        setWindowTitle( tr("Notes") );
#ifndef QTOPIA_PHONE
    else {
        QString s = name;
        if ( s.isNull() )
            s = doc->name();
        if ( s.isEmpty() )
            s = tr( "Unnamed" );
        setWindowTitle( s + " - " + tr("Notes") );
    }
#else
    Q_UNUSED( name )
#endif
}

void TextEdit::accept()
{
    fileOpen();
}

void TextEdit::message(const QString& msg, const QByteArray& data)
{
    if ( msg == "viewFile(QString)" || msg == "openFile(QString)" ) {
        save();
#ifdef QTOPIA_PHONE
        qCopActivated = true;
#endif
        QDataStream d(data);
        QString filename;
        d >> filename;

        //
        // .desktop files should _not_ be able to be edited easily,
        // as they are generated by the server.  Force opening the
        // file they refer to, rather than the .desktop file.
        //
        if (!filename.contains(".desktop")) {
            if (filename.trimmed().isEmpty()){
                newFile();
            }else{
            QContent dc;
                dc.setFile(filename);
                dc.setType("text/plain");
                openFile(dc);
            }
        } else {
        openFile(QContent(filename));
        }
        showEditTools();
        updateCaption( filename );
        if ( msg == "viewFile(QString)" )
            setReadOnly(true);
    }
}

void TextEdit::setReadOnly(bool y)
{
    editor->setReadOnly(y);
    if ( y )
        editor->document()->setModified(false);
}

void TextEdit::closeEvent( QCloseEvent* e )
{
#ifdef QTOPIA_PHONE
    if( searchVisible ) {
        findAction->setChecked( false );
    } else {
        if (editorStack->currentIndex() == 0) {
            if (qCopActivated) {
                if (!canceled)
                    save();
                e->accept();
            } else {
                fileOpen();
                e->ignore();
            }
        } else {
            e->accept();
        }
    }
#else
    if ( editorStack->currentIndex() == 0 )
        fileOpen();
    e->accept();
#endif
}

void TextEdit::print()
{
    QtopiaServiceRequest srv( "Print", "printHtml(QString)" );
    srv << editor->toHtml();
    srv.send();
}

#include "textedit.moc"
