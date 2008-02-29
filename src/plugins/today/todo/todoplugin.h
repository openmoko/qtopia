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
#ifndef TODO_PLUGIN_H
#define TODO_PLUGIN_H


#include <qstring.h>
#include <qwidget.h>
#include <qobject.h>
#include <qpixmap.h>

#include <qtopia/todayplugininterface.h>
#include <qtopia/pim/task.h>

class TodoAccess;
class TodoPluginPrivate;
class TodoPluginOptions;

class QTOPIA_PLUGIN_EXPORT TodoPlugin : public QObject, public TodayPluginInterface
{
    Q_OBJECT
public:
    TodoPlugin(QObject *parent, const char *name);
    ~TodoPlugin();

    QString name() const { return "TodoList plugin"; }
    virtual QPixmap icon() const;
    
    virtual QString html(uint charWidth, uint lineHeight) const;

    virtual bool configurable() const {return 1;}
    virtual QWidget* widget(QWidget *);
    virtual void accepted(QWidget *) const;

    virtual void itemSelected(const QString &) const;

signals:
    void reload();

protected slots:
    void tasksUpdated();

private:
    TodoPluginPrivate *d;
};

#endif
