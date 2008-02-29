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

#include <qtopia/pim/qcontact.h>
#include <qtopia/pim/qcontactmodel.h>

#include <qtopialog.h>
#include <qtopia/pim/qphonenumber.h>

#include <qtopiaipcenvelope.h>
#include "qcontactsqlio_p.h"
#ifdef Q_OS_WIN32
#include <process.h>
#else
#include <unistd.h>
#endif

// both... until I can get all the required widgets sorted out.
#include <qcategorymanager.h>
#include "qsqlpimtablemodel_p.h"

#include <QSettings>
#include <QString>
#include <QSqlQuery>
#include <QSqlError>
#include <QTimer>
#include <QDebug>

static const char *contextsource = "default";

/*
   data fields

title firstname middlename lastname suffix default_email jobtitle department company b_webpage office profession assistant manager h_webpage spouse gender birthday anniversary nickname children portrait lastname_pronunciation firstname_pronunciation company_pronunciation

data bindings
:t, :fn, :mn, :ln, :su, :de, :jt, :d, :c, :bphone, :bfax, :bmobile, :bstreet, :bcity, :bstate, :bzip, :bcountry, :bpager, :bwebpage, :o, :profession, :assistant, :manager, :hphone, :hfax, :hmobile, :hstreet, :hcity, :hstate, :hzip, :hcountry, :hwebpage, :sp, :g, :b, :a, :nickname, :childern, :portrait, :lnp, :fnp, :cp
*/


// XXX because of the large number of fields.... will _HAVE_ to implement 'sub field retrival' as
// time taken to check and set that many strings will probably be too much.

// may be better to over ride order text.
QStringList ContactSqlIO::sortColumns() const
{
    QList<QContactModel::Field> mOrder = formatFieldOrder();
    QStringList sc;
    foreach(QContactModel::Field f, mOrder) {
        sc << sqlColumn(f);
    }
    return sc;
}

QVariant ContactSqlIO::key(int row) const
{
    QList<QContactModel::Field> mOrder = formatFieldOrder();
    QStringList sc;
    foreach(QContactModel::Field f, mOrder) {
        sc << contactField(row, f).toString();
    }
    return sc;
}

QVariant ContactSqlIO::key(const QUniqueId &id) const
{
    QStringList keys = sortColumns();
    QString keystring = keys.join(", ");
    QSqlQuery q(database());
    if (!q.prepare("SELECT " + keystring + " FROM contacts"
            " WHERE recid = :id"))
        qWarning("select record key failed: %s", (const char *)q.lastError().text().toLocal8Bit());

    q.bindValue(":id", id.toUInt());

    if (q.exec() && q.next()) {
        QStringList sl;
        for (int i = 0; i < keys.count(); i++)
            sl << q.value(i).toString();
        return sl;
    }
    return QVariant();
}

QString ContactSqlIO::sqlColumn(QContactModel::Field k) const {
    switch(k) {
        default:
            return QString();
        case QContactModel::Label:
            return sqlLabelCache;
        case QContactModel::NameTitle:
            return "title";
        case QContactModel::FirstName:
            return "firstname";
        case QContactModel::MiddleName:
            return "middlename";
        case QContactModel::LastName:
            return "lastname";
        case QContactModel::Suffix:
            return "suffix";

        case QContactModel::JobTitle:
            return "jobtitle";
        case QContactModel::Department:
            return "department";
        case QContactModel::Company:
            return "company";

        // email
        case QContactModel::DefaultEmail:
        case QContactModel::Emails:
            return "default_email";

        // business
        case QContactModel::BusinessWebPage:
            return "b_webpage";

        case QContactModel::Office:
            return "office";
        case QContactModel::Profession:
            return "profession";
        case QContactModel::Assistant:
            return "assistant";
        case QContactModel::Manager:
            return "manager";

        // home
        case QContactModel::HomeWebPage:
            return "h_webpage";

        //personal
        case QContactModel::Spouse:
            return "spouse";
        case QContactModel::Gender:
            return "gender";
        case QContactModel::Birthday:
            return "birthday";
        case QContactModel::Anniversary:
            return "anniversary";
        case QContactModel::Nickname:
            return "nickname";
        case QContactModel::Children:
            return "children";

        // other
        case QContactModel::LastNamePronunciation:
            return "lastname_pronunciation";
        case QContactModel::FirstNamePronunciation:
            return "firstname_pronunciation";
        case QContactModel::CompanyPronunciation:
            return "company_pronunciation";

    }
    return QString();
}

QString ContactSqlIO::sqlLabel() const
{
    int fc = formatCount();
    QString expression = "(CASE ";
    for (int i = 0; i < fc; i++) {
        QList<QVariant> f = format(i);
        expression += "WHEN ";
        bool firstkey = true;
        QListIterator<QVariant> it(f);
        while(it.hasNext()) {
            QVariant v = it.next();
            if (v.type() == QVariant::String)
                continue;
            if (!firstkey)
                expression += "AND ";
            firstkey = false;
            QContactModel::Field k = (QContactModel::Field)v.toInt();
            if (k == QContactModel::Invalid || k == QContactModel::Label)
                return sqlColumn(QContactModel::FirstName); // soft fail.
            expression += sqlColumn(k) + " IS NOT NULL ";
        }
        expression += "THEN ";
        QListIterator<QVariant> fit(f);
        while(fit.hasNext()) {
            QVariant v = fit.next();
            if (v.type() == QVariant::Int) {
                QContactModel::Field k = (QContactModel::Field)v.toInt();
                if (k == QContactModel::Invalid || k == QContactModel::Label)
                    return sqlColumn(QContactModel::FirstName); // soft fail.
                expression += sqlColumn(k) + " ";
            } else if (v.type() == QVariant::String) {
                expression += "\"" + v.toString() + "\" ";
            }
            if (fit.hasNext())
                expression += "|| ";
        }
    }
    expression += "ELSE NULL END)";
    return expression;
}

