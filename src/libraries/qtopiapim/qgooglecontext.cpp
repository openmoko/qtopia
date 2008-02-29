/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
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
#include <qtopiasql.h>
#include "qgooglecontext_p.h"
#include "qappointment.h"
#include <QString>

#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>
#include <QTimer>
#include <QBuffer>
#include <QXmlDefaultHandler>
#include <QSettings>

// temporary
#include <QFile>

#include <QXmlSimpleReader>
#include <QXmlInputSource>

#include <qtopiahttp.h>
#include <qtopianamespace.h>
#include <qtopiaipcenvelope.h>
#ifdef Q_OS_WIN32
#include <process.h>
#else
#include <unistd.h>
#endif

/**************
 * XML PARSER
 *************/

class GoogleCalHandler : public QXmlDefaultHandler
{
public:
    // will add to this as implementation progresses
    GoogleCalHandler();
    ~GoogleCalHandler();

    const QList<QAppointment> &appointments() const { return mAppointments; }
    const QList<QUniqueId> &removedAppointments() const { return mRemoved; }

    bool characters ( const QString & ch );
    bool endDocument ();
    bool endElement ( const QString & namespaceURI, const QString & localName, const QString & qName );
    QString errorString () const;
    bool startDocument ();
    bool startElement ( const QString & namespaceURI, const QString & localName, const QString & qName, const QXmlAttributes & atts );

    QString name() const { return mName; }
private:
    enum Element {
        Entry,
        Title,
        Feed,
        Content,
        Recurrence,
        Id,
        Category,
        Reminder,
        When,
        Where,
        Published,
        Updated,
        Status,
        Unrecognized,
    };

    Element token(const QString &) const;
    static QUniqueId parseId(const QString &);
    static QDateTime parseDateTime(const QString &, const QTimeZone &);
    static QTimeZone parseTimeZone(const QString &);

    enum State {
        StartState,
        FeedState,
        EntryState,
        EventState
    };

    QString lastText;
    QString mName;
    QAppointment lastAppointment;
    State state;

    int ignoreDepth;
    bool removeCurrentAppointment;

    QList<QAppointment> mAppointments;
    QList<QUniqueId> mRemoved;
};

GoogleCalHandler::GoogleCalHandler()
{
    startDocument(); // does the same work as init state anyway.
}

GoogleCalHandler::~GoogleCalHandler()
{
}

bool GoogleCalHandler::startDocument()
{
    // reset variables.
    lastAppointment = QAppointment();
    lastText.clear();
    state = StartState;
    mAppointments.clear();
    ignoreDepth = 0;
    removeCurrentAppointment = false;

    // published and updated dates for feed as a whole?

    return true;
}

bool GoogleCalHandler::endDocument()
{
    return true;
}

