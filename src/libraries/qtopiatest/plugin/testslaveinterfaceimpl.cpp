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

#include <private/testslaveinterface_p.h>

#include <qtestslave.h>
#include <QTestMessage>
#include <QApplication>
#include <qtopiabase/qtopialog.h>


/* The renaming of the class and the slave variable allows singleexec to work.
   Without it, we have duplicate symbols in the two plugins.
 */
#ifdef APP_PLUGIN
# include <qtopiasystemtestslave.h>
# define TestSlaveImpl AppTestSlaveImpl
#endif

#ifdef SERVER_PLUGIN
# include <qtopiaservertestslave.h>
# define TestSlaveImpl ServerTestSlaveImpl
#endif

class TestSlaveImpl : public QObject, public TestSlaveInterface {
    Q_OBJECT
    Q_INTERFACES(TestSlaveInterface)
public:
    TestSlaveImpl();

    void postMessage(QString const &name, QVariantMap const &data);
    bool isConnected() const;
    void qwsEventFilter(QWSEvent *event);

private slots:
    void init();

private:
    QTestSlave *slave;
};

#include "testslaveinterfaceimpl.moc"

TestSlaveImpl::TestSlaveImpl() : QObject()
{
    slave = 0;
    /* We shouldn't construct a slave until entering the event loop - otherwise
     * we might be created before the application object has fully constructed,
     * hence we can't tell what kind of app we are. */
    QTimer::singleShot(0, this, SLOT(init()));
}

void TestSlaveImpl::init()
{
    if (!qApp || QCoreApplication::startingUp()) {
        QTimer::singleShot(0, this, SLOT(init()));
        return;
    }

    if (slave) return;

#ifdef SERVER_PLUGIN
    if (!slave && qApp->inherits("QtopiaServerApplication")) {
        slave = new QtopiaServerTestSlave;
        qLog(Qtopiatest) << "Using QtopiaServerTestSlave as slave";
    }
#endif
#ifdef APP_PLUGIN
    if (!slave && qApp->inherits("QtopiaApplication")) {
        slave = new QtopiaSystemTestSlave;
        qLog(Qtopiatest) << "Using QtopiaSystemTestSlave as slave";
    }
#endif
    if (!slave) {
        slave = new QTestSlave;
        qLog(Qtopiatest) << "Using QTestSlave as slave";
    }
}

bool TestSlaveImpl::isConnected() const
{
    return (slave && slave->isConnected());
}

void TestSlaveImpl::postMessage(QString const &name, QVariantMap const &data)
{
    if (!isConnected()) return;
    QTestMessage msg(name);
    foreach(QString k, data.keys()) {
        msg[k] = data[k];
    }
    slave->postMessage(msg);
}

void TestSlaveImpl::qwsEventFilter(QWSEvent *event)
{
    if (!isConnected()) return;
    slave->qwsEventFilter(event);
}

QTOPIA_EXPORT_PLUGIN( TestSlaveImpl )