ContactSqlIO::ContactSqlIO(QObject *parent, const QString &)
    : QContactIO(parent),
    QPimSqlIO( contextId(), "contacts", "contactcategories", "contactcustom",

            "title = :t, firstname = :fn, middlename = :mn, lastname = :ln, suffix = :su, "
            "default_email = :de, jobtitle = :jt, department = :d, company = :c, "
            "b_webpage = :bwebpage, office = :o, profession = :profession, "
            "assistant = :assistant, manager = :manager, "
            "h_webpage = :hwebpage, spouse = :sp, "
            "gender = :g, birthday = :b, anniversary = :a, nickname = :nickname, "
            "children = :childern, portrait = :portrait, "
            "lastname_pronunciation = :lnp, firstname_pronunciation = :fnp, "
            "company_pronunciation = :cp",

            "(recid, context, title, firstname, middlename, lastname, suffix, default_email, "
            "jobtitle, department, company, "
            "b_webpage, office, profession, assistant, manager, "
            "h_webpage, spouse, gender, birthday, anniversary, nickname, children, portrait, "
            "lastname_pronunciation, firstname_pronunciation, "
            "company_pronunciation)"
            " VALUES "
            "(:i, :context, :t, :fn, :mn, :ln, :su, :de, :jt, :d, :c, "
            ":bwebpage, :o, "
            ":profession, :assistant, :manager, "
            ":hwebpage, :sp, :g, :b, :a, :nickname, :childern, "
            ":portrait, :lnp, :fnp, :cp)"),

            orderKey(QContactModel::Label), contactByRowValid(false),
            contactCache(5), tmptable(false),
            contactQuery("SELECT recid, title, firstname, " // 3
                    "middlename, lastname, suffix, " // 7
                    "default_email, jobtitle, department, company, " // 11
                    "b_webpage, office, profession, assistant, manager, " // 16
                    "h_webpage, spouse, gender, birthday, anniversary, " // 22
                    "nickname, children, portrait, lastname_pronunciation, " // 26
                    "firstname_pronunciation, company_pronunciation " // 29
                    "FROM contacts WHERE recid = :i"),
            categoryQuery("SELECT categoryid from contactcategories where recid=:id"),
            emailsQuery("SELECT addr from emailaddresses where recid=:id"),
            addressesQuery("SELECT addresstype, street, city, state, zip, country from contactaddresses where recid=:id"),
            phoneQuery("SELECT phone_type, phone_number from contactphonenumbers where recid=:id"),
            customQuery("SELECT fieldname, fieldvalue from contactcustom where recid=:id"),
            insertEmailsQuery("INSERT INTO emailaddresses (recid, addr) VALUES (:i, :a)"),
            insertAddressesQuery("INSERT INTO contactaddresses (recid, addresstype, street, city, state, zip, country) VALUES (:i, :t, :s, :c, :st, :z, :co)"),
            insertPhoneQuery("INSERT INTO contactphonenumbers (recid, phone_type, phone_number) VALUES (:i, :t, :ph)"),
            removeEmailsQuery("DELETE from emailaddresses WHERE recid = :i"),
            removeAddressesQuery("DELETE from contactaddresses WHERE recid = :i"),
            removePhoneQuery("DELETE from contactphonenumbers WHERE recid = :i")
{
    QPimSqlIO::setOrderBy(sortColumns());

    sqlLabelCache = sqlLabel();
    initMaps();

    connect(this, SIGNAL(labelFormatChanged()), this, SLOT(updateSqlLabel()));
}

ContactSqlIO::~ContactSqlIO()
{
}

QUuid ContactSqlIO::contextId() const
{
    // generated with uuidgen
    static QUuid u("a7a2832c-cdb3-40b6-9d95-6cd31e05647d");
    return u;
}
void ContactSqlIO::bindFields(const QPimRecord& r, QSqlQuery &q) const
{
    const QContact &t = (const QContact &)r;
    q.bindValue(":t", t.nameTitle());
    q.bindValue(":fn", t.firstName());
    q.bindValue(":mn", t.middleName());
    q.bindValue(":ln", t.lastName());
    q.bindValue(":su", t.suffix());
    q.bindValue(":de", t.defaultEmail());
    q.bindValue(":jt", t.jobTitle());
    q.bindValue(":d", t.department());
    q.bindValue(":c", t.company());

    q.bindValue(":bwebpage", t.businessWebpage());
    q.bindValue(":o", t.office());
    q.bindValue(":profession", t.profession());
    q.bindValue(":assistant", t.assistant());
    q.bindValue(":manager", t.manager());

    q.bindValue(":hwebpage", t.homeWebpage());
    q.bindValue(":sp", t.spouse());
    q.bindValue(":g", t.gender());
    q.bindValue(":b", t.birthday());
    q.bindValue(":a", t.anniversary());
    q.bindValue(":nickname", t.nickname());
    q.bindValue(":childern", t.children());
    q.bindValue(":portrait", t.portraitFile());
    q.bindValue(":lnp", t.lastNamePronunciation());
    q.bindValue(":fnp", t.firstNamePronunciation());
    q.bindValue(":cp", t.companyPronunciation());
}

