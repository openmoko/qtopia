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
#ifndef DATEBOOK_PLUGIN_H
#define DATEBOOK_PLUGIN_H


#include <qstring.h>
#include <qwidget.h>
#include <qobject.h>
#include <qpixmap.h>

#include <qtopia/todayplugininterface.h>

class DatebookPluginPrivate;

class QTOPIA_PLUGIN_EXPORT DatebookPlugin : public QObject, public TodayPluginInterface
{
    Q_OBJECT
public:
    DatebookPlugin(QObject *parent, const char *name);
    ~DatebookPlugin();

    virtual QString name() const { return "DateBook plugin"; }
    virtual QPixmap icon() const;
    
    virtual QString html(uint charWidth, uint lineHeight) const;

    virtual bool configurable() const {return 1;}
    virtual QWidget* widget(QWidget *);
    virtual void accepted(QWidget *) const;

    virtual void itemSelected(const QString &) const;
    
signals:
    void reload();

protected slots:
    void datebookUpdated();

private:
    DatebookPluginPrivate *d;
};

#endif
