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
/**********************************************************************
** Copyright (C) 2001 Devin Butterfield.  All rights reserved.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
**********************************************************************/

#include "cardmon.h"

#include <qtopia/resource.h>

#ifdef QWS
#include <qcopchannel_qws.h>
#endif
#include <qpainter.h>
#include <qmessagebox.h>
#include <qpopupmenu.h>

#include <stdio.h>
#if defined(Q_OS_LINUX) || defined(_OS_LINUX_)
#include <unistd.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

/*! \class CardMonitor
  \brief The CardMonitor class provides a widget to monitor CF cards.

  \legalese
  The CardMonitor applet is distributed under the terms of the GNU
  General Public License, Version 2.  The CardMonitor applet monitors
  PCMCIA and CF card insertion and removal.  The author and primary
  copyright holder of the CardMonitor applet is <a href="mailto:dbutter@wireless.net">Devin Butterfield</a>.

*/

// QDOC_SKIP_BEGIN

CardMonitor::CardMonitor( QWidget *parent ) : QWidget( parent ),
	pm( Resource::loadPixmap( "cardmon/pcmcia" ) ), cardIn( FALSE )
{
#ifndef QT_NO_COP
    QCopChannel* pcmciaChannel = new QCopChannel( "QPE/Card", this );
    connect( pcmciaChannel, SIGNAL(received(const QCString&,const QByteArray&)),
             this, SLOT(pcmciaMessage(const QCString&,const QByteArray&)) );
#endif

    setFixedSize( pm.size() );
    hide();
    getStatus();
}

/*! */

CardMonitor::~CardMonitor()
{
}

/*! */

void CardMonitor::mousePressEvent( QMouseEvent * )
{
    QPopupMenu *menu = new QPopupMenu();
    QString cmd;
    int err=0;
    menu->insertItem( tr("Eject card"), 0 );
    QPoint p = mapToGlobal( QPoint(1, -menu->sizeHint().height()-1) );

    if ( menu->exec( p, 1 ) == 0 ) {
        cmd = "/sbin/cardctl eject"; // No tr
	err = system( (const char *) cmd );
	if ( ( err == 127 ) || ( err < 0 ) ) {
	    qDebug("Could not execute `/sbin/cardctl eject'! err=%d", err);
	    QMessageBox::warning( this, tr("CardMonitor"), tr("<qt>Card eject failed!</qt>"),
	    			  tr("&OK") );
	}
    }
    delete menu;
}


void CardMonitor::pcmciaMessage( const QCString &msg, const QByteArray & )
{
    if ( msg == "stabChanged()" ) {
	if ( getStatus() ) repaint(FALSE);
    }
}

/*! */

bool CardMonitor::getStatus( void )
{
    FILE *fd;
    char line[80];
    int	socknum;
    char id[80];
    bool cardWas=cardIn; // remember last state

    fd = fopen("/var/run/stab", "r");
    if (fd == NULL)
	fd = fopen("/var/state/pcmcia/stab", "r");
    if (fd == NULL )
	fd = fopen("/var/lib/pcmcia/stab", "r");
    if (fd != NULL) {
        fgets(line, 80, fd);
        while (!feof(fd)) {
	    if ((line[0] != '0') && (line[0] != '\n')) {
	    	// currently only socket 0 is supported
	        if (!strncmp(line, "Socket", 6)) { // No tr
		    sscanf(line, "Socket %d: %[^\n]", &socknum, id); // No tr
		    if ( socknum == 0 ) {
		        if ((!strncmp(id, "empty", 5)) && cardIn) { // No tr
		            cardIn = FALSE;
			    hide();
		        } else if (strncmp(id, "empty", 5) && !cardIn) { // No tr
		            cardIn = TRUE;
			    show();
		        }
		    }
		}
	    }
	    fgets(line, 80, fd);
	}
    } else {
	// stab is missing
	cardIn = FALSE;
	hide();
	return FALSE;
    }
    fclose(fd);
    return ((cardWas == cardIn) ? FALSE : TRUE);
}

/*! */

void CardMonitor::paintEvent( QPaintEvent * )
{
    QPainter p( this );

    if ( cardIn ) {
	p.drawPixmap( 0, 0, pm );
    }
    else {
	p.eraseRect( rect() );
    }
}

// QDOC_SKIP_END
