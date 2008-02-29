/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
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

#ifndef _e2_BROWSER_H_
#define _e2_BROWSER_H_

#include "phone/qabstractbrowserscreen.h"
#include <QStringList>
class E2Menu;
class E2Button;
class E2BrowserStack;
class E2BrowserScreen : public QAbstractBrowserScreen
{
Q_OBJECT
public:
    E2BrowserScreen(QWidget *parent = 0, Qt::WFlags flags = 0);

    virtual QString currentView() const;
    virtual bool viewAvailable(const QString &) const;
    virtual void resetToView(const QString &);
    virtual void moveToView(const QString &);

private slots:
    void categoryChanged(int);
    void viewChanged(const QString &);
    void menuClicked(int);

private:
    void toggleViewMode();
    enum { ListView, IconView } m_mode;
    E2Button *m_catMenu;
    QStringList m_categories;
    QStringList m_categoryNames;
    E2BrowserStack *m_stack;
    E2Menu *m_menu;
};

#endif // _e2_BROWSER_H_
