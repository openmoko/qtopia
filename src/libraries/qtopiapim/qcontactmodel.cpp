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
#include <qtopia/pim/qcontactmodel.h>
#include "qcontactsqlio_p.h"
#include "vobject_p.h"
#include <qtopianamespace.h>
#ifdef QTOPIA_CELL
#include "qsimcontext_p.h"
#endif
#ifdef QTOPIA_PHONE
#include <qtopia/inputmatch/pkimmatcher.h>
#endif
#include <QSettings>
#include <QPainter>
#include <QMap>
#include <QSet>
#include <QPixmap>
#include <QGlobalPixmapCache>
#include <QImageReader>
#include <QFile>
#include <QTextDocument>

#include <QDebug>

#include "qrecordiomerge_p.h"

QMap<QContactModel::Field, QString> QContactModel::k2i;
QMap<QString, QContactModel::Field> QContactModel::i2k;
QMap<QContactModel::Field, QString>  QContactModel::k2t;

/*!
  \internal

  Initializes mappings from column enums to translated and non-translated strings.
*/
void QContactModel::initMaps()
{
    if (k2t.count() > 0)
        return;
    struct KeyLookup {
        const char* ident;
        const char* trans;
        Field key;
    };
    static const KeyLookup l[] = {
        // name
        { "title", QT_TR_NOOP("Title"), NameTitle },
        { "firstname", QT_TR_NOOP( "First Name" ), FirstName },
        { "middlename", QT_TR_NOOP( "Middle Name" ), MiddleName },
        { "lastname", QT_TR_NOOP( "Last Name" ), LastName },
        { "suffix", QT_TR_NOOP( "Suffix" ), Suffix },
        { "label", QT_TR_NOOP( "Label" ), Label },

        // email
        { "defaultemail", QT_TR_NOOP( "Default Email" ), DefaultEmail },
        { "emails", QT_TR_NOOP( "Emails" ), Emails },

        // other
        { "otherphone", QT_TR_NOOP( "Home Phone" ), OtherPhone },
        { "otherfax", QT_TR_NOOP( "Home Fax" ), OtherFax},
        { "othermobile", QT_TR_NOOP( "Home Mobile" ), OtherMobile},
        { "otherpager", QT_TR_NOOP( "Home Pager" ), OtherPager},

        // home
        { "homestreet", QT_TR_NOOP( "Home Street" ), HomeStreet },
        { "homecity", QT_TR_NOOP( "Home City" ), HomeCity },
        { "homestate", QT_TR_NOOP( "Home State" ), HomeState },
        { "homezip", QT_TR_NOOP( "Home Zip" ), HomeZip },
        { "homecountry", QT_TR_NOOP( "Home Country" ), HomeCountry },
        { "homephone", QT_TR_NOOP( "Home Phone" ), HomePhone },
        { "homefax", QT_TR_NOOP( "Home Fax" ), HomeFax},
        { "homemobile", QT_TR_NOOP( "Home Mobile" ), HomeMobile},
        { "homepager", QT_TR_NOOP( "Home Pager" ), HomePager},
        { "homewebpage", QT_TR_NOOP( "Home Web Page" ), HomeWebPage},

        // business
        { "company", QT_TR_NOOP( "Company" ), Company},
        { "businessstreet", QT_TR_NOOP( "Business Street" ), BusinessStreet},
        { "businesscity", QT_TR_NOOP( "Business City" ), BusinessCity},
        { "businessstate", QT_TR_NOOP( "Business State" ), BusinessState},
        { "businesszip", QT_TR_NOOP( "Business Zip" ), BusinessZip},
        { "businesscountry", QT_TR_NOOP( "Business Country" ), BusinessCountry},
        { "businesswebPage", QT_TR_NOOP( "Business Web Page" ), BusinessWebPage},
        { "jobtitle", QT_TR_NOOP( "Job Title" ), JobTitle},
        { "department", QT_TR_NOOP( "Department" ), Department},
        { "office", QT_TR_NOOP( "Office" ), Office},
        { "businessphone", QT_TR_NOOP( "Business Phone" ), BusinessPhone},
        { "businessfax", QT_TR_NOOP( "Business Fax" ), BusinessFax},
        { "businessmobile", QT_TR_NOOP( "Business Mobile" ), BusinessMobile},
        { "businesspager", QT_TR_NOOP( "Business Pager" ), BusinessPager},
        { "profession", QT_TR_NOOP( "Profession" ), Profession},
        { "assistant", QT_TR_NOOP( "Assistant" ), Assistant},
        { "manager", QT_TR_NOOP( "Manager" ), Manager},

        //personal
        { "spouse", QT_TR_NOOP( "Spouse" ), Spouse},
        { "gender", QT_TR_NOOP( "Gender" ), Gender},
        { "birthday", QT_TR_NOOP( "Birthday" ), Birthday},
        { "anniversary", QT_TR_NOOP( "Anniversary" ), Anniversary},
        { "nickname", QT_TR_NOOP( "Nickname" ), Nickname},
        { "children", QT_TR_NOOP( "Children" ), Children},

        // other
        { "notes", QT_TR_NOOP( "Notes" ), Notes},
        // next to added in 4.0
        { "photofile", QT_TR_NOOP( "Photo File" ), Portrait},

        // Added in Qtopia 1.6
        { "lastnamepronunciation", QT_TR_NOOP( "Pronunciation" ), LastNamePronunciation},
        { "firstnamepronunciation", QT_TR_NOOP( "Pronunciation" ), FirstNamePronunciation},
        { "companypronunciation", QT_TR_NOOP( "Pronunciation" ), CompanyPronunciation},
        { "identifier", QT_TR_NOOP( "Identifier" ), Identifier},
        { "categories", QT_TR_NOOP( "Categories" ), Categories},
        { 0, 0, Invalid }
    };

    const KeyLookup *k = l;
    while (k->key != Invalid) {
        k2t.insert(k->key, tr(k->trans));
        k2i.insert(k->key, k->ident);
        i2k.insert(k->ident, k->key);
        ++k;
    }
}

/*!
  Returns a translated string describing the contact model field \a k.

  \sa fieldIcon(), fieldIdentifier(), identifierField()
*/
QString QContactModel::fieldLabel(Field k)
{
    if (k2t.count() == 0)
        initMaps();
    if (!k2t.contains(k))
        return QString();
    return k2t[k];
}

