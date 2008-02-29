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
#include "qcategorymanager.h"
#ifndef QTOPIA_CONTENT_INSTALLER
#include <qtopiaapplication.h>
#else
#include <qtopianamespace.h>
#endif
#include <qtopiasql.h>
#include <qtopiaipcenvelope.h>
#include <qtopianamespace.h>

#include <QSettings>
#include <QMap>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

class QCategoryFilterData : public QSharedData
{
public:
    QCategoryFilterData() : type(QCategoryFilter::All) {}

    QList<QString> required;
    QCategoryFilter::FilterType type;
};

/*!
  \class QCategoryFilter
  \brief The QCategoryFilter class filters category sets.

  The QCategoryFilter class allows consistent filtering of records or objects
  that have a set of categories assigned.  It also provides methods to store
  the current filtering method in a QSettings file.

  \ingroup categories
*/

/*!
  \enum QCategoryFilter::FilterType

  \value List Only accept category sets that contain all of the required categories.
  \value Unfiled Only accept empty category sets.
  \value All Accept all category sets.
*/

/*!
  Constructs a QCategoryFilter object.  By default this object will accept
  all category sets.
*/
QCategoryFilter::QCategoryFilter()
{
    d = new QCategoryFilterData();
}

/*!
  Constructs a QCategoryFilter object of the type \a t.

  \sa FilterType
*/
QCategoryFilter::QCategoryFilter(FilterType t)
{
    d = new QCategoryFilterData();
    d->type = t;
}

/*!
  Constructs a QCategoryFilter object that only accepts category sets containing
  all the categories listed in \a set.
*/
QCategoryFilter::QCategoryFilter(const QList<QString>&set)
{
    d = new QCategoryFilterData();
    d->type = List;
    d->required = set;
}

/*!
  Constructs a QCategoryFilter object that only accepts category sets containing the category
  \a c.
*/
QCategoryFilter::QCategoryFilter(const QString &c)
{
    d = new QCategoryFilterData();
    d->type = List;
    d->required.append(c);
}

/*!
  Constructs a QCategoryFilter object as a copy of \a other.
*/
QCategoryFilter::QCategoryFilter(const QCategoryFilter &other)
{
    d = other.d;
}

/*!
  Destroys a QCategoryFilter object.
*/
QCategoryFilter::~QCategoryFilter() {}

/*!
  Assigns the QCategoryFilter object to be a copy of \a other.
*/
QCategoryFilter &QCategoryFilter::operator=(const QCategoryFilter &other)
{
    d = other.d;
    return *this;
}

/*!
  Returns true if the set of categories described by \a list is accepted by the QCategoryFilter
  object.  Otherwise returns false.
*/
bool QCategoryFilter::accepted(const QList<QString> &list) const
{
    if (d->type == Unfiled)
        return (list.count() == 0);
    if (d->type == All)
        return true;

    foreach(QString cat, d->required) {
        if (!list.contains(cat))
            return false;
    }
    return true;
}

/*!
  Returns true if the QCategoryFilterObject accepts all category sets.
  Otherwise returns false.
*/
bool QCategoryFilter::acceptAll() const
{
    return d->type == All;
}

/*!
  Returns true if the QCategoryFilterObject only accepts empty category sets.
  Otherwise returns false.
*/
bool QCategoryFilter::acceptUnfiledOnly() const
{
    return d->type == Unfiled;
}

/*!
  Returns the list of categories that must be in a set for the set to be accepted.
*/
QList<QString> QCategoryFilter::requiredCategories() const
{
    if (d->type == List)
        return d->required;
    return QList<QString>();
}

static const QString QCategoryFilter_AcceptAll = "AcceptAll";// no tr
static const QString QCategoryFilter_RequireEmpty = "RequireEmpty";// no tr
static const QString QCategoryFilter_RequireList = "RequireList";// no tr

