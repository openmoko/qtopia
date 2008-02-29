/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
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

#include <qtopia/qperformancelog.h>
#include <qtopia/private/qperformancelog_p.h>

#include <QThread>
#include <QTimer>
#include <QStringList>
#include <QApplication>
#include <sys/times.h>
#include <unistd.h>

#ifdef QTOPIA_TEST_HOST
# include <QDebug>
# define qLog(A) qDebug()
# define qLogEnabled(A) (true)
#else
# ifndef Q_WS_X11
#  include <qtopia/private/testslaveinterface_p.h>
#  define QTOPIA_USE_TEST_SLAVE 1
# endif
# include <qtopia/qtopiaapplication.h>
# include <qtopiabase/qtopialog.h>
# include <qtopiabase/Qtopia>
# include <sys/ipc.h>
# include <sys/shm.h>
# include <errno.h>
# define SHMKEY 45628540
#endif // ! QTOPIA_TEST_HOST


const int QPerformanceLogData::Timeout = 2000;

QString QPerformanceLogData::toString() const
{
    return QString("%1 : %2 : %3 : %4%5%6").arg(ident).arg(appTime).arg(serverTime).arg(QPerformanceLog::stringFromEvent(event)).arg((event != QPerformanceLog::NoEvent) ? " " : "").arg(msg);
}

class QPerformanceLogPrivate : public QObject
{
Q_OBJECT
public:
    void send( const QPerformanceLogData &msg );

    static QPerformanceLogPrivate *instance();

private slots:
    void sendUnsent();

private:
    QPerformanceLogPrivate();
    ~QPerformanceLogPrivate();

    QList<QPerformanceLogData> unsent;
    QTimer timer;
};
#include "qperformancelog.moc"

static struct tms dummy;
static clock_t start_time = times(&dummy);
static clock_t *server_start_time;
static qreal ticks_to_ms = 1000.0/qreal(sysconf(_SC_CLK_TCK));

#ifndef QTOPIA_TEST_HOST
class StartTimeSetup
{
public:
    StartTimeSetup();
    ~StartTimeSetup();

    bool boss;
    void *ptr;
    int shmid;
};

StartTimeSetup::StartTimeSetup()
{
    boss = false;
    /* Create or open shared memory.  If we successfully create "exclusively", we created
       rather than opened, so we are the "server" process. */
    shmid = shmget(SHMKEY, sizeof(clock_t), IPC_CREAT | IPC_EXCL | 00777);
    if (-1 == shmid) {
        shmid = shmget(SHMKEY, sizeof(clock_t), IPC_CREAT | 00777);
        if (-1 == shmid) {
            qWarning("%s:%d shmget failed: %s\n", __FILE__, __LINE__, strerror(errno));
        }
        /* If the shmem already existed but no-one is attached, qpe must have crashed
           or been killed without cleaning up.  We can inherit ownership. */
        struct shmid_ds shminfo;
        if (-1 != shmctl(shmid, IPC_STAT, &shminfo) && !shminfo.shm_nattch)
            boss = true;
    } else {
        boss = true;
    }

    ptr = shmat(shmid, 0, (!boss) ? SHM_RDONLY : 0);
    if (-1 == (long)ptr) qWarning("%s:%d shmat failed: %s\n", __FILE__, __LINE__, strerror(errno));
    server_start_time = static_cast<clock_t*>(ptr);

    if (boss) {
        bool set = false;
        do {
            QByteArray t = qgetenv("QTOPIA_PERFTEST_LAUNCH");
            if (t.isEmpty() || t.count(':') != 2) break;

            QList<QByteArray> tl = t.split(':');
            bool ok = true;
            int h, m, s, ms = 0;
            h = tl[0].toInt(&ok); if (!ok) break;
            m = tl[1].toInt(&ok); if (!ok) break;

            if (tl[2].count('.') == 1) {
                QList<QByteArray> tll = tl[2].split('.');
                s = tll[0].toInt(&ok); if (!ok) break;
                ms = tll[1].left(3).toInt(&ok); if (!ok) break;
            } else {
                s = tl[2].toInt(&ok); if (!ok) break;
            }

            uint launchMs = ((h*60 + m)*60 + s)*1000 + ms;
            QTime now(QTime::currentTime());
            uint nowMs = ((now.hour()*60 + now.minute())*60 + now.second())*1000 + now.msec();
            *server_start_time = clock_t(times(&dummy) - (nowMs - launchMs)/ticks_to_ms);

            set = true;

        } while(0);

        if (!set)
            *server_start_time = times(&dummy);
        QPerformanceLog() << "QPerformanceLog server_start_time set to " << QString::number(*server_start_time);
    }
}

