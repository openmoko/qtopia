/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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
#ifndef PLUGINCHOOSER_H
#define PLUGINCHOOSER_H

#include <qdplugindefs.h>

#include <QFrame>

class PluginChooserPrivate;

class QMenu;

class PluginChooser : public QFrame
{
    Q_OBJECT
public:
    PluginChooser( QWidget *parent = 0 );
    virtual ~PluginChooser();

    enum Orientation { Vertical, Horizontal };
    void setOrientation( Orientation orientation );

    enum WidgetType { App, Settings };
    void setWidgetType( WidgetType widgetType );

    void setWindowMenu( QMenu *windowMenu );

public slots:
    void highlightPlugin( QDAppPlugin *plugin );

signals:
    void showPlugin( QDAppPlugin *plugin );

private slots:
    void pluginsChanged();

private:
    PluginChooserPrivate *d;
};

#endif
