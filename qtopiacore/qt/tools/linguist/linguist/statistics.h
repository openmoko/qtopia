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

#ifndef STATISTICS_H
#define STATISTICS_H

#include "ui_statistics.h"
#include <QVariant>

class Statistics : public QDialog, public Ui::Statistics
{
    Q_OBJECT

public:
    Statistics(QWidget *parent = 0, Qt::WindowFlags fl = 0);
    ~Statistics() {}

public slots:
    virtual void updateStats(int w1, int c1, int cs1, int w2, int c2, int cs2);

protected slots:
    virtual void languageChange();
};

#endif // STATISTICS_H