StartTimeSetup::~StartTimeSetup()
{
    shmdt(ptr);
    if (boss) {
        shmctl(shmid, IPC_RMID, 0);
    }
}

static StartTimeSetup time_setup;
#endif // QTOPIA_TEST_HOST

QPerformanceLogPrivate *QPerformanceLogPrivate::instance()
{
    static QPerformanceLogPrivate *s = new QPerformanceLogPrivate();
    return s;
}

QPerformanceLogPrivate::QPerformanceLogPrivate() : QObject()
{
    connect(&timer, SIGNAL(timeout()), this, SLOT(sendUnsent()));
}

QPerformanceLogPrivate::~QPerformanceLogPrivate()
{
    sendUnsent();
}

void QPerformanceLogPrivate::sendUnsent()
{
#if !defined(QTOPIA_TEST_HOST) && defined(QTOPIA_USE_TEST_SLAVE)
    if (unsent.count() == 0) {
        timer.stop();
        return;
    }

    TestSlaveInterface *i = 0;
    if (QtopiaApplication::instance()) i = QtopiaApplication::instance()->testSlave();
    if (!i || !i->isConnected()) return;

    QVariantList vl;
    /* Send all unsent messages as a single QTestMessage, to keep overhead as small as possible */
    while (!unsent.isEmpty()) {
        QVariantMap map;
        QPerformanceLogData data = unsent.takeFirst();
        map["message"] = data.msg;
        map["event"] = (int)data.event;
        map["ident"] = data.ident;
        map["appTime"] = data.appTime;
        map["serverTime"] = data.serverTime;
        vl.append(map);
    }

    QVariantMap msgMap;
    msgMap["data"] = vl;
    i->postMessage("Performance", msgMap);

    timer.stop();
#endif
}

void QPerformanceLogPrivate::send( const QPerformanceLogData &data )
{
#ifndef QTOPIA_TEST_HOST
    if (QtopiaApplication::instance() && !QtopiaApplication::instance()->testSlave()) return;
    unsent << data;
    if (!timer.isActive() && qApp) {
        if (timer.thread() != qApp->thread()) timer.moveToThread(qApp->thread());
        timer.start(QPerformanceLogData::Timeout);
    }
#else
    Q_UNUSED(data);
#endif
}

/*!
  \class QPerformanceLog
  \brief The QPerformanceLog class implements a performance logging mechanism available to all Qtopia applications.
\if defined(QTOPIA_TEST)
  \ingroup qtopiatest_systemtest
\endif

  It provides a similar behaviour to qDebug() and qLog(), but every
  message automatically contains two timestamps: milliseconds since the current application
  has started, and milliseconds since the Qtopia Core window server has started.

  By default, messages are output via qLog(Performance), respecting the qLog() settings.

  If the QTOPIA_PERFTEST environment variable is set, messages will also be sent to
  a connected QtopiaTest system test (if any), and will always be output to the local
  console, overriding qLog() settings.

  Any string data can be output in a performance log.  To make log parsing easier,
  some predefined values are provided for events which are commonly of interest for
  performance testing.  These are represented by the QPerformanceLog::Event flags.

  The below example shows how this class can be used to measure the time for a specific task
  from within a system test.

  Example code residing in a Qtopia application named "Dog Walker":
  \code
    QPerformanceLog() << QPerformanceLog::Begin << "walk to park";
    // Outputs 'Dog Walker : <ms_since_appstart> : <ms_since_qpestart> : begin walk to park'
    while ( !at( Locations::Park ) ) {
        walk( directionOf(Locations::Park) );
    }
    QPerformanceLog() << QPerformanceLog::End << "walk to park";
    // Outputs 'Dog Walker : <ms_since_appstart> : <ms_since_qpestart> : end walk to park'
  \endcode

\if defined(QTOPIA_TEST)
  \sa QSystemTest
\endif
*/