// by uid doesn't neeed caching... always fast and unlikely to be in order?
QContact ContactSqlIO::contact( const QUniqueId & u ) const
{
    if (u.isNull())
        return QContact();
    // Getting a whole contact can be expensive.  Cache if able.
    // We're very likely to require the previous contact again.
    if (contactByRowValid && u == lastContact.uid())
        return lastContact;
    // Maybe it is in our cache?
    QContact *cc = contactCache.object(u);
    if (cc)
        return *cc;

    contactQuery.prepare();
    if (!contactQuery.isValid())
        return QContact();

    contactQuery.bindValue(":i", u.toUInt());

    QContact t;

    // get common parts
    retrieveRecord(u.toUInt(), t);

    if (!contactQuery.exec()) {
        contactByRowValid = false;
        contactCache.remove(u);
        qWarning("failed to select contact: %s", (const char *)contactQuery.lastError().text().toLocal8Bit());
        return t;
    }

    if ( contactQuery.next() ) {
        // XXX should check uid against u.
        t.setUid(QUniqueId::fromUInt(contactQuery.value(0).toUInt()));
        t.setNameTitle(contactQuery.value(1).toString());
        t.setFirstName(contactQuery.value(2).toString());
        t.setMiddleName(contactQuery.value(3).toString());
        t.setLastName(contactQuery.value(4).toString());
        t.setSuffix(contactQuery.value(5).toString());
        t.setDefaultEmail(contactQuery.value(6).toString());
        t.setJobTitle(contactQuery.value(7).toString());
        t.setDepartment(contactQuery.value(8).toString());
        t.setCompany(contactQuery.value(9).toString());
        t.setBusinessWebpage(contactQuery.value(10).toString());
        t.setOffice(contactQuery.value(11).toString());
        t.setProfession(contactQuery.value(12).toString());
        t.setAssistant(contactQuery.value(13).toString());
        t.setManager(contactQuery.value(14).toString());
        t.setHomeWebpage(contactQuery.value(15).toString());
        t.setSpouse(contactQuery.value(16).toString());
        t.setGender((QContact::GenderType)contactQuery.value(17).toInt());
        t.setBirthday(contactQuery.value(18).toDate());
        t.setAnniversary(contactQuery.value(19).toDate());
        t.setNickname(contactQuery.value(20).toString());
        t.setChildren(contactQuery.value(21).toString());
        t.setPortraitFile(contactQuery.value(22).toString());
        t.setLastNamePronunciation(contactQuery.value(23).toString());
        t.setFirstNamePronunciation(contactQuery.value(24).toString());
        t.setCompanyPronunciation(contactQuery.value(25).toString());

        contactQuery.reset();

        // categories for this contact;
        qLog(Sql) << "Read email addresses";
        emailsQuery.prepare();
        if(emailsQuery.isValid()) {
            emailsQuery.bindValue(":id", u.toUInt());
            if (!emailsQuery.exec()) {
                qWarning("select email failed: %s", (const char *)emailsQuery.lastError().text().toLocal8Bit());
            }
            QStringList tlist;
            while(emailsQuery.next())
                tlist.append(emailsQuery.value(0).toString());
            t.setEmailList(tlist);
            emailsQuery.reset();
        }

        qLog(Sql) << "Read addresses";
        // and contact addresses
        addressesQuery.prepare();
        if(addressesQuery.isValid()) {
            addressesQuery.bindValue(":id", u.toUInt());
            if (!addressesQuery.exec()) {
                qWarning("select addresses failed: %s", (const char *)addressesQuery.lastError().text().toLocal8Bit());
            }
            QStringList tlist;
            while(addressesQuery.next()) {
                QContactAddress a;
                QContact::Location l;
                l = (QContact::Location)addressesQuery.value(0).toInt();
                a.street = addressesQuery.value(1).toString();
                a.city = addressesQuery.value(2).toString();
                a.state = addressesQuery.value(3).toString();
                a.zip = addressesQuery.value(4).toString();
                a.country = addressesQuery.value(5).toString();
                t.setAddress(l, a);
            }
            addressesQuery.reset();
        }

        // and contact addresses
        qLog(Sql) << "Read phone numbers";
        phoneQuery.prepare();
        if(phoneQuery.isValid()) {
            phoneQuery.bindValue(":id", u.toUInt());
            if (!phoneQuery.exec()) {
                qWarning("select phone numbers failed: %s", (const char *)phoneQuery.lastError().text().toLocal8Bit());
            }
            QString tlist;
            while(phoneQuery.next()) {
                QString number;
                QContact::PhoneType type;
                type = (QContact::PhoneType)phoneQuery.value(0).toInt();
                number = phoneQuery.value(1).toString();
                qLog(Sql) << "set phone number" << type << number;
                t.setPhoneNumber(type, number);
            }
            phoneQuery.reset();
        }


        customQuery.prepare();
        if(customQuery.isValid()) {
        // custom fields for this appointment
            customQuery.bindValue(":id", u.toUInt());
            if (!customQuery.exec()) {
                qWarning("select fieldname, fieldvalue failed: %s",
                        (const char *)customQuery.lastError().text().toLocal8Bit());
            }
            QMap<QString, QString> tMap;
            while (customQuery.next())
                t.setCustomField(customQuery.value(0).toString(), 
                                 customQuery.value(1).toString());
            customQuery.reset();
        }

        lastContact = t;
        contactByRowValid = true;
        contactCache.insert(u, new QContact(t));

    } else {
        contactQuery.reset();
        contactByRowValid = false;
        contactCache.remove(u);
    }

    return t;
}

void ContactSqlIO::setSortKey(QContactModel::Field s)
{
    if (orderKey != s) {
        orderKey = s;
        invalidateCache();
    }
}

