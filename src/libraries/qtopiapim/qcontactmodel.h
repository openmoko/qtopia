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

#ifndef QTOPIA_CONTACTMODEL_H
#define QTOPIA_CONTACTMODEL_H

#include <QStyleOptionViewItem>
#include <QAbstractListModel>
#include <QAbstractItemDelegate>
#include <QSet>
#include <QSharedDataPointer>
#include <qtopia/pim/qcontact.h>
#include <qtopia/pim/qpimsource.h>

#include <qcategorymanager.h>

class QContactModelData;
class QTOPIAPIM_EXPORT QContactModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit QContactModel(QObject *parent = 0);
    virtual ~QContactModel();

    enum Field {
        Invalid = -1,

        Label, // read only

        NameTitle,
        FirstName,
        MiddleName,
        LastName,
        Suffix,

        JobTitle,
        Department,
        Company,
        BusinessPhone,
        BusinessFax,
        BusinessMobile,

        // email
        DefaultEmail,
        Emails,

        OtherPhone,
        OtherFax,
        OtherMobile,
        OtherPager,

        HomePhone,
        HomeFax,
        HomeMobile,
        HomePager,

        // business
        BusinessStreet,
        BusinessCity,
        BusinessState,
        BusinessZip,
        BusinessCountry,
        BusinessPager,
        BusinessWebPage,

        Office,
        Profession,
        Assistant,
        Manager,

        // home
        HomeStreet,
        HomeCity,
        HomeState,
        HomeZip,
        HomeCountry,
        HomeWebPage,

        //personal
        Spouse,
        Gender,
        Birthday,
        Anniversary,
        Nickname,
        Children,

        // other
        Portrait,
        Notes,

        LastNamePronunciation,
        FirstNamePronunciation,
        CompanyPronunciation,

        Identifier,
        Categories
    };

    enum QContactModelRole {
        LabelRole = Qt::UserRole,
        SubLabelRole = Qt::UserRole+1,
        PortraitRole = Qt::UserRole+2,
        StatusIconRole = Qt::UserRole+3
    };

    const QList<QContactContext*> &contexts() const;

    QSet<QPimSource> visibleSources() const;
    void setVisibleSources(const QSet<QPimSource> &);
    QSet<QPimSource> availableSources() const;

    QPimSource phoneSource() const;
    QPimSource simSource() const;

    bool mirrorToSource(const QPimSource &source, const QUniqueId &);
    bool mirrorAll(const QPimSource &source, const QPimSource &dest);

    QPimSource source(const QUniqueId &) const;
    QContactContext *context(const QUniqueId &) const;

    bool sourceExists(const QPimSource &source, const QUniqueId &id) const;

    static QList<Field> phoneFields();
    static QStringList localeNameTitles();
    static QStringList localeSuffixes();

    static QString fieldLabel(Field);
    static QIcon fieldIcon(Field k);

    static QString fieldIdentifier(Field);
    static Field identifierField(const QString &);

    int rowCount(const QModelIndex & = QModelIndex()) const;
    int columnCount(const QModelIndex & = QModelIndex()) const;

    // overridden so can change later and provide drag-n-drop (via vcard)
    // later without breaking API compatibility.
    QMimeData * mimeData(const QModelIndexList &) const;
    QStringList mimeTypes() const;

    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);

    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &, const QVariant &, int) const;
    bool setItemData(const QModelIndex &, const QMap<int,QVariant> &) const;
    QMap<int,QVariant> itemData(const QModelIndex &) const;

    QVariant headerData(int section, Qt::Orientation orientation,
            int role = Qt::DisplayRole) const;


    int count() const;
    bool contains(const QModelIndex &) const;
    bool contains(const QUniqueId &) const;
    bool exists(const QUniqueId &) const;

    QModelIndex index(const QUniqueId &) const;
    QUniqueId id(const QModelIndex &) const;
    QUniqueId id(int) const;
    QModelIndex index(int r,int c = 0,const QModelIndex & = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &) const;
    bool hasChildren(const QModelIndex & = QModelIndex()) const;

    Qt::ItemFlags flags(const QModelIndex &index) const;

    QContact contact(const QModelIndex &index) const;
    QContact contact(int) const;
    QContact contact(const QUniqueId &) const;

    static QVariant contactField(const QContact &c, QContactModel::Field k);
    static bool setContactField(QContact &c, QContactModel::Field k,  const QVariant &);

    bool updateContact(const QContact& contact);
    bool removeContact(const QContact& contact);
    QUniqueId addContact(const QContact& contact, const QPimSource &source = QPimSource());

    bool removeList(const QList<QUniqueId> &);

    void setCategoryFilter(const QCategoryFilter &);
    QCategoryFilter categoryFilter() const;

    QUniqueId personalID() const;
    QContact personalDetails() const;
    bool hasPersonalDetails() const;

    void setPersonalDetails(const QUniqueId &);
    void clearPersonalDetails();
    bool isPersonalDetails(const QModelIndex &) const;
    bool isPersonalDetails(int r) const { return isPersonalDetails(index(r, 0, QModelIndex())); }
    bool isPersonalDetails(const QUniqueId &) const;

    bool flush();
    bool refresh();

    // starting at 0x0100 so can later maybe |' with
    // Qt::MatchFlags.
    enum FilterFlags {
        ContainsPhoneNumber = 0x00100,
        ContainsEmail = 0x0200,
        ContainsMailing = 0x0400,
    };

    void setFilter(const QString &, int flags = 0);
    QString filterText() const;
    int filterFlags() const;
    void clearFilter();

    /* used in phone a lot */
#ifdef QTOPIA_PHONE
    QContact matchPhoneNumber(const QString &);
#endif

    /* need a match function since otherwise QAbstractItemModel will do it poorly */
    QModelIndexList match(const QModelIndex &start, int role, const QVariant &,
            int hits = 1, Qt::MatchFlags flags = Qt::MatchFlags(Qt::MatchStartsWith | Qt::MatchWrap)) const;


    bool writeVCard( const QString &filename );

#ifdef QTOPIA_PHONE
    bool isSIMCardContact(const QModelIndex &) const;
    bool isSIMCardContact(const QUniqueId &) const;
#endif

    // missing function to aid in rendering searched for contact.
    // QContact renderedSearchItem(QContact &)?

private slots:
    void voidCache();

private:
    void setSortField(Field);
    Field sortField() const;

    static void initMaps();
    static QMap<Field, QString> k2t;
    static QMap<Field, QString> k2i;
    static QMap<QString, Field> i2k;

    void updateBusinessCard(const QContact &cnt);
    void removeBusinessCard();
    bool removeContact(const QUniqueId& contact);

    QContactModelData *d;
};

#endif // QTOPIA_CONTACTMODEL_H
