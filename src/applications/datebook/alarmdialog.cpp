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

#include "alarmdialog.h"

#include "qappointmentmodel.h"

#include "qsoftmenubar.h"

#include <qtopiaapplication.h>
#include <qtimestring.h>

#include <qspinbox.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qtimer.h>
#include <qlayout.h>
#include <QVBoxLayout>
#include <QListView>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QGroupBox>
#include <QPainter>
#include <QKeyEvent>

#include <QSortFilterProxyModel>

#include "qpimdelegate.h"

class AlarmFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    AlarmFilterModel(QObject *parent) : QSortFilterProxyModel(parent) {}
    ~AlarmFilterModel();

    void setAcceptedRows(QList<int> acceptRows) {acceptedRows = acceptRows;}

protected:
    virtual bool filterAcceptsRow ( int source_row, const QModelIndex & ) const {return acceptedRows.contains(source_row);}

private:
    QList<int> acceptedRows; 
};

class AlarmDelegate : public QPimDelegate
{
    Q_OBJECT
public:
    explicit AlarmDelegate( QObject * parent = 0 );
    virtual ~AlarmDelegate();

    QList<StringPair> subTexts(const QStyleOptionViewItem &option, const QModelIndex& index) const;
    SubTextAlignment subTextAlignment(const QStyleOptionViewItem &option, const QModelIndex& index) const;

    void drawDecorations(QPainter* p, bool rtl, const QStyleOptionViewItem &option, const QModelIndex& index, QList<QRect>& leadingFloats, QList<QRect>& trailingFloats) const;
    QSize decorationsSizeHint(const QStyleOptionViewItem& option, const QModelIndex& index, const QSize& textSize) const;

private:
    static void formatDateTime(const QOccurrence& ev, bool useStartTime, QString& localDateTime, QString& realDateTime);
};

AlarmDialog::AlarmDialog( QWidget *parent, Qt::WFlags f )
    : QDialog( parent, f ),
    mButton( Cancel ), mAlarmCount(0), mAlarmTimerId(0), mModel(0), mAlarmList(0)
{
    init();
}

void AlarmDialog::init()
{
    /* Create stuff! */
    QVBoxLayout *mainL = new QVBoxLayout();
    mainL->setSpacing(2);
    mainL->setMargin(2);

    mAlarmList = new QListView();
    mAlarmList->setSelectionMode(QListView::SingleSelection);
    mAlarmList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mAlarmList->setItemDelegate(new AlarmDelegate(mAlarmList));

    mFilterModel = new AlarmFilterModel(this);

    mainL->addWidget(mAlarmList);
    setLayout(mainL);

    connect(mAlarmList, SIGNAL(activated(const QModelIndex&)), this, SLOT(alarmSelected(const QModelIndex&)));
    QSoftMenuBar::setLabel(this, Qt::Key_Select, QSoftMenuBar::View);
}

AlarmDialog::Button AlarmDialog::exec(QOccurrenceModel* model, const QDateTime& startTime, int warnDelay)
{
    bool playSound = false;

    mAlarmCount = 0;
    mAlarmTimerId = 0;
    mModel = model;

    mFilterModel->setSourceModel(model);

    QList<int> validRows;

    // Filter out occurrences that do not have an alarm
    for (int i=0; i < model->rowCount(); i++) {
        QOccurrence o = model->occurrence(i);
        QAppointment a = o.appointment();
        if (a.hasAlarm() && (o.startInCurrentTZ() == startTime) && (o.alarmDelay() == warnDelay)) {
            if (!playSound && (a.alarm() == QAppointment::Audible))
                playSound = true;

            validRows.append(i);

            /* Clear the alarm now, since we're going to display it */
            a.clearAlarm();
        }
    }

    mFilterModel->setAcceptedRows(validRows);

    mAlarmList->setModel(mFilterModel);
    connect(mAlarmList->selectionModel(), SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)),
            this, SLOT(currentAlarmChanged(const QModelIndex&)));

    int rowCount = validRows.size();

    // Select the first item
    mAlarmList->setCurrentIndex(mFilterModel->index(0,0));

    if (rowCount < 2) {
        setWindowTitle(tr("Reminder"));
        mAlarmList->setFocusPolicy(Qt::NoFocus);
        mAlarmList->setSelectionMode(QAbstractItemView::NoSelection);
        mAlarmList->selectionModel()->clearSelection();
        mAlarmList->clearFocus();
    } else {
        setWindowTitle(tr("Reminders"));
        mAlarmList->setSelectionMode(QAbstractItemView::SingleSelection);
        mAlarmList->setFocus();
    }

    // If we actually got any matching alarms...
    if (rowCount > 0) {
        if (playSound) {
            Qtopia::soundAlarm();
            mAlarmTimerId = startTimer(5000);
        }

        int ret = QtopiaApplication::execDialog( this );

        // Turn off sound
        if (playSound)
            killTimer(mAlarmTimerId);

        if (ret)
            return mButton;
        else
            return Cancel;
    } else
        return Cancel;
}

void AlarmDialog::timerEvent(QTimerEvent *e)
{
    if (e->timerId() == mAlarmTimerId) {
        if (mAlarmCount < 10) {
            Qtopia::soundAlarm();
            mAlarmCount++;
        } else {
            killTimer(mAlarmTimerId);
        }
    } else {
        QDialog::timerEvent(e);
    }
}
void AlarmDialog::keyPressEvent( QKeyEvent * ke)
{
    if (ke->key() == Qt::Key_Select) {
        mButton = Details;
        accept();
    } else
        QDialog::keyPressEvent(ke);
}

