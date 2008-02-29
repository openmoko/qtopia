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

#include "e3_phonebrowser.h"
#include "qtopiaserverapplication.h"
#include <QCategoryManager>
#include <QContent>
#include <QContentFilter>
#include <QVBoxLayout>
#include <QHash>
#include <QKeyEvent>
#include "phone/phonebrowser.h"
#include <QSoftMenuBar>
#include <QMenu>
#include <QAction>

#include <QDebug>

QTOPIA_REPLACE_WIDGET(QAbstractBrowserScreen, E3BrowserScreen);

class E3BrowserScreenStack : public LazyContentStack
{
Q_OBJECT
public:
    E3BrowserScreenStack(QWidget *parent);

    QString currentName() const;

    void back() { LazyContentStack::back(); }
    void setMode(QListView::ViewMode);

protected:
    virtual QObject* createView(const QString &);
    virtual void raiseView(const QString &, bool reset);

private:
    QListView::ViewMode m_mode;
    QContentList mainMenu() const;
    QStackedWidget *stack;
    QHash<QString, int> stackContents;
};

E3BrowserScreenStack::E3BrowserScreenStack(QWidget *parent)
: LazyContentStack(NoFlags, parent), m_mode(QListView::IconMode), stack(0)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setMargin(0);
    setLayout(layout);
    stack = new QStackedWidget(this);
    layout->addWidget(stack);
}

void E3BrowserScreenStack::setMode(QListView::ViewMode mode)
{
    m_mode = mode;
    for(int ii = 0; ii < stack->count(); ++ii) {
        LauncherView *v = static_cast<LauncherView *>(stack->widget(ii));

        v->setViewMode(mode);
        if(mode == QListView::ListMode) {
            v->setColumns(1);
        } else {
            v->setColumns(3);
        }
    }
}

QObject* E3BrowserScreenStack::createView(const QString &name)
{
    LauncherView *view = 0;
    if("Main" == name) {

        view = new LauncherView(stack);
        QContentList list = mainMenu();
        for(int ii = 0; ii < list.count(); ++ii) 
            view->addItem(&list[ii]);

    } else if(name.startsWith("Folder/")) {

        QString category = name.mid(7);
        view = new LauncherView(stack);
        view->showCategory(QContentFilter(QCategoryFilter(category)));
    }

    if(view) {
        view->setViewMode(m_mode);
        if(QListView::IconMode == m_mode)
            view->setColumns(3);
        stackContents[name] = stack->addWidget(view);
    }

    return view;
}

QContentList E3BrowserScreenStack::mainMenu() const
{
    QContentSet set(QContentFilter::Category, "MainApplications");
    QContentList rv = set.items();

    QCategoryManager man("Applications");
    QStringList cats = man.categoryIds();
    for(int ii = 0; ii < cats.count(); ++ii) {
        const QString &cat = cats.at(ii);

        if(man.isGlobal(cat))
            continue;

        QContent content;
        content.setIcon(man.iconFile(cat));
        content.setName(man.label(cat));
        content.setType("Folder/" + cat);

        rv.append(content);

    }

    return rv;
}

void E3BrowserScreenStack::raiseView(const QString &view, bool) 
{
    stack->setCurrentIndex(stackContents[view]);
}

E3BrowserScreen::E3BrowserScreen(QWidget *parent, Qt::WFlags flags)
: QAbstractBrowserScreen(parent, flags)
{
    setWindowTitle(tr("Menu"));

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setMargin(0);
    setLayout(layout);

    m_stack = new E3BrowserScreenStack(this);
    layout->addWidget(m_stack);

    QMenu *menu = QSoftMenuBar::menuFor(this);
    listAction = new QAction(tr("List View"), this);
    gridAction = new QAction(tr("Grid View"), this);
    gridAction->setEnabled(false);
    menu->addAction(listAction);
    menu->addAction(gridAction);
    QObject::connect(listAction, SIGNAL(triggered()), this, SLOT(listMode()));
    QObject::connect(gridAction, SIGNAL(triggered()), this, SLOT(gridMode()));
}

void E3BrowserScreen::listMode()
{
    gridAction->setEnabled(true);
    listAction->setEnabled(false);
    m_stack->setMode(QListView::ListMode);
}

void E3BrowserScreen::gridMode()
{
    gridAction->setEnabled(false);
    listAction->setEnabled(true);
    m_stack->setMode(QListView::IconMode);
}

QString E3BrowserScreen::currentView() const
{
    return m_stack->currentView();
}

bool E3BrowserScreen::viewAvailable(const QString &name) const
{
    return "Main" == name || name.startsWith("Folder/");
}

void E3BrowserScreen::resetToView(const QString &name)
{
    m_stack->resetToView(name);
}

void E3BrowserScreen::moveToView(const QString &name)
{
    m_stack->showView(name);
}

void E3BrowserScreen::closeEvent(QCloseEvent *e)
{
    m_stack->back();

    if(m_stack->isDone()) {
        e->accept();
    } else {
        e->ignore();
    }
}

#include "e3_phonebrowser.moc"
