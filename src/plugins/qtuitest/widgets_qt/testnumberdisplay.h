/****************************************************************************
**
** This file is part of the Qt Extended Opensource Package.
**
** Copyright (C) 2008 Trolltech ASA.
**
** Contact: Qt Extended Information (info@qtextended.org)
**
** This file may be used under the terms of the GNU General Public License
** version 2.0 as published by the Free Software Foundation and appearing
** in the file LICENSE.GPL included in the packaging of this file.
**
** Please review the following information to ensure GNU General Public
** Licensing requirements will be met:
**     http://www.fsf.org/licensing/licenses/info/GPLv2.html.
**
**
****************************************************************************/

//
//  W A R N I N G
//  -------------
//
// This file is part of QtUiTest and is released as a Technology Preview.
// This file and/or the complete System testing solution may change from version to
// version without notice, or even be removed.
//

#ifndef TESTNUMBERDISPLAY_H
#define TESTNUMBERDISPLAY_H

#include "testwidget.h"

class TestNumberDisplay : public TestWidget,
    public QtUiTest::TextWidget
{
    Q_OBJECT
    Q_INTERFACES(
            QtUiTest::TextWidget
            )

public:
    TestNumberDisplay(QObject*);

    virtual QString text() const;

    virtual bool canEnter(QVariant const&) const;
    virtual bool enter(QVariant const&,bool);

    static bool canWrap(QObject*);

signals:
    void entered(const QVariant&);

private slots:
    void onNumberChanged(QString const&);

private:
    QObject *q;
};

#endif

