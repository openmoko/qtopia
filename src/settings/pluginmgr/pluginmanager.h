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

#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include "pluginmanagerbase.h"

class QCheckListItem;
class QLabel;

class PluginManager : public PluginManagerBase
{
    Q_OBJECT
public:
    PluginManager( QWidget *parent=0, const char *name=0, bool modal=FALSE, WFlags f=0 );

protected slots:
    void init();
    void selectionChanged( QListViewItem *i );
    void restart();

protected:
    void drawWait( bool on );
    void accept();
    void done( int );
    void save();

private:
    QLabel *commentLabel;
};

#endif
