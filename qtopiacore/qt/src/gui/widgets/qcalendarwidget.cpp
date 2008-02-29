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

#include "qcalendarwidget.h"

#ifndef QT_NO_CALENDARWIDGET

#include <qabstractitemmodel.h>
#include <qitemdelegate.h>
#include <qdatetime.h>
#include <qtableview.h>
#include <qlayout.h>
#include <qevent.h>
#include <qtextformat.h>
#include <qheaderview.h>
#include <private/qwidget_p.h>
#include <qpushbutton.h>
#include <qtoolbutton.h>
#include <qlabel.h>
#include <qspinbox.h>
#include <qmenu.h>

enum {
    RowCount = 6,
    ColumnCount = 7,
    HeaderColumn = 0,
    HeaderRow = 0,
    MinimumDayOffset = 1
};

class QCalendarView;

class QCalendarModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    QCalendarModel(QObject *parent = 0);

    int rowCount(const QModelIndex &) const
        { return RowCount + m_firstRow; }
    int columnCount(const QModelIndex &) const
        { return ColumnCount + m_firstColumn; }
    QVariant data(const QModelIndex &index, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;

    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex())
    {
        beginInsertRows(parent, row, row + count - 1);
        endInsertRows();
        return true;
    }
    bool insertColumns(int column, int count, const QModelIndex &parent = QModelIndex())
    {
        beginInsertColumns(parent, column, column + count - 1);
        endInsertColumns();
        return true;
    }
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex())
    {
        beginRemoveRows(parent, row, row + count - 1);
        endRemoveRows();
        return true;
    }
    bool removeColumns(int column, int count, const QModelIndex &parent = QModelIndex())
    {
        beginRemoveColumns(parent, column, column + count - 1);
        endRemoveColumns();
        return true;
    }

    void showMonth(int year, int month);
    void setDate(const QDate &d);

    void setMinimumDate(const QDate &date);
    void setMaximumDate(const QDate &date);

    void setRange(const QDate &min, const QDate &max);

    void setHorizontalHeaderFormat(QCalendarWidget::HorizontalHeaderFormat format);

    void setFirstColumnDay(Qt::DayOfWeek dayOfWeek);
    Qt::DayOfWeek firstColumnDay() const;

    bool weekNumbersShown() const;
    void setWeekNumbersShown(bool show);

    QTextCharFormat formatForCell(int row, int col) const;
    int dayOfWeekForColumn(int section) const;
    int columnForDayOfWeek(int day) const;
    QDate dateForCell(int row, int column) const;
    void cellForDate(const QDate &date, int *row, int *column) const;
    QString dayName(int day) const;

    void setView(QCalendarView *view)
        { m_view = view; }

    void internalUpdate();

    int m_firstColumn;
    int m_firstRow;
    QDate date;
    QDate minimumDate;
    QDate maximumDate;
    int shownYear;
    int shownMonth;
    Qt::DayOfWeek m_firstDay;
    QCalendarWidget::HorizontalHeaderFormat horizontalHeaderFormat;
    bool m_weekNumbersShown;
    QMap<Qt::DayOfWeek, QTextCharFormat> m_dayFormats;
    QMap<QDate, QTextCharFormat> m_dateFormats;
    QTextCharFormat m_headerFormat;
    QCalendarView *m_view;
};

class QCalendarView : public QTableView
{
    Q_OBJECT
public:
    QCalendarView(QWidget *parent = 0);

    void internalUpdate() { updateGeometries(); }
    void setReadOnly(bool enable);

signals:
    void changeDate(const QDate &date, bool changeMonth);
    void clicked(const QDate &date);
    void editingFinished();
protected:
    QModelIndex moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);
    bool event(QEvent *event);

    QDate handleMouseEvent(QMouseEvent *event);
public:
    bool readOnly;
private:
    bool validDateClicked;
};

QCalendarModel::QCalendarModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    date = QDate::currentDate();
    minimumDate = QDate::fromJulianDay(0);
    maximumDate = QDate(7999, 12, 31);
    shownYear = date.year();
    shownMonth = date.month();
    m_firstDay = Qt::Sunday;
    horizontalHeaderFormat = QCalendarWidget::ShortDayNames;
    m_weekNumbersShown = true;
    m_firstColumn = 1;
    m_firstRow = 1;
    m_view = 0;
}

int QCalendarModel::dayOfWeekForColumn(int column) const
{
    int col = column - m_firstColumn;
    if (col < 0 || col > 6)
        return 0;
    int day = m_firstDay + col;
    if (day > 7)
        day -= 7;
    return Qt::DayOfWeek(day);
}

int QCalendarModel::columnForDayOfWeek(int day) const
{
    if (day < 1 || day > 7)
        return -1;
    int column = (int)day - (int)m_firstDay;
    if (column < 0)
        column += 7;
    return column + m_firstColumn;
}

QDate QCalendarModel::dateForCell(int row, int column) const
{
    if (row < m_firstRow || row > m_firstRow + RowCount - 1 ||
                column < m_firstColumn || column > m_firstColumn + ColumnCount - 1)
        return QDate();
    QDate firstDate(shownYear, shownMonth, 15);
    if (!firstDate.isValid()) {
        return QDate();
    }
    int columnForFirstOfShownMonth = columnForDayOfWeek(firstDate.dayOfWeek());
    if (columnForFirstOfShownMonth - m_firstColumn < MinimumDayOffset)
        row -= 1;
    int daysInShownMonth = firstDate.daysInMonth();
    int requestedDay = 7 * (row - m_firstRow) + column - columnForFirstOfShownMonth + 1;
    if (requestedDay > daysInShownMonth) {
        QDate nextMonth = firstDate.addMonths(1);
        if (!nextMonth.isValid()) {
            return QDate();
        }
        return QDate(nextMonth.year(), nextMonth.month(),
                    requestedDay - daysInShownMonth);
    } else if (requestedDay <= 0) {
        QDate previousMonth = firstDate.addMonths(-1);
        if (!previousMonth.isValid()) {
            return QDate();
        }
        int daysInMonth = previousMonth.daysInMonth();
        return QDate(previousMonth.year(), previousMonth.month(),
                    daysInMonth + requestedDay);
    }
    return QDate(shownYear, shownMonth, requestedDay);
}

void QCalendarModel::cellForDate(const QDate &date, int *row, int *column) const
{
    int day = date.day();
    QDate firstDate(shownYear, shownMonth, 15);
    int columnForFirstOfShownMonth = columnForDayOfWeek(firstDate.dayOfWeek());
    int daysInShownMonth = firstDate.daysInMonth();

    QDate previousMonth = firstDate.addMonths(-1);
    QDate nextMonth = firstDate.addMonths(1);

    if (date.year() == shownYear && date.month() == shownMonth) {
        *column = (day + columnForFirstOfShownMonth - m_firstColumn - 1) % 7;
        *row = (day + columnForFirstOfShownMonth - m_firstColumn - 1) / 7;
    } else if (previousMonth.isValid() &&
            date.year() == previousMonth.year() && date.month() == previousMonth.month()) {
        int daysInMonth = previousMonth.daysInMonth();
        *column = (day - daysInMonth + columnForFirstOfShownMonth - m_firstColumn - 1) % 7;
        *row = (day - daysInMonth + columnForFirstOfShownMonth - m_firstColumn - 1) / 7;
        if (*column < 0) {
            *column += 7;
            *row -= 1;
        }
    } else if (nextMonth.isValid() &&
            date.year() == nextMonth.year() && date.month() == nextMonth.month()) {
        *column = (day + daysInShownMonth + columnForFirstOfShownMonth - m_firstColumn - 1) % 7;
        *row = (day + daysInShownMonth + columnForFirstOfShownMonth - m_firstColumn - 1) / 7;
    } else {
        *row = -1;
        *column = -1;
        return;
    }
    if (columnForFirstOfShownMonth - m_firstColumn < MinimumDayOffset)
        *row += 1;
    if (*row < 0 || *row > RowCount - 1 || *column < 0 || *column > ColumnCount - 1) {
        *row = -1;
        *column = -1;
        return;
    }
    *row += m_firstRow;
    *column += m_firstColumn;
}