void ContactSqlIO::setCategoryFilter(const QCategoryFilter &f)
{
    if (f != categoryFilter()) {
        QPimSqlIO::setCategoryFilter(f);
        emit filtersUpdated();
    }
}

void ContactSqlIO::setContextFilter(const QSet<int> &list, ContextFilterType type)
{
    if (list != contextFilter() || type != contextFilterType()) {
        QPimSqlIO::setContextFilter(list, type);
        invalidateCache();
    }
}

QContactModel::Field ContactSqlIO::sortKey() const
{
    return orderKey;
}

void ContactSqlIO::invalidateCache()
{
    QPimSqlIO::invalidateCache();
    contactByRowValid = false;
    contactCache.clear();
    emit filtersUpdated();
}

// if filtering/sorting/contacts doesn't change.
QContact ContactSqlIO::contact(int row) const
{
    return contact(recordId(row));
}

void ContactSqlIO::updateSqlLabel()
{
    sqlLabelCache = sqlLabel();
    setOrderBy(sortColumns());
    invalidateCache();
}

// assumes storage is row based.
// override if more efficient to grap partial contact.
QVariant ContactSqlIO::contactField(int row, QContactModel::Field k) const
{
    QUniqueId u = recordId(row);
    if (k == QContactModel::Identifier)
        return QVariant(u.toUInt());
    if (u.isNull())
        return QVariant();

    // handle special list-value keys
    if (k == QContactModel::Categories) {
        categoryQuery.prepare();
        if(!categoryQuery.isValid()) 
            return QVariant(QList<QString>());

        categoryQuery.bindValue(":id", u.toUInt());
        if (!categoryQuery.exec()) {
            qWarning("select categoryid failed: %s", (const char *)categoryQuery.lastError().text().toLocal8Bit());
        }
        QList<QString> tlist;
        while(categoryQuery.next())
            tlist.append(categoryQuery.value(0).toString());
        categoryQuery.reset();
        return QVariant(tlist);
    } else if (k == QContactModel::Emails) {
        emailsQuery.prepare();
        if(!emailsQuery.isValid())
            return QVariant(QList<QString>());

        emailsQuery.bindValue(":id", u.toUInt());
        if (!emailsQuery.exec()) {
            qWarning("select email failed: %s", (const char *)emailsQuery.lastError().text().toLocal8Bit());
        }
        QList<QString> tlist;
        while(emailsQuery.next())
            tlist.append(emailsQuery.value(0).toString());

        emailsQuery.reset();
        return QVariant(tlist);
    }

    QSqlQuery q(database());
    QString column = sqlField(k);

    q.prepare("SELECT " + column + " FROM contacts WHERE recid = :id");
    q.bindValue(":id", u.toUInt());
    if (!q.exec() || !q.next()) {
        qWarning ("couldn't select for record %s, column %s : %s", (const char *)u.toString().toLatin1(), (const char *)column.toLocal8Bit(), (const char *)q.lastError().text().toLocal8Bit());
        return QVariant();
    }
    return q.value(0);
}

bool ContactSqlIO::setContactField(int row, QContactModel::Field k,  const QVariant &v)
{
    QUniqueId u = recordId(row);
    if (u.isNull() || k == QContactModel::Identifier)
        return false;

    if (k == QContactModel::Categories) {
        if (!v.canConvert(QVariant::StringList))
            return false;
        database().transaction();
        QList<QString> cats = v.toStringList();
        QSqlQuery q(database());
        q.prepare(selectCategoriesText);
        q.bindValue(":i", u.toUInt());
        q.exec();
        if(q.next() && q.value(0).toInt() > 0) {
            q.prepare(deleteCategoriesText);
            q.bindValue(":i", u.toUInt());
            if (!q.exec()) {
                qWarning("failed clean up: %s", (const char *)q.lastError().text().toLocal8Bit());
                database().rollback();
                return false;
            }
        }
        if (cats.count()) {
            q.prepare(insertCategoriesText);
            q.bindValue(":i", u.toUInt());
            foreach(QString v, cats) {
                q.bindValue(":v", v);
                if (!q.exec()) {
                    qWarning("failed category table insert: %s", (const char *)q.lastError().text().toLocal8Bit());
                    database().rollback();
                    return false;
                }
            }
        }
        if (database().commit()) {
            emit recordsUpdated();
            return true;
        } else {
            database().rollback();
            return false;
        }
    } else if (k == QContactModel::Emails) {
        if (!v.canConvert(QVariant::StringList))
            return false;
        database().transaction();
        QList<QString> emails = v.toStringList();
        QSqlQuery q(database());
        q.prepare(selectCategoriesText);
        q.bindValue(":i", u.toUInt());
        q.exec();
        if(q.next() && q.value(0).toInt() > 0) {
            q.prepare(deleteCategoriesText);
            q.bindValue(":i", u.toUInt());
            if (!q.exec()) {
                qWarning("failed clean up: %s", (const char *)q.lastError().text().toLocal8Bit());
                database().rollback();
                return false;
            }
        }
        if (emails.count()) {
            q.prepare(insertCategoriesText);
            q.bindValue(":i", u.toUInt());
            foreach(QString v, emails) {
                q.bindValue(":v", v);
                if (!q.exec()) {
                    qWarning("failed category table insert: %s", (const char *)q.lastError().text().toLocal8Bit());
                    database().rollback();
                    return false;
                }
            }
        }
        if (database().commit()) {
            emit recordsUpdated();
            return true;
        } else {
            database().rollback();
            return false;
        }

    }

    QString column = sqlField(k);

    if (!column.isEmpty() && canUpdate(k)) {
        database().transaction();
        QSqlQuery q(database());
        q.prepare("UPDATE contacts SET " + column + " = :value WHERE recid = :id");
        q.bindValue(":value", v);
        q.bindValue(":id", u.toUInt());
        if (q.exec()) {
            if (database().commit()) {
                emit recordsUpdated();
                return true;
            } else {
                database().rollback();
                return false;
            }
        } else {
            database().rollback();
            return false;
        }
    }

    return false;
}

