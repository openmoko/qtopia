/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#include "scribble.h"
#include "scribbler.h"

#include <qtopiaapplication.h>
#include <qdocumentselector.h>

#include <qpemenubar.h>
#include <qpetoolbar.h>
#include <resource.h>

#include <qaction.h>
#include <qdir.h>
#include <qfile.h>
#include <qlistview.h>

#include <unistd.h>

//
//
//
Scribble::Scribble(QWidget *parent, const char *name, int wFlags):
    QMainWindow(parent, name, wFlags)
{
    setCaption(tr("Scribble Pad"));

    setToolBarsMovable(false);

    toolbar = new QPEToolBar(this);
    toolbar->setHorizontalStretchable(true);

    menubar = new QPEMenuBar(toolbar);
    QPopupMenu *file = new QPopupMenu(menubar);

    menubar->insertItem(tr("File"), file);

    toolbar = new QPEToolBar(this);

    newAction = new QAction(tr("New"), QIcon(":icon/new"),
        QString(), 0, this, 0);
    newAction->addTo(toolbar);
    newAction->addTo(file);
    connect(newAction, SIGNAL(triggered()), this, SLOT(newScribble()));


    filelist = new QDocumentSelector("image/x-notepad", this, "fs", false, false);
    connect(filelist, SIGNAL(documentSelected(const QContent&)),
            this, SLOT(editScribble(const QContent&)));

    setCentralWidget(filelist);
}

//
//
//
Scribble::~Scribble(void)
{
}

//
//
//
void
Scribble::newScribble(void)
{
    Scribbler   scrib;
    QPixmap     foo(qApp->desktop()->width(), qApp->desktop()->height());

    foo.fill(Qt::white);
    scrib.setPicture(foo);

    if (QtopiaApplication::execDialog(&scrib) == true) {
        //
        // TODO: save
        //
    }
}

//
//
//
void Scribble::editScribble(const QContent &f)
{
    Scribbler   scrib;

    if (load(f.fileName(), scrib)) {
        if (QtopiaApplication::execDialog(&scrib) == true) {
            //
            // TODO: save
            //
        }
    }
}

//
//
//
void
Scribble::save(const QString &, const Scribbler &)
{
    //
    // TODO: implement.
    //
}

//
//
//
bool
Scribble::load(const QString &filename, Scribbler &scrib)
{
    QImageIO    iio;
    QPixmap     pm;
    bool        loaded = false;

    iio.setFileName(filename);
    iio.read();
    if (iio.image().bits()) {
        //
        // Convert to current depth (Palm NotePad images are
        // depth 1 (?)).
        //
        pm.convertFromImage(iio.image(), ColorOnly);
        scrib.setPicture(pm);
        scrib.setDesc(filename);

        loaded = true;
    }

    return loaded;
}