bool GoogleCalHandler::endElement( const QString & namespaceURI, const QString & localName, const QString & qName )
{
    Q_UNUSED(namespaceURI);
    Q_UNUSED(localName);

    if (ignoreDepth) {
        ignoreDepth--;
        Q_ASSERT(ignoreDepth >= 0);
        return true;
    }
    Element t = token(qName);
    switch(t) {
        case Entry:
            if (removeCurrentAppointment) {
                mRemoved.append(lastAppointment.uid());
                removeCurrentAppointment = false;
            } else {
                mAppointments.append(lastAppointment);
            }
            lastAppointment = QAppointment();
            state = FeedState;
            break;
        case Feed:
            state = StartState;
        case Title:
            if (state == FeedState)
                mName = lastText;
            else
                lastAppointment.setDescription(lastText);
            break;
        case Content:
            lastAppointment.setNotes(lastText);
            break;
        case Recurrence:
            {
            // this is the MAIN TODO, seeing as once this works,
                /*
                   iCal format, is not compatible with vcard parsing.
                   however it may be similar enough that we can still use existing vcard
                   parser.

                   NOTE: RRULE format is different in iCal to vCard
               */
            /* Example RRULE text from recurrence section.
               can be given instead of when
               e.g.
                DTSTART;TZID=Australia/Brisbane:20060615T113000
                DURATION:PT1800S
                RRULE:FREQ=WEEKLY;INTERVAL=2;BYDAY=TH

                BEGIN:VTIMEZONE
                TZID:Australia/Brisbane
                X-LIC-LOCATION:Australia/Brisbane
                BEGIN:STANDARD
                TZOFFSETFROM:+1000
                TZOFFSETTO:+1000
                TZNAME:EST
                DTSTART:19700101T000000
                END:STANDARD
                END:VTIMEZONE
             */

                // pad out data to make it look like a vcal and get through our parser
                // qappointment also has some special ical handling in it.
                QByteArray data = "BEGIN:VCALENDAR\r\nVERSION:1.0\r\nBEGIN:VEVENT\r\n"
                    + lastText.toUtf8() + "END:VEVENT\r\nEND:VCALENDAR\r\n";

                QList<QAppointment> result = QAppointment::readVCalendarData(data.constData(), data.length());
                if (result.count() > 0) {
                    QAppointment a = result[0];
                    lastAppointment.setStart(a.start());
                    lastAppointment.setEnd(a.end());
                    lastAppointment.setRepeatRule(a.repeatRule());
                    lastAppointment.setFrequency(a.frequency());
                    lastAppointment.setRepeatUntil(a.repeatUntil());
                    lastAppointment.setWeekFlags(a.weekFlags());
                }
            }
            break;
        case Id:
            {
                QUniqueId u = parseId(lastText);
                lastAppointment.setUid(u);
                // a lot more redundency in a google id.
                lastAppointment.setCustomField("GoogleId", lastText);
            }
            break;
        case Published:
            {
                // TODO should support published/updated fields natively in PimRecords?
                QDateTime dt = parseDateTime(lastText, QTimeZone::utc());
                lastAppointment.setCustomField("GooglePublished", dt.toString(Qt::ISODate));
            }
            break;
        case Updated:
            {
                QDateTime dt = parseDateTime(lastText, QTimeZone::utc());
                lastAppointment.setCustomField("GoogleUpdated", dt.toString(Qt::ISODate));
            }
            break;
        case Category:
        case Reminder:
        case When:
        case Where:
        case Status:
            break;
        case Unrecognized:
            break;

    }
    return true;
}

bool GoogleCalHandler::startElement( const QString & namespaceURI, const QString & localName, const QString & qName, const QXmlAttributes & atts )
{
    Q_UNUSED(namespaceURI);
    Q_UNUSED(localName);
    // if we are in a tree we dont' recognize, ignore the lot of it.
    if (ignoreDepth) {
        ignoreDepth++;
        return true;
    }

    Element t = token(qName);
    switch(t) {
        case Entry:
            state = EntryState;
            break;
        case Feed:
            state = FeedState;
            break;
        case Category:
            if (atts.value("scheme").endsWith("#kind")
                    && atts.value("term").endsWith("#event"))
                state = EventState;
            // TODO other kinds of categories?
            break;
        case Status:
            // this is how deleted events are detected
            if (atts.value("value").endsWith("#event.canceled"))  {
                removeCurrentAppointment = true;
            }
            break;
        case Reminder:
            if (!atts.value("minutes").isEmpty()) {
                lastAppointment.setAlarm(atts.value("minutes").toInt(), QAppointment::Visible);
            } else if (!atts.value("hours").isEmpty()) {
                lastAppointment.setAlarm(atts.value("minutes").toInt()*60, QAppointment::Visible);
            }
            // TODO doesn't handle days or absolute time.
            break;
        case When:
            {
                QString st = atts.value("startTime");
                QString et = atts.value("endTime");
                if (st.isEmpty() || et.isEmpty()) {
                    ignoreDepth++;
                } else {
                    QTimeZone tz = parseTimeZone(st);
                    QDateTime start = parseDateTime(st, tz);
                    QDateTime end = parseDateTime(et, tz);
                    if (st.length() < 19)
                        lastAppointment.setAllDay(true);
                    lastAppointment.setTimeZone(tz);
                    lastAppointment.setStart(start);
                    lastAppointment.setEnd(end);
                }
            }
            break;
        case Where:
            lastAppointment.setLocation(atts.value("valueString"));
            break;
        case Title:
        case Content:
            if (atts.value("type") != "text") {
                ignoreDepth++;
            }
            break;
        case Recurrence:
        case Id:
        case Published:
        case Updated:
            if (state != EntryState && state != EventState) {
                ignoreDepth++;
                break;
            }
            break;
        case Unrecognized:
            ignoreDepth++;
    }
    return true;
}