QMap<QContactModel::Field, QString> ContactSqlIO::mFields;
QMap<QContactModel::Field, bool> ContactSqlIO::mUpdateable;

void ContactSqlIO::initMaps()
{
    struct csiIM {
        QContactModel::Field k;
        const char *field;
        bool u;
    };

    static const csiIM i[] = {
        { QContactModel::NameTitle, "title", true },
        { QContactModel::FirstName, "firstname", true },
        { QContactModel::MiddleName, "middlename", true },
        { QContactModel::LastName, "lastname", true },
        { QContactModel::Suffix, "suffix", true },
        { QContactModel::Label, 0, false }, // meta field

        { QContactModel::JobTitle, "jobtitle", true },
        { QContactModel::Department, "department", true },
        { QContactModel::Company, "company", true },

        // email
        { QContactModel::DefaultEmail, "default_email", true },
        { QContactModel::Emails, 0, true },

        // business
        { QContactModel::BusinessWebPage, "b_webpage", true },

        { QContactModel::Office, "office", true },
        { QContactModel::Profession, "profession", true },
        { QContactModel::Assistant, "assistant", true },
        { QContactModel::Manager, "manager", true },

        // home
        { QContactModel::HomeWebPage, "h_webpage", true },

        //personal
        { QContactModel::Spouse, "spouse", true },
        { QContactModel::Gender, "gender", true },
        { QContactModel::Birthday, "birthday", true },
        { QContactModel::Anniversary, "anniversary", true },
        { QContactModel::Nickname, "nickname", true },
        { QContactModel::Children, "children", true },

        // other
        { QContactModel::Portrait, "portrait", true },

        { QContactModel::LastNamePronunciation, "lastname_pronunciation", true },
        { QContactModel::FirstNamePronunciation, "firstname_pronunciation", true },
        { QContactModel::CompanyPronunciation, "company_pronunciation", true },
        { QContactModel::Invalid, 0, false }
    };

    const csiIM *item = i;
    while (item->k != QContactModel::Invalid) {
        if (item->field)
            mFields.insert(item->k, item->field);
        mUpdateable.insert(item->k, item->u);
        ++item;
    }

}

QString ContactSqlIO::sqlField(QContactModel::Field k) const
{
    // special for the label field
    if (k == QContactModel::Label)
        return sqlLabelCache;
    if (mFields.contains(k))
        return mFields[k];
    return QString();
}

bool ContactSqlIO::canUpdate(QContactModel::Field k) const
{
    if (mUpdateable.contains(k))
        return mUpdateable[k];
    return false;
}


bool ContactSqlIO::removeContact(int row)
{
    QUniqueId u = recordId(row);
    return removeContact(u);
}

bool ContactSqlIO::removeContact(const QUniqueId & id)
{
    if (id.isNull())
        return false;

    if (QPimSqlIO::removeRecord(id)) {
        notifyRemoved(id);
        emit recordsUpdated();
        return true;
    }
    return false;
}

bool ContactSqlIO::removeContact(const QContact &t)
{
    return removeContact(t.uid());
}

bool ContactSqlIO::removeContacts(const QList<int> &rows)
{
    QList<QUniqueId> ids = recordIds(rows);
    return removeContacts(ids);
}

bool ContactSqlIO::removeContacts(const QList<QUniqueId> &ids)
{
    if (QPimSqlIO::removeRecords(ids)) {
        notifyRemoved(ids);
        emit recordsUpdated();
        return true;
    }
    return false;
}

bool ContactSqlIO::updateContact(const QContact &t)
{
    if (QPimSqlIO::updateRecord(t)) {
        notifyUpdated(t.uid());
        emit recordsUpdated();
        return true;
    }
    return false;
}

QUniqueId ContactSqlIO::addContact(const QContact &contact, const QPimSource &source, bool createuid)
{
    QPimSource s;
    s.identity = contextsource;
    s.context = contextId();
    QUniqueId i = addRecord(contact, source.isNull() ? s : source, createuid);
    if (!i.isNull()) {
        QContact added = contact;
        added.setUid(i);
        notifyAdded(i);
        invalidateCache();
    }
    return i;
}

static void mergeContact(QContact &destination, const QContact &source)
{
    // initially for importing sim contacts.  hence 'merge' only applies to
    // phone numbers of the contact.
    QStringList destNumbers = destination.phoneNumbers().values();
    QStringList sourceNumbers = source.phoneNumbers().values();

    QList<QContact::PhoneType> types = QContact::phoneTypes();

    foreach(QContact::PhoneType t, types) {
        QString number = source.phoneNumber(t);
        QString dnumber = destination.phoneNumber(t);
        // continue if number is already stored.
        if (number.isEmpty() || destNumbers.contains(number))
            continue;
        // continue if storing a number already from this contact (so above statement doesn't invalidate)
        if (!dnumber.isEmpty() && sourceNumbers.contains(dnumber)) {
            continue;
        }
        // now, we have a genuine conflict.
        // hard to say if should favor dest or source.  For now, siding with dest
        // since contact is still on source hence favoring dest does not reduce
        // information stored overall.
        if (!dnumber.isEmpty())
            continue;
        destination.setPhoneNumber(t, number);
    }
}

