/****************************************************************************
**
** Copyright (C) 2007-2008 TROLLTECH ASA. All rights reserved.
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

#include "infoscreen.h"
#include "iviewer.h"
#include <QStringList>

InfoScreen::InfoScreen(IViewer *iviewer)
: QTextEdit(), _viewer(iviewer)
{
    setupUi();
    createActions();
    createMenu();
}

void InfoScreen::createActions()
{
}

void InfoScreen::createMenu()
{
}

void InfoScreen::setupUi() 
{
    setReadOnly(true);
}

void InfoScreen::setImage(const QContent &content) 
{
    QStringList keys;
    QStringList values;

    keys << "Name:"; 
    values << content.name();
    keys << "Type:";
    values << content.type();
    keys << "Size:";
    values << QString("%1kB").arg(content.size()/1024);
    keys << "Modified:";
    values << content.lastUpdated().toString(Qt::LocalDate);

    QString html = "<table>";
    for (int i=0; i<keys.count(); i++) {
        QString key = keys[i];
        QString value = values[i];
        html += QString("<tr><th>%1</th><td>%2</td></tr>").arg(key, value);
    }

    html += "</table>";
    document()->setHtml(html);
}