QString QCalendarModel::dayName(int day) const
{
    switch (horizontalHeaderFormat) {
        case QCalendarWidget::SingleLetterDayNames:
            return QDate::shortDayName(day).left(1);
        case QCalendarWidget::ShortDayNames:
            return QDate::shortDayName(day);
        case QCalendarWidget::LongDayNames:
            return QDate::longDayName(day);
        default:
            break;
    }
    return QString();
}

QTextCharFormat QCalendarModel::formatForCell(int row, int col) const
{
    QPalette pal;
    QPalette::ColorGroup cg = QPalette::Active;
    if (m_view) {
        pal = m_view->palette();
        if (!m_view->isEnabled())
            cg = QPalette::Disabled;
        else if (!m_view->isActiveWindow())
            cg = QPalette::Inactive;
    }

    QTextCharFormat format;
    format.setFont(m_view->font());
    bool header = (m_weekNumbersShown && col == HeaderColumn)
                  || (horizontalHeaderFormat != QCalendarWidget::NoHorizontalHeader && row == HeaderRow);
    format.setBackground(pal.brush(cg, header ? QPalette::AlternateBase : QPalette::Base));
    if (header) {
        format.merge(m_headerFormat);
    }

    if (col >= m_firstColumn && col < m_firstColumn + ColumnCount) {
        Qt::DayOfWeek dayOfWeek = Qt::DayOfWeek(dayOfWeekForColumn(col));
        if (m_dayFormats.contains(dayOfWeek))
            format.merge(m_dayFormats.value(dayOfWeek));
    }

    if(!header) {
        QDate date = dateForCell(row, col);
        format.merge(m_dateFormats.value(date));
        if(date < minimumDate || date > maximumDate)
            format.setBackground(pal.brush(cg, QPalette::Window));
        if (shownMonth != date.month())
            format.setForeground(pal.brush(QPalette::Disabled, QPalette::Text));
    }
    return format;
}

QVariant QCalendarModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::TextAlignmentRole)
        return (int) Qt::AlignCenter;

    int row = index.row();
    int column = index.column();

    if(role == Qt::DisplayRole) {
        if (m_weekNumbersShown && column == HeaderColumn
            && row >= m_firstRow && row < m_firstRow + RowCount) {
            QDate date = dateForCell(row, columnForDayOfWeek(Qt::Monday));
            if (date.isValid())
                return date.weekNumber();
        }
        if (horizontalHeaderFormat != QCalendarWidget::NoHorizontalHeader && row == HeaderRow
            && column >= m_firstColumn && column < m_firstColumn + ColumnCount)
            return dayName(dayOfWeekForColumn(column));
        QDate date = dateForCell(row, column);
        if (date.isValid())
            return date.day();
        return QString();
    }

    QTextCharFormat fmt = formatForCell(row, column);
    if (role == Qt::BackgroundColorRole)
        return fmt.background().color();
    if (role == Qt::TextColorRole)
        return fmt.foreground().color();
    if (role == Qt::FontRole)
        return fmt.font();
    return QVariant();
}

Qt::ItemFlags QCalendarModel::flags(const QModelIndex &index) const
{
    QDate date = dateForCell(index.row(), index.column());
    if (!date.isValid())
        return QAbstractTableModel::flags(index);
    if (date < minimumDate)
        return 0;
    if (date > maximumDate)
        return 0;
    return QAbstractTableModel::flags(index);
}

void QCalendarModel::setDate(const QDate &d)
{
    date = d;
    if (date < minimumDate)
        date = minimumDate;
    else if (date > maximumDate)
        date = maximumDate;
}

void QCalendarModel::showMonth(int year, int month)
{
    if (shownYear == year && shownMonth == month)
        return;

    shownYear = year;
    shownMonth = month;

    internalUpdate();
}

void QCalendarModel::setMinimumDate(const QDate &d)
{
    if (!d.isValid() || d == minimumDate)
        return;

    minimumDate = d;
    if (maximumDate < minimumDate)
        maximumDate = minimumDate;
    if (date < minimumDate)
        date = minimumDate;
    internalUpdate();
}

void QCalendarModel::setMaximumDate(const QDate &d)
{
    if (!d.isValid() || d == maximumDate)
        return;

    maximumDate = d;
    if (minimumDate > maximumDate)
        minimumDate = maximumDate;
    if (date > maximumDate)
        date = maximumDate;
    internalUpdate();
}

void QCalendarModel::setRange(const QDate &min, const QDate &max)
{
    minimumDate = min;
    maximumDate = max;
    if(minimumDate > maximumDate)
        qSwap(minimumDate, maximumDate);
    if (date < minimumDate)
        date = minimumDate;
    if (date > maximumDate)
        date = maximumDate;
    internalUpdate();
}

void QCalendarModel::internalUpdate()
{
    QModelIndex begin = index(0, 0);
    QModelIndex end = index(m_firstRow + RowCount - 1, m_firstColumn + ColumnCount - 1);
    emit dataChanged(begin, end);
    emit headerDataChanged(Qt::Vertical, 0, m_firstRow + RowCount - 1);
    emit headerDataChanged(Qt::Horizontal, 0, m_firstColumn + ColumnCount - 1);
}

void QCalendarModel::setHorizontalHeaderFormat(QCalendarWidget::HorizontalHeaderFormat format)
{
    if (horizontalHeaderFormat == format)
        return;

    int oldFormat = horizontalHeaderFormat;
    horizontalHeaderFormat = format;
    if (oldFormat == QCalendarWidget::NoHorizontalHeader) {
        m_firstRow = 1;
        insertRow(0);
    } else if (horizontalHeaderFormat == QCalendarWidget::NoHorizontalHeader) {
        m_firstRow = 0;
        removeRow(0);
    }
    internalUpdate();
}

void QCalendarModel::setFirstColumnDay(Qt::DayOfWeek dayOfWeek)
{
    if (m_firstDay == dayOfWeek)
        return;

    m_firstDay = dayOfWeek;
    internalUpdate();
}

Qt::DayOfWeek QCalendarModel::firstColumnDay() const
{
    return m_firstDay;
}

bool QCalendarModel::weekNumbersShown() const
{
    return m_weekNumbersShown;
}

void QCalendarModel::setWeekNumbersShown(bool show)
{
    if (m_weekNumbersShown == show)
        return;

    m_weekNumbersShown = show;
    if (show) {
        m_firstColumn = 1;
        insertColumn(0);
    } else {
        m_firstColumn = 0;
        removeColumn(0);
    }
    internalUpdate();
}