/*!
  Writes the QCategoryFilter to the QSettings file \a c using the key \a key.

  \sa readConfig()
*/
void QCategoryFilter::writeConfig(QSettings &c, const QString &key) const
{
    QString value;
    // version
    value += "0000 ";
    // type
    switch(d->type) {
        case All:
            value += QCategoryFilter_AcceptAll;
            break;
        case Unfiled:
            value += QCategoryFilter_RequireEmpty;
            break;
        case List:
            value += QCategoryFilter_RequireList;
            value += " " + QStringList(d->required).join(" ");
            break;
    }
    c.setValue(key, value);
}

/*!
  Reads the QCategoryFilter from the QSettings file \a c using the key \a key.

  \sa writeConfig()
*/
void QCategoryFilter::readConfig(const QSettings &c, const QString &key)
{
    QString value = c.value(key).toString();
    if (value.left(4) == "0000") {
        // latest version.
        value = value.mid(5);
        if (value.left(QCategoryFilter_AcceptAll.length()) == QCategoryFilter_AcceptAll) {
            d->type = All;
            return;
        }
        if (value.left(QCategoryFilter_RequireEmpty.length()) == QCategoryFilter_RequireEmpty) {
            d->type = Unfiled;
            return;
        }
        if (value.left(QCategoryFilter_RequireList.length()) == QCategoryFilter_RequireList) {
            d->type = List;
            value = value.mid(QCategoryFilter_RequireList.length()+1);
            d->required = value.split(" ", QString::SkipEmptyParts);
            return;
        }
    }
}

/*!
  Returns a translated string that briefly describe the QCategoryFilter object.
  \a scope specifies the scope in which to search for the label.
*/
QString QCategoryFilter::label(const QString &scope) const
{
    if (d->type == Unfiled)
        return QCategoryManager::unfiledLabel();
    if (d->type == All || d->required.count() == 0)
        return QCategoryManager::allLabel();

    if (d->required.count() == 1) {
        QCategoryManager cat(scope);
        return cat.label(d->required[0]);
    }

    return QCategoryManager::multiLabel();
}

/*!
  Returns true if the QCategoryFilter object is equivilent to \a other.
  Otherwise returns false.
*/
bool QCategoryFilter::operator==(const QCategoryFilter &other) const
{
    if (other.d->type != d->type)
        return false;
    return other.d->required == d->required;
}

/*!
  Returns true if the QCategoryFilter object is not equivilent to \a other.
  Otherwise returns false.
*/
bool QCategoryFilter::operator!=(const QCategoryFilter &other) const
{
    return !( *this == other );
}

/* Must be self loading/self reloading. */

class QCategoryManagerData
{
public:
    QCategoryManagerData(const QString &s)
        : scope(s){}

    const QString scope;
};

/*!
  \class QCategoryManager
  \brief The QCategoryManager class provides functionality for modifying the set of available Categories.

  The QCategoryManager class provides a set of functions to create, modify, and remove
  Categories.  It also provides signals to notify when another program or object has
  modified the set of available Categories.

  \ingroup categories
*/

/*!
  \fn void QCategoryManager::categoriesChanged()

  This signal is emitted when any change are made to any global categories or any
  category in the scope of the QCategoryManager object.
*/

/*!
  Constructs a QCategoryManager object with parent \a parent.
  Only global categories will be managed by the QCategoryManager object.
*/
QCategoryManager::QCategoryManager(QObject *parent)
    : QObject(parent)
{
    d = new QCategoryManagerData(QString());
#ifndef QTOPIA_CONTENT_INSTALLER
    if (qApp)
        connect(qApp, SIGNAL(categoriesChanged()), this, SLOT(reloadCategories()));
#endif
}

/*!
  Constructs a QCategoryManager object with parent \a parent.
  If \a scope is null then only global categories will be managed by the QCategoryManager object.
  Otherwise will manage both global categories and categories restricted to \a scope.
*/
QCategoryManager::QCategoryManager(const QString &scope, QObject *parent)
    : QObject(parent)
{
    d = new QCategoryManagerData(scope);
#ifndef QTOPIA_CONTENT_INSTALLER
    if (qApp)
        connect(qApp, SIGNAL(categoriesChanged()), this, SLOT(reloadCategories()));
#endif
}

