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

#include "qpropertyeditor_delegate_p.h"
#include "qpropertyeditor_model_p.h"
#include <iconloader_p.h>

#include <QtGui/QPainter>
#include <QtGui/QFrame>
#include <QtGui/QKeyEvent>
#include <QtGui/QApplication>
#include <QtGui/QSpinBox>
#include <QtGui/QToolButton>
#include <QtGui/QHBoxLayout>
#include <QtGui/QMessageBox>
#include <QtGui/QLabel>
#include <QtGui/qdrawutil.h>

#include <QtCore/qdebug.h>
#include <private/qfont_p.h>

#include <limits.h>

#ifndef Q_MOC_RUN
using namespace qdesigner_internal;
#endif

class EditorWithReset : public QWidget
{
    Q_OBJECT
public:
    EditorWithReset(const IProperty *property, QPropertyEditorModel *model, QWidget *parent = 0);
    void setChildEditor(QWidget *child_editor);
    QWidget *childEditor() const { return m_child_editor; }
private slots:
    void emitResetProperty();
signals:
    void sync();
    void resetProperty(const IProperty *property, QPropertyEditorModel *model);
private:
    QWidget *m_child_editor;
    QHBoxLayout *m_layout;
    const IProperty *m_property;
    QPropertyEditorModel *m_model;
};

EditorWithReset::EditorWithReset(const IProperty *property, QPropertyEditorModel *model, QWidget *parent)
    : QWidget(parent)
{
    setAutoFillBackground(true);
    m_property = property;
    m_child_editor = 0;
    m_layout = new QHBoxLayout(this);
    m_layout->setMargin(0);
    m_layout->setSpacing(0);
    m_model = model;

    QToolButton *button = new QToolButton(this);
    button->setToolButtonStyle(Qt::ToolButtonIconOnly);
    button->setIcon(createIconSet(QLatin1String("resetproperty.png")));
    button->setIconSize(QSize(8,8));
    button->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding));
    m_layout->addWidget(button);
    connect(button, SIGNAL(clicked()), this, SLOT(emitResetProperty()));
}

void EditorWithReset::emitResetProperty()
{
    emit resetProperty(m_property, m_model);
}

void EditorWithReset::setChildEditor(QWidget *child_editor)
{
    m_child_editor = child_editor;

    m_child_editor->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding));
    m_layout->insertWidget(0, m_child_editor);
    setFocusProxy(m_child_editor);
}

QPropertyEditorDelegate::QPropertyEditorDelegate(QObject *parent)
    : QItemDelegate(parent),
      m_readOnly(false)
{
}

QPropertyEditorDelegate::~QPropertyEditorDelegate()
{
}

bool QPropertyEditorDelegate::eventFilter(QObject *object, QEvent *event)
{
    QWidget *editor = qobject_cast<QWidget*>(object);
    if (editor && qobject_cast<EditorWithReset*>(editor->parent()))
        editor = editor->parentWidget();

    switch (event->type()) {
        case QEvent::KeyPress:
        case QEvent::KeyRelease: {
            QKeyEvent *ke = static_cast<QKeyEvent*>(event);
            if (!(ke->modifiers() & Qt::ControlModifier)
                && (ke->key() == Qt::Key_Up || ke->key() == Qt::Key_Down)) {
                event->ignore();
                return true;
            }
            if (ke->key() == Qt::Key_Enter || ke->key() == Qt::Key_Return) {
                QWidget *widget = qobject_cast<QWidget*>(object);
                if (QSpinBox *spinBox = qobject_cast<QSpinBox*>(widget)) { // ### hack (remove me)
                    spinBox->interpretText();
                }
                emit commitData(widget);
                return true;
            }
        } break;

        case QEvent::FocusOut:
            if (!editor->isActiveWindow() || (QApplication::focusWidget() != editor)) {
                QWidget *w = QApplication::focusWidget();
                while (w) { // dont worry about focus changes internally in the editor
                    if (w == editor)
                        return false;
                    w = w->parentWidget();
                }

                emit commitData(editor);
            }
            return false;

        default:
            break;
    }

    return QItemDelegate::eventFilter(editor ? editor : object, event);
}

