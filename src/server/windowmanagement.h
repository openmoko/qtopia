/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
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

#ifndef _WINDOWMANAGEMENT_H_
#define _WINDOWMANAGEMENT_H_

#include <qobject.h>

class QRect;
class QString;
class QSize;
class QWSWindow;
class QWidget;
class WindowManagementPrivate;
class WindowManagement : public QObject
{
    Q_OBJECT
public:
    WindowManagement(QObject *parent = 0);
    virtual ~WindowManagement();

    enum DockArea { Top, Bottom, Left, Right };
    static void protectWindow(QWidget *);
    static void dockWindow(QWidget *, DockArea, int screen=-1);
    static void dockWindow(QWidget *, DockArea, const QSize &, int screen=-1);

signals:
    void windowActive(const QString &, const QRect &, QWSWindow *);
    void windowCaption(const QString &);

private:
    WindowManagementPrivate * d;
};

#endif // _WINDOWMANAGEMENT_H_
