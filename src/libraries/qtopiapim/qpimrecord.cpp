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

#include <qtopia/pim/qpimrecord.h>
#include "vobject_p.h"
#include <qtopia/qcategorymanager.h>
#include <qdatastream.h>
#include <qtranslator.h>
#include <qtextcodec.h>
#include <qtopia/qtopiaapplication.h>
#include <qsettings.h>
#include <qobject.h>
#include <stdlib.h>

/*!
  \class QPimRecord
  \mainclass
  \module qpepim
  \ingroup pim
  \brief The QPimRecord class is the base class for PIM data recorded in the
  Qtopia database.

  The QPimRecord class contains data that is common to the differing kinds
  of PIM records such as contacts, tasks and appointments.  It is an also
  abstract and as such should not be created explicitly.  Instead use one
  of the subclasses, QContact, QAppointment or QTask.
*/

/*!
  \fn QMap<QString, QString> QPimRecord::customFields() const

  Returns a map of custom field key and value for the record.

  \sa setCustomFields()
*/


/*!
  \fn QString QPimRecord::notes() const
  Returns the notes for the record.

  \sa setNotes()
*/

/*!
  \fn QString QPimRecord::setNotes(const QString &text)
  Sets the notes for the record to the given \a text.

  \sa notes()
*/

/*!
  \fn QUniqueId &QPimRecord::uidRef()

  Subclass should re-implement this function to return a reference to the identifier for this object.
*/

/*!
  \fn const QUniqueId &QPimRecord::uidRef() const
  Subclass should re-implement this function to return a reference to the identifier for this object.
*/

/*!
  \fn QList<QString> &QPimRecord::categoriesRef()
  Subclass should re-implement this function to return a reference to the categories for this object.
*/

/*!
  \fn const QList<QString> &QPimRecord::categoriesRef() const
  Subclass should re-implement this function to return a reference to the categories for this object.
*/

/*!
  \fn QMap<QString, QString> &QPimRecord::customFieldsRef()
  Subclass should re-implement this function to return a reference to the custom fields for this object.
*/

/*!
  \fn const QMap<QString, QString> &QPimRecord::customFieldsRef() const
  Subclass should re-implement this function to return a reference to the custom fields for this object.
*/

/*!
  Destroys the record
*/

QPimRecord::~QPimRecord()
{
}

/*!
  Returns true if the record and the \a other record have all the same:
  \list
  \o UID
  \o categories
  \o custom fields
  \o notes
  \endlist
  and otherwise returns false.

  \sa operator!=()
*/
bool QPimRecord::operator==( const QPimRecord &other ) const
{
    if (uidRef() != other.uidRef())
        return false;
    if (categoriesRef() != other.categoriesRef())
        return false;
    if (customFieldsRef() != other.customFieldsRef())
        return false;
    if (notes() != other.notes())
        return false;
    return true;
}

/*!
  Returns false if the record and the \a other record have all the same:
  \list
  \o UID
  \o categories
  \o custom fields
  \o notes
  \endlist
  and otherwise returns true.

  \sa operator==()
*/
bool QPimRecord::operator!=( const QPimRecord &other ) const
{
    return !(*this == other);
}

/*!
  Sets the record to belong to the given set of \a categories.

  \sa categories(), Categories
*/
void QPimRecord::setCategories( const QList<QString> &categories )
{
    categoriesRef() = categories;
}

/*!
  Sets the record to belong only to the given category \a identifier.

  \sa categories(), Categories
*/
void QPimRecord::setCategories( const QString & identifier )
{
    QList<QString> newcats;
    newcats.append(identifier);
    categoriesRef() = newcats;
}

/*!
  Renames category \a oldId in record to category \a newId
*/
void QPimRecord::reassignCategoryId( const QString & oldId, const QString & newId )
{
    QList<QString> &cRef = categoriesRef();
    // workaround for qt bug which gives qWarnings on calling find on an empty array
    if ( !cRef.count() )
        return;

    QMutableListIterator<QString> it(cRef);
    if (it.findNext(oldId))
        it.setValue(newId);
}

/*!
  Removes categories from record that do not appear in the
  given set of valid \a categories.
  Returns true if the record was modified.
*/
bool QPimRecord::pruneDeadCategories(const QList<QString> &categories)
{
    QList<QString> &cRef = categoriesRef();
    QMutableListIterator<QString> it(cRef);
    bool ret = false;
    while(it.hasNext()) {
        QString id = it.next();
        if (!categories.contains(id)) {
            ret = true;
            it.remove();
        }
    }

    return ret;
}