/*!
  Destroys a CategoryManager object.
*/
QCategoryManager::~QCategoryManager()
{
    delete d;
}

/*!
  If there is a category id \a id in the scope of the QCategoryManager
  returns the translated label for the category id.  Otherwise returns a
  null string.
*/
QString QCategoryManager::label(const QString &id) const
{
    static const QString labelquery = QLatin1String("SELECT categorytext FROM categories WHERE categoryid = :id");

    foreach(const QSqlDatabase &db, QtopiaSql::databases())
    {
        QSqlQuery q(db);
        q.prepare(labelquery);
        q.bindValue(":id", id);

        QtopiaSql::logQuery( q );
        if (!q.exec())
            qWarning() << "QCategoryManager::label failed:" << q.lastError();

        if (q.next()) {
            QString text = q.value(0).toString();
            if ( isSystem(id) )
                text = Qtopia::translate("Categories-*", "Categories", text);
            return text;
        }
    }
    return QString();
}

/*!
  If there is a category id \a id in the scope of the QCategoryManager
  returns the icon for the category id.  Otherwise returns a
  null icon.
*/
QIcon QCategoryManager::icon(const QString &id) const
{
    static const QString labelquery = QLatin1String("SELECT categoryicon FROM categories WHERE categoryid = :id");

    foreach(const QSqlDatabase &db, QtopiaSql::databases())
    {
        QSqlQuery q(db);
        q.prepare(labelquery);
        q.bindValue(":id", id);

        QtopiaSql::logQuery( q );
        if (!q.exec())
            qWarning() << "QCategoryManager::label failed:" << q.lastError();

        if (q.next())
            return QIcon(":image/" + q.value(0).toString());
    }
    return QIcon();
}

/*!
  If there is a category id \a id in the scope of the QCategoryManager
  returns the icon filename for the category id.  Otherwise returns an
  empty string.
 */
QString QCategoryManager::iconFile(const QString &id) const
{
    static const QString labelquery = QLatin1String("SELECT categoryicon FROM categories WHERE categoryid = :id");

    foreach(const QSqlDatabase &db, QtopiaSql::databases())
    {
        QSqlQuery q(db);
        q.prepare(labelquery);
        q.bindValue(":id", id);

        QtopiaSql::logQuery( q );
        if (!q.exec())
            qWarning() << "QCategoryManager::label failed:" << q.lastError();

        if (q.next())
            return q.value(0).toString();
    }
    return QString();
}

/*!
  Returns a list containing the translated label for each
  category id in the list \a l that is in the scope of the QCategoryManager.
  The list returned will have a count smaller than the list if id's \a l
  if one or more of the ids in the list \a l are not in the scope of the QCategoryManager.
*/
QList<QString> QCategoryManager::labels(const QList<QString> &l) const
{
    QList<QString> r;
    foreach(QString i, l) {
        if (contains(i))
            r.append(label(i));
    }
    return r;
}

/*!
  Returns the translated label for the empty set of categories.  Also known as unfiled
  due to the set not yet having any categories assigned.
*/
QString QCategoryManager::unfiledLabel()
{
    return tr("Unfiled");
}

/*!
  Returns the translated label for the set of all categories.
*/
QString QCategoryManager::allLabel()
{
    return tr("All");
}

/*!
  Returns a translated label for a set containing two or more categories.
*/
QString QCategoryManager::multiLabel()
{
    return tr("(Multi) ...");
}