QCalendarView::QCalendarView(QWidget *parent)
    : QTableView(parent),
    readOnly(false),
    validDateClicked(false)
{
    setTabKeyNavigation(false);
    setShowGrid(false);
    verticalHeader()->setVisible(false);
    horizontalHeader()->setVisible(false);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

QModelIndex QCalendarView::moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers)
{
    QCalendarModel *calendarModel = ::qobject_cast<QCalendarModel *>(model());
    if (!calendarModel)
        return QTableView::moveCursor(cursorAction, modifiers);

    if (readOnly)
        return currentIndex();

    QModelIndex index = currentIndex();
    QDate currentDate = static_cast<QCalendarModel*>(model())->dateForCell(index.row(), index.column());
    switch (cursorAction) {
        case QAbstractItemView::MoveUp:
            currentDate = currentDate.addDays(-7);
            break;
        case QAbstractItemView::MoveDown:
            currentDate = currentDate.addDays(7);
            break;
        case QAbstractItemView::MoveLeft:
            currentDate = currentDate.addDays(-1);
            break;
        case QAbstractItemView::MoveRight:
            currentDate = currentDate.addDays(1);
            break;
        case QAbstractItemView::MoveHome:
            currentDate = QDate(currentDate.year(), currentDate.month(), 1);
            break;
        case QAbstractItemView::MoveEnd:
            currentDate = QDate(currentDate.year(), currentDate.month(), currentDate.daysInMonth());
            break;
        case QAbstractItemView::MovePageUp:
            currentDate = currentDate.addMonths(-1);
            break;
        case QAbstractItemView::MovePageDown:
            currentDate = currentDate.addMonths(1);
            break;
        case QAbstractItemView::MoveNext:
        case QAbstractItemView::MovePrevious:
            return currentIndex();
        default:
            break;
    }
    emit changeDate(currentDate, true);
    return currentIndex();
}

void QCalendarView::keyPressEvent(QKeyEvent *event)
{
    if (!readOnly) {
        switch (event->key()) {
            case Qt::Key_Return:
            case Qt::Key_Enter:
            case Qt::Key_Space:
            case Qt::Key_Select:
                emit editingFinished();
                return;
            default:
                break;
        }
    }
    QTableView::keyPressEvent(event);
}

bool QCalendarView::event(QEvent *event)
{
    return QTableView::event(event);
}

QDate QCalendarView::handleMouseEvent(QMouseEvent *event)
{
    QCalendarModel *calendarModel = ::qobject_cast<QCalendarModel *>(model());
    if (!calendarModel)
        return QDate();

    QPoint pos = event->pos();
    QModelIndex index = indexAt(pos);
    QDate date = calendarModel->dateForCell(index.row(), index.column());
    if (date.isValid() && date >= calendarModel->minimumDate
            && date <= calendarModel->maximumDate) {
        return date;
    }
    return QDate();
}

void QCalendarView::mouseDoubleClickEvent(QMouseEvent *event)
{
    QCalendarModel *calendarModel = ::qobject_cast<QCalendarModel *>(model());
    if (!calendarModel) {
        QTableView::mouseDoubleClickEvent(event);
        return;
    }

    if (readOnly)
        return;

    QDate date = handleMouseEvent(event);
    validDateClicked = false;
    if (date == calendarModel->date && !style()->styleHint(QStyle::SH_ItemView_ActivateItemOnSingleClick)) {
        emit editingFinished();
    }
}

void QCalendarView::mousePressEvent(QMouseEvent *event)
{
    QCalendarModel *calendarModel = ::qobject_cast<QCalendarModel *>(model());
    if (!calendarModel) {
        QTableView::mousePressEvent(event);
        return;
    }

    if (readOnly)
        return;

    if (event->button() != Qt::LeftButton)
        return;

    QDate date = handleMouseEvent(event);
    if (date.isValid()) {
        validDateClicked = true;
        int row = -1, col = -1;
        static_cast<QCalendarModel *>(model())->cellForDate(date, &row, &col);
        if (row != -1 && col != -1) {
            selectionModel()->setCurrentIndex(model()->index(row, col), QItemSelectionModel::NoUpdate);
        }
    } else {
        validDateClicked = false;
        event->ignore();
    }
}

void QCalendarView::mouseMoveEvent(QMouseEvent *event)
{
    QCalendarModel *calendarModel = ::qobject_cast<QCalendarModel *>(model());
    if (!calendarModel) {
        QTableView::mouseMoveEvent(event);
        return;
    }

    if (readOnly)
        return;

    if (validDateClicked) {
       QDate date = handleMouseEvent(event);
        if (date.isValid()) {
            int row = -1, col = -1;
            static_cast<QCalendarModel *>(model())->cellForDate(date, &row, &col);
            if (row != -1 && col != -1) {
                selectionModel()->setCurrentIndex(model()->index(row, col), QItemSelectionModel::NoUpdate);
            }
        }
    } else {
        event->ignore();
    }
}

void QCalendarView::mouseReleaseEvent(QMouseEvent *event)
{
    QCalendarModel *calendarModel = ::qobject_cast<QCalendarModel *>(model());
    if (!calendarModel) {
        QTableView::mouseReleaseEvent(event);
        return;
    }

    if (event->button() != Qt::LeftButton)
        return;

    if (readOnly)
        return;

    if (validDateClicked) {
        QDate date = handleMouseEvent(event);
        if (date.isValid()) {
            emit changeDate(date, true);
            emit clicked(date);
            if (style()->styleHint(QStyle::SH_ItemView_ActivateItemOnSingleClick))
                emit editingFinished();
        }
        validDateClicked = false;
    } else {
        event->ignore();
    }
}

class QCalendarDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    QCalendarDelegate(QCalendarWidgetPrivate *w, QObject *parent = 0)
        : QItemDelegate(parent), calendarWidgetPrivate(w)
            { }
    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option,
                const QModelIndex &index) const;
    void paintCell(QPainter *painter, const QRect &rect, const QDate &date) const;
private:
    QCalendarWidgetPrivate *calendarWidgetPrivate;
    mutable QStyleOptionViewItem storedOption;
};

//Private tool button class
class QCalToolButton: public QToolButton
{
public:
    QCalToolButton(QWidget * parent)
        :QToolButton(parent), hover(false) ,oldState(false)
         {  }
private:
    bool hover, oldState;
protected:
    void enterEvent(QEvent * e)
    {
        hover = true;
        QToolButton::enterEvent(e);
    }

    void leaveEvent(QEvent * e)
    {
        hover = false;
        QToolButton::leaveEvent(e);
    }

    void mousePressEvent(QMouseEvent * e)
    {
        hover = true;
        QToolButton::mousePressEvent(e);
    }

    void mouseReleaseEvent(QMouseEvent * e)
    {
        hover = true;
        QToolButton::mouseReleaseEvent(e);
    }

    void paintEvent(QPaintEvent *e)
    {
        QPalette toolPalette = parentWidget()->palette();
        bool newState = (menu()) ? menu()->isVisible():isDown();

#ifndef Q_WS_MAC
        if (newState || hover) //act as normal button
            setPalette(toolPalette);
        else {
            //set the highlight color for button text
            toolPalette.setColor(QPalette::ButtonText, toolPalette.color(QPalette::HighlightedText));
            setPalette(toolPalette);
        }
#endif
        if (newState != oldState) {
            //update the hover if the button is released
            hover = oldState ? false : hover;
            oldState = newState;
        }
        QToolButton::paintEvent(e);
    }
};

class QCalendarWidgetPrivate : public QWidgetPrivate
{
    Q_DECLARE_PUBLIC(QCalendarWidget)
public:
    QCalendarWidgetPrivate();

    void showMonth(int year, int month);
    void update();
    void paintCell(QPainter *painter, const QRect &rect, const QDate &date) const;