/*
   needs to export just this filter, not all shown
   by access belong to this context
*/
QList<QContact> QContactDefaultContext::exportContacts(const QPimSource &, bool &ok) const
{
    int context = QPimSqlIO::sourceContext(defaultSource());
    // unfiltered? current filter?
    QList<QContact> result;
    // temporary to allow separate filters
    ContactSqlIO *exportAccess = new ContactSqlIO(0);
    exportAccess->setCategoryFilter(mAccess->categoryFilter());
    QSet<int> set;
    set.insert(context);
    exportAccess->setContextFilter(set, QPimSqlIO::RestrictToContexts);

    // don't even try exporting more than this.  Exporting involves going via a list,
    // which can be expensive.  Since most sim cards only handle about 200 contacts,
    // this is mostly a 'don't try something silly' check.
    if (exportAccess->count() > 1000) {
        ok = false;
        delete exportAccess;
        return result;
    }
    for(int i = 0; i < exportAccess->count(); ++i) {
        result.append(exportAccess->contact(i));
    }
    ok = true;
    delete exportAccess;
    return result;
}

QContact QContactDefaultContext::exportContact(const QUniqueId &id, bool &ok) const
{
    int context = QPimSqlIO::sourceContext(defaultSource());
    ok = mAccess->exists(id) && mAccess->context(id) == context;
    if (ok)
        return mAccess->contact(id);
    return QContact();
}

/* for each contact, check if an identical named one is in the list.  if so, update it rather than add.
   */
bool QContactDefaultContext::importContacts(const QPimSource &s, const QList<QContact> &list)
{
    if (list.isEmpty())
        return false;
    int context = QPimSqlIO::sourceContext(s);

    QDateTime syncTime = QTimeZone::current().toUtc(QDateTime::currentDateTime());

    if (!mAccess->startSync(s, syncTime))
        return false;

    foreach(QContact c, list) {
        // contacts are pretty much uniquely identified by name.  hence do a search for uid on the name fields.
        importQuery.prepare();
        if (!importQuery.isValid()) {
            qWarning("Failed to prepare find named contact: %s", importQuery.lastError().text().toLocal8Bit().constData());
            mAccess->abortSync();
            return false;
        }
        importQuery.bindValue(":cont", context);
        importQuery.bindValue(":t", c.nameTitle());
        importQuery.bindValue(":t2", c.nameTitle());
        importQuery.bindValue(":fn", c.firstName());
        importQuery.bindValue(":fn2", c.firstName());
        importQuery.bindValue(":mn", c.middleName());
        importQuery.bindValue(":mn2", c.middleName());
        importQuery.bindValue(":ln", c.lastName());
        importQuery.bindValue(":ln2", c.lastName());
        importQuery.bindValue(":s", c.suffix());
        importQuery.bindValue(":s2", c.suffix());
        importQuery.bindValue(":c", c.company());


        if (qLogEnabled(Sql))
        {
            qLog(Sql) << "executing:" << importQuery.lastQuery();
            if (!importQuery.boundValues().empty())
                qLog(Sql) << "   params:" << importQuery.boundValues();
        }

        if (!importQuery.exec()) {
            qWarning("Failed to execute find named contact: %s", importQuery.lastError().text().toLocal8Bit().constData());
            mAccess->abortSync();
            return false;
        }
        if (importQuery.next()) {
            QUniqueId u = QUniqueId::fromUInt(importQuery.value(0).toUInt());
            QContact current = mAccess->contact(u);
            mergeContact(current, c); // merges phone number information from sim to phone.  favours phone.
            importQuery.reset();
            updateContact(current);
        } else {
            importQuery.reset();
            addContact(c, s);
        }
    }
    return mAccess->commitSync();
}

bool ContactSqlIO::updateExtraTables(uint uid, const QPimRecord &r)
{
    if (!removeExtraTables(uid))
        return false;
    return insertExtraTables(uid, r);
}

bool ContactSqlIO::removeExtraTables(uint uid)
{
    removeEmailsQuery.prepare();
    removeAddressesQuery.prepare();
    removePhoneQuery.prepare();
    if (!removeEmailsQuery.isValid() || !removeAddressesQuery.isValid() || ! removePhoneQuery.isValid())
        return false;


    removeEmailsQuery.bindValue(":i", uid);
    if (!removeEmailsQuery.exec())
        return false;
    removeEmailsQuery.reset();

    removeAddressesQuery.bindValue(":i", uid);
    if (!removeAddressesQuery.exec())
        return false;
    removeAddressesQuery.reset();

    removePhoneQuery.bindValue(":i", uid);
    if (!removePhoneQuery.exec())
        return false;
    removePhoneQuery.reset();

    return true;
}

