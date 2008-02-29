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

#include "tabordereditor.h"

#include <QtGui/QLabel>
#include <QtGui/QPainter>
#include <QtCore/qdebug.h>
#include <QtGui/QRegion>
#include <QtGui/qevent.h>
#include <QtGui/QFontMetrics>
#include <QtGui/QApplication>

#include <QtDesigner/QtDesigner>

#include <QtGui/QUndoCommand>
#include <qdesigner_command_p.h>
#include <qdesigner_widget_p.h>
#include <qdesigner_utils_p.h>
#include <qlayout_widget_p.h>
#include <qdesigner_promotedwidget_p.h>

#define BG_ALPHA                32
#define VBOX_MARGIN             1
#define HBOX_MARGIN             4

using namespace qdesigner_internal;

static QRect fixRect(const QRect &r)
{
    return QRect(r.x(), r.y(), r.width() - 1, r.height() - 1);
}

TabOrderEditor::TabOrderEditor(QDesignerFormWindowInterface *form, QWidget *parent)
    : QWidget(parent), m_font_metrics(font())
{
    m_form_window = form;
    m_bg_widget = 0;
    m_undo_stack = form->commandHistory();
    connect(form, SIGNAL(widgetRemoved(QWidget*)), this, SLOT(widgetRemoved(QWidget*)));

    QFont font = this->font();
    font.setPointSize(font.pointSize()*2);
    font.setBold(true);
    setFont(font);
    m_font_metrics = QFontMetrics(font);
    m_current_index = 0;

    setAttribute(Qt::WA_MouseTracking, true);
}

QDesignerFormWindowInterface *TabOrderEditor::formWindow() const
{
    return m_form_window;
}

void TabOrderEditor::setBackground(QWidget *background)
{
    if (background == m_bg_widget) {
        return;
    }

    m_bg_widget = background;
    updateBackground();
}

void TabOrderEditor::updateBackground()
{
    if (m_bg_widget == 0) {
        // nothing to do
        return;
    }

    initTabOrder();
    update();
}

void TabOrderEditor::widgetRemoved(QWidget*)
{
}

void TabOrderEditor::showEvent(QShowEvent *e)
{
    QWidget::showEvent(e);
    updateBackground();
}

QRect TabOrderEditor::indicatorRect(int index) const
{
    if (index < 0 || index >= m_tab_order_list.size())
        return QRect();

    QWidget *w = m_tab_order_list.at(index);
    QString text = QString::number(index + 1);

    QPoint tl = mapFromGlobal(w->mapToGlobal(w->rect().topLeft()));
    QSize size = m_font_metrics.size(Qt::TextSingleLine, text);
    QRect r(tl - QPoint(size.width(), size.height())/2, size);
    r = QRect(r.left() - HBOX_MARGIN, r.top() - VBOX_MARGIN,
                r.width() + HBOX_MARGIN*2, r.height() + VBOX_MARGIN*2);

    return r;
}

static bool isWidgetVisible(QWidget *widget)
{
    while (widget && widget->parentWidget()) {
        if (!widget->isVisibleTo(widget->parentWidget()))
            return false;

        widget = widget->parentWidget();
    }

    return true;
}

void TabOrderEditor::paintEvent(QPaintEvent *e)
{
    QPainter p(this);
    p.setClipRegion(e->region());

    for (int i = 0; i < m_tab_order_list.size(); ++i) {
        QWidget *widget = m_tab_order_list.at(i);
        if (!isWidgetVisible(widget))
            continue;

        QRect r = indicatorRect(i);

        QColor c = Qt::blue;
        p.setPen(c);
        c.setAlpha(BG_ALPHA);
        p.setBrush(c);
        p.drawRect(fixRect(r));

        p.setPen(Qt::white);
        p.drawText(r, QString::number(i + 1), QTextOption(Qt::AlignCenter));
    }
}