bool GoogleCalHandler::characters( const QString & ch )
{
    if (!ignoreDepth)
        lastText = ch;
    return true;
}

GoogleCalHandler::Element GoogleCalHandler::token(const QString &name) const
{
    // takes state into account as well.
    switch (state) {
        case StartState:
            if (name == "feed") return Feed;
            break;
        case FeedState:
            if (name == "entry") return Entry;
            if (name == "feed") return Feed;
            if (name == "title") return Title;
            break;
        case EntryState:
        case EventState:
            if (name == "entry") return Entry;
            if (name == "id") return Id;
            if (name == "content") return Content;
            if (name == "title") return Title;
            if (name == "gd:recurrence") return Recurrence;
            if (name == "category") return Category;
            if (name == "gd:reminder") return Reminder;
            if (name == "gd:when") return When;
            if (name == "gd:where") return Where;
            if (name == "published") return Published;
            if (name == "updated") return Updated;
            if (name == "gd:eventStatus") return Status;
            break;
    }
    return Unrecognized;
}

QString GoogleCalHandler::errorString() const
{
    return QString();
}

QTimeZone GoogleCalHandler::parseTimeZone(const QString &)
{
    // TODO parse timezone part of strings sent.
    // for now, treat everything as local.
    return QTimeZone();
}

QDateTime GoogleCalHandler::parseDateTime(const QString &time, const QTimeZone &destZone)
{
    QDateTime result;
    // format year-month-dayThour:min:sec... we skip the rest, assuming is part of timezone.
    QTimeZone sourceZone = parseTimeZone(time);
    int year = time.mid(0,4).toInt();
    int month = time.mid(5,2).toInt();
    int day = time.mid(8,2).toInt();
    result.setDate(QDate(year, month, day));
    if (time.length() >= 19) {
        int hour = time.mid(11,2).toInt();
        int minute = time.mid(14,2).toInt();
        int second = time.mid(17,2).toInt();
        result.setTime(QTime(hour, minute, second));
    }
    if (destZone.isValid() && sourceZone.isValid())
        return destZone.convert(result, sourceZone);
    return result;
}

QUniqueId GoogleCalHandler::parseId(const QString &gid)
{// create and maintain list of uid->googleid mappings.
    QSqlQuery q(QPimSqlIO::database());
    if (!q.prepare("SELECT id, gid FROM googleid WHERE gid = :g")) {
        qWarning("failed to prepare main google id lookup: %s", (const char *)q.lastError().text().toLocal8Bit());
    }
    q.bindValue(":g", gid);
    if (!q.exec()) {
        qWarning("failed main google id lookup: %s", (const char *)q.lastError().text().toLocal8Bit());
        return QUniqueId();
    }
    if (q.next())
        return QUniqueId::fromUInt(q.value(0).toUInt());

    // TODO stolen from QAppointmentGCalIO.  Will later need to make sure all id creation mapping
    // goes through same code, rather than pimsqlio.
    // note uid's from two different devices (e.g. desktop/pda) will not match
    static QUuid appScope("672cd357-c984-40e2-b47d-ffde5a65137c");
    QUniqueIdGenerator g(appScope); // later, same scop method as xml
    QUniqueId u = g.createUniqueId();
    q.prepare("INSERT INTO googleid (id, gid) VALUES (:i, :g)");
    q.bindValue(":i", u.toUInt());
    q.bindValue(":g", gid);
    // TODO error handling for sql statements.
    if (!q.exec())
        qWarning("failed main google id store: %s", (const char *)q.lastError().text().toLocal8Bit());
    return u;
}
/**************
 * CONTEXT
 *************/

QGoogleCalendarContext::QGoogleCalendarContext(QObject *parent, QObject *access)
    : QAppointmentContext(parent)
{
    mAccess = qobject_cast<QAppointmentSqlIO *>(access);
    syncAccountList();
}

QIcon QGoogleCalendarContext::icon() const
{
    return QPimContext::icon(); // redundent, but will do for now.
}

QString QGoogleCalendarContext::description() const
{
    return tr("Appointments synced from Google Calendar");
}

QString QGoogleCalendarContext::title() const
{
    return tr("Google Calendar");
}

