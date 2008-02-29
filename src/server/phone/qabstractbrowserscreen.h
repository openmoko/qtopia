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

#ifndef _QABSTRACTBROWSERSCREEN_H_
#define _QABSTRACTBROWSERSCREEN_H_

#include <QWidget>

class QAbstractBrowserScreen : public QWidget
{
Q_OBJECT
public:
    QAbstractBrowserScreen(QWidget *parent = 0, Qt::WFlags f = 0)
    : QWidget(parent, f) {}

    virtual QString currentView() const = 0;
    virtual bool viewAvailable(const QString &) const = 0;

public slots:
    virtual void resetToView(const QString &) = 0;
    virtual void moveToView(const QString &) = 0;

signals:
    void currentViewChanged(const QString &);
    void applicationLaunched(const QString &);
};

#endif // _QABSTRACTBROWSERSCREEN_H_