/*!
  Returns a icon representing the contact model field \a k.

  Returns a null icon if no icon is available.

  \sa fieldLabel(), fieldIdentifier(), identifierField()
*/
QIcon QContactModel::fieldIcon(Field k)
{
    QString ident = fieldIdentifier(k);

    if (ident.isEmpty() || !QFile::exists(":image/addressbook/" + ident))
        return QIcon();

    return QIcon(":image/addressbook/" + ident);
}


/*!
  Returns a non-translated string describing the contact model field \a k.

  \sa fieldLabel(), fieldIcon(), identifierField()
*/
QString QContactModel::fieldIdentifier(Field k)
{
    if (k2i.count() == 0)
        initMaps();
    if (!k2i.contains(k))
        return QString();
    return k2i[k];
}

/*!
  Returns the contact model field for the non-translated string identifier \a i.

  \sa fieldLabel(), fieldIcon(), fieldIdentifier()
*/
QContactModel::Field QContactModel::identifierField(const QString &i)
{
    if (i2k.count() == 0)
        initMaps();
    if (!i2k.contains(i))
        return Invalid;
    return i2k[i];
}

/*!
  Returns the contact model fields that represent phone numbers for a contact.
*/
QList<QContactModel::Field> QContactModel::phoneFields()
{
    // Used by QContact::phoneNumbers.  thats why its a static function.
    QList<Field> result;
    //result.append(OtherPhone);
    //result.append(OtherMobile);
    //result.append(OtherFax);
    //result.append(OtherPager);
    result.append(HomePhone);
    result.append(HomeMobile);
    result.append(HomeFax);
    //result.append(HomePager);
    result.append(BusinessPhone);
    result.append(BusinessMobile);
    result.append(BusinessFax);
    result.append(BusinessPager);
    return result;
}

/*!
  \internal
  Was added in 4.1.6, will become public in 4.2

  Returns known name titles for the current language settings.  These
  are used to assist in parsing and constructing contact name information.
  Contacts are not restricted to these name titles.
*/
QStringList QContactModel::localeNameTitles()
{
    return tr( "Mr;Mrs;Ms;Miss;Dr;Prof;" ).split(";", QString::SkipEmptyParts);
}

/*!
  \internal
  Was added in 4.1.6, will become public in 4.2

  Returns known suffixes for the current language settings.  These
  are used to assist in parsing and constructing contact name information.
  Contacts are not restricted to these suffixes.
*/
QStringList QContactModel::localeSuffixes()
{
    return tr( "Jr;Sr;I;II;III;IV;V" ).split(";", QString::SkipEmptyParts);
}

class QContactModelData
{
public:
    QContactModelData()
        : searchModel(0), filterFlags(0)
        {
        }

    ~QContactModelData() {
        /* other pointers collected via parenting */
        if (searchModel)
            delete searchModel;
    }

    QBiasedRecordIOMerge *mio;
    QContactIO *defaultmodel;
    QContactContext *defaultContext;

    QList<QContactIO *>models;
    QList<QContactContext *> contexts;

    mutable QContactModel *searchModel;
    mutable QString filterText;
    mutable int filterFlags;
};

/*!
  \class QContactModel
  \module qpepim
  \ingroup qpepim
  \brief The QContactModel class provides access to the the Contacts data.

  The QContactModel is used to access the Contact data.  It is a descendent of QAbstractItemModel
  so is suitable for use with the Qt View classes such as QListView and QTableView as well as
  any developer custom Views.

  QContactModel provides functions for sorting and some filtering of items.
  For filters or sorting that is not provided by QContactModel it is recommend that
  QSortFilterProxyModel is used to wrap QContactModel.

  QContactModel may merge data from multiple sources such as sql files, xml files, or
  SIM card access.

  QContactModel will also refresh when changes are made in other instances of QContactModel or
  from other applications.
*/

/*!
  \enum QContactModel::QContactModelRole

  Extends Qt::ItemDataRole

  \value LabelRole
    A short formatted text label of the contacts name.
  \value SubLabelRole
    An alternative formated text label of the contacts name.
  \value PortraitRole
    A Pixmap of the contacts image, or a default image if
    none is specifically set.
  \value StatusIconRole
    An icon providing addional information about the contact.
    For instance marking as a business contact or marking the contact
    as recently called.
*/

/*!
  \enum QContactModel::Field

  Enumerates the columns when in table mode and columns used for sorting.
  Is a subset of data retrievable from a QContact.

  \omitvalue Invalid
  \omitvalue Label
  \omitvalue NameTitle
  \omitvalue FirstName
  \omitvalue MiddleName
  \omitvalue LastName
  \omitvalue Suffix
  \omitvalue JobTitle
  \omitvalue Department
  \omitvalue Company
  \omitvalue BusinessPhone
  \omitvalue BusinessFax
  \omitvalue BusinessMobile
  \omitvalue DefaultEmail
  \omitvalue Emails
  \omitvalue OtherPhone
  \omitvalue OtherFax
  \omitvalue OtherMobile
  \omitvalue OtherPager
  \omitvalue HomePhone
  \omitvalue HomeFax
  \omitvalue HomeMobile
  \omitvalue HomePager
  \omitvalue BusinessStreet
  \omitvalue BusinessCity
  \omitvalue BusinessState
  \omitvalue BusinessZip
  \omitvalue BusinessCountry
  \omitvalue BusinessPager
  \omitvalue BusinessWebPage
  \omitvalue Office
  \omitvalue Profession
  \omitvalue Assistant
  \omitvalue Manager
  \omitvalue HomeStreet
  \omitvalue HomeCity
  \omitvalue HomeState
  \omitvalue HomeZip
  \omitvalue HomeCountry
  \omitvalue HomeWebPage
  \omitvalue Spouse
  \omitvalue Gender
  \omitvalue Birthday
  \omitvalue Anniversary
  \omitvalue Nickname
  \omitvalue Children
  \omitvalue Portrait
  \omitvalue Notes
  \omitvalue LastNamePronunciation
  \omitvalue FirstNamePronunciation
  \omitvalue CompanyPronunciation
  \omitvalue Identifier
  \omitvalue Categories
 */

/*!
  \fn bool QContactModel::isPersonalDetails(int row) const

  Returns true if the contact at \a row represents the personal details of the
  device owner.  Otherwise returns false.
*/

/*!
  Constructs a QContactModel with parent \a parent.
*/
QContactModel::QContactModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    d = new QContactModelData();
    d->mio = new QBiasedRecordIOMerge(this);

    d->defaultmodel = new ContactSqlIO(this);

    QContactDefaultContext *dcon = new QContactDefaultContext(this, d->defaultmodel);

    d->defaultContext = dcon;

    d->models.append(d->defaultmodel);
    d->contexts.append(dcon);
    d->mio->setPrimaryModel(d->defaultmodel);