QString QGoogleCalendarContext::title(const QPimSource &source) const
{
    if (name(source.identity).isEmpty())
        return source.identity;
    return name(source.identity);
}

bool QGoogleCalendarContext::editable() const
{
    return false;
}

QSet<QPimSource> QGoogleCalendarContext::sources() const
{
    QMapIterator<QString, Account> it(mAccounts);
    QSet<QPimSource> list;
    while(it.hasNext()) {
        it.next();
        QPimSource s;
        s.context = id();
        s.identity = it.key();
        list.insert(s);
    }
    return list;
}

QUuid QGoogleCalendarContext::id() const
{
    // generated with uuidgen
    static QUuid u("672cd357-c984-40e2-b47d-ffde5a65137c");
    return u;
}

void QGoogleCalendarContext::setVisibleSources(const QSet<QPimSource> &set)
{
    syncAccountList();
    QMapIterator<QString, Account> it(mAccounts);
    QSet<int> show;
    QSet<int> hide;
    while(it.hasNext()) {
        it.next();
        QPimSource s;
        s.context = id();
        s.identity = it.key();
        int context = QPimSqlIO::sourceContext(s);
        if (set.contains(s)) {
            show.insert(context);
        } else {
            hide.insert(context);
        }
    }

    QSet<int> filter = mAccess->contextFilter();
    filter.unite(hide);
    filter.subtract(show);
    mAccess->setContextFilter(filter);
}

QSet<QPimSource> QGoogleCalendarContext::visibleSources() const
{
    QSet<int> filter = mAccess->contextFilter();
    QSet<QPimSource> list;
    QMapIterator<QString, Account> it(mAccounts);
    while(it.hasNext()) {
        it.next();
        QPimSource s;
        s.context = id();
        s.identity = it.key();
        int context = QPimSqlIO::sourceContext(s);
        if (!filter.contains(context))
            list.insert(s);
    }
    return list;
}

bool QGoogleCalendarContext::exists(const QUniqueId &id) const
{
    return !source(id).isNull();
}

QPimSource QGoogleCalendarContext::source(const QUniqueId &id) const
{
    if (!mAccess->exists(id))
        return QPimSource();

    int itemContext = mAccess->context(id);
    QMapIterator<QString, Account> it(mAccounts);
    while(it.hasNext()) {
        it.next();
        QPimSource s;
        s.context = QGoogleCalendarContext::id();
        s.identity = it.key();
        int context = QPimSqlIO::sourceContext(s);
        if (context == itemContext)
            return s;
    }
    return QPimSource();
}

bool QGoogleCalendarContext::updateAppointment(const QAppointment &appointment)
{
    if (mAccess)
        return mAccess->updateAppointment(appointment);
    return false;
}

bool QGoogleCalendarContext::removeAppointment(const QUniqueId &id)
{
    if (mAccess)
        return mAccess->removeAppointment(id);
    return false;
}

QUniqueId QGoogleCalendarContext::addAppointment(const QAppointment &appointment, const QPimSource &source)
{
    if (!mAccounts.keys().contains(source.identity) || source.context != id())
        return QUniqueId();
    if (mAccess)
        return mAccess->addAppointment(appointment, source, true);
    return QUniqueId();
}

bool QGoogleCalendarContext::removeOccurrence(const QUniqueId &original, const QDate &date)
{
    if (mAccess)
        return mAccess->removeOccurrence(original, date);
    return false;
}

QUniqueId QGoogleCalendarContext::replaceOccurrence(const QUniqueId &original, const QOccurrence &occurrence, const QDate& date)
{
    if (mAccess)
        return mAccess->replaceOccurrence(original, occurrence, date);
    return QUniqueId();
}

QUniqueId QGoogleCalendarContext::replaceRemaining(const QUniqueId &original, const QAppointment &r, const QDate& date)
{
    if (mAccess)
        return mAccess->replaceRemaining(original, r, date);
    return QUniqueId();
}
void QGoogleCalendarContext::syncAccountList()
{
    QSettings settings("Trolltech", "Calendar");
    settings.beginGroup("GoogleAccounts");
    QStringList accounts = settings.childGroups();
    mAccounts.clear();
    foreach(QString email, accounts) {
        settings.beginGroup(email);

        Account a;
        a.name = settings.value("name").toString();
        a.password = settings.value("password").toString();
        if (settings.contains("type"))
            a.type = (FeedType)settings.value("type").toInt();

        mAccounts.insert(email, a);
        settings.endGroup();
    }
}