    void _q_slotChangeDate(const QDate &date, bool changeMonth);
    void _q_editingFinished();
    void _q_monthChanged(QAction*);
    void _q_prevMonthClicked();
    void _q_nextMonthClicked();
    void _q_yearEditingFinished();
    void _q_yearClicked();

    void createHeader(QWidget *widget);
    void updateMonthMenu();
    void updateHeader();
    void updateCurrentPage(QDate &newDate);
    inline QDate getCurrentDate();

    QCalendarModel *m_model;
    QCalendarView *m_view;
    QCalendarDelegate *m_delegate;
    QItemSelectionModel *m_selection;

    QToolButton *nextMonth;
    QToolButton *prevMonth;
    QCalToolButton *monthButton;
    QMenu *monthMenu;
    QMap<int, QAction *> monthToAction;
    QCalToolButton *yearButton;
    QSpinBox *yearEdit;
    QWidget *headerBackground;
    QSpacerItem *spaceHolder;

    bool headerVisible;
};

void QCalendarDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
            const QModelIndex &index) const
{
    QDate date = calendarWidgetPrivate->m_model->dateForCell(index.row(), index.column());
    if (date.isValid()) {
        storedOption = option;
        QRect rect = option.rect;
        calendarWidgetPrivate->paintCell(painter, rect, date);
    } else {
        QItemDelegate::paint(painter, option, index);
    }
}

void QCalendarDelegate::paintCell(QPainter *painter, const QRect &rect, const QDate &date) const
{
    storedOption.rect = rect;
    int row = -1;
    int col = -1;
    calendarWidgetPrivate->m_model->cellForDate(date, &row, &col);
    QModelIndex idx = calendarWidgetPrivate->m_model->index(row, col);
    QItemDelegate::paint(painter, storedOption, idx);
}

QCalendarWidgetPrivate::QCalendarWidgetPrivate()
    : QWidgetPrivate()
{
    m_model = 0;
    m_view = 0;
    m_delegate = 0;
    m_selection = 0;
    headerVisible = true;
}

void QCalendarWidgetPrivate::createHeader(QWidget *widget)
{
    Q_Q(QCalendarWidget);
    headerBackground = new QWidget(widget);
    headerBackground->setAutoFillBackground(true);
    headerBackground->setBackgroundRole(QPalette::Highlight);

    prevMonth = new QToolButton(headerBackground);
    nextMonth = new QToolButton(headerBackground);
    prevMonth->setAutoRaise(true);
    nextMonth->setAutoRaise(true);
    prevMonth->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
    nextMonth->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
    nextMonth->setAutoRaise(true);
    prevMonth->setIcon(q->style()->standardPixmap(QStyle::SP_ArrowLeft));
    nextMonth->setIcon(q->style()->standardPixmap(QStyle::SP_ArrowRight));
    prevMonth->setAutoRepeat(true);
    nextMonth->setAutoRepeat(true);
    prevMonth->setFocusProxy(m_view);
    nextMonth->setFocusProxy(m_view);

    monthButton = new QCalToolButton(headerBackground);
    monthButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
    monthButton->setAutoRaise(true);
    monthButton->setPopupMode(QToolButton::InstantPopup);
    monthMenu = new QMenu(monthButton);
    for (int i = 1; i <= 12; i++) {
        QString monthName(QDate::longMonthName(i));
        QAction *act = monthMenu->addAction(monthName);
        act->setData(i);
        monthToAction[i] = act;
    }
    monthButton->setMenu(monthMenu);
    yearButton = new QCalToolButton(headerBackground);
    yearButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
    yearButton->setAutoRaise(true);
    yearEdit = new QSpinBox(headerBackground);

    QFont font = q->font();
    font.setBold(true);
    monthButton->setFont(font);
    yearButton->setFont(font);
    yearEdit->setFrame(false);
    yearEdit->setMinimum(m_model->minimumDate.year());
    yearEdit->setMaximum(m_model->maximumDate.year());
    yearEdit->hide();
    spaceHolder = new QSpacerItem(0,0);

    QHBoxLayout *headerLayout = new QHBoxLayout;
    headerLayout->setMargin(0);
    headerLayout->setSpacing(0);
    headerLayout->addWidget(prevMonth);
    headerLayout->insertStretch(headerLayout->count());
    headerLayout->addWidget(monthButton);
    headerLayout->addItem(spaceHolder);
    headerLayout->addWidget(yearButton);
    headerLayout->insertStretch(headerLayout->count());
    headerLayout->addWidget(nextMonth);
    headerBackground->setLayout(headerLayout);

    yearEdit->setFocusPolicy(Qt::StrongFocus);
    prevMonth->setFocusPolicy(Qt::StrongFocus);
    nextMonth->setFocusPolicy(Qt::StrongFocus);

    //set names for the header controls.
    prevMonth->setObjectName("qt_calendar_prevmonth");
    nextMonth->setObjectName("qt_calendar_nextmonth");
    monthButton->setObjectName("qt_calendar_monthbutton");
    yearButton->setObjectName("qt_calendar_yearbutton");
    yearEdit->setObjectName("qt_calendar_yearedit");

    updateMonthMenu();
    showMonth(m_model->date.year(),m_model->date.month());
}

void QCalendarWidgetPrivate::updateMonthMenu()
{
    int beg = 1, end = 12;
    bool prevEnabled = true;
    bool nextEnabled = true;
    if (m_model->shownYear == m_model->minimumDate.year()) {
        beg = m_model->minimumDate.month();
        if (m_model->shownMonth == m_model->minimumDate.month())
            prevEnabled = false;
    }
    if (m_model->shownYear == m_model->maximumDate.year()) {
        end = m_model->maximumDate.month();
        if (m_model->shownMonth == m_model->maximumDate.month())
            nextEnabled = false;
    }
    prevMonth->setEnabled(prevEnabled);
    nextMonth->setEnabled(nextEnabled);
    for (int i = 1; i <= 12; i++) {
        bool monthEnabled = true;
        if (i < beg || i > end)
            monthEnabled = false;
        monthToAction[i]->setEnabled(monthEnabled);
    }
}

void QCalendarWidgetPrivate::updateCurrentPage(QDate &newDate)
{
    Q_Q(QCalendarWidget);

    QDate minDate = q->minimumDate();
    QDate maxDate = q->maximumDate();
    if (minDate.isValid()&& minDate.daysTo(newDate) < 0)
        newDate = minDate;
    if (maxDate.isValid()&& maxDate.daysTo(newDate) > 0)
        newDate = maxDate;
    showMonth(newDate.year(), newDate.month());
    int row = -1, col = -1;
    m_model->cellForDate(newDate, &row, &col);
    if (row != -1 && col != -1)
    {
        m_view->selectionModel()->setCurrentIndex(m_model->index(row, col),
                                                  QItemSelectionModel::NoUpdate);
    }
}

void QCalendarWidgetPrivate::_q_monthChanged(QAction *act)
{
    monthButton->setText(act->text());
    QDate currentDate = getCurrentDate();
    QDate newDate = currentDate.addMonths(act->data().toInt()-currentDate.month());
    updateCurrentPage(newDate);
}

QDate QCalendarWidgetPrivate::getCurrentDate()
{
    QModelIndex index = m_view->currentIndex();
    return m_model->dateForCell(index.row(), index.column());
}

void QCalendarWidgetPrivate::_q_prevMonthClicked()
{
    QDate currentDate = getCurrentDate().addMonths(-1);
    updateCurrentPage(currentDate);
}

void QCalendarWidgetPrivate::_q_nextMonthClicked()
{
    QDate currentDate = getCurrentDate().addMonths(1);
    updateCurrentPage(currentDate);
}