#ifdef QTOPIA_CELL
    QContactSimContext *scon = new QContactSimContext(this, d->defaultmodel);
    d->contexts.append(scon);
#endif
    connect(d->mio, SIGNAL(reset()), this, SLOT(voidCache()));
}

/*!
  Destructs the QContactModel.
*/
QContactModel::~QContactModel() {}

void QContactModel::voidCache()
{
    reset();
}

/*!
  Return the number of contacts visible in the in the current filter mode.
*/
int QContactModel::count() const
{
    return d->mio->count();
}

/*!
  \overload

  Returns an object that contains a serialized description of the specified \a indexes.
  The format used to describe the items corresponding to the \a indexes is obtained from
  the mimeTypes() function.

  If the list of indexes is empty, 0 is returned rather than a serialized empty list.

  Currently returns 0 but may be implemented at a future date.
*/
QMimeData * QContactModel::mimeData(const QModelIndexList &indexes) const
{
    Q_UNUSED(indexes)

    return 0;
}

/*!
  \overload

  Returns a list of MIME types that can be used to describe a list of model indexes.

  Currently returns an empty list but may be implemented at a future date.
*/
QStringList QContactModel::mimeTypes() const
{
    return QStringList();
}

/*!
  \overload

  Sorts the model by \a column in ascending order

  Currently \a order is ignored but may be implemented at a future date.
*/
void QContactModel::sort(int column, Qt::SortOrder order)
{
    Q_UNUSED(order)

    // TODO handle order later.
    if (column >= 0 && column < columnCount())
        setSortField((Field)column);
}

/*!
  \overload

  Returns the data stored under the given \a role for the item referred to by the \a index.
*/
QVariant QContactModel::data(const QModelIndex &index, int role) const
{
    //generic = QPixmap(":image/addressbook/generic-contact");
    //sim = QPixmap(":image/addressbook/sim-contact");
    QContact c(contact(index));
    switch(index.column()) {
        case Label:
            if (index.row() < rowCount()){
                // later, take better advantage of roles.
                switch(role) {
                    default:
                        break;
                    case Qt::DecorationRole:
                    case PortraitRole:
#ifdef QTOPIA_PHONE
                        if (c.portraitFile().isEmpty() && isSIMCardContact(index)) {
                            QPixmap pm;
                            static const QLatin1String key("pimcontact-sim-thumb");
                            if (!QGlobalPixmapCache::find(key, pm)) {
                                QImageReader reader(":image/addressbook/sim-contact");
                                reader.setScaledSize(QContact::thumbnailSize());
                                QImage img = reader.read();
                                if (!img.isNull()) {
                                    pm = QPixmap::fromImage(img);
                                    QGlobalPixmapCache::insert(key, pm);
                                }
                            }

                            return qvariant_cast<QPixmap>(pm);
                        }
#endif
                        return qvariant_cast<QPixmap>(c.thumbnail());
                    case Qt::DisplayRole:
                        return QVariant(c.label());
                    case Qt::EditRole:
                        return QVariant(c.uid().toByteArray());
                    case LabelRole:
                        {
                            QString l = c.label();
#if 0
                            /* only way to really match is to get
                               the contactio to do the underlining/build the label.
                               leave out for the moment till can be right.
                               */
                            if (!d->searchText.isEmpty()) {
                                int pos = searchIndex(l);
                                if (pos != -1) {
                                    // should scrub each.
                                    l = Qt::escape(l.left(pos)) +
                                        "<u>" + Qt::escape(l.mid(pos, d->searchText.length())) +
                                        "</u>" + Qt::escape(l.mid(pos + d->searchText.length()));
                                }
                            }A
#endif
                            return "<b>" + l + "</b>";
                        }
                    case SubLabelRole:
#ifdef QTOPIA_PHONE
                        if (!c.defaultPhoneNumber().isEmpty())
                            return Qt::escape(c.defaultPhoneNumber());
#endif
                        if (c.label() != c.company())
                            return Qt::escape(c.company());
                        if (c.categories().contains("Business"))
                            return Qt::escape(c.businessCity());
                        return Qt::escape(c.homeCity());
                    case StatusIconRole:
                        if (isPersonalDetails(c.uid()))
                            return QPixmap(":image/addressbook/business");
                        return QPixmap();
                }
            }
            break;
        default:
            if (index.column() > 0 && index.column() < columnCount())
                return contactField(c, (Field)index.column());
            break;
    }
    return QVariant();
}

/*!
  \overload

    Returns the number of rows under the given \a parent.
*/
int QContactModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return count();
}

/*!
  \overload

    Returns the number of columns for the given \a parent.
*/
int QContactModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return Categories+1;// last column + 1
}

/*!
  \overload
  Sets the \a role data for the item at \a index to \a value. Returns true if successful,
  otherwise returns false.
*/
bool QContactModel::setData(const QModelIndex &index, const QVariant &value, int role) const
{
    if (role != Qt::EditRole)
        return false;
    if (!index.isValid())
        return false;

    int i = index.row();
    const QContactIO *model = qobject_cast<const QContactIO*>(d->mio->model(i));
    int r = d->mio->row(i);
    if (model)
        return ((QContactIO *)model)->setContactField(r, (Field)index.column(), value);
    return false;
}

/*!
  \overload
  For every Qt::ItemDataRole in \a roles, sets the role data for the item at \a index to the
  associated value in \a roles. Returns true if successful, otherwise returns false.
*/
bool QContactModel::setItemData(const QModelIndex &index, const QMap<int,QVariant> &roles) const
{
    if (roles.count() != 1 || !roles.contains(Qt::EditRole))
        return false;
    return setData(index, roles[Qt::EditRole], Qt::EditRole);
}

/*!
  \overload

  Returns a map with values for all predefined roles in the model for the item at the
  given \a index.
*/
QMap<int,QVariant> QContactModel::itemData(const QModelIndex &index) const
{
    QMap<int, QVariant> result;
    switch (index.column()) {
        case Label:
            result.insert(Qt::DecorationRole, data(index, Qt::DecorationRole));
            result.insert(PortraitRole, data(index, PortraitRole));
            result.insert(Qt::DisplayRole, data(index, Qt::DisplayRole));
            result.insert(Qt::EditRole, data(index, Qt::EditRole));
            result.insert(LabelRole, data(index, LabelRole));
            result.insert(SubLabelRole, data(index, SubLabelRole));
            result.insert(StatusIconRole, data(index, StatusIconRole));
            break;
        default:
            result.insert(Qt::DecorationRole, data(index, Qt::DecorationRole));
            break;
    }
    return result;
}

