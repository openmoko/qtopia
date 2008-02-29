/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS and its licensors.
** All rights reserved.
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
** See below for additional copyright and license information
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#include "pickboard.h"
#include "pickboardpicks.h"
#include "pickboardcfg.h"

#include <qtopia/global.h>
#include <qtopia/resource.h>

#include <qpainter.h>
#include <qlist.h>
#include <qbitmap.h>
#include <qlayout.h>
#include <qvbox.h>
#include <qdialog.h>
#include <qscrollview.h>
#include <qpopupmenu.h>
#include <qhbuttongroup.h>
#include <qpushbutton.h>
#include <qmessagebox.h>
#ifdef QWS
#include <qwindowsystem_qws.h>
#endif

/*! \class Pickboard
  \brief The Pickboard class provides an input method
  based on a virtual keyboard combined with word-completion.

*/

class PickboardPrivate {
public:
    PickboardPrivate(Pickboard* parent)
    {
	picks = new PickboardPicks(parent);
	picks->initialise();
	picks->setBackgroundMode(QWidget::PaletteButton);
	menu = new QPushButton(parent);
	menu->setSizePolicy(QSizePolicy(QSizePolicy::Minimum,QSizePolicy::Expanding));
	menu->setPixmap(Resource::loadPixmap("pickboard/menu"));
	QObject::connect(menu,SIGNAL(clicked()),picks,SLOT(doMenu()));
	QObject::connect(picks,SIGNAL(key(ushort,ushort,ushort,bool,bool)),
	    parent,SIGNAL(key(ushort,ushort,ushort,bool,bool)));
    }

    PickboardPicks* picks;
    QPushButton* menu;
};

// QDOC_SKIP_BEGIN

Pickboard::Pickboard(QWidget* parent, const char* name, WFlags f) :
    QFrame(parent,name,f)
{
    (new QHBoxLayout(this))->setAutoAdd(TRUE);
    d = new PickboardPrivate(this);
// under Win32 we may not have smallsmooth font
#ifndef Q_OS_WIN32
    setFont( QFont( "smallsmooth", 9 ) );
#endif
}

/*! \internal */

Pickboard::~Pickboard()
{
    delete d;
}

/*! \internal */

void Pickboard::resetState()
{
    d->picks->resetState();
}

// QDOC_SKIP_END
