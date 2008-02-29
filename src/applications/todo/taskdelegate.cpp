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

#include "taskdelegate.h"

#include <qtimestring.h>
#include <quniqueid.h>

#include <QListWidget>
#include <QApplication>
#include <QDesktopWidget>
#include <QKeyEvent>
#include <QPainter>

static const int BoxSize = 14;

class PriorityEdit : public QListWidget
{
    Q_OBJECT
public:
    PriorityEdit(QWidget *parent)
        : QListWidget(parent)
    {
        setWindowFlags(Qt::Popup);
        setAutoScroll( false );
        setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        // populate
        insertItem( 0, "1" );
        insertItem( 1, "2" );
        insertItem( 2, "3" );
        insertItem( 3, "4" );
        insertItem( 4, "5" );

        // no such signal
        connect(this, SIGNAL(itemActivated(QListWidgetItem *)),
                this, SLOT(internalSelectPriority()));
    }

    ~PriorityEdit() {}

    int priority() const { return currentIndex().row() + 1; }
    void setPriority(int p) {
        if (p > 5 || p < 1)
            p = 1;
        setCurrentItem(item(p - 1));
    }

    void setCellGeometry(const QRect &r)
    {
        // r is in contents position, not global.
        QPoint pos = r.bottomLeft();
        if (parentWidget())
            pos = parentWidget()->mapToGlobal(pos);


        QDesktopWidget *desktop = QApplication::desktop();
        int sh = desktop->availableGeometry(desktop->primaryScreen()).height();                     // screen height

        QSize size(r.width() + frameWidth() * 2,
                5 * sizeHintForRow(0) + frameWidth() * 2);

        if (pos.y() + size.height() > sh) {
            pos.setY(pos.y() - size.height() - r.height());
        }

        setGeometry(QRect(pos, size));
    }

signals:
    void cancelEdit(QWidget *);
    void prioritySelected(QWidget *);

protected:
    void keyPressEvent(QKeyEvent *e)
    {
        switch (e->key()) {
#ifdef QTOPIA_PHONE
            case Qt::Key_No:
            case Qt::Key_Back:
#endif
            case Qt::Key_Escape:
                emit cancelEdit(this);
                break;
            default:
                QListWidget::keyPressEvent(e);
        }
    }

private slots:
    void internalSelectPriority()
    {
        emit prioritySelected(this);
        emit cancelEdit(this);
    }
};

/// Delegate

TaskTableDelegate::TaskTableDelegate(QObject *parent) : QAbstractItemDelegate(parent) { }

TaskTableDelegate::~TaskTableDelegate() {}

QRect TaskTableDelegate::checkBoxGeom(const QRect &cell)
{
    int x = cell.x() + ( cell.width() - BoxSize ) / 2;
    int y = cell.y() + ( cell.height() - BoxSize ) / 2;
    return QRect( x, y, BoxSize, BoxSize );
}
// drawing
void TaskTableDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
        const QTaskModel *c, const QModelIndex &i) const
{
    painter->save();

    bool selected = (option.state & QStyle::State_Selected) == QStyle::State_Selected;
    QBrush baseBrush = selected ? option.palette.highlight() : option.palette.base();
    QBrush textBrush = selected ? option.palette.highlightedText() : option.palette.text();
    QPalette modpalette(option.palette);

    modpalette.setBrush(QPalette::Text, textBrush);
    modpalette.setBrush(QPalette::Base, baseBrush);

    painter->setBrush(baseBrush);
    painter->setPen(textBrush.color());
    if (selected)
        painter->fillRect(option.rect, baseBrush);

    QVariant value = c->data(i, Qt::DisplayRole);
    QRect textbox(option.rect.x()+2, option.rect.y()+2, option.rect.width()-4, option.rect.height()-4);
    QFont f = option.font;
    QFontMetrics fm(f);

    switch(i.column()) {
        case QTaskModel::Invalid:
        case QTaskModel::Categories:
            break;
        case QTaskModel::Completed:
            {
                QRect box = checkBoxGeom(option.rect);
#if 0
                QStyleOptionButton sob;
                if ( value.toBool() )
                    sob.state = QStyle::Style_On;
                painter->translate(option.rect.x(), option.rect.y());
                qApp->style()->drawPrimitive(QStyle::PE_IndicatorCheckBox, &sob, painter);

#else
                painter->setPen( modpalette.color(QPalette::Text) );
                painter->drawRect( box );
                painter->fillRect( box.x()+1, box.y()+1, box.width()-2, box.height()-2, modpalette.base() );

                // darkGreen might not fit with palette
                //painter->setPen( Qt::darkGreen );
                if ( value.toBool() ) {
                    int i, xx, yy;
                    int sseg = BoxSize / 4;
                    int lseg = BoxSize / 2;
                    sseg -=1; // to fit in BoxSize.
                    //lseg -=1;
                    xx = box.x() + sseg;
                    yy = box.y() + lseg;
                    QPolygon a( 6*2 );
                    // tripple thickens line.
                    for (i=0; i < 3; i++) {
                        a.setPoint(4*i, xx, yy);
                        a.setPoint(4*i+1, xx+sseg, yy+sseg);
                        a.setPoint(4*i+2, xx+sseg, yy+sseg);
                        a.setPoint(4*i+3, xx+sseg+lseg, yy+sseg-lseg);
                        yy++;
                    }
                    painter->drawLines( a );
                }
#endif
            }
            break;
        case QTaskModel::Priority:
            painter->drawText(textbox, Qt::AlignVCenter, QString::number(value.toInt()));
            break;
        default:
        case QTaskModel::Description:
            painter->drawText(textbox, Qt::AlignVCenter, value.toString() );
            break;
        case QTaskModel::Notes:
            painter->drawText(textbox, Qt::AlignVCenter, value.toString().simplified() );
            break;
        case QTaskModel::StartedDate:
            {
                QDate sd = value.toDate();
                if (sd.isValid())
                    painter->drawText(textbox, Qt::AlignVCenter, QTimeString::localYMD( sd ) );
                else
                    painter->drawText(textbox, Qt::AlignVCenter, tr("Not started") );
            }
            break;
        case QTaskModel::DueDate:
            {
                QDate sd = value.toDate();
                if (sd.isValid())
                    painter->drawText(textbox, Qt::AlignVCenter, QTimeString::localYMD( sd ) );
                else
                    painter->drawText(textbox, Qt::AlignVCenter, tr("None", "No due date") );
            }
            break;
        case QTaskModel::CompletedDate:
            {
                QDate cd = value.toDate();
                if (cd.isValid())
                    painter->drawText(textbox, Qt::AlignVCenter, QTimeString::localYMD( cd ) );
                else
                    painter->drawText(textbox, Qt::AlignVCenter, tr("Unfinished") );
            }
            break;
        case QTaskModel::PercentCompleted:
            painter->drawText(textbox, Qt::AlignVCenter, QString::number( value.toInt() ) + "%" );
            break;
        case QTaskModel::Status:
            painter->drawText(textbox, Qt::AlignVCenter,
                    QTask::statusToText(
                        (QTask::Status)value.toInt() ));
            break;
        case QTaskModel::Identifier:
            painter->drawText(textbox, Qt::AlignVCenter,
                    QUniqueId(value.toByteArray()).toString());
            break;
    }
    painter->restore();


}