/*!
  \overload

  Returns the data for the given \a role and \a section in the header with the
  specified \a orientation.
*/
QVariant QContactModel::headerData(int section, Qt::Orientation orientation,
        int role) const
{
    if (orientation != Qt::Horizontal)
        return QVariant();

    if (section >= 0 && section < columnCount()) {
        if (role == Qt::DisplayRole)
            return fieldLabel((Field)section);
        else if (role == Qt::EditRole)
            return fieldIdentifier((Field)section);
        else if (role == Qt::DecorationRole)
            return qvariant_cast<QIcon>(fieldIcon((Field)section));
    }
    return QVariant();
}

/*!
  \overload
    Returns the item flags for the given \a index.
*/
Qt::ItemFlags QContactModel::flags(const QModelIndex &index) const
{
    Q_UNUSED(index);
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

/*!
  \overload
    Returns the parent of the model item with the given \a index.
*/
QModelIndex QContactModel::parent(const QModelIndex &index) const
{
    Q_UNUSED(index);
    return QModelIndex();
}

/*!
  \overload
  Returns true if \a parent has any children; otherwise returns false.
  Use rowCount() on the parent to find out the number of children.

  \sa parent(), index()
*/
bool QContactModel::hasChildren(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return false;
}

/*!
  Ensures the data in Contacts is in a state suitable for syncing.
*/
bool QContactModel::flush() { return true; }

/*!
  Forces a refresh of the Contact data.
*/
bool QContactModel::refresh() { reset(); return true; }

/*!
  Returns the contact for the row specified by \a index.
  The column of \a index is ignored.
*/
QContact QContactModel::contact(const QModelIndex &index) const
{
    return contact(index.row());
}

/*!
  Returns the id for contact at the \a row specified.
*/
QUniqueId QContactModel::id(int row) const
{
    const QContactIO *model = qobject_cast<const QContactIO *>(d->mio->model(row));
    int r = d->mio->row(row);
    if (model)
        return model->id(r);
    return QUniqueId();
}

/*!
  Return the contact for the \a row specified.
*/
QContact QContactModel::contact(int row) const
{
    const QContactIO *model = qobject_cast<const QContactIO*>(d->mio->model(row));
    int r = d->mio->row(row);
    if (model)
        return model->contact(r);
    return QContact();
}

/*!
  Returns the contact with the identefier \a id.  The contact does
  not have to be in the current filter mode for it to be returned.
*/
QContact QContactModel::contact(const QUniqueId & id) const
{
    foreach(const QContactIO *model, d->models) {
        if (model->exists(id))
            return model->contact(id);
    }
    return QContact();
}

/*!
  Returns the value from \a contact that would be returned for
  field \a f as it would from a row in the QContactModel.
*/
QVariant QContactModel::contactField(const QContact &contact, QContactModel::Field f)
{
    switch(f) {
        default:
        case QContactModel::Invalid:
            break;
        case QContactModel::Identifier:
            return QVariant(contact.uid().toByteArray());
        case QContactModel::Categories:
            return QVariant(contact.categories());
        case QContactModel::NameTitle:
            return contact.nameTitle();
        case QContactModel::FirstName:
            return contact.firstName();
        case QContactModel::MiddleName:
            return contact.middleName();
        case QContactModel::LastName:
            return  contact.lastName();
        case QContactModel::Suffix:
            return contact.suffix();
        case QContactModel::Label:
            return contact.label();
        case QContactModel::JobTitle:
            return contact.label();
        case QContactModel::Department:
            return contact.department();
        case QContactModel::Company:
            return contact.company();
        case QContactModel::BusinessPhone:
            return contact.businessPhone();
        case QContactModel::BusinessFax:
            return contact.businessFax();
        case QContactModel::BusinessMobile:
            return contact.businessMobile();
        case QContactModel::DefaultEmail:
            return contact.defaultEmail();
        case QContactModel::Emails:
            return contact.emailList();

        case QContactModel::OtherPhone:
            return contact.phoneNumber(QContact::OtherPhone);
        case QContactModel::OtherFax:
            return contact.phoneNumber(QContact::Fax);
        case QContactModel::OtherMobile:
            return contact.phoneNumber(QContact::Mobile);
        case QContactModel::OtherPager:
            return contact.phoneNumber(QContact::Pager);

        case QContactModel::HomePhone:
            return contact.phoneNumber(QContact::HomePhone);
        case QContactModel::HomeFax:
            return contact.phoneNumber(QContact::HomeFax);
        case QContactModel::HomeMobile:
            return contact.phoneNumber(QContact::HomeMobile);
        case QContactModel::HomePager:
            return contact.phoneNumber(QContact::HomePager);

        case QContactModel::BusinessStreet:
            return contact.businessStreet();
        case QContactModel::BusinessCity:
            return contact.businessCity();
        case QContactModel::BusinessState:
            return contact.businessState();
        case QContactModel::BusinessZip:
            return contact.businessZip();
        case QContactModel::BusinessCountry:
            return contact.businessCountry();
        case QContactModel::BusinessPager:
            return contact.businessPager();
        case QContactModel::BusinessWebPage:
            return contact.businessWebpage();
        case QContactModel::Office:
            return contact.office();
        case QContactModel::Profession:
            return contact.profession();
        case QContactModel::Assistant:
            return contact.assistant();
        case QContactModel::Manager:
            return contact.manager();
        case QContactModel::HomeStreet:
            return contact.homeStreet();
        case QContactModel::HomeCity:
            return contact.homeCity();
        case QContactModel::HomeState:
            return contact.homeState();
        case QContactModel::HomeZip:
            return contact.homeZip();
        case QContactModel::HomeCountry:
            return contact.homeCountry();
        case QContactModel::HomeWebPage:
            return contact.homeWebpage();
        case QContactModel::Spouse:
            return contact.spouse();
        case QContactModel::Gender:
            return contact.gender();
        case QContactModel::Birthday:
            return contact.birthday();
        case QContactModel::Anniversary:
            return contact.anniversary();
        case QContactModel::Nickname:
            return contact.nickname();
        case QContactModel::Children:
            return contact.children();
        case QContactModel::Portrait:
            return contact.portraitFile();
        case QContactModel::Notes:
            return contact.notes();
        case QContactModel::LastNamePronunciation:
            return contact.lastNamePronunciation();
        case QContactModel::FirstNamePronunciation:
            return contact.firstNamePronunciation();
        case QContactModel::CompanyPronunciation:
            return contact.companyPronunciation();
    }
    return QVariant();
}

/*!
  Sets the value in \a contact that would be set for field \a f as it would
  if modified for a contact in the QContactModel to \a v.

  Returns true if the contact was modified.  Otherwise returns false.
*/
bool QContactModel::setContactField(QContact &contact, QContactModel::Field f,  const QVariant &v)
{
    switch(f) {
        default:
        case QContactModel::Invalid:
        case QContactModel::Identifier: // not a setable field
        case QContactModel::Label:
            return false;
        case QContactModel::Categories:
            if (v.canConvert(QVariant::StringList)) {
                contact.setCategories(v.toStringList());
                return true;
            }
            return false;
        case QContactModel::NameTitle:
            if (v.canConvert(QVariant::String)) {
                contact.setNameTitle(v.toString());
                return true;
            }
            return false;
        case QContactModel::FirstName:
            if (v.canConvert(QVariant::String)) {
                contact.setFirstName(v.toString());
                return true;
            }
            return false;
        case QContactModel::MiddleName:
            if (v.canConvert(QVariant::String)) {
                contact.setMiddleName(v.toString());
                return true;
            }
            return false;
        case QContactModel::LastName:
            if (v.canConvert(QVariant::String)) {
                contact.setLastName(v.toString());
                return true;
            }
            return false;
        case QContactModel::Suffix:
            if (v.canConvert(QVariant::String)) {
                contact.setSuffix(v.toString());
                return true;
            }
            return false;
        case QContactModel::JobTitle:
            if (v.canConvert(QVariant::String)) {
                contact.setJobTitle(v.toString());
                return true;
            }
            return false;
        case QContactModel::Department:
            if (v.canConvert(QVariant::String)) {
                contact.setDepartment(v.toString());
                return true;
            }
            return false;
        case QContactModel::Company:
            if (v.canConvert(QVariant::String)) {
                contact.setCompany(v.toString());
                return true;
            }
            return false;
        case QContactModel::BusinessPhone:
            if (v.canConvert(QVariant::String)) {
                contact.setBusinessPhone(v.toString());
                return true;
            }
            return false;
        case QContactModel::BusinessFax:
            if (v.canConvert(QVariant::String)) {
                contact.setBusinessFax(v.toString());
                return true;
            }
            return false;
        case QContactModel::BusinessMobile:
            if (v.canConvert(QVariant::String)) {
                contact.setBusinessMobile(v.toString());
                return true;
            }
            return false;
        case QContactModel::DefaultEmail:
            if (v.canConvert(QVariant::String)) {
                contact.setDefaultEmail(v.toString());
                return true;
            }
            return false;
        case QContactModel::Emails:
            if (v.canConvert(QVariant::StringList)) {
                contact.setEmailList(v.toStringList());
                return true;
            }
            return false;
        case QContactModel::OtherPhone:
            if (v.canConvert(QVariant::String)) {
                contact.setPhoneNumber(QContact::OtherPhone, v.toString());
                return true;
            }
            return false;
        case QContactModel::OtherFax:
            if (v.canConvert(QVariant::String)) {
                contact.setPhoneNumber(QContact::Fax, v.toString());
                return true;
            }
            return false;
        case QContactModel::OtherMobile:
            if (v.canConvert(QVariant::String)) {
                contact.setPhoneNumber(QContact::Mobile, v.toString());
                return true;
            }
            return false;
        case QContactModel::OtherPager:
            if (v.canConvert(QVariant::String)) {
                contact.setPhoneNumber(QContact::Pager, v.toString());
                return true;
            }
            return false;
        case QContactModel::HomePhone:
            if (v.canConvert(QVariant::String)) {
                contact.setPhoneNumber(QContact::HomePhone, v.toString());
                return true;
            }
            return false;
        case QContactModel::HomeFax:
            if (v.canConvert(QVariant::String)) {
                contact.setPhoneNumber(QContact::HomeFax, v.toString());
                return true;
            }
            return false;
        case QContactModel::HomeMobile:
            if (v.canConvert(QVariant::String)) {
                contact.setPhoneNumber(QContact::HomeMobile, v.toString());
                return true;
            }
            return false;
        case QContactModel::HomePager:
            if (v.canConvert(QVariant::String)) {
                contact.setPhoneNumber(QContact::HomePager, v.toString());
                return true;
            }
            return false;
        case QContactModel::BusinessStreet:
            if (v.canConvert(QVariant::String)) {
                contact.setBusinessStreet(v.toString());
                return true;
            }
            return false;
        case QContactModel::BusinessCity:
            if (v.canConvert(QVariant::String)) {
                contact.setBusinessCity(v.toString());
                return true;
            }
            return false;
        case QContactModel::BusinessState:
            if (v.canConvert(QVariant::String)) {
                contact.setBusinessState(v.toString());
                return true;
            }
            return false;
        case QContactModel::BusinessZip:
            if (v.canConvert(QVariant::String)) {
                contact.setBusinessZip(v.toString());
                return true;
            }
            return false;
        case QContactModel::BusinessCountry:
            if (v.canConvert(QVariant::String)) {
                contact.setBusinessCountry(v.toString());
                return true;
            }
            return false;
        case QContactModel::BusinessPager:
            if (v.canConvert(QVariant::String)) {
                contact.setBusinessPager(v.toString());
                return true;
            }
            return false;
        case QContactModel::BusinessWebPage:
            if (v.canConvert(QVariant::String)) {
                contact.setBusinessWebpage(v.toString());
                return true;
            }
            return false;
        case QContactModel::Office:
            if (v.canConvert(QVariant::String)) {
                contact.setOffice(v.toString());
                return true;
            }
            return false;
        case QContactModel::Profession:
            if (v.canConvert(QVariant::String)) {
                contact.setProfession(v.toString());
                return true;
            }
            return false;
        case QContactModel::Assistant:
            if (v.canConvert(QVariant::String)) {
                contact.setAssistant(v.toString());
                return true;
            }
            return false;
        case QContactModel::Manager:
            if (v.canConvert(QVariant::String)) {
                contact.setManager(v.toString());
                return true;
            }
            return false;
        case QContactModel::HomeStreet:
            if (v.canConvert(QVariant::String)) {
                contact.setHomeStreet(v.toString());
                return true;
            }
            return false;
        case QContactModel::HomeCity:
            if (v.canConvert(QVariant::String)) {
                contact.setHomeCity(v.toString());
                return true;
            }
            return false;
        case QContactModel::HomeState:
            if (v.canConvert(QVariant::String)) {
                contact.setHomeState(v.toString());
                return true;
            }
            return false;
        case QContactModel::HomeZip:
            if (v.canConvert(QVariant::String)) {
                contact.setHomeZip(v.toString());
                return true;
            }
            return false;
        case QContactModel::HomeCountry:
            if (v.canConvert(QVariant::String)) {
                contact.setHomeCountry(v.toString());
                return true;
            }
            return false;
        case QContactModel::HomeWebPage:
            if (v.canConvert(QVariant::String)) {
                contact.setHomeWebpage(v.toString());
                return true;
            }
            return false;
        case QContactModel::Spouse:
            if (v.canConvert(QVariant::String)) {
                contact.setSpouse(v.toString());
                return true;
            }
            return false;
        case QContactModel::Gender:
            if (v.canConvert(QVariant::Int)) {
                contact.setGender((QContact::GenderType)v.toInt());
                return true;
            }
            return false;
        case QContactModel::Birthday:
            if (v.canConvert(QVariant::Date)) {
                contact.setBirthday(v.toDate());
                return true;
            }
            return false;
        case QContactModel::Anniversary:
            if (v.canConvert(QVariant::Date)) {
                contact.setAnniversary(v.toDate());
                return true;
            }
            return false;
        case QContactModel::Nickname:
            if (v.canConvert(QVariant::String)) {
                contact.setNickname(v.toString());
                return true;
            }
            return false;
        case QContactModel::Children:
            if (v.canConvert(QVariant::String)) {
                contact.setChildren(v.toString());
                return true;
            }
            return false;
        case QContactModel::Portrait:
            if (v.canConvert(QVariant::String)) {
                contact.setPortraitFile(v.toString());
                return true;
            }
            return false;
        case QContactModel::Notes:
            if (v.canConvert(QVariant::String)) {
                contact.setNotes(v.toString());
                return true;
            }
            return false;
        case QContactModel::LastNamePronunciation:
            if (v.canConvert(QVariant::String)) {
                contact.setLastNamePronunciation(v.toString());
                return true;
            }
            return false;
        case QContactModel::FirstNamePronunciation:
            if (v.canConvert(QVariant::String)) {
                contact.setFirstNamePronunciation(v.toString());
                return true;
            }
            return false;
        case QContactModel::CompanyPronunciation:
            if (v.canConvert(QVariant::String)) {
                contact.setCompanyPronunciation(v.toString());
                return true;
            }
            return false;
    }
    return false;
}
/*!
  Updates the contact \a contact so long as a there is a contact in the
  QContactModel with the same uid as \a contact.

  Returns true if the contact was successfully updated.  Otherwise return false.
*/
bool QContactModel::updateContact(const QContact& contact)
{
    foreach(QContactContext *context, d->contexts) {
        if (context->exists(contact.uid())) {
            bool result = context->updateContact(contact);
            if (result && contact.uid() == personalID())
                updateBusinessCard(contact);
            return result;
        }
    }
    return false;
}

/*!
  Removes the contact \a contact so long as there is a contact in the QContactModel with
  the same uid as \a contact.

  Returns true if the contact was successfully removed.  Otherwise return false.
*/
bool QContactModel::removeContact(const QContact& contact)
{
    return removeContact(contact.uid());
}

/*!
  Removes the contact that has the uid \a id from the ContactModel;

  Returns true if the contact was successfully removed.  Otherwise return false.
*/
bool QContactModel::removeContact(const QUniqueId& id)
{
    foreach(QContactContext *context, d->contexts) {
        if (context->exists(id)) {
            bool result = context->removeContact(id);
            if (result && id == personalID())
                clearPersonalDetails();
            return result;
        }
    }
    return false;
}

/*!
  Adds the contact \a contact to the QContactModel under the storage source \a source.
  If source is empty will add the contact to the default storage source.

  Returns true if the contact was successfully added.  Otherwise return false.
*/
QUniqueId QContactModel::addContact(const QContact& contact, const QPimSource &source)
{
    if (source.isNull()) {
        return d->defaultContext->addContact(contact, source);
    } else {
        foreach(QContactContext *context, d->contexts) {
            if (context->sources().contains(source))
                return context->addContact(contact, source);
        }
    }
    return QUniqueId();
}

/*!
  Removes the contacts specified by the list of uids \a ids.

  Returns true if contacts are successfully removed.  Otherwise returns  false.
*/
bool QContactModel::removeList(const QList<QUniqueId> &ids)
{
    QUniqueId id;
    foreach(id, ids) {
        if (!exists(id))
            return false;
    }
    QUniqueId pid = personalID();
    foreach(id, ids) {
        removeContact(id);
        if (id == pid)
            removeBusinessCard();
    }
    return true;
}

/*!
  Returns the uid for the contact representing the personal details of the device owner.

  If no contact is specified as the personal details of the device owner will return a
  null id.
*/
QUniqueId QContactModel::personalID() const
{
    QSettings c("Trolltech","Pim");
    c.beginGroup("Contacts");
    if (!c.value("personalid").isValid())
        return QUniqueId();
    return QUniqueId(c.value("personalid").toString());
}

/*!
  Returns the contact representing the personal details of the device owner.

  If no contact is spefied as the personal details of the device owner will return a null
  contact.
*/
QContact QContactModel::personalDetails() const
{
    return contact(personalID());
}

/*!
  Returns true if a contact in the QContactModel is specified as the personal details of the
  device owner.  Otherwise returns false.
*/
bool QContactModel::hasPersonalDetails() const
{
    QUniqueId id = personalID();
    return exists(id);
}

/*!
  Returns true if a contact with the uid \a id is stored in the contact model.  Otherwise
  return false.

  The contact with uid id does not need to be in the current filter mode.

  \sa contains()
*/
bool QContactModel::exists(const QUniqueId &id) const
{
    foreach(const QContactIO *model, d->models) {
        if (model->exists(id))
            return true;
    }
    return false;
}

/*!
  Returns true if the contact for the row specified by \a i represents the personal details
  of the device owner.  Otherwise return false.
*/
bool QContactModel::isPersonalDetails(const QModelIndex &i) const
{
    if (hasPersonalDetails() && personalID() == id(i))
        return true;
    return false;
}

/*!
  Sets no contact to represent the personal details of the device owner.  Does not remove
  any contacts from the QContactModel.
*/
void QContactModel::clearPersonalDetails()
{
    setPersonalDetails(QUniqueId());
}

/*!
  Sets the contact representing the personal details of the device owner to the contact
  identified by the uid \a i.  If there is no contact with the uid \a i the QContactModel
  or \a i is null will clear the personal details.

  \sa clearPersonalDetails()
*/
void QContactModel::setPersonalDetails(const QUniqueId & i)
{
    {
        QSettings c("Trolltech","Pim");
        c.beginGroup("Contacts");
        c.setValue("personalid", i.toString());
    }

    updateBusinessCard(contact(i));
}

/*!
  Returns true if the contact with uid \a i represents the personal details
  of the device owner.  Otherwise return false.
*/
bool QContactModel::isPersonalDetails(const QUniqueId & i) const
{
    if (hasPersonalDetails() && personalID() == i)
        return true;
    return false;
}

static QString businessCardName() {
    return Qtopia::applicationFileName("addressbook",
            "businesscard.vcf");
}

/*!
  \internal
  */
void QContactModel::updateBusinessCard(const QContact &cnt)
{
    QSettings cfg("Trolltech","Security");
    cfg.beginGroup("Sync");
    cfg.setValue("ownername", cnt.label());

    if (cnt.uid().isNull())
        QFile::remove( businessCardName() );
    else
        cnt.writeVCard( businessCardName());
}

/*!
  \internal
  */
void QContactModel::removeBusinessCard()
{
    QSettings cfg("Trolltech","Security");
    cfg.beginGroup("Sync");
    cfg.setValue("ownername", "");

    QFile::remove( businessCardName() );
}

#ifdef QTOPIA_PHONE
/*!
  Returns the best match for the phone number \a text.  If no contact
  in the model has a phone number matching \a text returns a
  null contact.
*/
QContact QContactModel::matchPhoneNumber(const QString &text)
{
    int bestMatch = 0;
    QUniqueId bestId;
    foreach(const QContactIO *model, d->models) {
        int match;
        QUniqueId id = model->matchPhoneNumber(text, match);
        if (match > bestMatch) {
            bestMatch = match;
            bestId = id;
        }
    }
    if (bestMatch > 0)
        return contact(bestId);
    return QContact();
}
#endif

/*!
  \enum QContactModel::FilterFlags

  These flags describe what kind of contact information to filter contacts on.

  \value ContainsPhoneNumber
      The contact must provide one or more phone number.
  \value ContainsEmail
      The contact must provide one or more email addresses.
  \value ContainsMailing
      The contact must provide one or more mailing addresses.

  \sa setFilter()
*/

/*!
  Sets the model to filter contacts by labels that start with \a text
  and contact type information specified \a flags.  By default flags
  is 0 which means filtering will only occur based of label text.

  \sa filterFlags(), filterText()
*/
void QContactModel::setFilter(const QString &text, int flags)
{
    if (text == filterText() && flags == filterFlags())
        return;

    d->filterText = text;
    d->filterFlags = flags;
    foreach(QContactIO *model, d->models)
        model->setFilter(text, flags);
    d->mio->rebuildCache();
}

/*!
  Returns the filter text being used by the model to filter
  contact labels.

  \sa setFilter()
*/
QString QContactModel::filterText() const
{
    return d->filterText;
}

/*!
  Returns the filter flags being used by the model to filter
  contact type information

  \sa setFilter()
*/
int QContactModel::filterFlags() const
{
    return d->filterFlags;
}

/*!
  Clear Contact name and type filterng for the model.
  Does not affect category filtering.
*/
void QContactModel::clearFilter()
{
    if (d->filterText.isEmpty() && d->filterFlags == 0)
        return;

    foreach(QContactIO *model, d->models)
        model->clearFilter();

    d->filterText.clear();
    d->filterFlags = 0;
}

#ifdef QTOPIA_PHONE
/*!
  Returns true if the contact at \a index is stored on the SIM card.
  Otherwise returns false.
*/
bool QContactModel::isSIMCardContact(const QModelIndex &index) const
{
    return isSIMCardContact(id(index));
}

/*!
  Returns true fi the contact identified by \a id is on the SIM card.
  Otherwise returns false.
*/
bool QContactModel::isSIMCardContact(const QUniqueId & id) const
{
#ifdef QTOPIA_CELL
    // sim source changes on loading.
    bool b = sourceExists(simSource(), id);
    return b;
#else
    Q_UNUSED(id);
    return false;
#endif
}

#endif

/*!
  \internal
  */
void QContactModel::setSortField(Field s)
{
    if (s == sortField())
        return;

    foreach(QContactIO *model, d->models)
        model->setSortKey(s);
    d->mio->rebuildCache();
}

/*!
  \internal
  */
QContactModel::Field QContactModel::sortField() const
{
    // assumed others are the same.
    return d->defaultmodel->sortKey();
}

/*!
  Set the model to only contain contacts accepted by the QCategoryFilter \a f.
*/
void QContactModel::setCategoryFilter(const QCategoryFilter &f)
{
    if (f == categoryFilter())
        return;

    foreach(QContactIO *model, d->models)
        model->setCategoryFilter(f);
    d->mio->rebuildCache();
}

/*!
  Returns the QCategoryFilter that contacts are tested against for the current filter mode.
*/
QCategoryFilter QContactModel::categoryFilter() const
{
    // assumed others are the same.
    return d->defaultmodel->categoryFilter();
}

/*!
  \overload
  Returns the index of the item in the model specified by the given \a row, \a column
  and \a parent index.
*/
QModelIndex QContactModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    if (row < 0 || row >= count() || column < 0 || column >= columnCount())
        return QModelIndex();
    return createIndex(row,column);
}

