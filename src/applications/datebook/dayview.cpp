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

#include "dayview.h"

#include <qtimestring.h>
#include <qtopia/pim/qappointment.h>
#include <qtopia/pim/qappointmentmodel.h>
#include <qappointmentview.h>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QKeyEvent>
#include <QItemDelegate>
#include <QItemSelectionModel>

DayView::DayView(QWidget *parent, const QCategoryFilter& c)
    : QWidget(parent), timedModel(0), allDayModel(0)
{
    mWeekdayLabel = new QLabel();
    mDateLabel = new QLabel();
    mHiddenIndicator = new QLabel();
    // RTL language support, not applicable, as this is always supposed to hug the outer.
    mWeekdayLabel->setAlignment(Qt::AlignLeft);
    mDateLabel->setAlignment(Qt::AlignRight);

    QHBoxLayout *ll = new QHBoxLayout();
    ll->setMargin(2);
    ll->setSpacing(0);
    ll->addWidget(mWeekdayLabel);
    ll->addWidget(mDateLabel);

    mTimeManager = new CompressedTimeManager();
    mTimedView = new TimedView();
    QAppointmentDelegate *id = new QAppointmentDelegate(this);

    QDateTime start(mTimedView->date(), QTime(0, 0));
    QDateTime end(mTimedView->date().addDays(1), QTime(0, 0));

    timedModel = new QOccurrenceModel(start, end, this);
    timedModel->setDurationType(QAppointmentModel::TimedDuration);
    timedModel->setCategoryFilter(c);
    mTimedView->setModel(timedModel);
    mTimedView->setItemDelegate(id);

    allDayModel = new QOccurrenceModel(start, end, this);
    allDayModel->setDurationType(QAppointmentModel::AllDayDuration);
    allDayModel->setCategoryFilter(c);
    mAllDayList = new AppointmentList();
    mAllDayList->setModel(allDayModel);
    mAllDayList->setItemDelegate(id);
    mAllDayList->setFolded(true);

    connect(mTimedView, SIGNAL(selectionChanged(const QModelIndex&)), this, SLOT(timedSelectionChanged(const QModelIndex&)));
    connect(timedModel, SIGNAL(modelReset()), this, SLOT(modelsReset()));
    connect(mAllDayList, SIGNAL(changeHiddenCount(int)), this, SLOT(updateHiddenIndicator(int)));
    connect(mAllDayList, SIGNAL(activated(const QModelIndex &)), this, SLOT(allDayOccurrenceActivated(const QModelIndex &)));
    connect(mAllDayList->selectionModel(), SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)), this, SLOT(allDayOccurrenceChanged(const QModelIndex &)));
    connect(allDayModel, SIGNAL(modelReset()), this, SLOT(modelsReset()));

    mScrollArea = new QScrollArea();
    mScrollArea->setWidgetResizable(true);
    mScrollArea->viewport()->installEventFilter(this);
    mScrollArea->setFrameShape(QFrame::NoFrame);

    QVBoxLayout *ml = new QVBoxLayout();
    ml->addLayout(ll);
    ml->addWidget(mAllDayList);
    ml->addWidget(mHiddenIndicator);
    ml->addWidget(mScrollArea);
    ml->setMargin(0);
    ml->setSpacing(0);
    setLayout(ml);

    QWidget *contents = new QWidget();
    QHBoxLayout *hl = new QHBoxLayout(contents);


    hl->addWidget(mTimeManager);
    hl->addWidget(mTimedView);
    hl->setMargin(0);
    hl->setSpacing(0);

    contents->setLayout(hl);

    mScrollArea->setWidget(contents);
    updateHeaderText();

    mTimedView->setTimeManager(mTimeManager);

    firstTimed();

    setFocusPolicy(Qt::StrongFocus);
}

