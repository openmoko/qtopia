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
#ifndef TRAYICON_H
#define TRAYICON_H

#include <QObject>

class TrayIconPrivate;

class QIcon;
class QMenu;
class QPoint;
class QAction;
class QPixmap;

class TrayIcon : public QObject
{
    friend class TrayIconPrivate;
    Q_OBJECT
public:
    TrayIcon( QObject *parent = 0 );
    virtual ~TrayIcon();

    bool installed();
    void show();
    void hide();

signals:
    void clicked();
    void sync();
    void quit();

private slots:
    void setState( int state );
    void lastWindowClosed();
    void connectionMessage( const QString &message, const QByteArray &data );

private:
    void popup( const QPoint &p );
    void iconClicked();

    QMenu *popupMenu;
    QAction *openAction;
    QAction *syncAction;
    int mState;

    TrayIconPrivate *d;
    void setIcon( const QPixmap &pm );
};

#endif