void QGoogleCalendarContext::addAccount(const QString &email, const QString &password)
{
    syncAccountList();
    Account a;
    a.password = password;
    mAccounts.insert(email, a);
    saveAccount(email);
}

void QGoogleCalendarContext::saveAccount(const QString &email)
{
    Account a = mAccounts[email];
    QSettings settings("Trolltech", "Calendar");
    settings.beginGroup("GoogleAccounts");
    settings.beginGroup(email);
    settings.setValue("password", a.password);
    settings.setValue("name", a.name);
    settings.setValue("type", (int)a.type);
}

void QGoogleCalendarContext::removeAccount(const QString &email)
{
    syncAccountList();
    foreach (QGoogleCalendarFetcher *f, mFetchers) {
        if (f->account() == email)
            return;
    }
    mAccounts.remove(email);
    QSettings settings("Trolltech", "Calendar");
    settings.beginGroup("GoogleAccounts");
    settings.remove(email);


    // TODO should also clear any remaining items from SQL database
    // might be best to create a 'worker' thread to do the delete?
}

QStringList QGoogleCalendarContext::accounts() const
{
    return mAccounts.keys();
}

QString QGoogleCalendarContext::password(const QString &email) const
{
    if (mAccounts.contains(email))
        return mAccounts[email].password;
    return QString();
}

QGoogleCalendarContext::FeedType QGoogleCalendarContext::feedType(const QString &email) const
{
    if (mAccounts.contains(email))
        return mAccounts[email].type;
    return FullPrivate;
}

QString QGoogleCalendarContext::name(const QString &email) const
{
    if (mAccounts.contains(email))
        return mAccounts[email].name;
    return QString();
}

void QGoogleCalendarContext::setPassword(const QString &email, const QString &password)
{
    if (!mAccounts.contains(email))
        return;
    Account a = mAccounts[email];
    a.password = password;
    mAccounts[email] = a;
    saveAccount(email);
}

void QGoogleCalendarContext::setFeedType(const QString &email, FeedType type)
{
    if (!mAccounts.contains(email))
        return;
    Account a = mAccounts[email];
    a.type = type;
    mAccounts[email] = a;
    saveAccount(email);
}

void QGoogleCalendarContext::syncAccount(const QString &email)
{
    if (!mAccounts.contains(email))
        return;

    // don't sync already working account.
    foreach (QGoogleCalendarFetcher *f, mFetchers) {
        if (f->account() == email)
            return;
    }
    QPimSource s;
    s.context = id();
    s.identity = email;

    QString url = "http://www.google.com/calendar/feeds/" + email;

    switch (mAccounts[email].type) {
        case FullPrivate:
            url += "/private/full";
            break;
        case FullPublic:
            url += "/public/full";
            break;
        case FreeBusyPublic:
            url += "/public/free-busy";
            break;
    }


    QGoogleCalendarFetcher *f = new QGoogleCalendarFetcher(s, email, mAccounts[email].password, url, this);

    connect(f, SIGNAL(fetchProgressChanged(int, int)), this, SLOT(updateFetchingProgress(int, int)));
    connect(f, SIGNAL(completed(QGoogleCalendarContext::Status)), this, SLOT(updateFetchingState(QGoogleCalendarContext::Status)));

    mFetchers.append(f);
    f->fetch();
}

void QGoogleCalendarContext::syncAllAccounts()
{
    syncAccountList();
    QMapIterator<QString, Account> it(mAccounts);
    while(it.hasNext()) {
        it.next();
        syncAccount(it.key());
    }
}

void QGoogleCalendarContext::updateFetchingState(Status status)
{
    Q_UNUSED(status);
    QGoogleCalendarFetcher *f = qobject_cast<QGoogleCalendarFetcher *>(sender());
    if (f) {
        mFetchers.removeAll(f);
        if (!f->name().isEmpty() && name(f->account()) != f->name()) {
            QString email = f->account();
            Account a = mAccounts[email];
            a.name = f->name();
            mAccounts[email] = a;
            saveAccount(email);
        }
        mAccess->refresh(); // e.g. from adds from the fetcher, which is a separate io.
        emit syncStatusChanged(f->account(), status);
        f->deleteLater();
    }
    if (mFetchers.count() == 0) {
        emit finishedSyncing();
    }
}