QOccurrence AlarmDialog::selectedOccurrence() const
{
    if (mModel && mAlarmList->currentIndex().isValid()) {
        return mModel->occurrence(mFilterModel->mapToSource(mAlarmList->currentIndex()));
    } else
        return QOccurrence();
}

AlarmDialog::Button AlarmDialog::result()
{
    return mButton;
}

void AlarmDialog::currentAlarmChanged(const QModelIndex &idx)
{
    mAlarmList->setCurrentIndex(idx);
}

void AlarmDialog::alarmSelected(const QModelIndex& idx)
{
    mAlarmList->setCurrentIndex(idx);
    mButton = Details;
    accept();
}

/// Delegate
AlarmDelegate::AlarmDelegate(QObject *parent) : QPimDelegate(parent)
{

}

AlarmDelegate::~AlarmDelegate()
{

}

QList<StringPair> AlarmDelegate::subTexts(const QStyleOptionViewItem &option, const QModelIndex& index) const
{
    Q_UNUSED(option);

    QList< StringPair > subList;
    // Darn, we need to get a full QOccurrence
    const QSortFilterProxyModel *fm = qobject_cast<const QSortFilterProxyModel*>(index.model());
    if (fm ) {
        const QOccurrenceModel *om = qobject_cast<const QOccurrenceModel*>(fm->sourceModel());
        if (om) {
            QOccurrence o = om->occurrence(fm->mapToSource(index));
            QAppointment a = o.appointment();

            QString localDT;
            QString tzDT;

            if (!a.location().isEmpty()) {
                subList.append(qMakePair(tr("Where: "), a.location()));
            }

            formatDateTime(o, true, localDT, tzDT);
            if (a.isAllDay()) {
                subList.append(qMakePair(tr("All day: "), localDT));
            } else {
                subList.append(qMakePair(tr("Starts: "), localDT));
                if (!tzDT.isEmpty())
                    subList.append(qMakePair(QString(""), tzDT));
                formatDateTime(o, false, localDT, tzDT);
                subList.append(qMakePair(tr("Ends: "), localDT));
                if (!tzDT.isEmpty())
                    subList.append(qMakePair(QString(""), tzDT));
            }
        }
    }
    return subList;
}

void AlarmDelegate::drawDecorations(QPainter* p, bool rtl, const QStyleOptionViewItem &option,
                                     const QModelIndex& index, QList<QRect>& leadingFloats, QList<QRect>& trailingFloats) const
{
    Q_UNUSED(option);
    Q_UNUSED(trailingFloats);

    int decorationSize = qApp->style()->pixelMetric(QStyle::PM_ListViewIconSize);
    QIcon i;

    const QSortFilterProxyModel *fm = qobject_cast<const QSortFilterProxyModel*>(index.model());

    if (fm ) {
        const QOccurrenceModel *om = qobject_cast<const QOccurrenceModel*>(fm->sourceModel());
        const QModelIndex& sourceIndex = fm->mapToSource(index);
        if (om) {
            QAppointment::AlarmFlag a = (QAppointment::AlarmFlag) om->data(sourceIndex.sibling(sourceIndex.row(), QAppointmentModel::Alarm), Qt::DisplayRole).toInt();

            if (a == QAppointment::Audible)
                i = QIcon(":icon/audible");
            else
                i = QIcon(":icon/silent");
        }
    }

    QRect drawRect = option.rect;
    if (rtl) {
        drawRect.setLeft(drawRect.right() - decorationSize);
    } else {
        drawRect.setRight(decorationSize);
    }
    QPoint drawOffset = QPoint(drawRect.left(), drawRect.top());
    if (drawRect.height() < 2 * decorationSize)
        drawOffset.ry() += (drawRect.height() - decorationSize) / 2;

    p->drawPixmap(drawOffset, i.pixmap(decorationSize));

    leadingFloats.append(drawRect);
}

QSize AlarmDelegate::decorationsSizeHint(const QStyleOptionViewItem &option, const QModelIndex& index, const QSize& s) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    int decorationSize = qApp->style()->pixelMetric(QStyle::PM_ListViewIconSize);
    return QSize(decorationSize + s.width(), qMax(decorationSize + 2, s.height()));
}

QPimDelegate::SubTextAlignment AlarmDelegate::subTextAlignment(const QStyleOptionViewItem &option, const QModelIndex& index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    return AlarmDelegate::CuddledPerItem;
}

void AlarmDelegate::formatDateTime(const QOccurrence& ev, bool useStartTime, QString& localDateTime, QString& realDateTime)
{
    QDate today = QDate::currentDate();
    QString text;
    QDateTime dt(useStartTime ? ev.startInCurrentTZ() : ev.endInCurrentTZ());

    if (dt.date() == today) {
        // Short version
        if (ev.appointment().isAllDay())
            localDateTime = tr("Today");
        else
            localDateTime = tr("Today, ", "Today, 11:59pm") + QTimeString::localHM( dt.time() );
    } else {
        // Long version
        if (ev.appointment().isAllDay())
            localDateTime = QTimeString::localYMD( dt.date() );
        else
            localDateTime = QTimeString::localYMD( dt.date() ) + ' ' + QTimeString::localHM( dt.time() );
    }

    realDateTime.clear();
    if ( ev.timeZone().isValid() && ev.timeZone() != QTimeZone::current() ) {
        QTime tzt(useStartTime ? ev.start().time() : ev.end().time());
        realDateTime = tr("(%1 %2 time)", "eg. 10:00 Sydney time").arg(QTimeString::localHM( tzt )).arg(ev.timeZone().city());
    }
}

AlarmFilterModel::~AlarmFilterModel()
{
}

#include "alarmdialog.moc"