bool TabOrderEditor::skipWidget(QWidget *w) const
{
    if (qobject_cast<QLayoutWidget*>(w)
            || w == formWindow()->mainContainer()
            || w->isHidden())
        return true;

    if (!formWindow()->isManaged(w)) {
        return true;
    }

    if (qobject_cast<QDesignerPromotedWidget*>(w) != 0)
        return true;

    QExtensionManager *ext = formWindow()->core()->extensionManager();
    if (QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension*>(ext, w)) {
        int index = sheet->indexOf(QLatin1String("focusPolicy"));
        if (index != -1) {
            bool ok = false;
            Qt::FocusPolicy q = (Qt::FocusPolicy) Utils::valueOf(sheet->property(index), &ok);
            return !ok || q == Qt::NoFocus;
        }
    }

    return true;
}

void TabOrderEditor::initTabOrder()
{
    m_tab_order_list.clear();

    QDesignerFormEditorInterface *core = formWindow()->core();

    if (QDesignerMetaDataBaseItemInterface *item = core->metaDataBase()->item(formWindow())) {
        m_tab_order_list = item->tabOrder();
    }

    // Remove any widgets that have been removed form the form
    for (int i = 0; i < m_tab_order_list.size(); ) {
        QWidget *w = m_tab_order_list.at(i);
        if (!formWindow()->mainContainer()->isAncestorOf(w) || skipWidget(w))
            m_tab_order_list.removeAt(i);
        else
            ++i;
    }

    // Append any widgets that are in the form but are not in the tab order
    QDesignerFormWindowCursorInterface *cursor = formWindow()->cursor();
    for (int i = 0; i < cursor->widgetCount(); ++i) {
        QWidget *widget = cursor->widget(i);
        if (skipWidget(widget))
            continue;

        if (!m_tab_order_list.contains(widget))
            m_tab_order_list.append(widget);
    }

    m_indicator_region = QRegion();
    for (int i = 0; i < m_tab_order_list.size(); ++i) {
        if (m_tab_order_list.at(i)->isVisible())
            m_indicator_region |= indicatorRect(i);
    }
}

void TabOrderEditor::mouseMoveEvent(QMouseEvent *e)
{
    e->accept();
    if (m_indicator_region.contains(e->pos()))
        setCursor(Qt::PointingHandCursor);
    else
        setCursor(QCursor());
}

int TabOrderEditor::widgetIndexAt(const QPoint &pos) const
{
    int target_index = -1;
    for (int i = 0; i < m_tab_order_list.size(); ++i) {
        if (!m_tab_order_list.at(i)->isVisible())
            continue;
        if (indicatorRect(i).contains(pos)) {
            target_index = i;
            break;
        }
    }

    return target_index;
}

void TabOrderEditor::mousePressEvent(QMouseEvent *e)
{
    e->accept();

    if (!m_indicator_region.contains(e->pos())) {
        if (QWidget *child = m_bg_widget->childAt(e->pos())) {
            QDesignerFormEditorInterface *core = m_form_window->core();
            if (core->widgetFactory()->isPassiveInteractor(child)) {

                QMouseEvent event(QEvent::MouseButtonPress,
                                    child->mapFromGlobal(e->globalPos()),
                                    e->button(), e->buttons(), e->modifiers());

                qApp->sendEvent(child, &event);

                QMouseEvent event2(QEvent::MouseButtonRelease,
                                    child->mapFromGlobal(e->globalPos()),
                                    e->button(), e->buttons(), e->modifiers());

                qApp->sendEvent(child, &event2);

                updateBackground();
            }
        }
        return;
    }

    int target_index = widgetIndexAt(e->pos());
    if (target_index == -1)
        return;

    update(indicatorRect(target_index));
    update(indicatorRect(m_current_index));
    m_tab_order_list.swap(target_index, m_current_index);
    update(indicatorRect(target_index));
    update(indicatorRect(m_current_index));

    ++m_current_index;
    if (m_current_index == m_tab_order_list.size())
        m_current_index = 0;

    TabOrderCommand *cmd = new TabOrderCommand(formWindow());
    cmd->init(m_tab_order_list);
    formWindow()->commandHistory()->push(cmd);
}

void TabOrderEditor::mouseDoubleClickEvent(QMouseEvent *e)
{
    m_current_index = 0;
    mousePressEvent(e);
}

void TabOrderEditor::resizeEvent(QResizeEvent *e)
{
    updateBackground();
    QWidget::resizeEvent(e);
}

