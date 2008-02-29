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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <qdplugindefs.h>

#include <QMainWindow>
#include <QMap>

#define QD_DEFAULT_WITDH 600
#define QD_DEFAULT_HEIGHT 500

class QtopiaDesktopApplication;
class ConnectionStatusWidget;
class StatusBar;
class PluginChooser;

class QStackedWidget;
class QProgressBar;
class QAction;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow( QDAppPlugin *plugin, QWidget *appWidget, QWidget *parent = 0 );
    virtual ~MainWindow();

    void loadGeometry();
    QIcon icon() const;

public slots:
    void slotSetSingle( bool single );

private slots:
    void statusMessage( const QString &message, const QByteArray &data );

signals:
    void setSingle( bool single );
    void closing( QDAppPlugin *plugin );

    void import();
    void sync();
    void syncall();
    void backuprestore();
    void settings();
    void manual();
    void about();
    void quit();

private:
    void closeEvent( QCloseEvent *e );
    void stealMenuAndToolbars( QMainWindow *mw );
    void setupMenuAndToolbars();
    void resizeEvent( QResizeEvent *e );

    QDAppPlugin *plugin;
    ConnectionStatusWidget *mConnectionStatusIcon;
    PluginChooser *pluginChooser;
    StatusBar *statusBar;
    QMenu *windowMenu;
    QAction *singleAction;
};

#endif
