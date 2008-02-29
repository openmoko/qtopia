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
#ifndef QTSINGLEAPPLICATION_H
#define QTSINGLEAPPLICATION_H

#include <qapplication.h>
#ifdef Q_WS_MAC
#include <CoreFoundation/CoreFoundation.h>
#endif

class QtSingletonPrivate;
class QtSingletonSysPrivate;

#if defined(Q_WS_WIN)
#  if !defined(QT_QTSINGLEAPPLICATION_EXPORT) && !defined(QT_QTSINGLEAPPLICATION_IMPORT)
#    define QT_QTSINGLEAPPLICATION_EXPORT
#  elif defined(QT_QTSINGLEAPPLICATION_IMPORT)
#    if defined(QT_QTSINGLEAPPLICATION_EXPORT)
#      undef QT_QTSINGLEAPPLICATION_EXPORT
#    endif
#    define QT_QTSINGLEAPPLICATION_EXPORT __declspec(dllimport)
#  elif defined(QT_QTSINGLEAPPLICATION_EXPORT)
#    undef QT_QTSINGLEAPPLICATION_EXPORT
#    define QT_QTSINGLEAPPLICATION_EXPORT __declspec(dllexport)
#  endif
#else
#  define QT_QTSINGLEAPPLICATION_EXPORT
#endif

class QT_QTSINGLEAPPLICATION_EXPORT QtSingleApplication : public QApplication
{
    Q_OBJECT
public:
    QtSingleApplication(const QString &id, int &argc, char **argv, Type = GuiClient);
#ifdef Q_WS_X11
    QtSingleApplication(Display* dpy, const QString &id, int argc, char **argv, Qt::HANDLE visual = 0, Qt::HANDLE colormap = 0);
#endif
    ~QtSingleApplication();

    bool isRunning() const;
    QString id() const;

    void initialize(bool activate = true);

protected:
#if defined(Q_WS_X11)
    bool x11EventFilter(XEvent *msg);
#endif

public slots:
    bool sendMessage(const QString &message, int timeout = 5000);
    virtual void activateMainWidget();

signals:
    void messageReceived(const QString& message);

private:
    friend class QtSingletonSysPrivate;
#ifdef Q_WS_MAC
    friend CFDataRef MyCallBack(CFMessagePortRef, SInt32, CFDataRef, void *);
#endif
    QtSingletonPrivate *d;
    QtSingletonSysPrivate *sysd;

    void sysInit();
    void sysCleanup();
};

#endif //QTSINGLEAPPLICATION_H