void QGoogleCalendarContext::updateFetchingProgress(int, int)
{
    int amount, total;
    syncProgress(amount, total);
}

void QGoogleCalendarContext::syncProgress(int &amount, int &total) const
{
    amount = 0;
    total = 0;
    foreach(QGoogleCalendarFetcher *f, mFetchers) {
        int a, t;
        f->fetchProgress(a, t);
        if (t < a || t == 0) {
            amount = 0; t = 0; return;
        }
        amount += a;
        total += t;
    }
}


bool QGoogleCalendarContext::syncing() const
{
    return mFetchers.count() > 0;
}


QGoogleCalendarFetcher::QGoogleCalendarFetcher(const QPimSource &sqlContext, const QString &email, const QString &password, const QString &url, QObject *parent)
    : QObject(parent)
    , lastProgress(0), lastTotal(0)
    , mState(IdleState), mStatus(QGoogleCalendarContext::NotStarted) , mUrl(url)
    , mContext(sqlContext), mAccount(email), mPassword(password)
    , xmlReader(0), mSource(0), mHandler(0), mDownloader(0), mSslSocket(0), mAccess(0)
{
    syncTime = QTimeZone::current().toUtc(QDateTime::currentDateTime());
    lastSyncTime = QPimSqlIO::lastSyncTime(mContext);
    if (lastSyncTime.isValid())
        mUrl.addQueryItem("updated-min", lastSyncTime.toString(Qt::ISODate) + "Z");

    // later, create all of this in thread, not constructor.
    // creates own access as is only using it for adding.
    // hence will be simpler when made threaded and will
    // avoid stacked calls to reset.
    mAccess = new QAppointmentSqlIO(this);

    mHandler = new GoogleCalHandler;
    xmlReader = new QXmlSimpleReader;

    xmlReader->setContentHandler(mHandler);

    mDownloader = new QtopiaHttp(this);

    mSslSocket = new QtSslSocket(QtSslSocket::Client, this);
    connect(mSslSocket, SIGNAL(connectionVerificationDone(QtSslSocket::VerifyResult, bool, const QString &)),
            SLOT(sendCertificateError(QtSslSocket::VerifyResult, bool, const QString &)));
    mDownloader->setSslSocket(mSslSocket);


    // later need separate slot for failed fetch.. Probably should retain existing
    // data on a failed fetch.
    connect(mDownloader, SIGNAL(completedFetch()), this, SLOT(parseRemaining()));
    connect(mDownloader, SIGNAL(failedFetch()), this, SLOT(abortParsing()));
    connect(mDownloader, SIGNAL(readyRead(const QHttpResponseHeader &)), this, SLOT(parsePartial(const QHttpResponseHeader &)));

    connect(mDownloader, SIGNAL(dataReadProgress(int, int)), this, SLOT(httpFetchProgress(int, int)));
}

QGoogleCalendarContext::Status QGoogleCalendarFetcher::status() const
{
    return mStatus;
}

QString QGoogleCalendarFetcher::account() const
{
    return mAccount;
}

QString QGoogleCalendarFetcher::name() const
{
    if (mHandler)
        return mHandler->name();
    return QString();
}

QString QGoogleCalendarFetcher::statusMessage() const
{
    return QGoogleCalendarContext::statusMessage(status());

}

QString QGoogleCalendarContext::statusMessage(Status status) {

    switch(status) {
        case QGoogleCalendarContext::NotStarted:
        case QGoogleCalendarContext::Completed:
            return QString();
        case QGoogleCalendarContext::InProgress:
            return tr("Downloading appointments.");
        case QGoogleCalendarContext::BadAuthentication:
            return tr("Invalid email or password.");
        case QGoogleCalendarContext::NotVerified:
        case QGoogleCalendarContext::TermsNotAgreed:
            return tr("Account not verified.  Please log into your account via "
                    "web browser to resolve this issue.");
        case QGoogleCalendarContext::CaptchaRequired:
            return tr("A CAPTCHA is required to unlock this account.  Please access"
                    " https://www.google.com/accounts/DisplayUnlockCaptcha"
                    " using a web-browser of your choice to re-enable the account.");
        case QGoogleCalendarContext::AccountDeleted:
            return tr("Attempted to access deleted account.");
        case QGoogleCalendarContext::AccountDisabled:
            return tr("Attempted to access disabled account.");
        case QGoogleCalendarContext::ServiceUnavailable:
            return tr("Authentication currently not available.  Please try again later.");
        case QGoogleCalendarContext::UnknownError:
        default:
            return tr("An unknown error has occurred.");
    }
}