bool ContactSqlIO::insertExtraTables(uint uid, const QPimRecord &r)
{
    const QContact &c = (const QContact &)r;

    insertEmailsQuery.prepare();
    insertAddressesQuery.prepare();
    insertPhoneQuery.prepare();
    if (!insertEmailsQuery.isValid() || !insertAddressesQuery.isValid() || ! insertPhoneQuery.isValid())
        return false;

    QStringList e = c.emailList();
    foreach(QString i, e) {
        // don't insert empties.
        if (!i.trimmed().isEmpty()) {
            insertEmailsQuery.bindValue(":i", uid);
            insertEmailsQuery.bindValue(":a", i);
            if (!insertEmailsQuery.exec()) {
                qWarning("Failed to insert email address: %s", insertEmailsQuery.lastError().text().toLocal8Bit().constData());
                return false;
            }
        }
    }
    insertEmailsQuery.reset();

    /* home address, business address */

    QMap<QContact::Location, QContactAddress> a = c.addresses();
    QMapIterator<QContact::Location, QContactAddress> i(a);
    while(i.hasNext()) {
        i.next();
        QContactAddress a = i.value();
        insertAddressesQuery.bindValue(":i", uid);
        insertAddressesQuery.bindValue(":t", i.key());
        insertAddressesQuery.bindValue(":s", a.street);
        insertAddressesQuery.bindValue(":c", a.city);
        insertAddressesQuery.bindValue(":st", a.state);
        insertAddressesQuery.bindValue(":z", a.zip);
        insertAddressesQuery.bindValue(":co", a.country);
        if (!insertAddressesQuery.exec()) {
            qWarning("Failed to insert contact address: %s", insertAddressesQuery.lastError().text().toLocal8Bit().constData());
            return false;
        }
    }
    insertAddressesQuery.reset();

    /* all phone numbers */
    /* home address, business address */
    QMap<QContact::PhoneType, QString> ph = c.phoneNumbers();
    QMapIterator<QContact::PhoneType, QString> phi(ph);
    while(phi.hasNext()) {
        phi.next();
        insertPhoneQuery.bindValue(":i", uid);
        insertPhoneQuery.bindValue(":t", phi.key());
        insertPhoneQuery.bindValue(":ph", phi.value());
        if (!insertPhoneQuery.exec()) {
            qWarning("Failed to insert contact phone number: %s", insertPhoneQuery.lastError().text().toLocal8Bit().constData());
            return false;
        }
    }
    insertPhoneQuery.reset();

    return true;
}

void ContactSqlIO::setFilter(const QString &text, int flags)
{
    /* assume text is not empty */
    QString mSearchFilter;

#ifdef QTOPIA_PHONE
    /* should do on construction? or in QContactIO construction */
    QMap<QChar, QString> pbt = phoneButtonText();

    QRegExp simIndex("^(\\d{1,3})#$");
    if (simIndex.exactMatch(text)) {
        QStringList sl; 
        sl.append("simcardidmap");
        QPimSqlIO::setJoins(sl);
        QPimSqlIO::setFilter("simcardidmap.cardindex = " + simIndex.cap(1));
        return;
    }
    bool allNumbers = true;
    for(int i = 0; i < text.length(); ++i) {
        if (!pbt.contains(text[i])) {
            allNumbers = false;
            break;
        }
    }
    if (allNumbers && !text.isEmpty()) {
        bool first = true;
        foreach (QContactModel::Field f, labelSearchFields()) {
            if (!first)
                mSearchFilter += " or ";
            else
                first = false;
            mSearchFilter += "(";
            QString fname = "t1."+sqlColumn(f);
            int i;
            /* Handle first letter with > and <, as will work on index and hence
               cut search results down much faster */
            QChar firstChar = text[0];
            mSearchFilter += fname + " >= '" + QString(pbt[firstChar][0]) + "'";
            if(firstChar.isDigit()) {
                int firstCharVal = firstChar.digitValue();
                QChar nextCharVal('0'+firstCharVal+1);
                if (firstCharVal < 9 && pbt.contains(nextCharVal))
                    mSearchFilter += " and " + fname + " < '" + QString(pbt[nextCharVal][0]) + "'";
            }
            for (i= 0; i < text.length(); i++) {
                mSearchFilter += " and ";
                /* changes based of db, Mimer for example is
                   substring(string from index for length)
                   */
#if 0
                mSearchFilter += "substring(" + fname + " from " + QString::number(i+1) + " for 1) in (";
#else
                mSearchFilter += "lower(substr(" + fname + ", " + QString::number(i+1) + ", 1)) in (";
#endif
                QString letters = pbt[text[i]];
                for (int pos = 0; pos < letters.length(); ++pos) {
                    //if (letters[pos] == '\'' || letters[pos] == '@')
                        //break;
                    if (pos != 0)
                        mSearchFilter += ", ";
                    if (letters[pos] == '\'')
                        mSearchFilter += "''''";
                    else
                        mSearchFilter += "'" + letters[pos] + "'";
                }
                mSearchFilter += ")";
            }
            mSearchFilter += ")";
        }
    } else if (!text.isEmpty())
    {
        /* TODO should escape out '%' */
        /* text fields as mere 'starts with' */
        bool first = true;
        foreach (QContactModel::Field f, labelSearchFields()) {
            if (!first)
                mSearchFilter += " or ";
            else
                first = false;

            QString fname = "t1."+sqlColumn(f);
            mSearchFilter += "lower("+fname+") like '" + text + "%' ";
        }
    }
#endif


    /* flags
       ContainsMobileNumber
       inner join contactphonenumbers on recid group by contacts.recid;
       ContainsEmail
       inner join emailaddresses on recid group by contacts.recid;
       ContainsMailing
       inner join contactaddresses on recid group by contacts.recid;

     */
    QStringList joins;
    if (flags & QContactModel::ContainsPhoneNumber)
        joins += "contactphonenumbers";
    if (flags & QContactModel::ContainsEmail)
        joins += "emailaddresses";
    if (flags & QContactModel::ContainsMailing)
        joins += "contactaddresses";

    if (!joins.isEmpty())
        QPimSqlIO::setJoins(joins);
    if (!mSearchFilter.isEmpty()) {
        mSearchFilter = "(" + mSearchFilter + ")";
        QPimSqlIO::setFilter(mSearchFilter);
    }
    emit filtersUpdated();
}