/*!
  Returns true if the category for id \a id is unscoped.
*/
bool QCategoryManager::isGlobal(const QString &id) const
{
    static const QString globalquery = QLatin1String("SELECT count(*) FROM categories WHERE categoryid = :id AND categoryscope IS NULL");

    foreach(const QSqlDatabase &db, QtopiaSql::databases())
    {
        QSqlQuery q(db);
        q.prepare(globalquery);
        q.bindValue(":id", id);

        QtopiaSql::logQuery( q );
        if (!q.exec())
            qWarning() << "QCategoryManager::isGlobal failed:" << q.lastError().text();
        if (q.next() && q.value(0).toInt() > 0)
            return true;
    }
    return false;
}

/*!
  Returns true if the category identified by \a id is a system category.
  Otherwise returns false.
*/
bool QCategoryManager::isSystem(const QString &id) const
{
    static const QString systemquery = QLatin1String("SELECT count(*) FROM categories WHERE categoryid = :id AND (flags & 1) = 1");

    foreach(const QSqlDatabase &db, QtopiaSql::databases())
    {
        QSqlQuery q(db);
        q.prepare(systemquery);
        q.bindValue(":id", id);

        QtopiaSql::logQuery( q );
        if (!q.exec())
            qWarning() << "QCategoryManager::isSystem failed:" << q.lastError().text();
        if (q.next() && q.value(0).toInt() > 0)
            return true;
    }
    return false;
}

/*!
    Sets the category with category id \a id to be a system category.
    Doing this will make the category read only and undeleteable. Use with care.
 */
bool QCategoryManager::setSystem(const QString &id)
{
    static const QString systemquery = QLatin1String("udpate categories set flags=flags|1 where categoryid=:id");

    bool allSucceeded = true;

    foreach(const QSqlDatabase &db, QtopiaSql::databases())
    {
        QSqlQuery q(db);
        q.prepare(systemquery);
        q.bindValue(":id", id);

        QtopiaSql::logQuery( q );
        if (!q.exec()) {
            qWarning() << "QCategoryManager::setSystem failed:" << q.lastError().text();
            allSucceeded = false;
        }
    }
    return allSucceeded;
}

/*!
  If \a global is true, sets the category with category id \a id to be a global category.
  Otherwise if there is a global category with category id \a id will make that category
  scoped to the current QCategoryManager object's scope.
*/
bool QCategoryManager::setGlobal(const QString &id, bool global)
{
    // will make any category global.
    if (!contains(id))
        return false;
    if (isGlobal(id) == global)
        return true;

    bool allSucceeded = true;

    if (global) {
        // straight update.
        static const QString makelocal = QLatin1String("UPDATE categories SET categoryscope = NULL WHERE categoryid = :id");
        foreach(const QSqlDatabase &db, QtopiaSql::databases())
        {
            QSqlQuery q(db);
            q.prepare(makelocal);
            q.bindValue(":id", id);

            QtopiaSql::logQuery( q );
            if (!q.exec()) {
                qWarning() << "QCategoryManager::setGlobal failed:" << q.lastError();
                allSucceeded = false;
            }
        }
        { QtopiaIpcEnvelope e("QPE/System", "categoriesChanged()" ); }
        return allSucceeded;
    } else {
        if (d->scope.isEmpty())
            return false;
        static const QString makelocal = QLatin1String("UPDATE categories SET categoryscope = :categoryscope WHERE categoryid = :id");
        foreach(const QSqlDatabase &db, QtopiaSql::databases())
        {
            QSqlQuery q(db);
            q.prepare(makelocal);
            q.bindValue(":id", id);
            q.bindValue(":categoryscope", d->scope);

            QtopiaSql::logQuery( q );
            if (!q.exec()) {
                qWarning() << "QCategoryManager::setGlobal failed:" << q.lastError();
                allSucceeded = false;
            }
        }
        { QtopiaIpcEnvelope e("QPE/System", "categoriesChanged()" ); }
        return allSucceeded;
    }
}