/*!
  If the model contains a contact with uid \a id returns the index of the contact.
  Otherwise returns a null QModelIndex

  \sa contains(), exists()
*/
QModelIndex QContactModel::index(const QUniqueId & id) const
{
    int i = d->mio->index(id);
    if (i == -1)
        return QModelIndex();
    return createIndex(i, 0);
}

/*!
  Return the id for the contact at the row specified by \a index.
  If index is null or out of the range of the model will return a null id.
*/
QUniqueId QContactModel::id(const QModelIndex &index) const
{
    int i = index.row();
    return id(i);
}

/*!
  Returns true if the current filter mode of the model contains \a index.
  Otherwise returns false.
*/
bool QContactModel::contains(const QModelIndex &index) const
{
    return (index.row() >= 0 && index.row() < count());
}

/*!
  Returns true if the current filter mode of the model contains the contact with the uid \a id.
  Otherwise returns false.
*/
bool QContactModel::contains(const QUniqueId & id) const
{
    return index(id).isValid();
}

/*!
  Returns the list of sources of contact data that are currently shown by the
  contact model.
*/
QSet<QPimSource> QContactModel::visibleSources() const
{
    QSet<QPimSource> set;
    foreach(QContactContext *c, d->contexts)
        set.unite(c->visibleSources());
    return set;
}

