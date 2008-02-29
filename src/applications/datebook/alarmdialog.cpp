/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
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

#include <qsoftmenubar.h>
#include <qtopiaapplication.h>
#include <qtimestring.h>

#include <qtimer.h>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QListView>
#include <QStandardItemModel>
#include <QStandardItem>

#include <QPushButton>
#include <QComboBox>
#include <QScrollArea>

// XXX this is a handy delegate - move it to common code?
#include <QFontMetrics>
#include <QFont>
#include <QPainter>
#include <QIcon>
#include <QKeyEvent>

typedef QPair<QString, QString> StringPair;
Q_DECLARE_METATYPE ( StringPair );
Q_DECLARE_METATYPE ( QList<StringPair> );

class TwoLevelDelegate : public QAbstractItemDelegate
{
public:
    explicit TwoLevelDelegate( QObject * parent = 0 );
    virtual ~TwoLevelDelegate();

    enum { SubLabelsRole = Qt::UserRole, SubIconRole, TwoLevelDelegateUserRole = Qt::UserRole + 100 };

    virtual void paint(QPainter *painter, const QStyleOptionViewItem & option,
            const QModelIndex & index ) const;

    virtual QSize sizeHint(const QStyleOptionViewItem & option,
            const QModelIndex &index) const;

private:
    QFont differentFont(const QFont& start, int step) const;
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
    QFormLayout *mainL = new QFormLayout();
    mainL->setSpacing(2);
    mainL->setMargin(2);

    mAlarmList = new QListView();
    mAlarmList->setSelectionMode(QListView::SingleSelection);
    mAlarmList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    //mAlarmList->setUniformItemSizes(true);
    mAlarmList->setAlternatingRowColors(true);
    mAlarmList->setItemDelegate(new TwoLevelDelegate(mAlarmList));
    mAlarmList->setResizeMode(QListView::Adjust);
    mAlarmList->setLayoutMode(QListView::Batched);

    mainL->addRow(mAlarmList);

    mSnoozeButton = new QPushButton(tr("Snooze"));
    mainL->addWidget(mSnoozeButton);

    mSnoozeChoices = new QComboBox();

    mSnoozeChoices->clear();
    mSnoozeChoices->addItem(tr("5 minutes"));
    mSnoozeChoices->addItem(tr("10 minutes"));
    mSnoozeChoices->addItem(tr("15 minutes"));
    mSnoozeChoices->addItem(tr("30 minutes"));
    mSnoozeChoices->addItem(tr("1 hour"));
    mSnoozeChoices->addItem(tr("1 day"));
    mSnoozeChoices->addItem(tr("1 week"));
    mSnoozeChoices->addItem(tr("1 month"));

    mainL->addRow(tr("Snooze delay:"), mSnoozeChoices);
    setLayout(mainL);

    mStandardModel = new QStandardItemModel(this);
    mAlarmList->setModel(mStandardModel);
    connect(mAlarmList->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            this, SLOT(currentAlarmChanged(QModelIndex)));
    connect(mAlarmList, SIGNAL(activated(QModelIndex)), this, SLOT(alarmSelected(QModelIndex)));

    connect(mSnoozeButton, SIGNAL(clicked()), this, SLOT(snoozeClicked()));

    QSoftMenuBar::setLabel(this, Qt::Key_Select, QSoftMenuBar::View);
}

void AlarmDialog::formatDateTime(const QOccurrence& ev, bool useStartTime, QString& localDateTime, QString& realDateTime)
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

void AlarmDialog::snoozeClicked()
{
    /* Snooze for some amount of time (configured in settings, say) */
    int snoozeindex = mSnoozeChoices->currentIndex();

    int snoozedelay = 60;
    // Make sure we set alarms on the minute by rounding
    QDateTime now = QDateTime::currentDateTime();
    int seconds = now.time().second();
    if (seconds >= 30)
        now = now.addSecs(60 - seconds);
    else
        now = now.addSecs(-seconds);

    switch(snoozeindex) {
        case 0: // 5 minutes
            snoozedelay = 300;
            break;
        case 1: // 10 minutes
            snoozedelay = 600;
            break;
        case 2: // 15 minutes
            snoozedelay = 900;
            break;
        case 3: // 30 minutes
            snoozedelay = 1800;
            break;
        case 4: // 1 hour
            snoozedelay = 3600;
            break;
        case 5: // 1 day
            snoozedelay = 24 * 60;
            break;
        case 6: // 1 week
            snoozedelay = 7 * 24 * 60;
            break;
        case 7: // 1 month hmm
            {
                QDateTime then = now.addMonths(1);
                snoozedelay = now.secsTo(then);
            }
            break;
    }

    Qtopia::addAlarm(now.addSecs(snoozedelay), "Calendar", "alarm(QDateTime,int)", (mStartTime.secsTo(now) + snoozedelay) / 60);
    reject();
}


