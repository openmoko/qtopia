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

#include "launchertab.h"
#include <QVariant>
#include <QStyle>

class LauncherTabStyle : public QWindowsStyle
{
public:
    void drawControl(ControlElement element, const QStyleOption *option,
                     QPainter *painter, const QWidget *widget=0) const;
    QSize sizeFromContents(ContentsType type, const QStyleOption *option,
                           const QSize &contentsSize, const QWidget *widget=0) const;
};

void LauncherTabStyle::drawControl(ControlElement element, const QStyleOption *option,
                                   QPainter *painter, const QWidget *widget) const
{
    if (element == CE_TabBarTab || element == CE_TabBarTabLabel) {
        const QStyleOptionTab *topt = qstyleoption_cast<const QStyleOptionTab*>(option);

        // We have to use the text as a key to work out which tab we're drawing.
        // There has to be a better way... :(
        const LauncherTabBar * tb = static_cast<const LauncherTabBar *>(widget);
        LauncherTab * tab = tb->lookup(topt->text);
        Q_ASSERT(tab);

        QStyleOptionTab opt = *topt;

        if(LauncherTabBar::NamesOff == tb->m_mode &&
           !(topt->state & State_Selected)) {
            opt.text = QString();
        } else {
            opt.text = tab->visibleLabel;
        }

        if(tab->fgColor.isValid() || tab->bgColor.isValid()) {
            if(tab->fgColor.isValid()) {
                opt.palette.setColor(QPalette::Active,
                                QPalette::Foreground,
                                tab->fgColor);
                opt.palette.setColor(QPalette::Active,
                                QPalette::Text,
                                tab->fgColor);
                opt.palette.setColor(QPalette::Inactive,
                                QPalette::Foreground,
                                tab->fgColor);
                opt.palette.setColor(QPalette::Inactive,
                                QPalette::Text,
                                tab->fgColor);
            }
            if(tab->bgColor.isValid()) {
                opt.palette.setColor(QPalette::Active,
                                QPalette::Background,
                                tab->bgColor);
                opt.palette.setColor(QPalette::Active,
                                QPalette::Button,
                                tab->bgColor);
                opt.palette.setColor(QPalette::Inactive,
                                QPalette::Background,
                                tab->bgColor);
                opt.palette.setColor(QPalette::Inactive,
                                QPalette::Button,
                                tab->bgColor);
            }
        }

        qApp->style()->drawControl(element, &opt, painter, widget);
        return;
    }

    qApp->style()->drawControl(element, option, painter, widget);
}

QSize LauncherTabStyle::sizeFromContents(ContentsType type, const QStyleOption *option,
                                         const QSize &contentsSize, const QWidget *widget) const
{
    if (type == CT_TabBarTab) {
        const QStyleOptionTab *topt = qstyleoption_cast<const QStyleOptionTab *>(option);

        const LauncherTabBar * tb = static_cast<const LauncherTabBar *>(widget);

        if(topt->state & State_Selected) {
            QSize s = qApp->style()->sizeFromContents(CT_TabBarTab, option, contentsSize, widget);

            if(-1 != tb->m_selectedSize) {
                s.setWidth(tb->m_selectedSize);
            }

            return s;
        } else {

            if(LauncherTabBar::NamesOff == tb->m_mode) {
                int iconSizeWidth = qApp->style()->pixelMetric(QStyle::PM_SmallIconSize);
                QSize csz(iconSizeWidth+10, contentsSize.height());

                return qApp->style()->sizeFromContents(CT_TabBarTab, option, csz, widget);
            }

        }
    }

    return qApp->style()->sizeFromContents(type, option, contentsSize, widget);
}

///////////////////////////////////////////////////////////////////////////////
// LauncherTabBar
///////////////////////////////////////////////////////////////////////////////
LauncherTabBar::LauncherTabBar(QWidget *parent)
    : QTabBar(parent), m_itemsOnEnd(0), m_mode(NamesOn), m_selectedSize(-1)
{
    setStyle(new LauncherTabStyle);
    connect(this, SIGNAL(currentChanged(int)), this, SLOT(changed(int)));
}