/*!
  Creates a new category with the user-supplied label \a trLabel and icon \a icon.
  The category is created in the scope of the QCategoryManager unless \a forceGlobal
  is true, when it will be created in the global scope.
  If the QCategoryManager does not have a scope, the category will be created in the global scope.

  Returns the id of the new category if the new category is successfully added.
  Otherwise returns the null string.

  Note that this function is not suitable for applications wishing to create categories
  programmatically. Instead, a system category should be created, using
  the \link addCategory() \endlink
  function.
*/
QString QCategoryManager::add( const QString &trLabel, const QString &icon, bool forceGlobal )
{
    QString id=trLabel;
    if ( id.isEmpty() ) {
        id = "empty";
    }

    if ( !contains(id) ) {
        if ( addCategory(id, trLabel, icon, forceGlobal) )
            return id;
        else
            return QString();
    }

    QString key;
    for ( int i = 1; i < 1000; i++ ) {
        key = QString("%1_%2").arg(id).arg(i);
        if ( !contains(key) ) {
            if ( addCategory(key, trLabel, icon, forceGlobal) )
                return key;
            else
                return QString();
    }
    }

    return QString();
}

/*!
  Creates a new category with id \a id, the user-supplied label \a trLabel and icon \a icon
  in the classes scope. If \a forceGlobal is true, when the category is created it will be
  created as a global category (ie with no scope). If the scope the class was created with is
  null the category will be global also. If \a isSystem is set to true, will set the system
  flag on the category, and will make it unmodifiable, and unremovable.

  QCategoryManager handles 2 types of categories.

  User categories have an id that does not start with an underscore. They use the trLabel field
  as a translated string which will not change when the user changes languages. The user can edit
  and delete user categories.

  System categories can only be created programmatically. The user cannot edit or delete
  system categories. System categories use the trLabel field as an untranslated string that
  is passed through the translation engine by QCategoryManager, allowing the text to change
  when the user changes languages. The translations should be installed by the application
  with a name like Categories-myapp.qm so that QCategoryManager can load them.

  Note that the \a id must be unique. Unless the category is intended to be used by multiple
  applications, it is recommended to prepend a unique string (eg. the binary name) to the id.

  Returns true if the new category is successfully added.  Otherwise returns false.

  \sa QCategoryManager()
*/
bool QCategoryManager::addCategory( const QString &id, const QString &trLabel, const QString &icon, bool forceGlobal, bool isSystem )
{
    if (contains(id))
        return false;

    static const QString insertquery = QLatin1String("INSERT INTO categories (categoryid, categorytext, categoryscope, categoryicon, flags) VALUES (:id, :label, :categoryscope, :categoryicon, :flags)");

    bool allSucceeded = true;

    foreach(const QSqlDatabase &db, QtopiaSql::databases())
    {
        QSqlQuery q(db);
        q.prepare(insertquery);
        q.bindValue(":id", id);
        q.bindValue(":label", trLabel);
        q.bindValue(":categoryscope", forceGlobal?QString():d->scope);
        q.bindValue(":categoryicon", icon);
        q.bindValue(":flags", isSystem ? 1:0);

        QtopiaSql::logQuery( q );
        if (!q.exec()) {
            qWarning() << "QCategoryManager::add failed:", q.lastError();
            allSucceeded = false;
        }
    }
    { QtopiaIpcEnvelope e("QPE/System", "categoriesChanged()" ); }
    return allSucceeded;
}

/*!
  Attempts to remove the category with category id \a id as long as the category is either
  global or in the scope of the QCategoryManager object.
  Returns true If the category is successfully removed.
  Otherwise returns false.
*/
bool QCategoryManager::remove( const QString &id )
{
    if (!contains(id) || isSystem(id))
        return false;

    static const QString remquery = QLatin1String("DELETE FROM categories WHERE categoryid = :id");

    bool allSucceeded = true;

    foreach(const QSqlDatabase &db, QtopiaSql::databases())
    {
        QSqlQuery q(db);
        q.prepare(remquery);
        q.bindValue(":id", id);

        QtopiaSql::logQuery( q );
        if (!q.exec()) {
            qWarning() << "QCategoryManager::remove failed:" << q.lastError();
            allSucceeded = false;
        }
    }
    { QtopiaIpcEnvelope e("QPE/System", "categoriesChanged()" ); }
    return allSucceeded;
}

