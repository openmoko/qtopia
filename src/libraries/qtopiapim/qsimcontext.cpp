/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
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

#include <qsimcontext_p.h>
#include <qsiminfo.h>
#include <qcontactsqlio_p.h>
#include <qtopialog.h>

#include <QDebug>
/***************
 * CONTEXT
 **************/

struct ExtensionMap
{
    QContactModel::Field type;
    const char * text;
};

// these are english versions.  should translate?
// also, first ones are Qtopia export extensions.  Could add
// another list for fuzzy matching.
// extensions the same length for label consistency
static const ExtensionMap SIMextensions[] = {
    { QContactModel::HomePhone, "/hp" }, // no tr
    { QContactModel::HomeMobile, "/hm" }, // no tr
    { QContactModel::HomeFax, "/hf" }, // no tr
    { QContactModel::BusinessPhone, "/bp" }, // no tr
    { QContactModel::BusinessMobile, "/bm" }, // no tr
    { QContactModel::BusinessFax, "/bf" }, // no tr
    { QContactModel::BusinessPager, "/bg" }, // no tr
    { QContactModel::Invalid, 0 }
};

static const ExtensionMap SIMRegExpExtensions[] = {
    { QContactModel::HomePhone, "[ /](h|hp|home)$" }, // no tr
    { QContactModel::HomeMobile, "[ /](m|hm|mob)$" }, // no tr
    { QContactModel::HomeFax, "[ /](o|other)$" }, // no tr
    { QContactModel::BusinessPhone, "[ /](b|w|wk|bp|wp|work)$" }, // no tr
    { QContactModel::BusinessFax, "[ /](bo|wo)$" }, // no tr
    { QContactModel::BusinessMobile, "[ /](bm|wm)$" }, // no tr
    { QContactModel::BusinessPager, "[ /](bpa|wpa)$" }, // no tr
    { QContactModel::Invalid, 0 }
};

QContactSimContext::QContactSimContext(QObject *parent, QObject *access)
    : QContactContext(parent), readState(0),
    SIMLabelLimit(20), SIMNumberLimit(60),
    SIMListStart(1), SIMListEnd(200),
    readingSim(true)
{
    QtopiaSql::ensureSchema("simcardidmap", QtopiaSql::systemDatabase(), true);

    mAccess = qobject_cast<ContactSqlIO *>(access);
    Q_ASSERT(mAccess);

    mSimInfo = new QSimInfo( QString(), this );
    connect( mSimInfo, SIGNAL(inserted()), this, SLOT(simIdentityChanged()) );
    connect( mSimInfo, SIGNAL(removed()), this, SLOT(simIdentityChanged()) );

    mPhoneBook = new QPhoneBook( QString(), this );

    connect( mPhoneBook, SIGNAL(entries(const QString&, const QList<QPhoneBookEntry>&)),
            this, SLOT(updatePhoneBook(const QString&, const QList<QPhoneBookEntry>&)) );
    connect( mPhoneBook, SIGNAL(limits(const QString&, const QPhoneBookLimits&)),
            this, SLOT(updatePhoneBookLimits(const QString&, const QPhoneBookLimits&)) );

    mPhoneBook->getEntries();
}

QIcon QContactSimContext::icon() const
{
    return QPimContext::icon(); // redundent, but will do for now.
}

QString QContactSimContext::description() const
{
    return tr("SIM Card Contact Access");
}

QString QContactSimContext::title() const
{
    return tr("SIM Card Contact Access");
}

bool QContactSimContext::editable() const
{
    return !readingSim;
}

bool QContactSimContext::editable(const QUniqueId &id) const
{
    return (!readingSim && cardIndex(id) != -1 && source(id) == defaultSource());
}

QPimSource QContactSimContext::defaultSource() const
{
    QPimSource s;
    s.context = id();
    s.identity = "sim";
    return s;
}

QSet<QPimSource> QContactSimContext::sources() const
{
    QSet<QPimSource> set;
    if (!mActiveCard.isEmpty())
        set.insert(defaultSource());
    return set;
}

QUuid QContactSimContext::id() const
{
    static QUuid u("b63abe6f-36bd-4bb8-9c27-ece5436a5130");
    return u;
}

