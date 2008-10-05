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

#ifndef _XOPEN_SOURCE
# define _XOPEN_SOURCE
#endif

#include <QTest>
#include <time.h>
#include <QtopiaApplication>
#include <QDateTime>
#include <shared/perftest.h>
#include <shared/qtopiaunittest.h>

//TESTED_CLASS=QDateTime
//TESTED_FILES=

/*
    Performance tests for Qt tool classes.
*/
class tst_QToolsPerf : public QObject
{
    Q_OBJECT

private slots:
    void qdatetime_fromString_stringFormat();
    void qdatetime_fromString_stringFormat_data();
};

QTEST_APP_MAIN( tst_QToolsPerf, QtopiaApplication )
#include "tst_qtoolsperf.moc"

void tst_QToolsPerf::qdatetime_fromString_stringFormat()
{
    QFETCH(QString,     dateTimeAsString);
    QFETCH(QDateTime, dateTime);
    QFETCH(QString,   qtFormat);
    QFETCH(QString,     cFormat);

    double qdatetime_time;
    double strptime_time;

    {
        QDateTime converted;

        PERF_BEGIN_TASK(2000) {
            converted = QDateTime::fromString(dateTimeAsString, qtFormat);

            PERF_BEGIN_SKIP() {
                QCOMPARE(converted, dateTime);
            } PERF_END_SKIP();

        } PERF_END_TASK();
        PERF_LOG(QString("QDateTime::fromString(\"%1\", \"%2\")").arg(dateTimeAsString).arg(qtFormat));
        qdatetime_time = PERF_MICROSECONDS_PER_TASK;
    }

    {
        struct tm converted;
        char *ret;
        char const *s = qPrintable(dateTimeAsString);
        char const *format = qPrintable(cFormat);

        PERF_BEGIN_TASK(2000) {
            ret = strptime(s, format, &converted);

            PERF_BEGIN_SKIP() {
                QVERIFY( ret );
                QCOMPARE( mktime(&converted), time_t(dateTime.toTime_t()) );
            } PERF_END_SKIP();

        } PERF_END_TASK();
        PERF_LOG(QString("strptime(\"%1\", \"%2\", ...)").arg(s).arg(format));
        strptime_time = PERF_MICROSECONDS_PER_TASK;
    }

    qDebug() << "QDateTime::fromString is" << qdatetime_time/strptime_time << "times slower than strptime";
}

void tst_QToolsPerf::qdatetime_fromString_stringFormat_data()
{
    QTest::addColumn<QString>("dateTimeAsString");
    QTest::addColumn<QDateTime>("dateTime");
    QTest::addColumn<QString>("qtFormat");
    QTest::addColumn<QString>("cFormat");

    QTest::newRow("simple")
        << "12/12/2010, 12:12:12"
        << QDateTime(QDate(2010, 12, 12), QTime(12, 12, 12))
        << "MM/dd/yyyy, HH:mm:ss"
        << "%m/%d/%Y, %H:%M:%S";

}