/*!
   Returns the contexts of contact data that can be shown by the contact model.
*/
const QList<QContactContext*> &QContactModel::contexts() const
{
    return d->contexts;
}

/*!
  Returns the context that contains the contact with identifer \a id.
  If the contact does not exists returns 0.
*/
QContactContext *QContactModel::context(const QUniqueId &id) const
{
    foreach(QContactContext *context, d->contexts) {
        if (context->exists(id))
            return context;
    }
    return 0;
}

/*!
  Returns the source identifier that contains the contact with identier \a id.
  If the contact does not exist returns a null source.
*/
QPimSource QContactModel::source(const QUniqueId &id) const
{
    foreach(QContactContext *context, d->contexts) {
        if (context->exists(id))
            return context->source(id);
    }
    return QPimSource();
}

/*!
  Sets the QContactModel to show only contacts contained in the storage sources specified
  by \a list.

  Also refreshes the model.
*/
void QContactModel::setVisibleSources(const QSet<QPimSource> &list)
{
    foreach (QContactContext *c, d->contexts) {
        QSet<QPimSource> cset = c->sources();
        cset.intersect(list);
        c->setVisibleSources(cset);
    }
    refresh();
}

/*!
  Returns true if the contact uid \a id is stored in the storage source \a source.
  Otherwise returns false.
*/
bool QContactModel::sourceExists(const QPimSource &source, const QUniqueId &id) const
{
    foreach (QContactContext *c, d->contexts) {
        if (c->sources().contains(source))
            return c->exists(id, source);
    }
    return false;
}