/*!
    \enum QPerformanceLog::EventType

    This enum provides a simple way of logging common occurrences such as the
    beginning and ending of a particular task.  Each QPerformanceLog instance has
    an associated event value which is constructed by combining values from the following
    list using the bitwise OR operator:

    \value NoEvent          Log message is not related to any event described by QPerformanceLog::EventType.
    \value Begin            Log message signifies the beginning of a specific event.
    \value End              Log message signifies the end of a specific event.
    \value LibraryLoading   Log message is related to the loading of shared libraries.  This value is used internally by Qtopia.
    \value EventLoop        Log message is related to the application's global event loop.  This value is used internally by Qtopia.
    \value MainWindow       Log message is related to the construction of the application's main window.  This value is used internally by Qtopia.

    By streaming these enum values into a QPerformanceLog(), processing of performance
    data from within a system test is made easier.  In particular, using Begin and End,
    along with a unique identifying string for a particular event in your program,
    allows the amount of time taken for a particular event to be easily determined.
*/

/*!
    Construct a performance logger for application \a applicationName.
    If \a applicationName is empty, the name of the current application is used.
*/
QPerformanceLog::QPerformanceLog( QString const &applicationName )
{
    if (enabled()) {
        data = new QPerformanceLogData;
        data->event = NoEvent;
        data->ident = ((applicationName.isEmpty() && qApp) ? qApp->applicationName() : applicationName);
        clock_t now = times(&dummy);
        {
            static char warned = 0;
            if (start_time > now || *server_start_time > now && !warned) {
                warned = 1;
                qWarning("QPerformanceLog: start time seems to be in the future!");
            }
        }
        data->appTime = quint64((quint64(now) - quint64(start_time))*ticks_to_ms);
        data->serverTime = quint64((quint64(now) - quint64(*server_start_time))*ticks_to_ms);
    }
}

/*!
    Destroy the performance log object and output the performance data.
*/
QPerformanceLog::~QPerformanceLog()
{
    if (!enabled()) return;

#ifndef QTOPIA_TEST_HOST
    if (QPerformanceLogPrivate::instance() && data)
        QPerformanceLogPrivate::instance()->send( *data );
#endif

    if (data) {
        qDebug() << qPrintable(data->toString());
        delete data;
    }
}

/*!
    Returns true if QPerformanceLog is enabled.
    QPerformanceLog is enabled if either qLog(Performance) is enabled or the
    QTOPIA_PERFTEST environment variable is set to "1".
*/
bool QPerformanceLog::enabled()
{
    static bool ret = qLogEnabled(Performance) || (!qgetenv("QTOPIA_PERFTEST").isEmpty());
    return ret;
}

/*!
    Append \a string to the log message.
*/
QPerformanceLog &QPerformanceLog::operator<<(QString const &string) {
    if (enabled())
        data->msg += string + " ";
    return *this;
}

/*!
    Append \a event to the log message.
    If this log message already has an event, the new event will be equal to
    the old event bitwise OR'd with \a event.
*/
QPerformanceLog &QPerformanceLog::operator<<(Event const &event) {
    if (enabled())
        data->event |= event;
    return *this;
}

/*!
    Returns a string representation of \a event.
*/
QString QPerformanceLog::stringFromEvent(Event const &event)
{
    QString ret;
    if (event == NoEvent) return ret;

    if (event.testFlag(Begin))          ret += " begin";
    if (event.testFlag(End))            ret += " end";

    if (event.testFlag(LibraryLoading)) ret += " loading_libraries";
    if (event.testFlag(EventLoop))      ret += " event_loop";
    if (event.testFlag(MainWindow))     ret += " main_window_create";

    return ret.trimmed();
}

/*!
    \internal
    \deprecated
    Can't remove, BIC.
*/
void QPerformanceLog::adjustTimezone( QTime &preAdjustTime )
{
    Q_UNUSED(preAdjustTime);
}