void QPropertyEditorDelegate::paint(QPainter *painter, const QStyleOptionViewItem &opt, const QModelIndex &index) const
{
    QStyleOptionViewItem option = opt;

    const QAbstractItemModel *model = index.model();
    IProperty *property = static_cast<const QPropertyEditorModel*>(model)->privateData(index);
    if (index.column() == 0 && property && property->changed()) {
        option.font.setBold(true);
    }

    if (property && property->isSeparator()) {
        option.palette.setColor(QPalette::Text, option.palette.color(QPalette::BrightText));
        option.font.setBold(true);
        option.state &= ~QStyle::State_Selected;
    }

    if (index.column() == 1) {
        option.state &= ~QStyle::State_Selected;
    }

    option.state &= ~QStyle::State_HasFocus;

    if (property && property->isSeparator()) {
        QBrush bg = option.palette.dark();
        painter->fillRect(option.rect, bg);
    }

    QPen savedPen = painter->pen();

    QItemDelegate::paint(painter, option, index);

    QColor color = static_cast<QRgb>(QApplication::style()->styleHint(QStyle::SH_Table_GridLineColor, &option));
    painter->setPen(QPen(color));
    if (index.column() == 1 || !(property && property->isSeparator())) {
        int right = (option.direction == Qt::LeftToRight) ? option.rect.right() : option.rect.left();
        painter->drawLine(right, option.rect.y(), right, option.rect.bottom());
    }
    painter->drawLine(option.rect.x(), option.rect.bottom(),
            option.rect.right(), option.rect.bottom());
    painter->setPen(savedPen);
}

QSize QPropertyEditorDelegate::sizeHint(const QStyleOptionViewItem &opt, const QModelIndex &index) const
{
    return QItemDelegate::sizeHint(opt, index) + QSize(4, 4);
}

bool QPropertyEditorDelegate::isReadOnly() const
{
    return m_readOnly;
}

void QPropertyEditorDelegate::setReadOnly(bool readOnly)
{
    // ### close the editor
    m_readOnly = readOnly;
}

QWidget *QPropertyEditorDelegate::createEditor(QWidget *parent,
                                const QStyleOptionViewItem &option,
                                const QModelIndex &index) const
{
    Q_UNUSED(option);

    QPropertyEditorModel *model = const_cast<QPropertyEditorModel *>(static_cast<const QPropertyEditorModel *>(index.model()));
    const IProperty *property = model->privateData(index);
    if (property == 0)
        return 0;

    QWidget *editor = 0;

    if (!isReadOnly() && property->hasEditor()) { // ### always true
        if (property->hasReset()) {
            EditorWithReset *editor_w_reset
                = new EditorWithReset(property, model, parent);
            QWidget *child_editor
                = property->createEditor(editor_w_reset, editor_w_reset, SIGNAL(sync()));
            editor_w_reset->setChildEditor(child_editor);
            connect(editor_w_reset, SIGNAL(sync()), this, SLOT(sync()));
            connect(editor_w_reset, SIGNAL(resetProperty(const IProperty *, QPropertyEditorModel *)),
                        this, SLOT(resetProperty(const IProperty *, QPropertyEditorModel *)));

            editor = editor_w_reset;
            child_editor->installEventFilter(const_cast<QPropertyEditorDelegate *>(this));
        } else {
            editor = property->createEditor(parent, this, SLOT(sync()));
            editor->installEventFilter(const_cast<QPropertyEditorDelegate *>(this));
        }
    }


    return editor;
}

void QPropertyEditorDelegate::setEditorData(QWidget *editor,
                             const QModelIndex &index) const
{
    if (EditorWithReset *editor_w_reset = qobject_cast<EditorWithReset*>(editor))
        editor = editor_w_reset->childEditor();

    const QAbstractItemModel *model = index.model();
    IProperty *property = static_cast<const QPropertyEditorModel*>(model)->privateData(index);
    if (property && property->hasEditor()) {
        property->updateEditorContents(editor);
    }
}

