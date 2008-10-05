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

#include <QTest>
#include <QObject>
#include <QPushButton>
#include <QRadioButton>

class tst_QObjectPerf : public QObject
{
    Q_OBJECT

private slots:
    void inherits_likely();
    void inherits_unlikely();
    void qobject_cast_likely();
    void qobject_cast_unlikely();
};

QTEST_MAIN(tst_QObjectPerf)

void tst_QObjectPerf::inherits_likely()
{
    QPushButton pb;
    const int MAX = 4000000;
    for (int i = 0; i < MAX; ++i)
        pb.inherits("QAbstractButton");
}

void tst_QObjectPerf::inherits_unlikely()
{
    QPushButton pb;
    const int MAX = 4000000;
    for (int i = 0; i < MAX; ++i)
        pb.inherits("QRadioButton");
}

void tst_QObjectPerf::qobject_cast_likely()
{
    QPushButton pb;
    const int MAX = 4000000;
    for (int i = 0; i < MAX; ++i)
        qobject_cast<QAbstractButton*>(&pb);
}

void tst_QObjectPerf::qobject_cast_unlikely()
{
    QPushButton pb;
    const int MAX = 4000000;
    for (int i = 0; i < MAX; ++i)
        qobject_cast<QRadioButton*>(&pb);
}


#include "tst_qobjectperf.moc"