/*!
  Attempts to rename the category with category id \a id to have the translated label \a trLabel.
  Returns true If the category is successfully renamed.
  Otherwise returns false.
*/
bool QCategoryManager::setLabel( const QString &id, const QString &trLabel )
{
    if (!contains(id) || isSystem(id))
        return false;

    if (label(id) == trLabel)
        return true;

    bool allSucceeded = true;

    static const QString namequery = QLatin1String("UPDATE categories SET categorytext = :label WHERE categoryid = :id");
    foreach(const QSqlDatabase &db, QtopiaSql::databases())
    {
        QSqlQuery q(db);
        q.prepare(namequery);
        q.bindValue(":id", id);
        q.bindValue(":label", trLabel);

        QtopiaSql::logQuery( q );
        if (!q.exec()) {
            qWarning() << "QCategoryManager::setLabel failed:" << q.lastError();
            allSucceeded = false;
        }
    }
    { QtopiaIpcEnvelope e("QPE/System", "categoriesChanged()" ); }
    return allSucceeded;
}

/*!
  Attempts to set the icon for the category with category id \a id to \a icon.
  Returns true If the category icon is changed successfully.
  Otherwise returns false.
*/
bool QCategoryManager::setIcon( const QString &id, const QString &icon )
{
    if (!contains(id) || isSystem(id))
        return false;

    bool allSucceeded = true;

    static const QString namequery = QLatin1String("UPDATE categories SET categoryicon = :icon WHERE categoryid = :id");
    foreach(const QSqlDatabase &db, QtopiaSql::databases())
    {
        QSqlQuery q(db);
        q.prepare(namequery);
        q.bindValue(":id", id);
        q.bindValue(":icon", icon);

        QtopiaSql::logQuery( q );
        if (!q.exec()) {
            qWarning() << "QCategoryManager::setIcon failed:" << q.lastError();
            allSucceeded = false;
        }
    }
    { QtopiaIpcEnvelope e("QPE/System", "categoriesChanged()" ); }
    return allSucceeded;
}

/*!
  Returns true if there is a global category or a category within the
  scope of the QCategoryManager with the category id \a id.  Otherwise returns false.
*/
bool QCategoryManager::contains(const QString &id) const
{
    bool result=false;
    static const QString existsGlobalQuery = QLatin1String(
            "SELECT count(*) FROM categories WHERE categoryid = :id AND categoryscope IS NULL");
    static const QString existsQuery = QLatin1String(
            "SELECT count(*) FROM categories WHERE categoryid = :id AND (categoryscope = :categoryscope OR categoryscope IS NULL)");

    foreach(const QSqlDatabase &db, QtopiaSql::databases())
    {
        QSqlQuery q(db);
        if( d->scope.isEmpty() )
            q.prepare(existsGlobalQuery);
        else
            q.prepare(existsQuery);
        q.bindValue(":id", id);
        if (!d->scope.isEmpty())
            q.bindValue(":categoryscope", d->scope);

        QtopiaSql::logQuery( q );
        if (!q.exec()) {
            qWarning() << "QCategoryManager::contains failed:" << q.lastError();
        }
        result |= q.next() && q.value(0).toInt() > 0;
    }
    return result;
}

/*!
    Returns true if there is a category in any scope with the category id \a id.
*/
bool QCategoryManager::exists( const QString &id ) const
{
    static const QString existsQuery = QLatin1String( "SELECT count(*) FROM categories WHERE categoryid = :id");

    foreach(const QSqlDatabase &db, QtopiaSql::databases())
    {
        QSqlQuery q(db);
        q.prepare(existsQuery);
        q.bindValue(":id", id);

        QtopiaSql::logQuery( q );
        if (!q.exec()) {
            qWarning() << "QCategoryManager::contains failed:" << q.lastError();
        }
        if( q.next() && q.value(0).toInt() > 0 )
            return true;
    }
    return false;
}

