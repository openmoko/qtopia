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

#include <QTest>
#include <QAction>
#include <QApplication>
#include <QObject>
#include <qtuitestnamespace.h>

#include <QTime>
#include <QTimer>

#include <qalternatestack_p.h>

#include <shared/util.h>

class tst_QtUiTestNamespace : public QObject
{
    Q_OBJECT

public slots:
    void setCounter(int);
    void resetCounter();

private slots:
    void wait();
    void connect();
    void connect_defaultparam();

private:
    int m_counter;
};

int      g_exit_code;
int      g_argc;
char**   g_argv;
bool     g_test_done;
QObject* g_test;

void run_test(QAlternateStack*,QVariant const&)
{
    g_exit_code = QTest::qExec(g_test, g_argc, g_argv);
    g_test_done = true;
}

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    tst_QtUiTestNamespace test;
    g_test = &test;
    g_argc = argc;
    g_argv = argv;

    // Most QtUiTest code runs in an alternate stack, while the original
    // stack runs the event loop.
    // Try to replicate that behavior for this test.
    QAlternateStack stack;

    g_test_done = false;
    stack.start(run_test);
    while (!g_test_done) {
        app.processEvents(QEventLoop::WaitForMoreEvents);
    }
    return g_exit_code;
}

/*
    Event loop where 'exec' is a slot, for convenience.
*/
class TestEventLoop : public QEventLoop
{
    Q_OBJECT
public:
    TestEventLoop(QObject* parent =0)
        :   QEventLoop(parent),
            m_execCount(0),
            m_exitCount(0)
    {}

    int m_execCount;
    int m_exitCount;

public slots:
    int exec(QEventLoop::ProcessEventsFlags flags = QEventLoop::AllEvents)
    {
        ++m_execCount;
        int ret = QEventLoop::exec(flags);
        ++m_exitCount;
        return ret;
    }
};

/*
    Class for simply emitting a particular integer value.
*/
class Emitter : public QObject
{
    Q_OBJECT

public:
    Emitter(QObject* parent =0)
        : QObject(parent)
    {}

    void emitValue(int value)
    { emit valueChanged(value); }

signals:
    void valueChanged(int);
};

// Verify that \a actual is bounded by \a min and \a max.
#define QCOMPARE_BOUNDED(actual, min, max)                           \
    do {                                                             \
        QVERIFY2(actual >= min && actual <= max, qPrintable(QString( \
            "actual %1, expected to be in range %2 .. %3"            \
        ).arg(actual).arg(min).arg(max)));                           \
    } while(0)

/*
    The most important test ever written.

    Verify that we can wait() for an arbitrary amount of time, while processing
    events, without hanging if a nested event loop occurs.

    See bug 194361.
*/
void tst_QtUiTestNamespace::wait()
{
    {
        QTime t;
        t.start();

        QtUiTest::wait(1000);

        QCOMPARE_BOUNDED(t.elapsed(), 1000, 30000);
    }

    // OK, so we can wait when there are no nested event loops.
    // Big deal.  The real test is:  if we have a nested event loop, can we
    // avoid hanging.
    {
        TestEventLoop loop;

        QTime t;
        t.start();

        // The nested event loop will run for 2.5 seconds.
        // But we only want to wait for 1 second.
        // What on earth will happen... ???
        QTimer::singleShot(0,    &loop, SLOT(exec()));
        QTimer::singleShot(2500, &loop, SLOT(quit()));
        QtUiTest::wait(1000);

        // Verify the loop really did exec.
        QCOMPARE(loop.m_execCount, 1);
        // Verify the loop really hasn't exited yet.
        QCOMPARE(loop.m_exitCount, 0);
        // Verify that we've waited for about as long as we wanted to wait.
        QCOMPARE_BOUNDED(t.elapsed(), 1000, 30000);

        // OK, now check inner loop really does exit, to ensure we haven't
        // screwed things up by switching stacks.
        for (int i = 0; i < 5000 && !loop.m_exitCount; i+=100, QtUiTest::wait(100))
        {}
        QCOMPARE(loop.m_exitCount, 1);
        QCOMPARE_BOUNDED(t.elapsed(), 2500, 30000);
    }
}

