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

#include <QVector>
#include <QList>
#include <QRect>
#include <QDateTime>
#include <QWidget>
#include <qtopia/pim/event.h>
#include <qtopia/pim/qappointmentmodel.h>

class LayoutItem
{
public:
    LayoutItem( const QOccurrence e );

    void setGeometry(const QRect &rect) { r = rect; }
    void setGeometry(int x, int y, int w, int h)
    { setGeometry(QRect(x,y,w,h)); }
    QRect geometry() const { return r; }

    QOccurrence occurrence() const { return eappointment; }
    QAppointment appointment() const { return eappointment.appointment(); }

private:
    QOccurrence eappointment;
    QRect r;
};

class LayoutItemSorter
{
public:
    bool operator()( LayoutItem *d1, LayoutItem *d2 );
};

class LayoutManager : public QWidget
{
public:
    LayoutManager(QWidget *parent);
    virtual ~LayoutManager();


    virtual void setDate(const QDate &d);
    QDate date() const { return referenceDate; }

    virtual void setSize(int w, int h);
    void setMaximumColumnWidth(int x) { maxWidth = x; };
    int maximumColumnWidth() const { return maxWidth; };
    void setOccurrences(QList<QOccurrence> &appointments);
    virtual void addOccurrence(QOccurrence &appointment);

    virtual void clear();

    QList<LayoutItem *> items() const { return mItems; }
    QSize size() const { return QSize(width, height); }
    int count() const { return mItems.count(); }

    virtual void layoutItems(bool resetMaxWidth = false);

    virtual int timeToHeight(const QTime &) const;
    virtual QTime heightToTime(int) const;

    virtual void appointmentsChanged() {}

protected:
    void initializeGeometry(LayoutItem *);
    LayoutItem *intersects(LayoutItem *, QRect, int = 0, int * = 0) const;
    void addItem(LayoutItem *);

    QList<LayoutItem *> mItems;
    int width;
    int height;
    int maxWidth;
    QDate referenceDate;
};