QWidget *TaskTableDelegate::createEditor(QWidget *w, const QStyleOptionViewItem &o, const QModelIndex &i) const
{
    const QTaskModel *c = qobject_cast<const QTaskModel *>(i.model());
    if (!c)
        return 0;

    if (i.column() == QTaskModel::Priority) {
        int p = c->data(i, Qt::EditRole).toInt();
        PriorityEdit *pe = new PriorityEdit(w);
        pe->setPriority(p);

        connect(pe, SIGNAL(prioritySelected(QWidget *)), this, SIGNAL(commitData(QWidget *)));
        connect(pe, SIGNAL(cancelEdit(QWidget *)), this, SIGNAL(closeEditor(QWidget *)));

        pe->setCellGeometry(o.rect);
        pe->raise();
        pe->show();
        return pe;
    }
    return 0;
}

void TaskTableDelegate::setEditorData(QWidget *w, const QModelIndex &i) const
{
    PriorityEdit *pe = qobject_cast<PriorityEdit *>(w);
    if (pe)
        pe->setPriority(i.model()->data(i, Qt::EditRole).toInt());
}

void TaskTableDelegate::setModelData(QWidget *w, QAbstractItemModel *m, const QModelIndex &i) const
{
    PriorityEdit *pe = qobject_cast<PriorityEdit *>(w);
    if (pe) {
        m->setData(i, QVariant(pe->priority()), Qt::EditRole);
    }
}

// still waiting for this function to be given some real power.
bool TaskTableDelegate::editorEvent(QEvent *e, QAbstractItemModel *model, const QStyleOptionViewItem &, const QModelIndex &i)
{
    QTaskModel *c = qobject_cast<QTaskModel *>(model);
    QKeyEvent *ke = (QKeyEvent *)e;
    QMouseEvent *me = (QMouseEvent *)e;
    if (!c)
        return false;
    if (( e->type() == QEvent::KeyRelease &&
                (ke->key() == Qt::Key_Space || ke->key() == Qt::Key_Select))
            || ( e->type() == QEvent::MouseButtonPress && me->button() == Qt::LeftButton))
    {
        switch(i.column()) {
            case QTaskModel::Priority:
                return false; // editor handles this
            case QTaskModel::Completed:
            {
                // the fact it is necessary to read DisplayRole and write
                // EditRole may be a bug
                bool current = c->data(i, Qt::DisplayRole).toBool();
                c->setData(i, QVariant(!current), Qt::EditRole);
                return true;
            }
            case QTaskModel::Description:
                emit showItem(i);
                return true;
            default:
                break;
        }
    }
    return false;
}

QSize TaskTableDelegate::sizeHint(const QStyleOptionViewItem &o, const QModelIndex &i) const
{
    return sizeHint(o, i.column());
}

QSize TaskTableDelegate::sizeHint(const QStyleOptionViewItem &o, int column) const
{
    QFontMetrics fm(o.font);
    switch(column) {
        default:
        case QTaskModel::Description:
            return QSize(157, fm.height());
        case QTaskModel::Completed:
            return QSize(20,fm.height());
        case QTaskModel::Priority:
            {
                return QSize(fm.width(" 8 "), fm.height());
            }
        case QTaskModel::Status:
            return QSize(70, fm.height());
        case QTaskModel::PercentCompleted:
            return QSize(45, fm.height());
        case QTaskModel::DueDate:
        case QTaskModel::StartedDate:
        case QTaskModel::CompletedDate:
            return QSize(100, fm.height());
    }
    return QSize(100, fm.height());
}

void TaskTableDelegate::updateEditorGeometry(QWidget *w, const QStyleOptionViewItem &o, const QModelIndex &) const
{
    PriorityEdit *pe = qobject_cast<PriorityEdit *>(w);
    if (pe)
        pe->setCellGeometry(o.rect);
}

void TaskTableDelegate::cancelEdit()
{
}

void TaskTableDelegate::setPriority(int)
{
}

#include "taskdelegate.moc"
