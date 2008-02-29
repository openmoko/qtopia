/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
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
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#include "addnetworksetting.h"
#include "networksettings.h"

#include <qtopia/qpeapplication.h>
#include <qtopia/config.h>
#include <qtopia/network.h>

#include <qlistbox.h>
#include <qlabel.h>
#include <qpushbutton.h>

/* 
 *  Constructs a AddNetworkSetting which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
AddNetworkSetting::AddNetworkSetting( QWidget* parent,  const char* name, WFlags fl )
    : AddNetworkSettingBase( parent, name, TRUE, fl )
{
#ifdef QWS
    filelist = Network::choices(settings, settingsDir());
#endif
    connect(settings, SIGNAL(highlighted(int)), this, SLOT(showInfo(int)));
    connect(ok, SIGNAL(clicked()), this, SLOT(accept()));
    connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
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

void AddNetworkSetting::showInfo(int i)
{
    Config cfg(*filelist.at(i), Config::File);
    QString h="<h3>";
    cfg.setGroup("Info");
    h += "<img src=Network/" + cfg.readEntry("Type") + "> ";
    h += cfg.readEntry("Name") + "</h3>";
    cfg.setGroup("Help");
    h += "<P>"+cfg.readEntry("Help");
    help->setText(h);
}
