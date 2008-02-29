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
#include "addnetworksetting.h"
#include "networksettings.h"

#include <qtopia/qpeapplication.h>
#include <qtopia/config.h>
#include <qtopia/contextmenu.h>
#include <qtopia/network.h>

#include <qlistbox.h>
#include <qaction.h>
#include <qmessagebox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>

/* 
 *  Constructs a AddNetworkSetting which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
AddNetworkSetting::AddNetworkSetting( bool first, QWidget* parent,  const char* name, WFlags fl ) :
    QDialog(parent,name,TRUE,fl)
{
    setCaption(tr("Add Service"));

    QVBoxLayout *vb = new QVBoxLayout(this,4);
    vb->setAutoAdd(TRUE);

#ifdef QTOPIA_PHONE
    if ( first )
	new QLabel(tr("<P>No services are currently defined. Choose service to add:"),this);

    ContextMenu *contextMenu = new ContextMenu(this);

    settings = new QListBox(this);
    connect(settings, SIGNAL(selected(int)), this, SLOT(accept()));
    connect(settings, SIGNAL(mouseButtonClicked(int,QListBoxItem*,const QPoint&)), this, SLOT(accept()) );
    QPEApplication::setMenuLike(this,TRUE);

    QAction *a_details = new QAction( tr("Details..."), QString::null, 0, this );
    connect( a_details, SIGNAL( activated() ), this, SLOT( showDetails() ) );
    a_details->addTo(contextMenu);
#else
    Q_UNUSED(first);
    base = new AddNetworkSettingBase(this);
    settings = base->settings;
    connect(settings, SIGNAL(highlighted(int)), this, SLOT(showInfo(int)));
    connect(base->ok, SIGNAL(clicked()), this, SLOT(accept()));
    connect(base->cancel, SIGNAL(clicked()), this, SLOT(reject()));
#endif

#ifdef QWS
    filelist = Network::choices(settings, settingsDir());
#endif

#ifdef QTOPIA_PHONE
    settings->setCurrentItem(0);
#endif
}

/*  
 *  Destroys the object and frees any allocated resources
 */
AddNetworkSetting::~AddNetworkSetting()
{
    // no need to delete child widgets, Qt does it all for us
}

QString AddNetworkSetting::settingsDir() const
{
    return QPEApplication::qpeDir() + "etc/network";
}

QString AddNetworkSetting::selectedFile() const
{
    return *filelist.at(settings->currentItem());
}

QString AddNetworkSetting::info(int i) const
{
    Config cfg(*filelist.at(i), Config::File);
    QString h="<h3>";
    cfg.setGroup("Info");
    
    h += "<img src=Network/" + cfg.readEntry("Type")+'/'+ cfg.readEntry("Type") + "> ";
    h += cfg.readEntry("Name") + "</h3>";
    cfg.setGroup("Help");
    h += "<P>"+cfg.readEntry("Help");
    return h;
}

void AddNetworkSetting::showInfo(int i)
{
    base->help->setText(info(i));
}

void AddNetworkSetting::showDetails()
{
    QMessageBox::information(this,"",info(settings->currentItem()));
}