void QCalendarWidgetPrivate::_q_yearEditingFinished()
{
    yearButton->setText(yearEdit->text());
    yearEdit->hide();
    spaceHolder->changeSize(0, 0);
    yearButton->show();
    QDate currentDate(yearEdit->text().toInt(), getCurrentDate().month(), getCurrentDate().day());
    updateCurrentPage(currentDate);
}

void QCalendarWidgetPrivate::_q_yearClicked()
{
    //show the spinbox on top of the button
    yearEdit->setGeometry(yearButton->x(), yearButton->y(),
                          yearEdit->sizeHint().width(), yearButton->height());
    spaceHolder->changeSize(yearButton->width(), 0);
    yearButton->hide();
    yearEdit->show();
    yearEdit->raise();
    yearEdit->selectAll();
    yearEdit->setFocus(Qt::MouseFocusReason);
}

void QCalendarWidgetPrivate::showMonth(int year, int month)
{
    if (m_model->shownYear == year && m_model->shownMonth == month)
        return;
    Q_Q(QCalendarWidget);
    m_model->showMonth(year, month);
    updateHeader();
    emit q->currentPageChanged(year, month);
    m_view->internalUpdate();
    update();
    updateMonthMenu();
}

void QCalendarWidgetPrivate::updateHeader()
{
    monthButton->setText(QDate::longMonthName(m_model->shownMonth));
    yearButton->setText(QString::number(m_model->shownYear));
    yearEdit->setValue(m_model->shownYear);

    QFontMetrics fm = monthButton->fontMetrics();
    monthButton->setMaximumWidth(fm.boundingRect(QDate::longMonthName(m_model->shownMonth)).width() +
        fm.boundingRect(QChar('y')).width());

    fm = yearButton->fontMetrics();
    yearButton->setMaximumWidth(fm.boundingRect(QString("55555")).width());
}

void QCalendarWidgetPrivate::update()
{
    QDate currentDate = m_model->date;
    int row, column;
    m_model->cellForDate(currentDate, &row, &column);
    QModelIndex idx;
    m_selection->clear();
    if (row != -1 && column != -1) {
        idx = m_model->index(row, column);
        m_selection->setCurrentIndex(idx, QItemSelectionModel::SelectCurrent);
    }
}

void QCalendarWidgetPrivate::paintCell(QPainter *painter, const QRect &rect, const QDate &date) const
{
    Q_Q(const QCalendarWidget);
    q->paintCell(painter, rect, date);
}

void QCalendarWidgetPrivate::_q_slotChangeDate(const QDate &date, bool changeMonth)
{
    QDate oldDate = m_model->date;
    m_model->setDate(date);
    QDate newDate = m_model->date;
    if (changeMonth)
        showMonth(newDate.year(), newDate.month());
    if (oldDate != newDate) {
        update();
        Q_Q(QCalendarWidget);
        emit q->selectionChanged();
    }
}

void QCalendarWidgetPrivate::_q_editingFinished()
{
    Q_Q(QCalendarWidget);
    emit q->activated(m_model->date);
}

/*!
    \class QCalendarWidget
    \brief The QCalendarWidget class provides a monthly based
    calendar widget allowing the user to select a date.
    \since 4.2

    \image cleanlooks-calendarwidget.png

    The widget is initialized with the current month and year, but
    QCalendarWidget provides several public slots to change the year
    and month that is shown.  The currently displayed month and year
    can be retrieved using the currentPageMonth() and currentPageYear()
    functions, respectively.

    By default, today's date is selected, and the user can select a
    date using both mouse and keyboard. The currently selected date
    can be retrieved using the selectedDate() function. It is
    possible to constrain the user selection to a given date range by
    setting the minimumDate and maximumDate properties.
    Alternatively, both properties can be set in one go using the
    setDateRange() convenience slot. Set the \l selectionMode
    property to NoSelection to prohibit the user from selecting at
    all. Note that a date also can be selected programmatically using
    the setSelectedDate() slot.

    A newly created calendar widget uses abbreviated day names, and
    both Saturdays and Sundays are marked in red. The calendar grid is
    not visible. The week numbers are displayed, and the first column
    day is Sunday.

    The notation of the days can be altered to a single letter
    abbreviations ("M" for "Monday") by setting the
    horizontalHeaderFormat property to
    QCalendarWidget::SingleLetterDayNames. Setting the same property
    to QCalendarWidget::LongDayNames makes the header display the
    complete day names. The week numbers can be removed by setting
    the verticalHeaderFormat property to
    QCalendarWidget::NoVerticalHeader.  The calendar grid can be
    turned on by setting the gridVisible property to true using the
    setGridVisible() function:

    \table
    \row \o
        \image qcalendarwidget-grid.png
    \row \o
        \code
            QCalendarWidget *calendar;
            calendar->setGridVisible(true);
        \endcode
    \endtable

    Finally, the day in the first column can be altered using the
    setFirstDayOfWeek() function.

    The QCalendarWidget class also provides three signals,
    selectionChanged(), activated() and currentPageChanged() making it
    possible to respond to user interaction.

    The rendering of the headers, weekdays or single days can be
    largely customized by setting QTextCharFormat's for some special
    weekday, a special date or for the rendering of the headers.

    Only a subset of the properties in QTextCharFormat are used by the
    calendar widget. Currently, the foreground, background and font
    properties are used to determine the rendering of individual cells
    in the widget.

    \sa QDate, QDateEdit, QTextCharFormat
*/

/*!
    \enum QCalendarWidget::SelectionMode

    This enum describes the types of selection offered to the user for
    selecting dates in the calendar.

    \value NoSelection      Dates cannot be selected.
    \value SingleSelection  Single dates can be selected.

    \sa selectionMode
*/

/*!
    Constructs a calendar widget with the given \a parent.

    The widget is initialized with the current month and year, and the
    currently selected date is today.

    \sa setCurrentPage()
*/
QCalendarWidget::QCalendarWidget(QWidget *parent)
    : QWidget(*new QCalendarWidgetPrivate, parent, 0)
{
    Q_D(QCalendarWidget);

    setAutoFillBackground(true);
    setBackgroundRole(QPalette::Window);

    QVBoxLayout *layoutV = new QVBoxLayout(this);
    layoutV->setMargin(0);
    d->m_model = new QCalendarModel(this);
    QTextCharFormat fmt;
    fmt.setForeground(QBrush(Qt::red));
    d->m_model->m_dayFormats.insert(Qt::Saturday, fmt);
    d->m_model->m_dayFormats.insert(Qt::Sunday, fmt);
    d->m_view = new QCalendarView(this);
    d->m_view->setObjectName("qt_calendar_calendarview");
    d->m_view->setModel(d->m_model);
    d->m_model->setView(d->m_view);
    d->m_view->setSelectionBehavior(QAbstractItemView::SelectItems);
    d->m_view->setSelectionMode(QAbstractItemView::SingleSelection);
    d->m_view->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
    d->m_view->horizontalHeader()->setClickable(false);
    d->m_view->verticalHeader()->setResizeMode(QHeaderView::Stretch);
    d->m_view->verticalHeader()->setClickable(false);
    d->m_selection = d->m_view->selectionModel();
    d->createHeader(this);
    d->m_view->setFrameStyle(QFrame::NoFrame);
    d->m_delegate = new QCalendarDelegate(d, this);
    d->m_view->setItemDelegate(d->m_delegate);
    d->update();
    d->updateHeader();
    setFocusPolicy(Qt::StrongFocus);
    setFocusProxy(d->m_view);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    connect(d->m_view, SIGNAL(changeDate(QDate,bool)),
            this, SLOT(_q_slotChangeDate(QDate,bool)));
    connect(d->m_view, SIGNAL(clicked(QDate)),
            this, SIGNAL(clicked(QDate)));
    connect(d->m_view, SIGNAL(editingFinished()),
            this, SLOT(_q_editingFinished()));

    connect(d->prevMonth, SIGNAL(clicked(bool)),
            this, SLOT(_q_prevMonthClicked()));
    connect(d->nextMonth, SIGNAL(clicked(bool)),
            this, SLOT(_q_nextMonthClicked()));
    connect(d->yearButton, SIGNAL(clicked(bool)),
            this, SLOT(_q_yearClicked()));
    connect(d->monthMenu, SIGNAL(triggered(QAction*)),
            this, SLOT(_q_monthChanged(QAction*)));
    connect(d->yearEdit, SIGNAL(editingFinished()),
            this, SLOT(_q_yearEditingFinished()));

    layoutV->setMargin(0);
    layoutV->setSpacing(0);
    layoutV->addWidget(d->headerBackground);
    layoutV->addWidget(d->m_view);
}