/*!
  Returns true if there is a global category or a category within the
  scope of the QCategoryManager with the label \a label.  Otherwise returns false.
  Note that this may produce unexpected results for system categories, in this case, set
  \a forceGlobal to true to detect the presence of global categories as well.
 */
bool QCategoryManager::containsLabel(const QString &label, bool forceGlobal) const
{
    static const QString existsQuery = QLatin1String("SELECT count(*) FROM categories WHERE categorytext = :label AND (categoryscope = :categoryscope OR categoryscope IS NULL)");
    static const QString existsGlobalQuery = QLatin1String("SELECT count(*) FROM categories WHERE categorytext = :label");
    bool result = false;
    foreach(const QSqlDatabase &db, QtopiaSql::databases())
    {
        QSqlQuery q(db);
        if ( forceGlobal )
            q.prepare(existsGlobalQuery);
        else
            q.prepare(existsQuery);
        q.bindValue(":label", label);
        if ( !forceGlobal )
            q.bindValue(":categoryscope", d->scope);

        QtopiaSql::logQuery( q );
        if (!q.exec()) {
            qWarning() << "Failed check for existing category with translated label" << label << "Error:" << q.lastError();
        }

        result |= q.next() && q.value(0).toInt() > 0;
    }
    return result;
}

/*!
  Returns the category ID if there is a global category or a category within the
  scope of the QCategoryManager with the label \a label.  Otherwise returns
  an empty string.

  Note that this may produce unexpected results for system categories.
 */
QString QCategoryManager::idForLabel(const QString &label) const
{
    static QString existsquery = QLatin1String("SELECT categoryid FROM categories WHERE categorytext = :label AND (categoryscope = :categoryscope OR categoryscope IS NULL)");
    static QString existsGlobalCategory = QLatin1String( "SELECT categoryid FROM categories WHERE categorytext = :label AND categoryscope IS NULL" );
    foreach(const QSqlDatabase &db, QtopiaSql::databases())
    {
        QSqlQuery q(db);
        q.prepare(existsquery);
        q.bindValue(":label", label);
        q.bindValue(":categoryscope", d->scope);

        QtopiaSql::logQuery( q );
        if (!q.exec()) {
            qWarning() << "Failed check for existing category with translated label" << label << "Error:" << q.lastError();
        }

        if (q.next())
            return q.value(0).toString();
    }
    return QString();
}

/*!
  Returns the set of global categories joined with the set of categories contained
  within the scope of the QCategoryManager object.
*/
QList<QString> QCategoryManager::categoryIds() const
{
    static const QString idquery = QLatin1String("SELECT categoryid, categorytext FROM categories WHERE categoryscope = :categoryscope OR categoryscope IS NULL");
    static const QString gidquery = QLatin1String("SELECT categoryid, categorytext FROM categories WHERE categoryscope IS NULL");
    QMap<QString,QString> result;
    foreach(const QSqlDatabase &db, QtopiaSql::databases())
    {
        QSqlQuery q(db);
        q.prepare(d->scope.isEmpty() ? gidquery : idquery);
        if (!d->scope.isEmpty())
            q.bindValue(":categoryscope", d->scope);

        QtopiaSql::logQuery( q );
        if (!q.exec())
        {
            qWarning() << "QCategoryManager::categoryIds failed" << q.lastError();
        }

        while (q.next())
        {
            QString id   = q.value(0).toString();
            QString text = q.value(1).toString();

            if( result.key( id ).isEmpty() )
                result.insert( isSystem(id) ? Qtopia::translate("Categories-*", "Categories", text) : text, id );
        }
    }

    return result.values();
}

void QCategoryManager::reloadCategories()
{
    emit categoriesChanged();
}