QString QContactSimContext::title(const QPimSource &source) const
{
    if (!readingSim && source == defaultSource())
        return tr("Active SIM Card");
    QDateTime sync = QPimSqlIO::lastSyncTime(source);
    return tr("Cached SIM Card - %1").arg(sync.toString());

}

/* TODO set mapping to int */
void QContactSimContext::setVisibleSources(const QSet<QPimSource> &set)
{
    QSet<int> show;
    QSet<int> hide;
    QSet<QPimSource> list = sources();
    foreach (QPimSource s, list) {
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

QSet<QPimSource> QContactSimContext::visibleSources() const
{
    QSet<int> filter = mAccess->contextFilter();
    QSet<QPimSource> result;

    QSet<QPimSource> list = sources();
    foreach (QPimSource s, list) {
        int context = QPimSqlIO::sourceContext(s);
        if (!filter.contains(context))
            result.insert(s);
    }
    return result;
}

bool QContactSimContext::exists(const QUniqueId &id) const
{
    return !source(id).isNull();
}

QPimSource QContactSimContext::source(const QUniqueId &id) const
{
    int itemContext = mAccess->context(id);
    QSet<QPimSource> list = sources();
    foreach(QPimSource s, list) {
        int context = QPimSqlIO::sourceContext(s);
        if (context == itemContext)
            return s;
    }
    return QPimSource();
}

bool QContactSimContext::updateContact(const QContact &contact)
{
    if (editable(contact.uid())) {
        QString label = contact.firstName();
        QString number = contact.homePhone();
        if (label.length() > SIMLabelLimit || number.length() > SIMNumberLimit)
            return false;
        if (mAccess->updateContact(contact)) {
            int i = cardIndex(contact.uid());
            QPhoneBookEntry entry;
            entry.setIndex(i);
            entry.setText(label);
            entry.setNumber(number);
            mPhoneBook->update(entry);
            return true;
        }
    }
    return false;
}

bool QContactSimContext::removeContact(const QUniqueId &id)
{
    if (editable(id)) {
        if (mAccess->removeContact(id)) {
            int i = cardIndex(id);
            mPhoneBook->remove(i);
            mPhoneBook->flush();
        }
    }
    return false;
}

QUniqueId QContactSimContext::addContact(const QContact &contact, const QPimSource &source)
{
    if (source != defaultSource() || readingSim)
        return QUniqueId();

    QContact c;
    c.setFirstName(contact.firstName());
    c.setHomePhone(contact.firstName());

    int i = nextFreeIndex();
    QUniqueId u = id(source.identity, i);
    c.setUid(u);
    // round trip via sim access?
    if (!mAccess->addContact(c, source, false).isNull()) {
        QPhoneBookEntry entry;
        entry.setIndex( i );
        entry.setNumber( c.homePhone() );
        entry.setText( c.firstName() );
        mPhoneBook->add(entry);
        return u;
    }
    return QUniqueId();
}

bool QContactSimContext::waitingOnSim() const
{
    return false;
}

QString QContactSimContext::card(const QUniqueId &u) const
{
    QSqlQuery q;
    if (!q.prepare("SELECT cardid FROM simcardidmap WHERE sqlid = :s")) {
        qWarning("Failed to prepare card id lookup: %s",
                q.lastError().text().toLocal8Bit().constData());
        return QString();
    }
    q.bindValue(":s", u.toByteArray());
    if (!q.exec()) {
        qWarning("Failed card id lookup: %s",
                q.lastError().text().toLocal8Bit().constData());
        return QString();
    }
    if (q.next())
        return q.value(0).toString();
    return QString();
}

int QContactSimContext::cardIndex(const QUniqueId &u) const
{
    QLocalUniqueId lu(u);
    QSqlQuery q;
    if (!q.prepare("SELECT cardindex FROM simcardidmap WHERE sqlid = :s")) {
        qWarning("Failed to prepare card index lookup: %s",
                q.lastError().text().toLocal8Bit().constData());
        return -1;
    }
    q.bindValue(":s", lu.toByteArray());
    QtopiaSql::logQuery(q);
    if (!q.exec()) {
        qWarning("Failed card index lookup: %s",
                q.lastError().text().toLocal8Bit().constData());
        return -1;
    }
    if (q.next())
        return q.value(0).toInt();
    return -1;
}

QUniqueId QContactSimContext::id(const QString &card, int index) const
{
    QSqlQuery q;
    if (!q.prepare("SELECT sqlid FROM simcardidmap WHERE cardid = :c AND cardindex = :i")) {
        qWarning("Failed to prepare sim card id lookup: %s", (const char *)q.lastError().text().toLocal8Bit());
    }
    q.bindValue(":c", card);
    q.bindValue(":i", index);
    if (!q.exec()) {
        qWarning("Failed sim card id lookup: %s", (const char *)q.lastError().text().toLocal8Bit());
        return QUniqueId();
    }

    if (q.next())
        return QUniqueId(q.value(0).toByteArray());

    static QUuid appScope("b63abe6f-36bd-4bb8-9c27-ece5436a5130");
    QUniqueIdGenerator g(appScope); // later, same scop method as xml
    QLocalUniqueId u = g.createUniqueId();
    q.prepare("INSERT INTO simcardidmap (sqlid, cardid, cardindex) VALUES (:s, :c, :i)");
    q.bindValue(":s", u.toByteArray());
    q.bindValue(":c", card);
    q.bindValue(":i", index);

    if (!q.exec())
        qWarning("Failed sim card id update: %s", (const char *)q.lastError().text().toLocal8Bit());
    return u;
}

int QContactSimContext::nextFreeIndex() const
{
    QSqlQuery q;
    if (!q.prepare("SELECT cardindex FROM simcardidmap WHERE cardid = :c ORDER BY cardindex"))
    {
        qWarning("Failed to prepare used index lookup: %s",
                q.lastError().text().toLocal8Bit().constData());
        return -1;
    }
    q.bindValue(":c", mActiveCard);
    if (!q.exec()) {
        qWarning("Failed used index lookup: %s",
                q.lastError().text().toLocal8Bit().constData());
        return -1;
    }

    int index = SIMListStart;
    while (q.next()) {
        int pos = q.value(0).toInt();
        if (pos != index)
            break;
        index ++;
    }
    if (index > SIMListEnd)
        return -1;
    return index;
}

QContact QContactSimContext::contact(const QPhoneBookEntry &entry) const
{
    QContact c;
    if (readingSim)
        return QContact(); // don't pollute generator.

    c.setUid(id(mActiveCard, entry.index()));
    c.setFirstName(entry.text());
    c.setHomePhone(entry.number());
    c.setCustomField("sim_index", QString::number(entry.index()));
    return c;
}
bool QContactSimContext::isSIMContactCompatible(const QContact &c) const
{
    if ( !c.homePhone().isEmpty() ||
         !c.homeMobile().isEmpty() ||
         !c.homeFax().isEmpty() ||
         !c.businessPhone().isEmpty() ||
         !c.businessMobile().isEmpty() ||
         !c.businessFax().isEmpty() ||
         !c.businessPager().isEmpty() )
        return true;
    return false;
}

QString QContactSimContext::typeToSIMExtension(QContactModel::Field type)
{
    const ExtensionMap *i = SIMextensions;
    while(i->type != QContactModel::Invalid) {
        if (i->type == type)
            return i->text;
        ++i;
    }
    return QString();
}

QContactModel::Field QContactSimContext::SIMExtensionToType(QString &label)
{
    // doesn't guess at ones we don't write.
    const ExtensionMap *i = SIMextensions;
    QString llabel = label.toLower();
    while(i->type != QContactModel::Invalid) {
        QString e(i->text);
        if (llabel.right(e.length()) == e) {
            label = label.left(label.length() - e.length());
            return i->type;
        }
        ++i;
    }
    // failed of the Qtopia map, try some regexp.
    i = SIMRegExpExtensions;
    while(i->type != QContactModel::Invalid) {
        QRegExp re(i->text);
        if (re.indexIn(llabel) != -1) {
            label = label.left(label.length() - re.matchedLength());
            return i->type;
        }
        ++i;
    }

    return QContactModel::Invalid;
}

QString QContactSimContext::createSIMLabel(const QContact &c)
{
    QString label = c.firstName();
    if (label.isEmpty())
        label = c.label(); // should ask for 'short' label.
    return label;
}

void QContactSimContext::simIdentityChanged()
{
    QString value = mSimInfo->identity();
    mActiveCard = value;

    if( value.isEmpty() ) {
        updateSqlEntries();
        return;
    }

    readState |= PhoneBookIdRead;

    if (readState == PhoneBookRead)
        updateSqlEntries();
}

void QContactSimContext::updatePhoneBook( const QString &store, const QList<QPhoneBookEntry> &list )
{
    Q_UNUSED(store);
    readState |= PhoneBookEntriesRead;
    // once phone book is known to be initialized, can get limits.
    if (!(readState & PhoneBookLimitsRead))
        mPhoneBook->requestLimits();

    // also request the sim card's id.
    if( mActiveCard.isNull() )
        simIdentityChanged();

    phoneData = list;

    if (readState == PhoneBookRead)
        updateSqlEntries();
}

void QContactSimContext::updatePhoneBookLimits( const QString &store, const QPhoneBookLimits &value )
{
    Q_UNUSED(store);
    SIMNumberLimit = value.numberLength();
    SIMLabelLimit = value.textLength();
    SIMListStart = value.firstIndex();
    SIMListEnd = value.lastIndex();

    readState |= PhoneBookLimitsRead;
    if (readState == PhoneBookRead)
        updateSqlEntries();
}

void QContactSimContext::updateSqlEntries()
{
    QPimSource s = defaultSource();
    int c = QPimSqlIO::sourceContext(s);

    readingSim = false;

    QDateTime syncTime = QTimeZone::current().toUtc(QDateTime::currentDateTime());
    if (mAccess->startSync(s, syncTime)) {
        QList<QUniqueId> existing;
        QSqlQuery q;
        q.prepare("SELECT recid FROM contacts WHERE context = :c");
        q.bindValue(":c", c);
        q.exec();
        while(q.next())
            existing.append(QUniqueId(q.value(0).toByteArray()));
        q.clear();

        foreach(QPhoneBookEntry e, phoneData) {
            QContact c = contact(e);
            QUniqueId u = c.uid();
            if (existing.contains(u)) {
                existing.removeAll(u);
                // only update if different.;
                QContact e = simContact(u);
                if (e.firstName() != c.firstName() || e.homePhone() != c.homePhone()) {
                    if (!mAccess->updateContact(c)) {
                        mAccess->abortSync();
                        return;
                    }
                }
            } else {
                if (mAccess->addContact(c, s, false).isNull()) {
                    mAccess->abortSync();
                    return;
                }
            }
        }

        // covers cards not from this sim, will have different id's.
        foreach(QUniqueId u, existing) {
            if (!mAccess->removeContact(u)) {
                mAccess->abortSync();
                return;
            }
        }

        if (!mAccess->commitSync())
            mAccess->abortSync();
        phoneData.clear();
    }
}

// Get only first name and home phone fields.
QContact QContactSimContext::simContact(const QUniqueId &u) const
{
    if (u.isNull())
        return QContact();

    QSqlQuery q;
    q.prepare("SELECT firstname FROM contacts WHERE recid = :i");
    const QLocalUniqueId &lid = (const QLocalUniqueId &)u;
    QByteArray uid = lid.toByteArray();
    q.bindValue(":i", uid);

    QContact t;

    q.setForwardOnly(true);
    if (!q.exec()) {
        qWarning("failed to select sim contact: %s", (const char *)q.lastError().text().toLocal8Bit());
        return t;
    }

    if ( q.next() ) {
        qLog(Sql) << "Read homephone number";
        QSqlQuery q2;
        if (!q2.prepare("SELECT phone_number from contactphonenumbers where recid=:id and phone_type=1")) // phone_type 1 == HomePhone
            qWarning("Failed to select contact homephone number: %s", q2.lastError().text().toLocal8Bit().constData());
        q2.bindValue(":id", uid);
        if (!q2.exec()) {
            qWarning("select phone numbers failed: %s", (const char *)q2.lastError().text().toLocal8Bit());
        }
        if(q2.next()) {
            QString number;
            number = q2.value(0).toString();
            qLog(Sql) << "set homephone number" << number;
            t.setPhoneNumber(QContact::HomePhone, number);
        }

        t.setUid(u);
        t.setFirstName(q.value(0).toString());
        // Don't bother caching this entry - no point caching enties that
        // won't be displayed.
    }

    return t;
}


QUniqueId QContactSimContext::findLabel(const QString &test) const
{
    int context = QPimSqlIO::sourceContext(defaultSource());
    QSqlQuery q;
    Q_ASSERT(q.prepare("SELECT recid FROM contacts WHERE context = :c AND firstname = :fn"));
    q.bindValue(":c", context);
    q.bindValue(":fn", test);
    Q_ASSERT(q.exec());
    if (q.next())
        return QUniqueId(q.value(0).toByteArray());
    return QUniqueId();
}

bool QContactSimContext::importContacts(const QPimSource &s, const QList<QContact> &list)
{
    if (readingSim || s != defaultSource())
        return false;

    foreach(QContact contact, list) {
        // check isn't already a special contact
        QMap<QString, QString> newContacts;
        QList<QContact> updateContacts;

        bool untypedLabelUpdated = false;
        // separate out phone numbers, and add one contact per phone number.
        // if exists, will update, not add.
        QList<QContactModel::Field> pFields = QContactModel::phoneFields();
        foreach(QContactModel::Field f, pFields) {
            QString v = QContactModel::contactField(contact, f).toString().simplified();
            if (!v.isEmpty()) {
                QString label;
                QUniqueId oldpos;
                if (contact.phoneNumbers().count() == 1) {
                    label = contact.label().left(SIMLabelLimit-3);
                    oldpos = findLabel(label);
                } else {
                    label = contact.label().left(SIMLabelLimit-3) + typeToSIMExtension(f);
                    oldpos = findLabel(label);
                    if (oldpos.isNull() && !untypedLabelUpdated) {
                        oldpos = findLabel(contact.label().left(SIMLabelLimit-3));
                        if (!oldpos.isNull())
                            untypedLabelUpdated = true;
                    }
                }
                if (!oldpos.isNull()) {
                    QContact c = mAccess->contact(oldpos);
                    c.setFirstName(label);
                    c.setHomePhone(v);
                    updateContacts.append(c);
                } else {
                    newContacts.insert(label, v);
                }
                // check if update or add.
            }
        }

        int avail = SIMListEnd - SIMListStart - phoneData.count() + 1;

        if (newContacts.count() > avail)
            return false;

        // assert, possible to do import.
        QMapIterator<QString, QString> ait(newContacts);
        while(ait.hasNext()) {
            ait.next();
            QContact c;
            c.setFirstName(ait.key());
            c.setHomePhone(ait.value());
            addContact(c, defaultSource());
        }
        foreach(QContact entry, updateContacts) {
            updateContact(entry);
        }
    }
    return true;
}

QContact QContactSimContext::exportContact(const QUniqueId &id, bool &ok) const
{
    QContact c;
    if (!exists(id)) {
        ok = false;
        return c;
    }

    c =  mAccess->contact(id);
    // how contacts are norally contructed.
    QString label = c.firstName();
    QString number = c.homePhone();

    QContactModel::Field key = SIMExtensionToType(label);
    if (key == QContactModel::Invalid)
        key = QContactModel::HomeMobile;

    c = QContact::parseLabel(label);

    QContactModel::setContactField(c, key, number);
    ok = true;
    return c;
}

QList<QContact> QContactSimContext::exportContacts(const QPimSource &s, bool &ok) const
{
    ok = true;
    // create io for exported context
    int context = QPimSqlIO::sourceContext(s);
    ContactSqlIO *exportAccess = new ContactSqlIO(0);
    QSet<int> set;
    set.insert(context);
    exportAccess->setContextFilter(set, QPimSqlIO::RestrictToContexts);

    QMap<QString, QContact> result;
    QList<QContactModel::Field> pfields = QContactModel::phoneFields();

    for (int i = 0; i < exportAccess->count(); i++) {
        QString label = exportAccess->contact(i).firstName();
        QString number = exportAccess->contact(i).homePhone();

        QContactModel::Field key = SIMExtensionToType(label);
        if (key == QContactModel::Invalid)
            key = QContactModel::HomeMobile;

        QContact c;
        if (result.contains(label))
            c = result[label];
        else
            c = QContact::parseLabel(label);

        if (!QContactModel::contactField(c, key).toString().isEmpty()) {
            // insert into first available phone number field.
            foreach(QContactModel::Field f, pfields) {
                if (QContactModel::contactField(c, f).toString().isEmpty()) {
                    QContactModel::setContactField(c, f, number);
                    break;
                }
            }
        } else {
            QContactModel::setContactField(c, key, number);
        }
        result.insert(label, c);
    }

    return result.values();
}
