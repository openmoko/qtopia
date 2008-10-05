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
#include <QObject>
#include <qalternatestack_p.h>
#include <shared/qtopiaunittest.h>

#include <signal.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <string.h>
#include <errno.h>

//TESTED_COMPONENT=QA: Testing Framework (18707)

class tst_QAlternateStack : public QObject
{
    Q_OBJECT

private slots:
    void start();
    void switching();

    void init();

    void sigaltstack();

private:
    static void stack_entry(QAlternateStack*, QVariant const&);

    int              m_shouldSwitchFrom;

    QAlternateStack* m_stack;
    QVariant         m_data;
    bool             m_isActive;
    bool             m_isCurrentStack;
};

QTEST_MAIN(tst_QAlternateStack)

void tst_QAlternateStack::stack_entry(QAlternateStack* stack, QVariant const& data)
{
    QVariantList list = data.toList();
    if (!list.count()) return;
    if (!stack)        return;

    tst_QAlternateStack* test = qobject_cast<tst_QAlternateStack*>(list.at(0).value<QObject*>());
    if (!test) return;

    test->m_stack          = stack;
    test->m_data           = data;
    test->m_isActive       = stack->isActive();
    test->m_isCurrentStack = stack->isCurrentStack();

    for (; test->m_shouldSwitchFrom > 0; --test->m_shouldSwitchFrom) {
        stack->switchFrom();
        test->m_isActive       = stack->isActive();
        test->m_isCurrentStack = stack->isCurrentStack();
    }
}

/*
    \req QTOPIA-78
    \usecase
    \revby Ed 08/04/2008
    \groups
*/
void tst_QAlternateStack::start()
{
    if (!QAlternateStack::isAvailable()) {
        QSKIP("QAlternateStack is not available on this platform.", SkipAll);
    }

    QAlternateStack stack;

    QCOMPARE(stack.isActive(),       false);
    QCOMPARE(stack.isCurrentStack(), false);

    QVariantList list;
    list.append( qVariantFromValue((QObject*)this) );

    // Switch to the stack...
    stack.start(tst_QAlternateStack::stack_entry, list);
    // ... it should have returned almost immediately.

    // Check that our private members were set to the expected values.
    QCOMPARE(m_stack,          &stack);
    QCOMPARE(m_data,           qVariantFromValue(list));
    QCOMPARE(m_isActive,       true);
    QCOMPARE(m_isCurrentStack, true);

    QCOMPARE(stack.isActive(),       false);
    QCOMPARE(stack.isCurrentStack(), false);
}

/*
    \req QTOPIA-78
    \usecase
    \revby Ed 08/04/2008
    \groups
*/
void tst_QAlternateStack::switching()
{
    if (!QAlternateStack::isAvailable()) {
        QSKIP("QAlternateStack is not available on this platform.", SkipAll);
    }

    QAlternateStack stack;

    QCOMPARE(stack.isActive(),       false);
    QCOMPARE(stack.isCurrentStack(), false);

    QVariantList list;
    list.append( qVariantFromValue((QObject*)this) );

    // Set it up so the alternate stack calls switchFrom() several times.
    const int switchCount = 5;
    m_shouldSwitchFrom = switchCount;

    stack.start(tst_QAlternateStack::stack_entry, list);

    for (int i = switchCount; i > 0; --i) {
        // Check that our private members were set to the expected values.
        QCOMPARE(m_stack,          &stack);
        QCOMPARE(m_data,           qVariantFromValue(list));
        QCOMPARE(m_isActive,       true);
        QCOMPARE(m_isCurrentStack, true);

        QCOMPARE(m_shouldSwitchFrom, i);

        // Still active, since we switched using switchFrom().
        QCOMPARE(stack.isActive(),       true);
        QCOMPARE(stack.isCurrentStack(), false);

        // Switch to the stack...
        stack.switchTo();
    }

    // No longer active.
    QCOMPARE(m_shouldSwitchFrom,     0);
    QCOMPARE(stack.isActive(),       false);
    QCOMPARE(stack.isCurrentStack(), false);
}

char* addressof_dummy;
int   got_signal;

void test_sighandler(int signum)
{
    got_signal = signum;

    int dummy = 1;
    addressof_dummy = (char*)&dummy;
}

/*
    \req QTOPIA-78
    \usecase
    \revby Rohan 14/04/2008
    \groups

    Tests that sigaltstack() actually works on the target platform.

    On some platforms, like the Greenphone, sigaltstack seems to be broken
    and always causes a segfault.  When porting QtUitest to a new platform,
    run this testfunction to make sure QAlternateStack will work properly.
*/
void tst_QAlternateStack::sigaltstack()
{
    // isAvailable() returns false on platforms where sigaltstack is known
    // to be broken.
    if (!QAlternateStack::isAvailable()) {
        QSKIP("QAlternateStack is not available on this platform.", SkipAll);
    }
    QByteArray buffer;
    buffer.resize(SIGSTKSZ);

    stack_t stack;
    stack.ss_sp    = buffer.data();
    stack.ss_size  = SIGSTKSZ;
    stack.ss_flags = 0;

    QVERIFY( 0 == ::sigaltstack(&stack, 0) );

    stack_t newstack;
    QVERIFY( 0 == ::sigaltstack(0, &newstack) );
    QVERIFY(stack.ss_sp    == newstack.ss_sp);
    QVERIFY(stack.ss_size  == newstack.ss_size);
    QVERIFY(stack.ss_flags == newstack.ss_flags);

    struct sigaction action;
    action.sa_handler = test_sighandler;
    action.sa_flags   = SA_ONSTACK;
    sigemptyset(&action.sa_mask);

    QVERIFY2( 0 == sigaction(SIGUSR2, &action, 0), strerror(errno) );

    struct sigaction newaction;
    QVERIFY2( 0 == sigaction(SIGUSR2,0,&newaction), strerror(errno) );
    QVERIFY(newaction.sa_handler == action.sa_handler);
    QVERIFY(newaction.sa_flags & SA_ONSTACK);

    addressof_dummy = 0;
    got_signal      = 0;
    qLog(Autotest) << "About to raise";

    // Greenphone crashes here.
    raise(SIGUSR2);

    QCOMPARE(got_signal, SIGUSR2);
    qLog(Autotest) << "dummy:" << (void*)addressof_dummy << "stack:" << stack.ss_sp;
    QVERIFY( (addressof_dummy > (char*)stack.ss_sp) && (addressof_dummy < (char*)stack.ss_sp + stack.ss_size) );
}

void tst_QAlternateStack::init()
{
    m_shouldSwitchFrom = 0;
    m_stack            = 0;
    m_data             = QVariant();
    m_isActive         = false;
    m_isCurrentStack   = false;
}

#include "tst_qalternatestack.moc"