/*!
   Destroys the calendar widget.
*/
QCalendarWidget::~QCalendarWidget()
{
}

/*!
   \reimp
*/
QSize QCalendarWidget::sizeHint() const
{
    return minimumSizeHint();
}

/*!
   \reimp
*/
QSize QCalendarWidget::minimumSizeHint() const
{
    Q_D(const QCalendarWidget);

    ensurePolished();

    int w = 0;
    int h = 0;

    int end = 53;
    int rows = 7;
    int cols = 8;
    int startRow = 0;
    int startCol = 0;

    const int marginH = (style()->pixelMetric(QStyle::PM_FocusFrameHMargin) + 1) * 2;

    if (horizontalHeaderFormat() == QCalendarWidget::NoHorizontalHeader) {
        rows = 6;
        startRow = 1;
    } else {
        for (int i = 1; i <= 7; i++) {
            QFontMetrics fm(d->m_model->formatForCell(0, i).font());
            w = qMax(w, fm.width(d->m_model->dayName(i)) + marginH);
            h = qMax(h, fm.height());
        }
    }

    if (verticalHeaderFormat() == QCalendarWidget::NoVerticalHeader) {
        cols = 7;
        startCol = 1;
    }

    QFontMetrics fm(d->m_model->formatForCell(1, 1).font());
    for (int i = 1; i <= end; i++) {
        w = qMax(w, fm.width(QString::number(i)) + marginH);
        h = qMax(h, fm.height());
    }

    if (d->m_view->showGrid()) {
        // hardcoded in tableview
        w += 1;
        h += 1;
    }

    w += 1; // default column span

    h = qMax(h, d->m_view->verticalHeader()->minimumSectionSize());
    w = qMax(w, d->m_view->horizontalHeader()->minimumSectionSize());

    //add the size of the header.
    QSize headerSize(0, 0);
    if (d->headerVisible) {
        int headerH = d->headerBackground->sizeHint().height();
        int headerW = 0;

        headerW += d->prevMonth->sizeHint().width();
        headerW += d->nextMonth->sizeHint().width();

        QFontMetrics fm = d->monthButton->fontMetrics();
        int monthW = 0;
        for (int i = 1; i < 12; i++)
            monthW = qMax(monthW, fm.boundingRect(QDate::longMonthName(i)).width());
        monthW += fm.boundingRect(QChar('y')).width();
        headerW += monthW;

        fm = d->yearButton->fontMetrics();
        headerW += fm.boundingRect(QString("55555")).width();

        headerSize = QSize(headerW, headerH);
    }
    w *= cols;
    w = qMax(headerSize.width(), w);
    h = (h * rows) + headerSize.height();
    return QSize(w , h);
}

/*!
    Paints the cell specified by the given \a date, using the given \a painter and \a rect.
*/

void QCalendarWidget::paintCell(QPainter *painter, const QRect &rect, const QDate &date) const
{
    Q_D(const QCalendarWidget);
    d->m_delegate->paintCell(painter, rect, date);
}

/*!
    \property QCalendarWidget::selectedDate
    \brief the currently selected date.

    The selected date must be within the date range specified by the
    minimumDate and maximumDate properties. By default, the selected
    date is the current date.

    \sa setDateRange()
*/

QDate QCalendarWidget::selectedDate() const
{
    Q_D(const QCalendarWidget);
    return d->m_model->date;
}

void QCalendarWidget::setSelectedDate(const QDate &date)
{
    Q_D(QCalendarWidget);
    if (d->m_model->date == date && date == d->getCurrentDate())
        return;

    if (!date.isValid())
        return;

    d->m_model->setDate(date);
    d->update();
    QDate newDate = d->m_model->date;
    d->showMonth(newDate.year(), newDate.month());
}

/*!
    Returns the year of the currently displayed month. Months are 
    numbered from 1 to 12.

    \sa monthShown(), setCurrentPage()
*/

int QCalendarWidget::yearShown() const
{
    Q_D(const QCalendarWidget);
    return d->m_model->shownYear;
}

/*!
    Returns the currently displayed month. Months are numbered from 1 to 
    12.

    \sa yearShown(), setCurrentPage()
*/

int QCalendarWidget::monthShown() const
{
    Q_D(const QCalendarWidget);
    return d->m_model->shownMonth;
}

/*!
    Displays the given \a month of the given \a year without changing
    the selected date. Use the setSelectedDate() function to alter the
    selected date.

    The currently displayed month and year can be retrieved using the
    currentPageMonth() and currentPageYear() functions respectively.

    \sa yearShown(), monthShown(), showPreviousMonth(), showNextMonth(),
    showPreviousYear(), showNextYear()
*/

void QCalendarWidget::setCurrentPage(int year, int month)
{
    Q_D(QCalendarWidget);
    d->showMonth(year, month);
}

/*!
    Shows the next month relative to the currently displayed
    month. Note that the selected date is not changed.

    \sa showPreviousMonth(), setCurrentPage(), setSelectedDate()
*/

void QCalendarWidget::showNextMonth()
{
    int year = yearShown();
    int month = monthShown();
    if (month == 12) {
        ++year;
        month = 1;
    } else {
        ++month;
    }
    setCurrentPage(year, month);
}

/*!
    Shows the previous month relative to the currently displayed
    month. Note that the selected date is not changed.

    \sa showNextMonth(), setCurrentPage(), setSelectedDate()
*/

void QCalendarWidget::showPreviousMonth()
{
    int year = yearShown();
    int month = monthShown();
    if (month == 1) {
        --year;
        month = 12;
    } else {
        --month;
    }
    setCurrentPage(year, month);
}

/*!
    Shows the currently displayed month in the \e next year relative
    to the currently displayed year. Note that the selected date is
    not changed.

    \sa showPreviousYear(), setCurrentPage(), setSelectedDate()
*/

void QCalendarWidget::showNextYear()
{
    int year = yearShown();
    int month = monthShown();
    ++year;
    setCurrentPage(year, month);
}

/*!
    Shows the currently displayed month in the \e previous year
    relative to the currently displayed year. Note that the selected
    date is not changed.

    \sa showNextYear(), setCurrentPage(), setSelectedDate()
*/

void QCalendarWidget::showPreviousYear()
{
    int year = yearShown();
    int month = monthShown();
    --year;
    setCurrentPage(year, month);
}