void DayView::keyPressEvent(QKeyEvent *e)
{
    QDate cDate = currentDate();
    switch(e->key()) {
        case Qt::Key_Up:
            previousOccurrence();
            break;
        case Qt::Key_Down:
            nextOccurrence();
            break;
        case Qt::Key_Left:
            selectDate(cDate.addDays(-1));
            break;
        case Qt::Key_Right:
            selectDate(cDate.addDays(1));
            break;
#ifdef QTOPIA_PHONE
        case Qt::Key_1:
            selectDate(cDate.addDays(-7));
            break;
        case Qt::Key_3:
            selectDate(cDate.addDays(7));
            break;
        case Qt::Key_4:
            if (cDate.month() == 1)
                selectDate(cDate.year()-1, 12);
            else
                selectDate(cDate.year(), cDate.month()-1);
            break;
        case Qt::Key_6:
            if (cDate.month() == 12)
                selectDate(cDate.year()+1, 1);
            else
                selectDate(cDate.year(), cDate.month()+1);
            break;
        case Qt::Key_7:
            selectDate(cDate.year()-1, cDate.month());
            break;
        case Qt::Key_9:
            selectDate(cDate.year()+1, cDate.month());
            break;
        case Qt::Key_5:
            selectDate(QDate::currentDate());
            break;
        case Qt::Key_Select:
            if ( currentIndex().isValid() )
                emit showDetails();
            break;
#else
        case Qt::Key_Space:
        case Qt::Key_Return:
            if ( currentIndex().isValid() )
                emit showDetails();
            break;
#endif
        default:
            e->ignore();
    }
}

void DayView::mouseReleaseEvent( QMouseEvent * event )
{
    QModelIndex mi = mTimedView->index ( event->globalPos() );
    if ( mi.isValid() ) {
        mTimedView->setCurrentIndex( mi );
        mAllDayList->selectionModel()->clear();
        emit showDetails();
    } else {
        // create an event at the hour represented by the click
        // if it is inside the timedview
        QDateTime startTime = mTimedView->timeAtPoint( event->globalPos(), -1 );
        if ( ! startTime.isNull() ) {
            QDateTime endTime = mTimedView->timeAtPoint( event->globalPos(), 1 );
            if ( ! endTime.isNull() )
                emit newAppointment(startTime, endTime);
        }
        // XXX or expand the appointments if
        // the "%d more all day appointments" label is clicked?
    }
}

QModelIndex DayView::currentIndex() const
{
    QModelIndex index = mTimedView->currentIndex();
    if (index.isValid())
        return index;

    return mAllDayList->currentIndex();
}

void DayView::categorySelected( const QCategoryFilter &c )
{
    timedModel->setCategoryFilter( c );
    allDayModel->setCategoryFilter( c );
}

QOccurrenceModel *DayView::currentModel() const
{
    QModelIndex index = mTimedView->currentIndex();
    if (index.isValid())
        return timedModel;
    else
        return allDayModel;
}

QAppointment DayView::currentAppointment() const
{
    return currentModel()->appointment(currentIndex());
}

QOccurrence DayView::currentOccurrence() const
{
    QModelIndex index = currentIndex();
    if (index.isValid())
        return currentModel()->occurrence(currentIndex());
    return QOccurrence();
}

void DayView::selectDate( const QDate &date )
{
    if (!date.isValid() || date == currentDate())
        return;

    QDateTime start(date, QTime(0,0));
    QDateTime end(date.addDays(1), QTime(0,0));

    mTimedView->setDate(date);

    allDayModel->setRange(start, end);

    firstTimed();

    updateHeaderText();

    emit dateChanged();
}

void DayView::selectDate( int y, int m)
{
    QDate cDate = currentDate();
    if ( y != cDate.year() || m != cDate.month() ) {
        QDate nd( y, m, 1 );
        if ( nd.daysInMonth() < cDate.day() )
            selectDate(QDate(y, m, nd.daysInMonth()));
        else
            selectDate(QDate(y, m, cDate.day()));
    }
}

