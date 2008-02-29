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
#ifndef LAUNCHERTAB_H
#define LAUNCHERTAB_H

#include <qcolor.h>
#include <qstring.h>
#include <qwidget.h>
#include <qrect.h>
//#include <qiconset.h>
#include <qlist.h>
#include <qtabbar.h>
#include <qmap.h>

#include <qapplication.h>
#include <qwindowsstyle.h>
#include <qstyleoption.h>

class LauncherView;

class LauncherTab;
class LauncherTabBar : public QTabBar
{
    Q_OBJECT
public:
    LauncherTabBar(QWidget *parent=0);
    ~LauncherTabBar();

    virtual LauncherTab * addTab(const QString &type,
                                 const QString &visibleText,
                                 const QIcon &icon,
                                 bool forceToEnd = false);

    virtual void removeTab(const QString &type);

    virtual LauncherTab * lookup(const QString &type) const;
    virtual LauncherTab * lookup(int num) const;

    virtual LauncherTab * currentTab() const;
    inline LauncherView * currentView() const;

    void layoutTabs();
public slots:
    virtual void setCurrentTab(const QString &type);

protected:
    virtual void resizeEvent(QResizeEvent *);

private slots:
    void changed(int idx);

private:
    friend class LauncherTabStyle;

    int requiredSpace() const;

    unsigned int m_itemsOnEnd;
    QList<LauncherTab*> m_items;
    QMap<QString, LauncherTab *> m_tabs;

    enum TabMode { NamesOn, NamesOff };
    TabMode m_mode;
    int m_selectedSize;
};

class LauncherView;
struct LauncherTab
{
    LauncherTab(const QString & v)
        : view(0)
        , visibleLabel(v)
    {
    }

    QColor bgColor;
    QColor fgColor;
    LauncherView *view;
    QString visibleLabel;
};

// inline
LauncherView * LauncherTabBar::currentView() const
{
    LauncherTab * t = currentTab();
    return t?t->view:0;
}

#endif