bool AlarmDialog::eventFilter( QObject *receiver, QEvent *event )
{
    /* XXX key nav for listview is required */
    Q_UNUSED(receiver);
    Q_UNUSED(event);
    return false;
}


AlarmDialog::Button AlarmDialog::exec(QOccurrenceModel* model, const QDateTime& startTime, int /*warnDelay*/)
{
    bool playSound = false;
    QIcon aicon(":icon/audible");
    QIcon sicon(":icon/silent");

    mAlarmCount = 0;
    mAlarmTimerId = 0;
    mModel = model;
    mStartTime = startTime;

    mStandardModel->clear();

    QString localDT;
    QString tzDT;

    // Filter out occurrences that do not have an alarm
    for (int i=0; i < model->rowCount(); i++) {
        QOccurrence o = model->occurrence(i);
        QAppointment a = o.appointment();
        if (a.hasAlarm() && (o.startInCurrentTZ() == startTime) /* && (o.alarmDelay() == warnDelay)*/) {
            if (!playSound && (a.alarm() == QAppointment::Audible)) {
                playSound = true;
            }
            QStandardItem* item = new QStandardItem();
            if (a.alarm() == QAppointment::Audible)
                item->setData(aicon, Qt::DecorationRole);
            else
                item->setData(sicon, Qt::DecorationRole);

            if (!a.description().isEmpty())
                item->setData(a.description(), Qt::DisplayRole);
            else
                item->setData(tr("No description", "no description for appointment"), Qt::DisplayRole);

            QList< StringPair > subList;
            if (!a.location().isEmpty()) {
                subList.append(qMakePair(QString(), a.location()));
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
            item->setData(QVariant::fromValue(subList), TwoLevelDelegate::SubLabelsRole);

            item->setData(i, TwoLevelDelegate::TwoLevelDelegateUserRole);
            mStandardModel->appendRow(item);

            /* Clear the alarm now, since we're going to display it */
            // a.clearAlarm(); // XXX??
        }
    }

    int rowCount = mStandardModel->rowCount();

    // Select the first item
    mAlarmList->setCurrentIndex(mStandardModel->index(0,0));

    if (rowCount < 2) {
        setWindowTitle(tr("Reminder"));
    } else {
        setWindowTitle(tr("Reminders"));
    }

    // XXX why do I have to do this?
    int h = 0;
    for (int i = 0; i < rowCount; i++) {
        h += mAlarmList->sizeHintForRow(i);
    }
    mAlarmList->setFixedHeight(qMax(h, 160));

    mSnoozeButton->setFocus();

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
        int row = mStandardModel->data(mAlarmList->currentIndex(), TwoLevelDelegate::TwoLevelDelegateUserRole).toInt();
        return mModel->occurrence(row);
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
    /*
    mDetails->init( selectedOccurrence(), true );
    if (idx.isValid())
        mShowButton->setEnabled(true);
    else
        mShowButton->setEnabled(false);
    */
}

void AlarmDialog::alarmSelected(const QModelIndex& idx)
{
    mAlarmList->setCurrentIndex(idx);
    mButton = Details;
    accept();
}


TwoLevelDelegate::TwoLevelDelegate( QObject * parent )
    : QAbstractItemDelegate(parent)
{
}

TwoLevelDelegate::~TwoLevelDelegate() {}

QFont TwoLevelDelegate::differentFont(const QFont& start, int step) const
{
    int osize = QFontMetrics(start).lineSpacing();
    QFont f = start;
    for (int t=1; t<6; t++) {
        int newSize = f.pointSize() + step;
        if ( newSize > 0 )
            f.setPointSize(f.pointSize()+step);
        else
            return start; // we cannot find a font -> return old one
        step += step < 0 ? -1 : +1;
        QFontMetrics fm(f);
        if ( fm.lineSpacing() != osize )
            break;
    }
    return f;
}


void TwoLevelDelegate::paint(QPainter *painter, const QStyleOptionViewItem & option,
        const QModelIndex & index ) const
{
    QString text = index.model()->data(index, Qt::DisplayRole).toString();
    QList< StringPair > subTexts = index.model()->data(index, SubLabelsRole).value<QList<StringPair> >();

    QIcon decoration = qvariant_cast<QIcon>(index.model()->data(index, Qt::DecorationRole));
    QIcon secondaryDecoration = qvariant_cast<QIcon>(index.model()->data(index, SubIconRole));

    bool rtl = QtopiaApplication::layoutDirection() == Qt::RightToLeft ;

    painter->save();

    // fill rect based on row background
    // or assume can be left to list class
    bool selected = (option.state & (QStyle::State_Active | QStyle::State_Selected)) == (QStyle::State_Active | QStyle::State_Selected);
    QBrush baseBrush = selected ? option.palette.highlight() : option.palette.base();
    QBrush textBrush = selected ? option.palette.highlightedText() : option.palette.text();
    QPalette modpalette(option.palette);
    modpalette.setBrush(QPalette::Text, textBrush);
    // To avoid messing with the alternating row color, don't set this...
//    modpalette.setBrush(QPalette::Base, baseBrush);
//    painter->setBrush(baseBrush);

    painter->setPen(textBrush.color());

    if (selected)
        painter->fillRect(option.rect, baseBrush);
    QFont fbold = option.font;
    QFont fsmall = differentFont(option.font, -2);
    QFont fsmallbold = fsmall;
    fbold.setWeight(QFont::Bold);
    fsmallbold.setWeight(QFont::Bold);

    QFontMetrics fboldM(fbold);
    QFontMetrics fsmallM(fsmall);
    QFontMetrics fsmallboldM(fsmallbold);

    int decorationSize = qApp->style()->pixelMetric(QStyle::PM_ListViewIconSize);

    int x = option.rect.x();
    int y = option.rect.y();
    int width = option.rect.width();
    int height = option.rect.height()-1;

    if ( rtl )
        painter->drawPixmap(width-decorationSize, y, decoration.pixmap(decorationSize) );
    else
        painter->drawPixmap(x, y, decoration.pixmap(decorationSize));

    x += decorationSize + 2;
    width -= decorationSize + 2;

    int bcardwidth = 0;
    int bcardbase = y;
    if (!secondaryDecoration.isNull()) {
        bcardwidth = decorationSize;
        bcardbase = y+decorationSize;
        if ( rtl )
            painter->drawPixmap( option.rect.x(), y, secondaryDecoration.pixmap(decorationSize));
        else
            painter->drawPixmap(x+width-bcardwidth, y,secondaryDecoration.pixmap(decorationSize));
    }

    // draw label bold
    painter->setFont(fbold);

    // fit inside available width;
    QRect space;
    if ( rtl )
        space = QRect(option.rect.x() + bcardwidth, y, width-bcardwidth, height);
    else
        space = QRect(x, y, width - bcardwidth, height);

    painter->drawText(space, Qt::AlignLeading, fboldM.elidedText(text, Qt::ElideRight, space.width()));

    y+=fboldM.height();


    QString secText;
    StringPair subLine;

    int headerWidth = 0;
    /* First, calculate the width of all the header sections */
    foreach(subLine, subTexts) {
        if (!subLine.first.isEmpty()) {
            int w = fsmallboldM.boundingRect(subLine.first).width();
            if (w > headerWidth)
                headerWidth = w;
        }
    }

    /* Now draw! */
    while (y + fsmallM.height() <= option.rect.bottom() && !subTexts.isEmpty()) {

        if (y > bcardbase) {
            bcardwidth = 0;
            if ( rtl )
                space = QRect(option.rect.x(), y, width, height);
            else
                space = QRect(x, y, width, height);
        } else
            space.setTop(y);

        subLine = subTexts.takeFirst();
        if (!subLine.first.isNull()) {
            if (!subLine.second.isEmpty()) {
                QRect headerRect = space;
                QRect valueRect = space;
                if (rtl) {
                    headerRect.setLeft(option.rect.x() - headerWidth);
                    valueRect.setRight(option.rect.x() - headerWidth);
                } else {
                    headerRect.setRight(x + headerWidth);
                    valueRect.setLeft(x + headerWidth);
                }
                painter->setFont(fsmallbold);
                painter->drawText(headerRect, Qt::AlignLeading, fsmallboldM.elidedText(subLine.first, Qt::ElideRight, headerRect.width()));
                painter->setFont(fsmall);
                painter->drawText(valueRect, Qt::AlignLeading, fsmallM.elidedText(subLine.second, Qt::ElideRight, valueRect.width()));
            } else {
                painter->setFont(fsmallbold);
                painter->drawText(space, Qt::AlignLeading, fsmallboldM.elidedText(subLine.first, Qt::ElideRight, space.width()));
            }
        } else {
            if (!subLine.second.isEmpty()) {
                painter->setFont(fsmall);
                painter->drawText(space, Qt::AlignLeading, fsmallM.elidedText(subLine.second, Qt::ElideRight, space.width()));
            }
        }

        y += fsmallM.height() + 1;
    }
    painter->restore();
}

/*!
   \overload

   Returns the size hint for objects drawn with the delgate with style options \a option for item at \a index.
*/
QSize TwoLevelDelegate::sizeHint(const QStyleOptionViewItem & option,
        const QModelIndex &index) const
{
    QList< StringPair > subTexts = index.model()->data(index, SubLabelsRole).value<QList<StringPair> >();

    QFontMetrics fm(option.font);
    QFontMetrics sfm(differentFont(option.font, -2));

    return QSize(qApp->style()->pixelMetric(QStyle::PM_ListViewIconSize) + fm.width("M")*10,
                 qMax(qApp->style()->pixelMetric(QStyle::PM_ListViewIconSize), fm.height() + ( subTexts.count() * (sfm.height() + 1))));
}
