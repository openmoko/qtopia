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
#include <sys/time.h>
#include <QtopiaApplication>
#include <QLabel>
#include <QGridLayout>
#include <shared/perftest.h>
#include <shared/qtopiaunittest.h>

//TESTED_CLASS=QLabel
//TESTED_FILES=

/*
    Performance tests for QLabel, especially construction and adding to layouts.
*/
class tst_QLabelPerf : public QObject
{
    Q_OBJECT

private:
    void data();

private slots:
    void gridlayout();
    void gridlayout_data();

    void constructor_reparent();
    void constructor_reparent_data();

    void minimumSizeHint();
    void minimumSizeHint_data();

    void setText_plain();
    void setText_plain_data();

    void setText_auto();
    void setText_auto_data();

    void destructor();
    void destructor_data();

    void constructor_withParent();
    void constructor_withParent_data();

};

QTEST_APP_MAIN( tst_QLabelPerf, QtopiaApplication )
#include "tst_qlabelperf.moc"

/*?
    Test the time taken to construct a QLabel passing a parent to the constructor.
*/
void tst_QLabelPerf::constructor_withParent()
{
    QFETCH(QString, text);
    QWidget widget;

    QLabel *label = 0;

    PERF_BEGIN_TASK(100) {
        label = new QLabel(text, &widget);

        /* Don't measure the time to destruct. */
        PERF_BEGIN_SKIP() {
            delete label;
        } PERF_END_SKIP();

    } PERF_END_TASK();

    PERF_LOG(QString("string length %1").arg(text.size()));
}

/*?
    Data for constructor_withParent(); uses data().
*/
void tst_QLabelPerf::constructor_withParent_data()
{
    data();
}

/*?
    Test how long it takes to destroy a QLabel which is the child
    of a widget.
*/
void tst_QLabelPerf::destructor()
{
    QFETCH(QString, text);
    QWidget widget;

    QLabel *label = 0;

    PERF_BEGIN_TASK(100) {

        /* Don't measure time to construct. */
        PERF_BEGIN_SKIP() {
            label = new QLabel(text, &widget);
        } PERF_END_SKIP();

        delete label;
    } PERF_END_TASK();

    PERF_LOG(QString("string length %1").arg(text.size()));
}

/*?
    Data for destructor(); uses data().
*/
void tst_QLabelPerf::destructor_data()
{
    data();
}

/*?
    Test how long it takes to set the text on a QLabel
    when the Qt::AutoText text format is used.
*/
void tst_QLabelPerf::setText_auto()
{
    QFETCH(QString, text);
    QWidget widget;

    QLabel *label = 0;
    PERF_BEGIN_TASK(100) {
        PERF_BEGIN_SKIP() {
            label = new QLabel(&widget);
            label->setTextFormat(Qt::AutoText);
        } PERF_END_SKIP();

        label->setText(text);

        PERF_BEGIN_SKIP() {
            delete label;
        } PERF_END_SKIP();
    } PERF_END_TASK();

    PERF_LOG(QString("string length %1").arg(text.size()));
}

/*?
    Data for setText_auto(); uses data().
*/
void tst_QLabelPerf::setText_auto_data()
{
    constructor_withParent_data();
}

/*?
    Test how long it takes to set the text on a QLabel
    when the Qt::PlainText text format is used.
*/
void tst_QLabelPerf::setText_plain()
{
    QFETCH(QString, text);
    QWidget widget;

    QLabel *label = 0;
    PERF_BEGIN_TASK(100) {
        PERF_BEGIN_SKIP() {
            label = new QLabel(&widget);
            label->setTextFormat(Qt::PlainText);
        } PERF_END_SKIP();

        label->setText(text);

        PERF_BEGIN_SKIP() {
            delete label;
        } PERF_END_SKIP();
    } PERF_END_TASK();

    PERF_LOG(QString("string length %1").arg(text.size()));
}

/*?
    Data for setText_plain(); uses data().
*/
void tst_QLabelPerf::setText_plain_data()
{
    constructor_withParent_data();
}