QDate DayView::currentDate() const
{
    return mTimedView->date();
}

bool DayView::allDayFolded() const
{
    return mAllDayList->isFolded();
}

bool DayView::allDayFoldingAvailable() const
{
    return mAllDayList->provideFoldingOption();
}

void DayView::setAllDayFolded(bool f)
{
    mAllDayList->setFolded(f);
}

void DayView::updateHeaderText()
{
    QDate cDate = currentDate();

    if (cDate == QDate::currentDate())
        mWeekdayLabel->setText(tr("Today (%1)", "Today (Fri)").arg(QTimeString::localDayOfWeek(cDate, QTimeString::Medium)));
    else
        mWeekdayLabel->setText(QTimeString::localDayOfWeek(cDate, QTimeString::Long));

    mDateLabel->setText(QTimeString::localYMD(cDate));
}

void DayView::setDaySpan( int starthour, int endhour )
{
    mTimedView->setDaySpan(starthour*60, endhour*60);
}

void DayView::firstTimed()
{
    timedModel->completeFetch();
    allDayModel->completeFetch();
    if (timedModel->rowCount()) {
        mAllDayList->selectionModel()->clear();
        mTimedView->setCurrentIndex(timedModel->index(0, 0));
    } else if (allDayModel->rowCount())
        mAllDayList->setCurrentIndex(allDayModel->index(0, 0));
}

void DayView::lastTimed()
{
    timedModel->completeFetch();
    allDayModel->completeFetch();
    if (timedModel->rowCount()) {
        mAllDayList->selectionModel()->clear();
        mTimedView->setCurrentIndex(timedModel->index(timedModel->rowCount() - 1, 0));
    }
}

void DayView::firstAllDay()
{
    timedModel->completeFetch();
    allDayModel->completeFetch();
    if (allDayModel->rowCount()) {
        mAllDayList->setCurrentIndex(allDayModel->index(0, 0));
        mTimedView->setCurrentIndex(QModelIndex());
    } else if ( timedModel->rowCount() > 0 ) {
        mTimedView->setCurrentIndex(timedModel->index(0,0));
    }
}

void DayView::lastAllDay()
{
    timedModel->completeFetch();
    allDayModel->completeFetch();
    if (allDayModel->rowCount()) {
        mAllDayList->setCurrentIndex(allDayModel->index(mAllDayList->visibleRowCount() - 1, 0));
        mTimedView->setCurrentIndex(QModelIndex());
    } else if ( timedModel->rowCount() > 0 ) {
        // Wrap around
        mTimedView->setCurrentIndex(timedModel->index(timedModel->rowCount()-1,0));
    }
}

void DayView::nextOccurrence()
{
    if (timedModel->fetching()) {
        return; //doesn't delay, just avoids.
    }
    if (currentModel() == allDayModel) {
        if (mAllDayList->currentIndex().row() == mAllDayList->visibleRowCount() - 1) {
            firstTimed();
        } else {
            mAllDayList->setCurrentIndex(allDayModel->index(mAllDayList->currentIndex().row() + 1,
                                         0));
        }
    } else {
        // Down key activated for multiple meetings/appointments.
        if (mTimedView->currentIndex().row() < timedModel->rowCount() - 1) {
            mTimedView->setCurrentIndex(timedModel->index(mTimedView->currentIndex().row() + 1, 0));
        } else {
            // Wrap around.
            firstAllDay();
        }
    }
}

void DayView::previousOccurrence()
{
    if (timedModel->fetching())
        return; //doesn't delay, just avoids.
    if (currentModel() == timedModel) {
        if (mTimedView->currentIndex().row() == 0) {
            lastAllDay();
        } else {
            mTimedView->setCurrentIndex(timedModel->index(mTimedView->currentIndex().row() - 1,
                                        0));
        }
    } else {
        if (mAllDayList->currentIndex().row() > 0) {
            mAllDayList->setCurrentIndex(allDayModel->index(mAllDayList->currentIndex().row() - 1,
                                        0));
        } else {
            lastTimed();
        }
    }
}

