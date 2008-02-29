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

#include "proxiespage.h"
#include "proxiespagebase_p.h"
#include <qtopia/config.h>
#include <qtopia/qpeapplication.h>

#include <qcombobox.h>
#include <qlineedit.h>
#include <qframe.h>
#include <qvalidator.h>
#include <qspinbox.h>
#include <qradiobutton.h>
#include <qlineedit.h>

ProxiesPage::ProxiesPage( QWidget* parent ) :
    QVBox(parent)
{
    d = new ProxiesPageBase( this );

#ifdef QTOPIA_PHONE
    QPEApplication::setInputMethodHint(d->http_host,"url");
    QPEApplication::setInputMethodHint(d->ftp_host,"url");
    QPEApplication::setInputMethodHint(d->noproxies,"url");
#endif

    connect(d->type,SIGNAL(activated(int)),this,SLOT(typeChanged(int)));
}

void ProxiesPage::typeChanged(int t)
{
    switch (t) {
     case 0: d->autopanel->hide(); d->setpanel->hide(); break;
     case 1: d->setpanel->hide(); d->autopanel->show(); break;
     case 2: d->autopanel->hide(); d->setpanel->show(); break;
    }
}

class ProxyValidator : public QValidator {
public:
    ProxyValidator( QWidget * parent, const char *name = 0 ) :
	QValidator(parent,name)
    {
    }
 
    State validate( QString &s, int &pos ) const
    {
	int i;
	for (i=0; i<(int)s.length(); i++) {
	    if ( s[i] == '/' || s[i] == ',' || s[i] == ';' || s[i] == '\n' || s[i] == '\r' )
		s[i] = ' ';
	}
	for (i=0; i<(int)s.length()-1; ) {
	    if ( s[i] == ' ' && s[i+1] == ' ' ) {
		if (pos>i) pos--;
		s = s.left(i)+s.mid(i+2);
	    } else
		i++;
	}
	return Valid;
    }
};

void ProxiesPage::readConfig(Config& cfg)
{
    int t = cfg.readNumEntry("type",0);
    d->type->setCurrentItem(t);
    typeChanged(t);

    QString s;

    s = cfg.readEntry("autoconfig");
    if ( !s.isEmpty() )
	d->autoconfig->insertItem(s);


    s = cfg.readEntry("httphost");
    if ( !s.isEmpty() )
	d->http_host->setText(s);

    int i;
    i = cfg.readNumEntry("httpport");
    if ( i>0 )
	d->http_port->setValue(i);

    s = cfg.readEntry("ftphost");
    if ( !s.isEmpty() )
	d->ftp_host->setText(s);

    i = cfg.readNumEntry("ftpport");
    if ( i>0 )
	d->ftp_port->setValue(i);

    s = cfg.readEntry("noproxies");
    d->noproxies->setValidator(new ProxyValidator(this));
    d->noproxies->setText(s);
}

void ProxiesPage::writeConfig( Config &cfg )
{
    cfg.writeEntry("type",d->type->currentItem());
    cfg.writeEntry("autoconfig", d->autoconfig->currentText());
    cfg.writeEntry("httphost", d->http_host->text());
    cfg.writeEntry("httpport", d->http_port->text());
    cfg.writeEntry("ftphost", d->ftp_host->text());
    cfg.writeEntry("ftpport", d->ftp_port->text());
    cfg.writeEntry("noproxies", d->noproxies->text());
}