/*
    Tests QtUiTest::connect(), which works like QObject::connect but guarantees
    that the connection comes before all existing connections.
*/
void tst_QtUiTestNamespace::connect()
{
    resetCounter();
    QCOMPARE( m_counter, 0 );

    {
        // First, check that using regular old QObject::connect activates the connections
        // in the order they are made.
        Emitter emitter;
        QVERIFY( QObject::connect(&emitter, SIGNAL(valueChanged(int)),
                                  this,     SLOT(setCounter(int))) );
        QVERIFY( QObject::connect(&emitter, SIGNAL(valueChanged(int)),
                                  this,     SLOT(resetCounter())) );
        emitter.emitValue(123);

        // m_counter should be zero, since resetCounter() came last.
        QCOMPARE( m_counter, 0 );

        // Now swap the order.
        QVERIFY( QObject::disconnect(&emitter,0,0,0) );
        QVERIFY( QObject::connect(&emitter, SIGNAL(valueChanged(int)),
                                  this,     SLOT(resetCounter())) );
        QVERIFY( QObject::connect(&emitter, SIGNAL(valueChanged(int)),
                                  this,     SLOT(setCounter(int))) );
        emitter.emitValue(234);

        // m_counter should be 234, since setCounter() came last.
        QCOMPARE( m_counter, 234 );
    }

    {
        // Now check that using QtUiTest::connect activates that connection
        // before any regular QObject connections.
        Emitter emitter;
        QVERIFY( QObject::connect(&emitter, SIGNAL(valueChanged(int)),
                                  this,     SLOT(resetCounter())) );
        QVERIFY( QtUiTest::connect(&emitter, SIGNAL(valueChanged(int)),
                                     this,     SLOT(setCounter(int)),
                                     QtUiTest::Before ));
        emitter.emitValue(123);

        // m_counter should be zero, since resetCounter() came last.
        QCOMPARE( m_counter, 0 );

        // Now swap the order.
        QVERIFY( QObject::disconnect(&emitter,0,0,0) );
        QVERIFY( QtUiTest::disconnect(&emitter,0,0,0) );
        QVERIFY( QtUiTest::connect(&emitter, SIGNAL(valueChanged(int)),
                                     this,     SLOT(setCounter(int)),
                                     QtUiTest::After ));
        QVERIFY( QObject::connect(&emitter, SIGNAL(valueChanged(int)),
                                  this,     SLOT(resetCounter())) );
        emitter.emitValue(234);

        // m_counter should be 234, since setCounter() came last.
        QCOMPARE( m_counter, 234 );
    }
}

/*
    Tests QtUiTest::connect() for signals which have default parameters.
*/
void tst_QtUiTestNamespace::connect_defaultparam()
{
    {
        QAction act("My action", 0);

        QTimer dummy;
        dummy.setInterval(1000);

        QVERIFY(QtUiTest::connect(&act, SIGNAL(triggered(bool)), &dummy, SLOT(start()), QtUiTest::Before));
        act.trigger();
        QVERIFY(dummy.isActive());
    }

    {
        QAction act("My action", 0);

        QTimer dummy;
        dummy.setInterval(1000);

        QVERIFY(QtUiTest::connect(&act, SIGNAL(triggered()), &dummy, SLOT(start()), QtUiTest::Before));
        act.trigger();

        QEXPECT_FAIL("", "Bug 227908 - QtUiTest::connect fails with signals with default parameters", Abort);
        QVERIFY(dummy.isActive());
    }
}

void tst_QtUiTestNamespace::resetCounter()
{ m_counter = 0; }

void tst_QtUiTestNamespace::setCounter(int value)
{ m_counter = value; }

#include "tst_qtuitestnamespace.moc"
