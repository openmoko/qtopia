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

#ifndef _E3PHONEBROWSER_H_
#define _E3PHONEBROWSER_H_

#include "phone/qabstractbrowserscreen.h"
#include <QContentSet>

class QAction;
class E3BrowserScreenStack;
class E3BrowserScreen : public QAbstractBrowserScreen
{
Q_OBJECT
public:
    E3BrowserScreen(QWidget *parent = 0, Qt::WFlags flags = 0);

    virtual QString currentView() const;
    virtual bool viewAvailable(const QString &) const;
    virtual void resetToView(const QString &);
    virtual void moveToView(const QString &);

protected:
    virtual void closeEvent(QCloseEvent *);

private slots:
    void listMode();
    void gridMode();

private:
    QAction *listAction;
    QAction *gridAction;
    E3BrowserScreenStack *m_stack;
};

#endif // _E3PHONEBROWSER_H_
