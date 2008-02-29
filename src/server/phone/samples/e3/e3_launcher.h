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

#ifndef _E3LAUNCHER_H_
#define _E3LAUNCHER_H_

#include "qabstractserverinterface.h"
#include <QContent>
#include <QContentSet>
#include <QPair>
#include <QtopiaServiceRequest>
#include <QHash>
#ifdef Q_WS_QWS
#include <qwindowsystem_qws.h>
#endif
#include "phone/themebackground_p.h"

class LauncherView;
class PhoneHeader;
class ContextLabel;
class QExportedBackground;
class QSettings;
class QAbstractBrowserScreen;
class E3Today;
class ThemedView;
class QAbstractDialerScreen;
class QPhoneCall;
class CallScreen;
class QUniqueId;
class CellModemManager;
class E3ServerInterface : public QAbstractServerInterface
{
Q_OBJECT
public:
    E3ServerInterface(QWidget *parent, Qt::WFlags flags);

protected:
    virtual void keyPressEvent(QKeyEvent *);
    virtual bool event(QEvent *);
    virtual bool eventFilter(QObject *, QEvent *);

private slots:
    void acceptIncoming();
    void loadTheme();
    void launch(QContent);
    void received(const QString &, const QByteArray &);
    void showCallscreen();
    void requestDial(const QString &);
    void showDialer(const QString &);
#ifdef Q_WS_QWS
    void windowEvent(QWSWindow*,QWSServer::WindowEvent);
#endif

private:
    void header();
    void context();
    void quickApps();
    void idleContext();
    void showApps();

    typedef QPair<QString, QtopiaServiceRequest> ButtonBinding;
    ButtonBinding buttonBinding(QSettings &) const;

    LauncherView *m_view;
    PhoneHeader *m_header;
    ContextLabel *m_context;
    QContentSet m_idleApps;
    QAbstractBrowserScreen *m_browser;
    QAbstractDialerScreen *dialer();
    QAbstractDialerScreen *m_dialer;
    CallScreen *callscreen();
    CallScreen *m_callscreen;
    E3Today *m_today;
    ThemedView *m_theme;
    ThemeBackground *m_tbackground;
    QWidget *m_titleSpacer;
    CellModemManager *m_cell;

    typedef QHash<int, QtopiaServiceRequest> IdleKeys;
    IdleKeys m_idleKeys;
};

#endif // _E3LAUNCHER_H_