/*!
  Returns the set of identifiers for storage sources that can be shown.
*/
QSet<QPimSource> QContactModel::availableSources() const
{
    QSet<QPimSource> set;
    foreach(QContactContext *c, d->contexts)
        set.unite(c->sources());
    return set;
}

/*!
  Returns the identifier for storage sources relating to the SIM Card.
*/
QPimSource QContactModel::simSource() const
{
#ifdef QTOPIA_MODEM
    foreach(QContactContext *c, d->contexts) {
        QContactSimContext *sim = qobject_cast<QContactSimContext *>(c);
        if (sim)
            return sim->defaultSource();
    }
#endif
    return QPimSource();
}

/*!
  Returns the identifier for storage sources relating to the Phone memory.
*/
QPimSource QContactModel::phoneSource() const
{
    foreach(QContactContext *c, d->contexts) {
        QContactDefaultContext *con = qobject_cast<QContactDefaultContext *>(c);
        if (con)
            return con->defaultSource();
    }
    return QPimSource();
}

/*!
  Exports the contact with identifer \a id and imports it into \a dest.
  The contact is modified by the source and destination storage contexts
  to account for storage restrictions.  For instance a contact may be split into
  multiple contacts when mirrored to the active SIM card and contacts from the
  SIM card will be merged into contacts in local storage if they have
  equal name information.

  \sa mirrorAll()
*/
bool QContactModel::mirrorToSource(const QPimSource &dest, const QUniqueId &id)
{
    QContact c;
    QContactContext *sourceContext = context(id);
    QPimSource source = QContactModel::source(id);
    QContactContext *destContext = 0;

    if (source == dest)
        return false;
    foreach(QContactContext *c, d->contexts) {
        if (c->sources().contains(dest)) {
            destContext = c;
            break;
        }
    }

    if (sourceContext && destContext) {
        bool result;
        QContact c = sourceContext->exportContact(id, result);
        if (result)
            return destContext->importContact(dest, c);
    }

    return false;
}