void QPropertyEditorDelegate::setModelData(QWidget *editor,
                            QAbstractItemModel *model,
                            const QModelIndex &index) const
{
    if (EditorWithReset *editor_w_reset = qobject_cast<EditorWithReset*>(editor))
        editor = editor_w_reset->childEditor();

    if (IProperty *property = static_cast<const QPropertyEditorModel*>(model)->privateData(index)) {
        property->updateValue(editor);
        if (property->propertyName() == QLatin1String("Family") ||
                property->propertyName() == QLatin1String("Point Size") ||
                property->propertyName() == QLatin1String("Bold") ||
                property->propertyName() == QLatin1String("Italic") ||
                property->propertyName() == QLatin1String("Underline") ||
                property->propertyName() == QLatin1String("Strikeout") ||
                property->propertyName() == QLatin1String("Kerning") ||
                property->propertyName() == QLatin1String("Antialiasing")) {
            QModelIndex parentIndex = index.parent();
            if (IProperty *fontProperty = static_cast<const QPropertyEditorModel*>(model)->privateData(parentIndex)) {
                QFont f = qvariant_cast<QFont>(fontProperty->value());
                if (property->propertyName() == QLatin1String("Family"))
                    f.setFamily(property->toString());
                else if (property->propertyName() == QLatin1String("Point Size"))
                    f.setPointSize(property->value().toInt());
                else if (property->propertyName() == QLatin1String("Bold"))
                    f.setBold(property->value().toBool());
                else if (property->propertyName() == QLatin1String("Italic"))
                    f.setItalic(property->value().toBool());
                else if (property->propertyName() == QLatin1String("Underline"))
                    f.setUnderline(property->value().toBool());
                else if (property->propertyName() == QLatin1String("Strikeout"))
                    f.setStrikeOut(property->value().toBool());
                else if (property->propertyName() == QLatin1String("Kerning"))
                    f.setKerning(property->value().toBool());
                else if (property->propertyName() == QLatin1String("Antialiasing"))
                    f.setStyleStrategy(property->value().toBool() ? QFont::PreferDefault : QFont::NoAntialias);
                fontProperty->setValue(f);
                model->setData(parentIndex, f, Qt::EditRole);
                return;
            }
        }
        model->setData(index, property->value(), Qt::EditRole);
    }
}

void QPropertyEditorDelegate::drawDecoration(QPainter *painter, const QStyleOptionViewItem &option,
                              const QRect &rect, const QPixmap &pixmap) const
{
    QItemDelegate::drawDecoration(painter, option, rect, pixmap);
}

void QPropertyEditorDelegate::sync()
{
    QWidget *w = qobject_cast<QWidget*>(sender());
    if (w == 0)
        return;
    emit commitData(w);
}

void QPropertyEditorDelegate::resetProperty(const IProperty *property, QPropertyEditorModel *model)
{
    QString propName = property->propertyName();
    if (propName == QLatin1String("Family") ||
            propName == QLatin1String("Point Size") ||
            propName == QLatin1String("Bold") ||
            propName == QLatin1String("Italic") ||
            propName == QLatin1String("Underline") ||
            propName == QLatin1String("Strikeout") ||
            propName == QLatin1String("Kerning") ||
            propName == QLatin1String("Antialiasing")) {
        IProperty *fontProperty = property->parent();
        if (fontProperty) {
            QFont f = qvariant_cast<QFont>(fontProperty->value());
            uint mask = f.resolve();
            if (property->propertyName() == QLatin1String("Family"))
                mask &= ~QFontPrivate::Family;
            else if (property->propertyName() == QLatin1String("Point Size"))
                mask &= ~QFontPrivate::Size;
            else if (property->propertyName() == QLatin1String("Bold"))
                mask &= ~QFontPrivate::Weight;
            else if (property->propertyName() == QLatin1String("Italic"))
                mask &= ~QFontPrivate::Style;
            else if (property->propertyName() == QLatin1String("Underline"))
                mask &= ~QFontPrivate::Underline;
            else if (property->propertyName() == QLatin1String("Strikeout"))
                mask &= ~QFontPrivate::StrikeOut;
            else if (property->propertyName() == QLatin1String("Kerning"))
                mask &= ~QFontPrivate::Kerning;
            else if (property->propertyName() == QLatin1String("Antialias"))
                mask &= ~QFontPrivate::StyleStrategy;
            f.resolve(mask);
            if (mask) {
                QModelIndex fontIndex = model->indexOf(fontProperty);
                fontProperty->setDirty(true);
                model->setData(fontIndex, f, Qt::EditRole);
                return;
            }
            propName = fontProperty->propertyName();
        }
    }
    emit resetProperty(propName);
}

void QPropertyEditorDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QItemDelegate::updateEditorGeometry(editor, option, index);
    editor->setGeometry(editor->geometry().adjusted(0, 0, -1, -1));
}


#include "qpropertyeditor_delegate.moc"