void QGoogleCalendarFetcher::fetch()
{
    if (!mAccess)
        return;

    if (mPassword.isEmpty() || !mAuth.isEmpty())
        // assume public stream and that authentication is not required.
        fetchAppointments();
    else
        fetchAuthentication();
}

void QGoogleCalendarFetcher::fetchAuthentication()
{
    static const char *loginurl = "https://www.google.com/accounts/ClientLogin";

    if (mState != IdleState)
        return;
    mStatus = QGoogleCalendarContext::InProgress;
    mState = AuthenticationState;

    QList< QPair<QString, QString> > postData;
    postData.append(QPair<QString, QString>("Email", mAccount));
    postData.append(QPair<QString, QString>("Passwd", mPassword));
    postData.append(QPair<QString, QString>("source", "Trolltech-Calendar-4.2.0"));
    postData.append(QPair<QString, QString>("service", "cl"));

#if 0
    if (!captchaText->text().isEmpty()) {
        postData.append(QPair<QString, QString>("logintoken", mCaptchaToken));
        postData.append(QPair<QString, QString>("logincaptcha", captchaText->text()));
    }
#endif

    mDownloader->startFetch(QUrl(loginurl), postData);
}

void QGoogleCalendarFetcher::sendCertificateError(QtSslSocket::VerifyResult result, bool hostNameWrong, const QString &str)
{
    return; // ignore all this for now.
    if (result == QtSslSocket::VerifyOk && !hostNameWrong)
        return;

    QStringList certStr = mSslSocket->peerCertificate().split('/', QString::SkipEmptyParts);
    QMap<QString, QString> cert;
    QStringListIterator it(certStr);
    while (it.hasNext()) {
        QStringList pair = it.next().split('=', QString::SkipEmptyParts);
        cert.insert(pair[0], pair.size() > 1 ? pair[1] : QString());
    }

    mState = ErrorState;
    mStatus = QGoogleCalendarContext::CertificateError;
    emit certificateError(cert, str);

    // non-recoverable.  We don't open a gui to ask the user if its ok in this thread.
    // minor-missing feature that it doesn't wait/pause while authentication is requested,
    // TODO
    // should be up to app to handle, not us.
    //mSslSocket->disconnectFromHost();
}

void QGoogleCalendarFetcher::fetchAppointments()
{
    if (mState != IdleState)
        return;
    mStatus = QGoogleCalendarContext::InProgress;
    mState = AppointmentDownloadState;

    if (!mAuth.isEmpty())
        mDownloader->setAuthorization(mAuth);

    mDownloader->startFetch(mUrl);
}

#define FAIL_TRANSACTION mStatus = QGoogleCalendarContext::UnknownError; \
                    mState = ErrorState; \
                    emit completed(mStatus); \
                    delete mSource; \
                    mSource = 0;

