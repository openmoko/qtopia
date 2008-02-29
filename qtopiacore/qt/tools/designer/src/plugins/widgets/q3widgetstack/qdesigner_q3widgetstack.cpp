/****************************************************************************
**
** Copyright (C) 1992-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qdesigner_q3widgetstack_p.h"
#include <QtDesigner/QtDesigner>
#include "../../../lib/shared/qdesigner_command_p.h"

#include <QtCore/QEvent>
#include <QtGui/QToolButton>

using namespace qdesigner_internal;

QDesignerQ3WidgetStack::QDesignerQ3WidgetStack(QWidget *parent)
    : Q3WidgetStack(parent), prev(0), next(0)
{
    prev = new QToolButton();
    prev->setAttribute(Qt::WA_NoChildEventsForParent, true);
    prev->setParent(this);

    prev->setObjectName(QLatin1String("__qt__passive_prev"));
    prev->setArrowType(Qt::LeftArrow);
    prev->setAutoRaise(true);
    prev->setAutoRepeat(true);
    prev->setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored));
    connect(prev, SIGNAL(clicked()), this, SLOT(prevPage()));

    next = new QToolButton();
    next->setAttribute(Qt::WA_NoChildEventsForParent, true);
    next->setParent(this);
    next->setObjectName(QLatin1String("__qt__passive_next"));
    next->setArrowType(Qt::RightArrow);
    next->setAutoRaise(true);
    next->setAutoRepeat(true);
    next->setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored));
    connect(next, SIGNAL(clicked()), this, SLOT(nextPage()));

    updateButtons();

    connect(this, SIGNAL(currentChanged(int)), this, SLOT(slotCurrentChanged(int)));
}

QDesignerFormWindowInterface *QDesignerQ3WidgetStack::formWindow()
{
    return QDesignerFormWindowInterface::findFormWindow(this);
}

QDesignerContainerExtension *QDesignerQ3WidgetStack::container()
{
    if (formWindow()) {
        QDesignerFormEditorInterface *core = formWindow()->core();
        return qt_extension<QDesignerContainerExtension*>(core->extensionManager(), this);
    }
    return 0;
}

int QDesignerQ3WidgetStack::count()
{
    return container() ? container()->count() : 0;
}

int QDesignerQ3WidgetStack::currentIndex()
{
    return container() ? container()->currentIndex() : -1;
}

void QDesignerQ3WidgetStack::setCurrentIndex(int index)
{
    if (container() && (index >= 0) && (index < count())) {
        container()->setCurrentIndex(index);
        emit currentChanged(index);
    }
}

QWidget *QDesignerQ3WidgetStack::widget(int index)
{
    return container() ? container()->widget(index) : 0;
}

void QDesignerQ3WidgetStack::updateButtons()
{
    if (prev) {
        prev->setGeometry(width() - 31, 1, 15, 15);
        prev->show();
        prev->raise();
    }

    if (next) {
        next->setGeometry(width() - 16, 1, 15, 15);
        next->show();
        next->raise();
    }
}

void QDesignerQ3WidgetStack::prevPage()
{
    if (count() == 0) {
        // nothing to do
        return;
    }

    if (QDesignerFormWindowInterface *fw = formWindow()) {
        int newIndex = currentIndex() - 1;
        if (newIndex < 0)
            newIndex = count() - 1;

        SetPropertyCommand *cmd = new SetPropertyCommand(fw);
        cmd->init(this, QLatin1String("currentIndex"), newIndex);
        fw->commandHistory()->push(cmd);
        updateButtons();
        fw->emitSelectionChanged();
    } else {
        setCurrentIndex(qMax(0, currentIndex() - 1));
        updateButtons();
    }
}

void QDesignerQ3WidgetStack::nextPage()
{
    if (count() == 0) {
        // nothing to do
        return;
    }

    if (QDesignerFormWindowInterface *fw = formWindow()) {
        SetPropertyCommand *cmd = new SetPropertyCommand(fw);
        cmd->init(this, QLatin1String("currentIndex"), (currentIndex() + 1) % count());
        fw->commandHistory()->push(cmd);
        updateButtons();
        fw->emitSelectionChanged();
    } else {
        setCurrentIndex((currentIndex() + 1) % count());
        updateButtons();
    }
}

QString QDesignerQ3WidgetStack::currentPageName()
{
    if (currentIndex() == -1)
        return QString();

    return widget(currentIndex())->objectName();
}

void QDesignerQ3WidgetStack::setCurrentPageName(const QString &pageName)
{
    if (currentIndex() == -1)
        return;

    if (QWidget *w = widget(currentIndex()))
        w->setObjectName(pageName);
}

bool QDesignerQ3WidgetStack::event(QEvent *e)
{
    if (e->type() == QEvent::LayoutRequest) {
        updateButtons();
    }

    return Q3WidgetStack::event(e);
}

void QDesignerQ3WidgetStack::childEvent(QChildEvent *e)
{
    Q3WidgetStack::childEvent(e);
    updateButtons();
}

void QDesignerQ3WidgetStack::resizeEvent(QResizeEvent *e)
{
    Q3WidgetStack::resizeEvent(e);
    updateButtons();
}

void QDesignerQ3WidgetStack::showEvent(QShowEvent *e)
{
    Q3WidgetStack::showEvent(e);
    updateButtons();
}

void QDesignerQ3WidgetStack::slotCurrentChanged(int index)
{
    if (widget(index)) {
        if (QDesignerFormWindowInterface *fw = formWindow()) {
            fw->clearSelection();
            fw->selectWidget(this, true);
        }
    }
}
