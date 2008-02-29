/**********************************************************************
** Copyright (C) 2000-2004 Trolltech AS.  All rights reserved.
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

#include "scribble.h"
#include "scribbler.h"

#include <qpe/qpeapplication.h>
#include <qpe/fileselector.h>
#include <qpe/global.h>
#include <qpe/qpemenubar.h>
#include <qpe/qpemessagebox.h>
#include <qpe/qpetoolbar.h>
#include <qpe/resource.h>

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

    setToolBarsMovable(FALSE);

    toolbar = new QPEToolBar(this);
    toolbar->setHorizontalStretchable(TRUE);

    menubar = new QPEMenuBar(toolbar);
    QPopupMenu *file = new QPopupMenu(menubar);

    menubar->insertItem(tr("File"), file);

    toolbar = new QPEToolBar(this);

    newAction = new QAction(tr("New"), Resource::loadIconSet("new"),
	QString::null, 0, this, 0);
    newAction->addTo(toolbar);
    newAction->addTo(file);
    connect(newAction, SIGNAL(activated()), this, SLOT(newScribble()));


    filelist = new FileSelector("image/x-notepad", this, "fs", FALSE, FALSE);
    connect(filelist, SIGNAL(fileSelected(const DocLnk&)),
	this, SLOT(editScribble(const DocLnk&)));

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
    Scribbler	scrib;
    QPixmap	foo(qApp->desktop()->width(), qApp->desktop()->height());

    foo.fill(Qt::white);
    scrib.setPicture(foo);

    if (QPEApplication::execDialog(&scrib) == TRUE) {
	//
	// TODO: save
	//
    }
}

//
//
//
void
Scribble::editScribble(const DocLnk &f)
{
    Scribbler	scrib;

    if (load(f.file(), scrib)) {
	if (QPEApplication::execDialog(&scrib) == TRUE) {
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
    QImageIO	iio;
    QPixmap	pm;
    bool	loaded = FALSE;

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

	loaded = TRUE;
    }

    return loaded;
}