void QGoogleCalendarFetcher::parseRemaining()
{
    if (mState == AuthenticationState) {
        QByteArray ba = mDownloader->readAll();
        QTextStream ts(&ba);
        QString text = ts.readLine();
        while (!text.isEmpty()) {
            int pos = text.indexOf('=');
            QString key = text.left(pos);
            QString value = text.mid(pos+1);
            if (key == "Auth")
                mAuth = "GoogleLogin " + text;
            text = ts.readLine();
        }
        mState = IdleState;
        mStatus = QGoogleCalendarContext::InProgress;
        fetchAppointments();
    } else if (mState == AppointmentDownloadState) {
        bool parsed;
        if (mSource) {
            QByteArray ba = mDownloader->readAll();
            if (ba.size() > 0) {
                mSource->setData(ba);
                parsed = xmlReader->parseContinue();
            } else {
                parsed = true;
            }
        } else {
            QByteArray ba = mDownloader->readAll();
            if (ba.size() > 0) {
                mSource->setData(ba);
                mSource = new QXmlInputSource();
                mSource->setData(ba);
                parsed = xmlReader->parse(mSource, false);
            } else {
                parsed = false;
            }
            mSource = new QXmlInputSource();
            parsed = xmlReader->parse(mSource, false);
        }

        if (parsed) {
            if (!mAccess->startSync(mContext, syncTime)) {
                FAIL_TRANSACTION;
            }

            foreach (QAppointment a, mHandler->appointments()) {
                // should also set updated time.
                // does its own transactioning... um, they probably don't stack.
                if (mAccess->exists(a.uid())) {
                    if (!mAccess->updateAppointment(a)) {
                        mAccess->abortSync();
                        FAIL_TRANSACTION;
                        return;
                    }
                } else {
                    if (mAccess->addAppointment(a, mContext, false).isNull()) {
                        mAccess->abortSync();
                        FAIL_TRANSACTION;
                        return;
                    }
                }
            }
            foreach (QUniqueId id, mHandler->removedAppointments()) {
                if (!mAccess->exists(id))
                    continue; // an appointment may have been created, then delted before syncing.
                if (!mAccess->removeAppointment(id)) {
                    mAccess->abortSync();
                    FAIL_TRANSACTION;
                    return;
                }
            }
            if (!mAccess->commitSync()) {
                mAccess->abortSync();
                FAIL_TRANSACTION;
                return;
            }

            mStatus = QGoogleCalendarContext::Completed;
            mState = IdleState;
            emit completed(mStatus);
        } else {
            mStatus = QGoogleCalendarContext::ParseError;
            mState = ErrorState;
            emit completed(mStatus);
        }


        delete mSource;
        mSource = 0;
    }
}

void QGoogleCalendarFetcher::parsePartial(const QHttpResponseHeader & /* resp */)
{
    if (mState != AppointmentDownloadState)
        return;
    bool ok;
    if (mSource) {
        mSource->setData(mDownloader->readAll());
        ok = xmlReader->parseContinue();
    } else {
        mSource =new QXmlInputSource();
        mSource->setData(mDownloader->readAll());
        ok = xmlReader->parse(mSource, true);
    }
    if (!ok) {
        qWarning("Failed to parse partial response");
    }
}

void QGoogleCalendarFetcher::abortParsing()
{
    if (mState == AuthenticationState) {
        QString errorId;
        QString captchaToken;
        QString captchaURL;

        QByteArray ba = mDownloader->readAll();
        QTextStream ts(&ba);
        QString text = ts.readLine();
        while (!text.isEmpty()) {
            int pos = text.indexOf('=');
            QString key = text.left(pos);
            QString value = text.mid(pos+1);
            if (key == "Error") {
                errorId = value;
            } else if (key == "CaptchaURL") {
                captchaURL = value;
            } else if (key == "CaptchaToken") {
                captchaToken = value;
            }
            text = ts.readLine();
        }

        // switch on error text
        QString userMessage;
        if (errorId == "BadAuthentication") {
            mStatus = QGoogleCalendarContext::BadAuthentication;
        } else if (errorId == "NotVerified") {
            mStatus = QGoogleCalendarContext::NotVerified;
        } else if (errorId == "TermsNotAgreed") {
            mStatus = QGoogleCalendarContext::TermsNotAgreed;
        } else if (errorId == "CaptchaRequired") {
            mStatus = QGoogleCalendarContext::CaptchaRequired;
        } else if (errorId == "AccountDeleted") {
            mStatus = QGoogleCalendarContext::AccountDeleted;
        } else if (errorId == "AccountDisabled") {
            mStatus = QGoogleCalendarContext::AccountDisabled;
        } else if (errorId == "ServiceUnavailable") {
            mStatus = QGoogleCalendarContext::ServiceUnavailable;
        } else {
            mStatus = QGoogleCalendarContext::UnknownError;
        }
    } else {
        mStatus = QGoogleCalendarContext::UnknownError;
    }
    mState = ErrorState;
    emit completed(mStatus);
    delete mSource;
    mSource = 0;
}

void QGoogleCalendarFetcher::fetchProgress(int &p, int &t) const
{
    p = lastProgress;
    t = lastTotal;
}

void QGoogleCalendarFetcher::httpFetchProgress(int progress, int total)
{
    lastProgress = progress;
    lastTotal = total;
    emit fetchProgressChanged(progress, total);
}