/*!
  Exports the contacts stored in \a source and imports tem into \a dest.
  The contacts are modified by the source and destination storage contexts
  to account for storage restrictions.

  \sa mirrorToSource()
*/
bool QContactModel::mirrorAll(const QPimSource &source, const QPimSource &dest)
{
    if (source == dest || source.isNull() || dest.isNull())
        return false;
    QContactContext *sourceContext = 0;
    QContactContext *destContext = 0;

    foreach(QContactContext *c, d->contexts) {
        if (c->sources().contains(source))
            sourceContext = c;
        if (c->sources().contains(dest))
            destContext = c;
    }
    if (sourceContext && destContext) {
        bool result;
        QList<QContact> c = sourceContext->exportContacts(source, result);
        if (result)
            return destContext->importContacts(dest, c);
    }
    return false;
}

/*!
   Write vcards for all visible contacts in the model to the file
   specified by \a filename.

   Returns true when the write was successful, or false on failure.
*/
bool QContactModel::writeVCard( const QString &filename )
{
    if( count() ) {

        QFile f( filename );
        if( ! f.open( QIODevice::WriteOnly ) ) {
            qWarning() << "Unable to open vcard file for write!";
            return false;
        }

        for( int i = 0; i < count(); i++ )
            contact(i).writeVCard( f );

        return true;
    }

    return false;
}

/* recommend using filter instead, better performance */
/*!
  \overload
    Returns a list of indexes for the items where the data
    matches the specified \a value.  The list that is returned may be empty.

    The search starts from the \a start index.

    The arguments \a role, \a hits and \a flags are currently ignored.
*/
QModelIndexList QContactModel::match(const QModelIndex &start, int role, const QVariant &value,
            int hits, Qt::MatchFlags flags) const
{
    Q_UNUSED(hits);
    Q_UNUSED(role);
    Q_UNUSED(flags);
    /* role, flags and count ignored. */
    if (d->searchModel == 0) {
        d->searchModel = new QContactModel(0);
    }
    d->searchModel->setCategoryFilter(categoryFilter());
    d->searchModel->setFilter(value.toString());

    QModelIndexList l;
    for (int i = 0; i < d->searchModel->count(); i++) {
        QModelIndex idx = d->searchModel->index(i, 0, QModelIndex());
        if (idx.isValid()) {
            QModelIndex foundidx = index(d->searchModel->id(idx));
            if (foundidx.row() >= start.row()) {
                l.append(foundidx);
                break;
            }
        }
    }
    return l;
}
