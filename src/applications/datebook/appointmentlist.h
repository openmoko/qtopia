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

#ifndef APPOINTMENT_LIST_H
#define APPOINTMENT_LIST_H

#include <qtopia/pim/qappointment.h>
#include <qtopia/pim/qappointmentmodel.h>

#include <QListView>

class AppointmentList : public QListView
{
    Q_OBJECT

public:
    AppointmentList(QWidget *parent = 0);
    virtual ~AppointmentList();

    void setFolded(bool f = true);
    void setModel(QAbstractItemModel *model);
    void setMaximumFoldedHeight(int height);
    void setMaximumUnfoldedHeight(int height);
    bool isFolded() const;
    bool provideFoldingOption();

    int visibleRowCount() const;

signals:
    void changeHiddenCount(int hidden);

public slots:
    void recalculateHeight();

private:
    bool folded;
    int maxFoldedHeight;
    int maxUnfoldedHeight;
    int maxRows;
    QOccurrenceModel *occurrenceModel;
};
#endif // __APPOINTMENT_LIST_H__
