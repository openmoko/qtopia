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

#include "textviewer.h"
#include <qdocumentselector.h>
#include <qsoftmenubar.h>
#include <qpushbutton.h>
#include <QFile>
#include <QTextStream>
#include <QMenu>
#include <QDebug>
#include <QKeyEvent>

/*
 *  Constructs a TextViewer which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'
 */
    TextViewer::TextViewer( QWidget *parent, Qt::WFlags f )
: QMainWindow( parent, f )
{
    textArea = new QTextEdit(this);
    textArea->setReadOnly(true);
    setCentralWidget(textArea);
    docSelector = new QDocumentSelector(QContentSet());

    QAction *actionOpen = new QAction(tr("Open Document"), this );
    connect(actionOpen, SIGNAL(triggered()), this, SLOT(openDocument()));
#ifdef QTOPIA_PHONE
    QMenu* menu = QSoftMenuBar::menuFor(textArea);
    menu->addAction(actionOpen);
#else
    ToolBar *toolbar = new QToolBar(this);
    toolbar->setMovable(false);
    addToolBar(toolbar);
    toolbar->addAction(actionOpen);
#endif
    connect(docSelector, SIGNAL(documentSelected(const QContent&)), this, SLOT(documentSelected(const QContent&)));
}

/*
 *  Destroys the object and frees any allocated resources
 */
TextViewer::~TextViewer()
{
    delete docSelector;
    // no need to delete child widgets, Qt does it all for us
}


/*
 * A matter of convenience pressing Key_Select will cause a document to be opened
 */
void TextViewer::keyPressEvent(QKeyEvent *e)
{
    if ( e->key() == Qt::Key_Select){
        e->accept();
        openDocument();
    }else{
        QMainWindow::keyPressEvent(e);
    }
}

void TextViewer::openDocument()
{
    // In this function we demonstrate two methods to set the file filter
    //  to be used with QDocumentSelector. The second method
    //  can be tested by uncomment the following #define

// #define USE_SORTEDFILTER

#ifndef USE_SORTEDFILTER
    // request all documents of mimetype text
    QContentSet docSet(QContentSet::MimeType, "text/*");
    docSelector->setDocuments(docSet);
#else
    // request that the matching documents be sorted in reverse alphanumeric order
    QContentSet::QContentFilter docFilter;
    docFilter.insert(QContentSet::SortOrder, "name desc");
    docFilter.insert(QContentSet::SortOrder, "text/*");
    docSelector->setFilter(docFilter);
#endif

    docSelector->showMaximized();
}

void TextViewer::documentSelected(const QContent & docContent)
{
    // make use of the document selected by the QDocumentSelector widget
    docSelector->hide();
    if (docContent.isValid()){
        QFile f(docContent.file());
        if (f.open(QIODevice::ReadOnly)){
            QTextStream fstream(&f);
            textArea->setHtml(fstream.readAll());
        }else{
            qWarning() << "Unable to read content from file" << docContent.file();
        }

    }else{
        qWarning()<< "Document " << docContent.file() << " is invalid";
    }
}

