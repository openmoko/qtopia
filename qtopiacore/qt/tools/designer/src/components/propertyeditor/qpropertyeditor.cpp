/****************************************************************************
**
** Copyright (C) 1992-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qpropertyeditor.h"
#include "qpropertyeditor_model_p.h"
#include "qpropertyeditor_delegate_p.h"

#include <QtGui/QHeaderView>
#include <QtGui/QApplication>
#include <QtGui/QPainter>
#include <QtGui/QScrollBar>
#include <qdebug.h>

using namespace qdesigner_internal;

Q_GLOBAL_STATIC_WITH_ARGS(PropertyCollection, dummy_collection, (QLatin1String("<empty>")))

QPropertyEditor::QPropertyEditor(QWidget *parent)
    : QTreeView(parent), contentsResized(false)
{
    m_model = new QPropertyEditorModel(this);
    setModel(m_model);
    m_itemDelegate = new QPropertyEditorDelegate(this);
    setItemDelegate(m_itemDelegate);
    connect(m_itemDelegate, SIGNAL(resetProperty(const QString &)), m_model, SIGNAL(resetProperty(const QString &)));
    setInitialInput(0);

    setAlternatingRowColors(true);

    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    connect(this, SIGNAL(activated(QModelIndex)),
            this, SLOT(expand(QModelIndex)));

    connect(m_model, SIGNAL(propertyChanged(IProperty*)),
            this, SIGNAL(propertyChanged(IProperty*)));
}

QPropertyEditor::~QPropertyEditor()
{
}

bool QPropertyEditor::isReadOnly() const
{
    return m_itemDelegate->isReadOnly();
}

void QPropertyEditor::setReadOnly(bool readOnly)
{
    m_itemDelegate->setReadOnly(readOnly);
}

void QPropertyEditor::setInitialInput(IProperty *initialInput)
{
    bool needResize = false;
    if (!m_model->initialInput() || m_model->initialInput() == dummy_collection()) {
        if (initialInput)
            needResize = true;
    }

    if (!initialInput)
        initialInput = dummy_collection();

    m_model->setInitialInput(initialInput);

    setSelectionMode(QTreeView::SingleSelection);
    setSelectionBehavior(QTreeView::SelectRows);
    setRootIsDecorated(true);

    setEditTriggers(QAbstractItemView::CurrentChanged|QAbstractItemView::SelectedClicked);
    setRootIndex(m_model->indexOf(initialInput));

    if (needResize && !contentsResized) {
        contentsResized = true;
        resizeColumnToContents(0);
    }
}


IProperty *QPropertyEditor::initialInput() const
{
    return m_model->initialInput();
}

void QPropertyEditor::drawBranches(QPainter *painter, const QRect &rect, const QModelIndex &index) const
{
    // designer figts the style it uses. :(
    static bool mac_style
                = QApplication::style()->inherits("QMacStyle");
    static const int windows_deco_size = 9;

    QStyleOptionViewItem opt = viewOptions();

    IProperty *property = static_cast<const QPropertyEditorModel*>(model())->privateData(index);
    if (index.column() == 0 && property && property->changed()) {
        opt.font.setBold(true);
    }

    if (property && property->isSeparator()) {
        painter->fillRect(rect, opt.palette.dark());
    }

    if (model()->hasChildren(index)) {
        opt.state |= QStyle::State_Children;

        QRect primitive(rect.left(), rect.top(), indentation(), rect.height());

        if (!mac_style) {
            primitive.moveLeft(primitive.left() + (primitive.width() - windows_deco_size)/2);
            primitive.moveTop(primitive.top() + (primitive.height() - windows_deco_size)/2);
            primitive.setWidth(windows_deco_size);
            primitive.setHeight(windows_deco_size);
        }

        opt.rect = primitive;

        if (isExpanded(index))
            opt.state |= QStyle::State_Open;
        style()->drawPrimitive(QStyle::PE_IndicatorBranch, &opt, painter, this);
    }
    QPen savedPen = painter->pen();
    QColor color = static_cast<QRgb>(QApplication::style()->styleHint(QStyle::SH_Table_GridLineColor, &opt));
    painter->setPen(QPen(color));
    painter->drawLine(rect.x(), rect.bottom(), rect.right(), rect.bottom());
    painter->setPen(savedPen);
}

void QPropertyEditor::keyPressEvent(QKeyEvent *ev)
{
/*    QApplication::syncX();*/
    QTreeView::keyPressEvent(ev);
}

QStyleOptionViewItem QPropertyEditor::viewOptions() const
{
    QStyleOptionViewItem option = QTreeView::viewOptions();
    option.showDecorationSelected = true;
    return option;
}

void QPropertyEditor::focusInEvent(QFocusEvent *event)
{
    QAbstractScrollArea::focusInEvent(event);
    viewport()->update();
}