void ContactSqlIO::clearFilter()
{
    QPimSqlIO::setFilters(QStringList());
    QPimSqlIO::setJoins(QStringList());
    emit filtersUpdated();
}

int ContactSqlIO::predictedRow(const QVariant &k, const QUniqueId &id) const
{
    /* key is likely string list, turn into QList with id bytearray
       at end */
    if (k.type() == QVariant::StringList) {
        QStringList sl = k.toStringList();
        QStringListIterator it(sl);
        QList<QContactModel::Field> mOrder = formatFieldOrder();
        QList<QVariant> keys;

        for (int i= 0; i < mOrder.count(); i++) {
            if (it.hasNext())
                keys.append(it.next());
            else
                keys.append(QVariant());
        }
        keys.append(id.toUInt());

        return model.predictedRow(keys);
    }
    return count(); // all at end if don't match sort key
}

#ifdef QTOPIA_PHONE
QUniqueId ContactSqlIO::matchPhoneNumber(const QString &phnumber, int &bestMatch) const
{
    /* SELECT recid FROM contactphonenumbers WHERE phone_number LIKE '%partial' */

    QString local = QPhoneNumber::localNumber(phnumber);
    if (local.isEmpty())
        return QUniqueId();

    QSqlQuery q(database());
    if (!q.prepare("SELECT recid, phone_number FROM contactphonenumbers WHERE phone_number LIKE '%" + local + "%'"))
    {
        qWarning("Failed to prepare phone number search: %s",
                q.lastError().text().toLocal8Bit().constData());
        qLog(Sql) << q.lastQuery();
        return QUniqueId();
    }

    if (!q.exec()) {
        qWarning("failed phone number search: %s", q.lastError().text().toLocal8Bit().constData());
        qLog(Sql) << q.executedQuery();
        return QUniqueId();
    }

    bestMatch = 0;
    QUniqueId bestContact;
    while (q.next()) {
        QUniqueId matchId = QUniqueId::fromUInt(q.value(0).toUInt());
        QString matchPh(q.value(1).toString());
        if (!contains(matchId))
            continue;

        int match = QPhoneNumber::matchNumbers(phnumber,matchPh);

        if (match > bestMatch) {
            bestMatch = match;
            bestContact = matchId;
        }
        if (match == 100)
            break;
    }
    qLog(Sql) << "QContactSqlIO::matchPhoneNumber() result:" << bestMatch << bestContact.toString();
    return bestContact;
}
#endif

/***************
 * CONTEXT
 **************/

QContactDefaultContext::QContactDefaultContext(QObject *parent, QObject *access)
    : QContactContext(parent),
    importQuery("SELECT recid FROM contacts WHERE context = :cont AND ("
            "(title = :t OR (title IS NULL AND :t2 IS NULL)) AND "
            "(firstname = :fn OR (firstname IS NULL AND :fn2 IS NULL)) AND "
            "(middlename = :mn OR (middlename IS NULL AND :mn2 IS NULL)) AND "
            "(lastname = :ln OR (lastname IS NULL AND :ln2 IS NULL)) AND "
            "(suffix = :s OR (suffix IS NULL AND :s2 IS NULL))) "
            "OR (company = :c AND title IS NULL AND firstname IS NULL "
            "AND middlename IS NULL AND lastname IS NULL AND suffix IS NULL)")
{
    mAccess = qobject_cast<ContactSqlIO *>(access);
    Q_ASSERT(mAccess);
}

QIcon QContactDefaultContext::icon() const
{
    return QPimContext::icon(); // redundent, but will do for now.
}

QString QContactDefaultContext::description() const
{
    return tr("Default contact storage");
}

QString QContactDefaultContext::title() const
{
    return tr("Contacts");
}

bool QContactDefaultContext::editable() const
{
    return true;
}

QSet<QPimSource> QContactDefaultContext::sources() const
{
    QSet<QPimSource> list;
    list.insert(defaultSource());
    return list;
}

QPimSource QContactDefaultContext::defaultSource() const
{
    QPimSource s;
    s.context = mAccess->contextId();
    s.identity = contextsource;
    return s;
}

QUuid QContactDefaultContext::id() const
{
    return mAccess->contextId();
}

/* TODO set mapping to int */
void QContactDefaultContext::setVisibleSources(const QSet<QPimSource> &set)
{
    int context = QPimSqlIO::sourceContext(defaultSource());

    QSet<int> filter = mAccess->contextFilter();

    if (set.contains(defaultSource()))
        filter.remove(context);
    else
        filter.insert(context);

    mAccess->setContextFilter(filter);
}

QSet<QPimSource> QContactDefaultContext::visibleSources() const
{
    int context = QPimSqlIO::sourceContext(defaultSource());

    QSet<int> filter = mAccess->contextFilter();
    if (!filter.contains(context))
        return sources();
    return QSet<QPimSource>();
}

bool QContactDefaultContext::exists(const QUniqueId &id) const
{
    int context = QPimSqlIO::sourceContext(defaultSource());
    return mAccess->exists(id) && mAccess->context(id) == context;
}

QPimSource QContactDefaultContext::source(const QUniqueId &id) const
{
    if (exists(id))
        return defaultSource();
    return QPimSource();
}

bool QContactDefaultContext::updateContact(const QContact &contact)
{
    return mAccess->updateContact(contact);
}

bool QContactDefaultContext::removeContact(const QUniqueId &id)
{
    return mAccess->removeContact(id);
}

QUniqueId QContactDefaultContext::addContact(const QContact &contact, const QPimSource &source)
{
    if (source.isNull() || source == defaultSource())
        return mAccess->addContact(contact, defaultSource());
    return QUniqueId();
}

