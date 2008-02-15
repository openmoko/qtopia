/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
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
#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <qdplugindefs.h>

#include <QDialog>

class PluginChooser;

class QStackedWidget;
class QFrame;

class SettingsDialog : public QDialog
{
    Q_OBJECT
public:
    SettingsDialog( QWidget *parent = 0 );
    virtual ~SettingsDialog();

    void loadGeometry();

signals:
    void applySettings();
    void revertSettings();
    void recheckSettings();

public slots:
    void reject();

private slots:
    void showPlugin( QDAppPlugin *plugin );
    void accept();
    void apply();

private:
    void showEvent( QShowEvent *e );
    void closeEvent( QCloseEvent *e );

    PluginChooser *pluginChooser;
    QStackedWidget *stack;
};

#endif