void DayView::setCurrentOccurrence(const QOccurrence &o)
{
    if (o.startInCurrentTZ().date() != mTimedView->date())
        selectDate(o.startInCurrentTZ().date());

    if (o.appointment().isAllDay()) {
        allDayModel->completeFetch();
        mAllDayList->setCurrentIndex(allDayModel->index(o));
    } else {
        timedModel->completeFetch();
        mTimedView->setCurrentIndex(timedModel->index(o));
    }
}

void DayView::setCurrentAppointment(const QAppointment &a)
{
    setCurrentOccurrence(a.firstOccurrence());
}

bool DayView::eventFilter(QObject *o, QEvent *e)
{
    if (o == mScrollArea->viewport() && e->type() == QEvent::Resize) {
        mTimeManager->setIdealHeight(mScrollArea->viewport()->height());
    }

    return false;
}

void DayView::resizeEvent(QResizeEvent *)
{
    //  Allow all day events to take no more than 1/3 of the screen.
    mAllDayList->setMaximumFoldedHeight(height() / 3);
    layout()->activate();
}

void DayView::timedSelectionChanged(const QModelIndex &index)
{
    // Save the last selected occurrence, too
    if (index.isValid()) {
        lastSelectedTimedId = timedModel->id(index);
        lastSelectedAllDayId = QUniqueId();

        QRect r = mTimedView->occurrenceRect(index);

        mScrollArea->ensureVisible( 0, r.bottom(), 10, 10 );
        mScrollArea->ensureVisible( 0, r.top(), 10, 10 );
    } else {
        lastSelectedTimedId = QUniqueId();
    }

    emit selectionChanged();
}

void DayView::allDayOccurrenceActivated(const QModelIndex &index)
{
    if ( index.isValid() )
        emit showDetails();
}

void DayView::allDayOccurrenceChanged(const QModelIndex &index)
{
    if (index.isValid()) {
        // Make sure this is cleared
        mTimedView->setCurrentIndex(QModelIndex());

        // and make sure this is selected
        mAllDayList->selectionModel()->setCurrentIndex(index, QItemSelectionModel::SelectCurrent);

        // And save the id
        lastSelectedAllDayId = allDayModel->id(index);
        lastSelectedTimedId = QUniqueId();
    } else {
        lastSelectedAllDayId = QUniqueId();
    }

    emit selectionChanged();
}

void DayView::modelsReset()
{
    bool foundSomething = false;
    // We lose our selections when the models get reset, so try and restore them
    if ( ! lastSelectedAllDayId.isNull()) {
        QModelIndex newsel = allDayModel->index(lastSelectedAllDayId);
        if ( newsel.isValid() ) {
            foundSomething = true;
            mAllDayList->selectionModel()->setCurrentIndex(newsel, QItemSelectionModel::SelectCurrent);
        }
    } else if ( !lastSelectedTimedId.isNull()) {
        QModelIndex newsel = timedModel->index(lastSelectedTimedId);
        if ( newsel.isValid() ) {
            foundSomething = true;
            mTimedView->setCurrentIndex(newsel);
        }
    }

    /* Well, didn't select anything, so select the first event */
    if (!foundSomething) {
        firstTimed(); // same behaviour as setDate
    }
}

void DayView::updateHiddenIndicator(int hidden)
{
    if (hidden) {
        mHiddenIndicator->setText(tr("(%1 more all day appointments)", "%1= number and always > 1").arg(hidden));
        mHiddenIndicator->show();

        // Also have to make sure if the previous selection is now hidden, that we select a different occurrence
        if (mAllDayList->currentIndex().isValid()
                && mAllDayList->currentIndex().row() >= mAllDayList->visibleRowCount()) {
            lastAllDay();
        }
    } else {
        mHiddenIndicator->hide();
    }
}