/*!
    Shows the month of the selected date.

    \sa selectedDate(), setCurrentPage()
*/
void QCalendarWidget::showSelectedDate()
{
    QDate currentDate = selectedDate();
    setCurrentPage(currentDate.year(), currentDate.month());
}

/*!
    Shows the month of the today's date.

    \sa selectedDate(), setCurrentPage()
*/
void QCalendarWidget::showToday()
{
    QDate currentDate = QDate::currentDate();
    setCurrentPage(currentDate.year(), currentDate.month());
}

/*!
    \property QCalendarWidget::minimumDate
    \brief the minimum date of the currently specified date range.

    The user will not be able to select a date that is before the
    currently set minimum date.

    \table
    \row
    \o \image qcalendarwidget-minimum.png
    \row
    \o
    \code
    QCalendarWidget *calendar;
    calendar->setGridVisible(true);
    calendar->setMinimumDate(QDate(2006, 6, 19));
    \endcode
    \endtable

    By default, the minimum date is the earliest date that the QDate
    class can handle.

    When setting a minimum date, the maximumDate and selectedDate
    properties are adjusted if the selection range becomes invalid. If
    the provided date is not a valid QDate object, the
    setMinimumDate() function does nothing.

    \sa setDateRange()
*/

QDate QCalendarWidget::minimumDate() const
{
    Q_D(const QCalendarWidget);
    return d->m_model->minimumDate;
}

void QCalendarWidget::setMinimumDate(const QDate &date)
{
    Q_D(QCalendarWidget);
    if (!date.isValid() || d->m_model->minimumDate == date)
        return;

    QDate oldDate = d->m_model->date;
    d->m_model->setMinimumDate(date);
    d->yearEdit->setMinimum(d->m_model->minimumDate.year());
    d->updateMonthMenu();
    QDate newDate = d->m_model->date;
    if (oldDate != newDate) {
        d->update();
        d->showMonth(newDate.year(), newDate.month());
        emit selectionChanged();
    }
}

/*!
    \property QCalendarWidget::maximumDate
    \brief the maximum date of the currently specified date range.

    The user will not be able to select a date which is after the
    currently set maximum date.

    \table
    \row
    \o \image qcalendarwidget-maximum.png
    \row
    \o
    \code
    QCalendarWidget *calendar;
    calendar->setGridVisible(true);
    calendar->setMaximumDate(QDate(2006, 7, 3));
    \endcode
    \endtable

    By default, the maximum date is the last day the QDate class can
    handle.

    When setting a maximum date, the minimumDate and selectedDate
    properties are adjusted if the selection range becomes invalid. If
    the provided date is not a valid QDate object, the
    setMaximumDate() function does nothing.

    \sa setDateRange()
*/

QDate QCalendarWidget::maximumDate() const
{
    Q_D(const QCalendarWidget);
    return d->m_model->maximumDate;
}

void QCalendarWidget::setMaximumDate(const QDate &date)
{
    Q_D(QCalendarWidget);
    if (!date.isValid() || d->m_model->maximumDate == date)
        return;

    QDate oldDate = d->m_model->date;
    d->m_model->setMaximumDate(date);
    d->yearEdit->setMaximum(d->m_model->maximumDate.year());
    d->updateMonthMenu();
    QDate newDate = d->m_model->date;
    if (oldDate != newDate) {
        d->update();
        d->showMonth(newDate.year(), newDate.month());
        emit selectionChanged();
    }
}

/*!
    Defines a date range by setting the minimumDate and maximumDate
    properties.

    The date range restricts the user selection, i.e. the user can
    only select dates within the specified date range. Note that

    \code
        QCalendarWidget *calendar;

        calendar->setDateRange(min, max);
    \endcode

    is analogous to

    \code
        QCalendarWidget *calendar;

        calendar->setMinimumDate(min);
        calendar->setMaximumDate(max);
    \endcode

    If either the \a min or \a max parameters are not valid QDate
    objects, this function does nothing.

    \sa setMinimumDate(), setMaximumDate()
*/

void QCalendarWidget::setDateRange(const QDate &min, const QDate &max)
{
    Q_D(QCalendarWidget);
    if (d->m_model->minimumDate == min && d->m_model->maximumDate == max)
        return;
    if (!min.isValid() || !max.isValid())
        return;

    QDate minimum = min;
    QDate maximum = max;
    if (min > max) {
        minimum = max;
        maximum = min;
    }

    QDate oldDate = d->m_model->date;
    d->m_model->setRange(min, max);
    d->yearEdit->setMinimum(d->m_model->minimumDate.year());
    d->yearEdit->setMaximum(d->m_model->maximumDate.year());
    d->updateMonthMenu();
    QDate newDate = d->m_model->date;
    if (oldDate != newDate) {
        d->update();
        d->showMonth(newDate.year(), newDate.month());
        emit selectionChanged();
    }
}


/*! \enum QCalendarWidget::HorizontalHeaderFormat

    This enum type defines the various formats the horizontal header can display.

    \value SingleLetterDayNames The header displays a single letter abbreviation for day names (e.g. M for Monday).
    \value ShortDayNames The header displays a short abbreviation for day names (e.g. Mon for Monday).
    \value LongDayNames The header displays complete day names (e.g. Monday).
    \value NoHorizontalHeader The header is hidden.

    \sa horizontalHeaderFormat(), VerticalHeaderFormat
*/

/*!
    \property QCalendarWidget::horizontalHeaderFormat
    \brief the format of the horizontal header.

    The default value is QCalendarWidget::ShortDayNames.
*/

void QCalendarWidget::setHorizontalHeaderFormat(QCalendarWidget::HorizontalHeaderFormat format)
{
    Q_D(QCalendarWidget);
    if (d->m_model->horizontalHeaderFormat == format)
        return;

    d->m_model->setHorizontalHeaderFormat(format);
    d->m_view->viewport()->update();
    d->m_view->updateGeometry();
}

QCalendarWidget::HorizontalHeaderFormat QCalendarWidget::horizontalHeaderFormat() const
{
    Q_D(const QCalendarWidget);
    return d->m_model->horizontalHeaderFormat;
}


/*! \enum QCalendarWidget::VerticalHeaderFormat

    This enum type defines the various formats the vertical header can display.

    \value ISOWeekNumbers The header displays a ISO week numbers \l QDate::weekNumber().
    \value NoVerticalHeader The header is hidden.

    \sa verticalHeaderFormat(), HorizontalHeaderFormat
*/

/*!
    \property QCalendarWidget::verticalHeaderFormat
    \brief the format of the vertical header.

    The default value is QCalendarWidget::ISOWeekNumber.
*/

QCalendarWidget::VerticalHeaderFormat QCalendarWidget::verticalHeaderFormat() const
{
    Q_D(const QCalendarWidget);
    bool shown = d->m_model->weekNumbersShown();
    if (shown)
        return QCalendarWidget::ISOWeekNumbers;
    return QCalendarWidget::NoVerticalHeader;
}

void QCalendarWidget::setVerticalHeaderFormat(QCalendarWidget::VerticalHeaderFormat format)
{
    Q_D(QCalendarWidget);
    bool show = false;
    if (format == QCalendarWidget::ISOWeekNumbers)
        show = true;
    if (d->m_model->weekNumbersShown() == show)
        return;
    d->m_model->setWeekNumbersShown(show);
    d->m_view->viewport()->update();
    d->m_view->updateGeometry();
}

/*!
    \property QCalendarWidget::gridVisible
    \brief whether the table grid is displayed.

    \table
    \row
        \o \inlineimage qcalendarwidget-grid.png
    \row
        \o
        \code
            QCalendarWidget *calendar;
            calendar->setGridVisible(true);
        \endcode
    \endtable

    The default value is false.
*/

