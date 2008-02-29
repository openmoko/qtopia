/**********************************************************************
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of Qtopia Environment.
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

#ifndef TODAY_PLUGIN_TASK_H
#define TODAY_PLUGIN_TASK_H

#include <qtopia/qcom.h>
#include <qobject.h>

class QString;
class QWidget;

#ifndef QT_NO_COMPONENT
#ifndef IID_TodayPlugin
#define IID_TodayPlugin QUuid( 0x3a8db66, 0xa2fd, 0xd80, 0x9a2d, 0x4ac, 0xa7, 0x31, 0x4a01, 0x31, 0x32a, 0x7a)
#endif
#endif

class TodayPluginInterface
{
public:
    virtual QString name() const = 0;
    virtual QPixmap icon() const = 0;
    
    virtual QString html(uint charWidth, uint lineHeight) const = 0;

    // Reimplement these if your plugin is configurable
    virtual bool configurable() const = 0;
    virtual QWidget* widget(QWidget *)= 0;
    virtual void accepted(QWidget *) const = 0;

    // Reimplement this method if your plugin should do something when links are clicked
    virtual void itemSelected(const QString &) const = 0;
};

struct TodayInterface:public QUnknownInterface {
    virtual TodayPluginInterface* object(QObject *parent, const char *name) = 0;
    virtual QObject* objectFor(TodayPluginInterface *) = 0;
};

#endif

