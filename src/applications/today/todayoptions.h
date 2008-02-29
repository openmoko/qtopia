/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** Licensees holding valid Qtopia Developer license may use this
** file in accordance with the Qtopia Developer License Agreement
** provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING
** THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
** PURPOSE.
**
** email sales@trolltech.com for information about Qtopia License
** Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/



#ifndef TODAY_OPTIONS_H
#define TODAY_OPTIONS_H

#include "todayoptionsbase.h"
#include <qvaluelist.h>

#include "today.h"

class TodayOptions : public TodayOptionsBase
{
    Q_OBJECT
public:
    TodayOptions(QWidget *parent, const char *name, WFlags fl = 0 );

    void setPlugins(QValueList<TodayPlugin> list);

protected slots:
    void accept();

    void itemMoveUp();
    void itemMoveDown();
    void itemCut();
    void addItemToView(QListViewItem *item);

private:
    void writeConfig();

private:
    QValueList<TodayPlugin> pList;
};

#endif