bool QCalendarWidget::isGridVisible() const
{
    Q_D(const QCalendarWidget);
    return d->m_view->showGrid();
}

void QCalendarWidget::setGridVisible(bool show)
{
    Q_D(QCalendarWidget);
    d->m_view->setShowGrid(show);
    d->m_view->viewport()->update();
    d->m_view->updateGeometry();
}

/*!
    \property QCalendarWidget::selectionMode
    \brief the type of selection the user can make in the calendar

    When this property is set to SingleSelection, the user can select a date
    within the minimum and maximum allowed dates, using either the mouse or
    the keyboard.

    When the property is set to NoSelection, the user will be unable to select
    dates, but they can still be selected programmatically. Note that the date 
    that is selected when the property is set to NoSelection will still be 
    the selected date of the calendar.

    The default value is SingleSelection.
*/

QCalendarWidget::SelectionMode QCalendarWidget::selectionMode() const
{
    Q_D(const QCalendarWidget);
    return d->m_view->readOnly ? QCalendarWidget::NoSelection : QCalendarWidget::SingleSelection;
}

void QCalendarWidget::setSelectionMode(SelectionMode mode)
{
    Q_D(QCalendarWidget);
    d->m_view->readOnly = (mode == QCalendarWidget::NoSelection);
}

/*!
    \property QCalendarWidget::firstDayOfWeek
    \brief a value identifying the day displayed in the first column.

    By default, the day displayed in the first column is Sunday
*/

void QCalendarWidget::setFirstDayOfWeek(Qt::DayOfWeek dayOfWeek)
{
    Q_D(QCalendarWidget);
    if ((Qt::DayOfWeek)d->m_model->firstColumnDay() == dayOfWeek)
        return;

    d->m_model->setFirstColumnDay(dayOfWeek);
    d->update();
}

Qt::DayOfWeek QCalendarWidget::firstDayOfWeek() const
{
    Q_D(const QCalendarWidget);
    return (Qt::DayOfWeek)d->m_model->firstColumnDay();
}

/*!
    Returns the text char format for rendering the header.
*/
QTextCharFormat QCalendarWidget::headerTextFormat() const
{
    Q_D(const QCalendarWidget);
    return d->m_model->m_headerFormat;
}

/*!
    Sets the text char format for rendering the header to \a format.
    If you also set a weekday text format, this format's foreground and
    background color will take precedence over the header's format. 
    The other formatting information will still be decided by 
    the header's format. 
*/
void QCalendarWidget::setHeaderTextFormat(const QTextCharFormat &format)
{
    Q_D(QCalendarWidget);
    d->m_model->m_headerFormat = format;
    d->m_view->viewport()->update();
    d->m_view->updateGeometry();
}

/*!
    Returns the text char format for rendering of day in the week \a dayOfWeek.

    \sa headerTextFormat()
*/
QTextCharFormat QCalendarWidget::weekdayTextFormat(Qt::DayOfWeek dayOfWeek) const
{
    Q_D(const QCalendarWidget);
    return d->m_model->m_dayFormats.value(dayOfWeek);
}

/*!
    Sets the text char format for rendering of day in the week \a dayOfWeek to \a format.
    The format will take precedence over the header format in case of foreground 
    and background color. Other text formatting information is taken from the headers format. 

    \sa setHeaderTextFormat()
*/
void QCalendarWidget::setWeekdayTextFormat(Qt::DayOfWeek dayOfWeek, const QTextCharFormat &format)
{
    Q_D(QCalendarWidget);
    d->m_model->m_dayFormats[dayOfWeek] = format;
    d->m_view->viewport()->update();
    d->m_view->updateGeometry();
}

/*!
    Returns a QMap from QDate to QTextCharFormat showing all dates
    that use a special format that alters their rendering.
*/
QMap<QDate, QTextCharFormat> QCalendarWidget::dateTextFormat() const
{
    Q_D(const QCalendarWidget);
    return d->m_model->m_dateFormats;
}

/*!
    Returns a QTextCharFormat for \a date. The char format can be be
    empty if the date is not renderd specially.
*/
QTextCharFormat QCalendarWidget::dateTextFormat(const QDate &date) const
{
    Q_D(const QCalendarWidget);
    return d->m_model->m_dateFormats.value(date);
}

/*!
    Sets \a format to render \a date.
*/
void QCalendarWidget::setDateTextFormat(const QDate &date, const QTextCharFormat &format)
{
    Q_D(QCalendarWidget);
    d->m_model->m_dateFormats[date] = format;
    d->m_view->viewport()->update();
    d->m_view->updateGeometry();
}

/*!
    \fn void QCalendarWidget::selectionChanged()

    This signal is emitted when the currently selected date is
    changed.

    The currently selected date can be changed by the user using the
    mouse or keyboard, or by the programmer using setSelectedDate().

    \sa selectedDate()
*/

/*!
    \fn void QCalendarWidget::currentPageChanged(int year, int month)

    This signal is emitted when the currently shown month is changed.
    The new \a year and \a month are passed as parameters.

    \sa setCurrentPage()
*/

/*!
    \fn void QCalendarWidget::activated(const QDate &date)

    This signal is emitted whenever the user presses the Return or
    Enter key, the space bar or double-clicks a \a date in the calendar
    widget.
*/

/*!
    \fn void QCalendarWidget::clicked(const QDate &date)

    This signal is emitted when a mouse button is clicked. The date the
    mouse was clicked on is specified by \a date. The signal is only
    emitted when clicked on a valid date.
*/

/*!
    \property QCalendarWidget::headerVisible
    \brief whether the Header is shown or not

    When this property is set to true the next month, previous month,
    month selection, year selection controls are shown on top

    When the property is set to false, these controls are hidden.

    The default value is true.
*/

bool QCalendarWidget::isHeaderVisible() const
{
    Q_D(const QCalendarWidget);
    return d->headerVisible;
}

void QCalendarWidget::setHeaderVisible(bool show)
{
    Q_D(QCalendarWidget);
    d->headerVisible = show;
    (show)?d->headerBackground->show():d->headerBackground->hide();
    updateGeometry();
}

/*!
  \reimp
*/
bool QCalendarWidget::event(QEvent *event)
{
    Q_D(QCalendarWidget);
    if (event->type() == QEvent::FontChange || event->type() == QEvent::ApplicationFontChange) {
        d->updateHeader();
        d->m_view->updateGeometry();
    }
    return QWidget::event(event);
}

/*!
  \reimp
*/
void QCalendarWidget::mousePressEvent(QMouseEvent *event)
{
    setAttribute(Qt::WA_NoMouseReplay);
    QWidget::mousePressEvent(event);
    setFocus();
}

/*!
  \reimp
*/
void QCalendarWidget::resizeEvent(QResizeEvent * event)
{
    Q_D(QCalendarWidget);
    if(d->yearEdit->isVisible())
        d->_q_yearEditingFinished();
    QWidget::resizeEvent(event);
}

/*!
  \reimp
*/
void QCalendarWidget::keyPressEvent(QKeyEvent * event)
{
    Q_D(QCalendarWidget);
    if(d->yearEdit->isVisible()&& event->key() == Qt::Key_Escape)
    {
        d->yearEdit->setValue(yearShown());
        d->_q_yearEditingFinished();
        return;
    }
    QWidget::keyReleaseEvent(event);
}

#include "qcalendarwidget.moc"
#include "moc_qcalendarwidget.cpp"

#endif //QT_NO_CALENDARWIDGET