LauncherTabBar::~LauncherTabBar()
{
}

void LauncherTabBar::changed(int idx)
{
    Q_UNUSED(idx);
    layoutTabs();
    resizeEvent(0);
}

LauncherTab * LauncherTabBar::addTab(const QString &type,
                                     const QString &visibleText,
                                     const QIcon &icon,
                                     bool forceToEnd)
{
    LauncherTab * tab = new LauncherTab(visibleText);
    if(forceToEnd) {
        m_items += tab;
        ++m_itemsOnEnd;
        Q_ASSERT(m_tabs.find(type) == m_tabs.end());
        m_tabs.insert(type, tab);
        QTabBar::addTab(icon, type);
    } else {
        int tabNum = m_items.count() - m_itemsOnEnd;
        m_items.insert(tabNum, tab);
        m_tabs.insert(type, tab);
        QTabBar::insertTab(tabNum, icon, type);
    }

    layoutTabs();
    resizeEvent(0);
    return tab;
}

void LauncherTabBar::removeTab(const QString &type)
{
    QMap<QString, LauncherTab *>::iterator iter = m_tabs.find(type);
    if(m_tabs.end() == iter) return; // Removing non-existent type

    LauncherTab * tab = *iter;
    m_tabs.erase(iter);

    // Find the appropriate tab number - as this is a rare operation and there
    // are never many tabs, a linear search will be fine.  There should only
    // ever be one entry for each type
    for(int ii = 0; ii < m_items.count(); ii++) {
        if(tab == m_items.at(ii)) {
            QTabBar::removeTab(ii);
            m_items.removeAt(ii);
            break; // Success
        }
    }

    layoutTabs();
    resizeEvent(0);
    if(tab) delete tab;
}

LauncherTab * LauncherTabBar::lookup(const QString &type) const
{
    QMap<QString, LauncherTab *>::const_iterator iter = m_tabs.find(type);
    if(m_tabs.end() != iter)
        return *iter;
    else
        return 0;
}

LauncherTab * LauncherTabBar::lookup(int num) const
{
    if(num < m_items.count())
        return m_items.at(num);
    else
        return 0;
}

LauncherTab * LauncherTabBar::currentTab() const
{
    if(-1 == currentIndex()) return 0;
    else return m_items.at(currentIndex());
}

void LauncherTabBar::setCurrentTab(const QString &type)
{
    QMap<QString, LauncherTab *>::iterator iter = m_tabs.find(type);
    if(m_tabs.end() == iter) return; // Non-existent type

    LauncherTab * tab = *iter;

    // Find the appropriate tab number - as this is a rare operation and there
    // are never many tabs, a linear search will be fine.  There should only
    // ever be one entry for each type
    for(int ii = 0; ii < m_items.count(); ii++) {
        if(tab == m_items.at(ii)) {
            QTabBar::setCurrentIndex(ii);
            break; // Success
        }
    }
}

void LauncherTabBar::layoutTabs()
{
    // Always try and layout with the names on first
    m_mode = NamesOn;
    // Can't have the size adjustment affecting anything
    m_selectedSize = -1;

    int required = requiredSpace();

    if(required == width()) {
        // Wow - what a coincidence
    } else if(required < width()) {
        // Cool, we can make the selected tab fill the rest
        m_selectedSize = width() - (required - tabSizeHint(currentIndex()).width());
    } else {
        // width() > required
        m_mode = NamesOff;
        required = requiredSpace();

        // We can try to make the selected tab fill the rest
        m_selectedSize = width() - (required - tabSizeHint(currentIndex()).width());
        if(m_selectedSize < tabSizeHint(currentIndex()).width())
            m_selectedSize = -1; // Not enough space
    }
}

void LauncherTabBar::resizeEvent(QResizeEvent * e)
{
    layoutTabs();
    QTabBar::resizeEvent(e);
}

int LauncherTabBar::requiredSpace() const
{
    int requiredSpace = 0;

    for(int ii = 0; ii < count(); ii++)
        requiredSpace += tabSizeHint(ii).width();

    return requiredSpace;
}