/*!
  Returns the set of categories the record belongs to.

  \sa setCategories()
*/
QList<QString> QPimRecord::categories() const
{
    return categoriesRef();
}

/*!
  \fn QUniqueId QPimRecord::uid() const

  Returns the unique identifier for this record.
  
  \sa setUid()
*/

/*!
  \fn void QPimRecord::setUid(const QUniqueId &identifier)

  Sets the record to have given unique \a identifier.

  \sa uid()
*/

/*!
  Returns the string stored for the custom field \a key.
  Returns a null string if the field does not exist.
 */
QString QPimRecord::customField(const QString &key) const
{
    if (customFieldsRef().contains(key))
        return customFieldsRef()[key];

    return QString();
}

/*!
\fn void QPimRecord::setCustomFields(const QMap<QString, QString> &fields)

    Sets the custom fields for the record to the given \a fields.

    Custom fields allow storing data that doesn't fit into the existing
    fields for a given PIM record.
*/

/*!
  Sets the string stored for the custom field \a key to the given \a value.
 */
void QPimRecord::setCustomField(const QString &key, const QString &value)
{
    customFieldsRef().insert(key, value);
}

/*!
  Removes the custom field for the given \a key.
 */
void QPimRecord::removeCustomField(const QString &key)
{
    customFieldsRef().remove(key);
}

static QTextCodec* vobj_codec=0;
static QStringList* comps=0;

void qpe_startVObjectInput()
{
}

void qpe_startVObjectOutput()
{
    QSettings pimConfig("Trolltech","Beam");
    pimConfig.beginGroup("Send");
    QString cs = "UTF-8";
    QString dc = pimConfig.value("DeviceConfig").toString();
    if ( !dc.isEmpty() ) {
        QSettings devcfg(pimConfig.value("DeviceConfig").toString(), QSettings::IniFormat);
        if ( devcfg.status()==QSettings::NoError ) {
            devcfg.beginGroup("Send");
            cs = devcfg.value("CharSet","UTF-8").toString();
            QString comp = devcfg.value("Compatibility").toString();
            comps = new QStringList(comp.split(" "));
        }
    }
    vobj_codec = QTextCodec::codecForName(cs.toLatin1());
}

bool qpe_vobjectCompatibility(const char* misfeature)
{
    return comps && comps->contains(misfeature);
}

void qpe_setVObjectProperty(const QString& name, const QString& value, const char* type, QPimRecord* r)
{
    // Ian says categories shouldn't be honoured for VObjects
#if 0
    QCategoryManager m(type, 0);
    if ( name == VCCategoriesProp ) {
        QStringList cl = value.split(";");
        QList<QString> ca;
        for (QStringList::ConstIterator it=cl.begin(); it!=cl.end(); ++it) {
            QString cname = *it;
            if ( cname.left(2) == "X-" )
                cname = cname.mid(2);
            // need an extended contains for 'close matches'
            if (cname != "Unfiled" && !m.contains(cname)) { // No tr
                m.addTr(cname);
            }
            ca.append(cname);
        }
        r->setCategories(ca);
    }
#else
    Q_UNUSED(name);
    Q_UNUSED(value);
    Q_UNUSED(type);
    Q_UNUSED(r);
#endif
}

VObject *qpe_safeAddPropValue( VObject *o, const char *prop, const QString &value )
{
    VObject *ret = 0;
    if ( o && !value.isEmpty() ) {
        if ( vobj_codec )
            ret = addPropValue( o, prop, vobj_codec->fromUnicode(value) );
        else
            ret = addPropValue( o, prop, value.toLatin1() ); // NOT UTF-8, that is by codec
    }
    return ret;
}

VObject *qpe_safeAddProp( VObject *o, const char *prop)
{
    VObject *ret = 0;
    if ( o )
        ret = addProp( o, prop );
    return ret;
}

void qpe_endVObjectInput()
{
}

void qpe_endVObjectOutput(VObject *o, const char* /*type*/, const QPimRecord* r)
{
    QStringList c = r->categories();
    qpe_safeAddPropValue( o, VCCategoriesProp, c.join(";") );
    delete comps;
    comps = 0;
}