/*?
    Test how long the minimumSizeHint() function takes to return
    a value.
*/
void tst_QLabelPerf::minimumSizeHint()
{
    QFETCH(QString, text);
    QWidget widget;

    QLabel *label = 0;
    PERF_BEGIN_TASK(100) {
        PERF_BEGIN_SKIP() {
            label = new QLabel(text, &widget);
        } PERF_END_SKIP();

        label->minimumSizeHint();

        PERF_BEGIN_SKIP() {
            delete label;
        } PERF_END_SKIP();
    } PERF_END_TASK();

    PERF_LOG(QString("string length %1").arg(text.size()));
}

/*?
    Data for minimumSizeHint(); uses data().
*/
void tst_QLabelPerf::minimumSizeHint_data()
{
    constructor_withParent_data();
}

/*?
    Test how long it takes to construct an orphan QLabel, then
    reparent it.
*/
void tst_QLabelPerf::constructor_reparent()
{
    QFETCH(QString, text);
    QWidget widget;

    QLabel *label = 0;
    PERF_BEGIN_TASK(100) {
        label = new QLabel(text);
        label->setParent(&widget);
        PERF_BEGIN_SKIP() {
            delete label;
        } PERF_END_SKIP();
    } PERF_END_TASK();

    PERF_LOG(QString("string length %1").arg(text.size()));
}

/*?
    Data for constructor_reparent(); uses data().
*/
void tst_QLabelPerf::constructor_reparent_data()
{
    constructor_withParent_data();
}

/*?
    Test how long it takes to construct a QGridLayout, add 10
    QLabels to it and apply that layout to a widget.
*/
void tst_QLabelPerf::gridlayout()
{
    QFETCH(QString, text);
    QWidget widget;


    PERF_BEGIN_TASK(25) {

        QGridLayout *layout = new QGridLayout;

        /* Construct and add 10 QLabels to grid layout */
        for (int i = 0; i < 5; ++i) {
            for (int j = 0; j < 2; ++j) {
                layout->addWidget(new QLabel(text), i, j);
            }
        }

        /* Set the widget's layout and update */
        widget.setLayout(layout);
        layout->update();

        /* Destroy all layout items */
        PERF_BEGIN_SKIP() {
            QLayoutItem *child;
            while ((child = layout->takeAt(0)) != 0) {
                delete child;
            }
            delete layout;
        } PERF_END_SKIP();


    } PERF_END_TASK();

    PERF_LOG(QString("10 labels into grid layout, string length %1").arg(text.size()));
}

/*?
    Data for gridlayout(); uses data().
*/
void tst_QLabelPerf::gridlayout_data()
{
    constructor_withParent_data();
}

/*?
    String data for use in QLabel performance tests.
    This data is made up of strings containing simple text, of various different lengths
    and with various different amounts of rich text blocks.
    The amount of rich text is significant for many QLabel functions, so should be clearly
    indicated in the data tag for all rows.
*/
void tst_QLabelPerf::data()
{
    QTest::addColumn<QString>("text");

    QTest::newRow("null") << QString();
    QTest::newRow("empty") << "";
    QTest::newRow("simple")       << "   some text here    , and    more text here    ";
    QTest::newRow("1 rich text")  << "<i>some text here</i>, and    more text here    ";
    QTest::newRow("2 rich texts") << "<i>some text here</i>, and <b>more text here</b>";
    QTest::newRow("3 rich texts") << "<i>some text</i>,<b>and</b><b>more text here</b>";

    for (int j = 1; j <= 5; ++j) {
        QString str = "";
        for (int i = 0; i < j*5; ++i) str += "    foo    ,    bar    ";
        QTest::newRow(QString("%1 plain texts").arg(j*5*2).toLatin1()) << str;
    }

    for (int j = 1; j <= 5; ++j) {
        QString str = "";
        for (int i = 0; i < j*5; ++i) str += " <i>foo</i>, <b>bar</b>";
        QTest::newRow(QString("%1 rich texts").arg(j*5*2).toLatin1()) << str;
    }
}
