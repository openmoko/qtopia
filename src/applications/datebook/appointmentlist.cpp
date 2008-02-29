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

#include "appointmentlist.h"

AppointmentList::AppointmentList(QWidget *parent)
: QListView(parent)
{
    maxFoldedHeight = 0;
    maxRows = 0;
    folded = false;
    occurrenceModel = 0;
    setUniformItemSizes(true);
    setFrameShape(QFrame::NoFrame);
}

AppointmentList::~AppointmentList()
{
}

void AppointmentList::setFolded(bool f)
{
    folded = f;
    recalculateHeight();
}

void AppointmentList::recalculateHeight()
{
    int rows = model()->rowCount();

    if (rows == 0) {
        hide();
        emit changeHiddenCount(0);
    } else {
        int itemHeight = sizeHintForRow(0);
        maxRows = qMax(1, maxFoldedHeight / itemHeight);

        if (folded && rows > maxRows) {
            //  Show 1 less than technically possible; save room for "more" label
            maxRows--;
            setFixedHeight((maxRows * itemHeight) + 1);
            emit changeHiddenCount(rows - maxRows);
        } else {
            setFixedHeight(qMin(parentWidget()->height(), (rows * itemHeight) + 1));
            emit changeHiddenCount(0);
        }

        if (folded)
            setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        else
            setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        show();
    }
}

void AppointmentList::setModel(QOccurrenceModel *model)
{
    occurrenceModel = model;
    connect(model, SIGNAL(modelReset()), this, SLOT(recalculateHeight()));
    QListView::setModel(model);
}

int AppointmentList::visibleRowCount() const
{
    if (folded)
        return qMin(model()->rowCount(), maxRows);
    else
        return model()->rowCount();
}

void AppointmentList::setMaximumFoldedHeight(int height)
{
    maxFoldedHeight = height;
    recalculateHeight();
}

bool AppointmentList::isFolded() const
{
    return folded;
}

bool AppointmentList::provideFoldingOption()
{
    int rows = model()->rowCount();
    recalculateHeight();

    if ( rows > maxRows ) {
        return true;
    } else {
        return false;
    }
}


