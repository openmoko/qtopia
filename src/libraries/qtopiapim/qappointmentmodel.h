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

#ifndef APPOINTMENTLISTMODEL_H
#define APPOINTMENTLISTMODEL_H

#include <QStyleOptionViewItem>
#include <QAbstractListModel>
#include <QAbstractItemDelegate>
#include <QSet>
#include <QSharedDataPointer>
#include <qappointment.h>

#include <qcategorymanager.h>

#include <qpimsource.h>

class QAppointmentModelData;
class QAppointmentIO;
class QTOPIAPIM_EXPORT QAppointmentModel : public QAbstractItemModel
{
    friend class QOccurrenceModel;
    Q_OBJECT

public:
    explicit QAppointmentModel(QObject *parent = 0);
    virtual ~QAppointmentModel();

    enum Field {
        Invalid = -1,

        Description,
        Location,
        Start,
        End,
        AllDay,
        TimeZone,

        Notes,
        Alarm,

        RepeatRule,
        RepeatFrequency,
        RepeatEndDate,
        RepeatWeekFlags,

        Identifier,
        Categories
    };

    const QList<QAppointmentContext*> &contexts() const;

    /* consider making these just generic added roles */
    enum QAppointmentModelRole {
        LabelRole = Qt::UserRole
    };

    QSet<QPimSource> visibleSources() const;
    void setVisibleSources(const QSet<QPimSource> &);
    QSet<QPimSource> availableSources() const;

    QPimSource source(const QUniqueId &) const;
    QAppointmentContext *context(const QUniqueId &) const;

    bool sourceExists(const QPimSource &source, const QUniqueId &id) const;

    static QString fieldLabel(Field);
    static QIcon fieldIcon(Field k);

    static QString fieldIdentifier(Field);
    static Field identifierField(const QString &);

    int rowCount(const QModelIndex & = QModelIndex()) const;

    int count() const { return rowCount(); }

    int columnCount(const QModelIndex & = QModelIndex()) const;

    // overridden so can change later and provide drag-n-drop (via vcard)
    // later without breaking API compatibility.
    QMimeData * mimeData(const QModelIndexList &) const;
    QStringList mimeTypes() const;

    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &, const QVariant &, int) const;
    bool setItemData(const QModelIndex &, const QMap<int,QVariant> &) const;
    QMap<int,QVariant> itemData(const QModelIndex &) const;

    QVariant headerData(int section, Qt::Orientation orientation,
            int role = Qt::DisplayRole) const;


    bool exists(const QUniqueId &) const;

    virtual QModelIndex index(const QUniqueId &) const;
    QModelIndex index(int,int = 0,const QModelIndex & = QModelIndex()) const;
    virtual QUniqueId id(const QModelIndex &) const;
    QModelIndex parent(const QModelIndex &) const;
    bool hasChildren(const QModelIndex & = QModelIndex()) const;
    QAppointment appointment(const QModelIndex &index) const;
    QAppointment appointment(const QUniqueId &) const;
    QAppointment appointment(int index) const;

    bool editable(const QModelIndex &index) const;
    bool editable(const QUniqueId &) const;

    static QVariant appointmentField(const QAppointment &c, QAppointmentModel::Field k);
    static bool setAppointmentField(QAppointment &c, QAppointmentModel::Field k,  const QVariant &);

    bool updateAppointment(const QAppointment& appointment);
    bool removeAppointment(const QAppointment& appointment);
    QUniqueId addAppointment(const QAppointment& appointment, const QPimSource & = QPimSource());

    bool removeOccurrence(const QOccurrence& occurrence);
    bool removeOccurrence(const QAppointment& appointment, const QDate &date);
    bool removeOccurrence(const QUniqueId &id, const QDate &date);
    QUniqueId replaceOccurrence(const QAppointment& appointment, const QOccurrence& replacement);
    QUniqueId replaceRemaining(const QAppointment& appointment, const QAppointment& replacement);

    // should also be QItemSelection, although isn't that iteratable.
    bool removeList(const QList<QUniqueId> &);

    void setCategoryFilter(const QCategoryFilter &);
    QCategoryFilter categoryFilter() const;

    bool flush();
    bool refresh();

    void setRange(const QDateTime &, const QDateTime &);
    QDateTime rangeStart() const;
    QDateTime rangeEnd() const;

    enum DurationType
    {
        TimedDuration = 0x01,
        AllDayDuration = 0x2,
        AnyDuration = TimedDuration | AllDayDuration,
    };

    void setDurationType(DurationType f);
    DurationType durationType() const;

private slots:
    void voidCache();

private:

    bool removeAppointment(const QUniqueId& appointment);

    static void initMaps();
    static QMap<Field, QString> k2t;
    static QMap<Field, QString> k2i;
    static QMap<QString, Field> i2k;

    QAppointmentModelData *d;
};


class QOccurrenceModelData;
class QTOPIAPIM_EXPORT QOccurrenceModel : public QAbstractItemModel
{
    Q_OBJECT
public:

    QOccurrenceModel(const QDateTime &start, const QDateTime &end, QObject *parent = 0);
    QOccurrenceModel(const QDateTime &start, int count, QObject *parent = 0);

    virtual ~QOccurrenceModel();

    int rowCount(const QModelIndex & = QModelIndex()) const;
    int columnCount(const QModelIndex & = QModelIndex()) const;

    QMimeData * mimeData(const QModelIndexList &) const;
    QStringList mimeTypes() const;

    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &, const QVariant &, int) const;
    bool setItemData(const QModelIndex &, const QMap<int,QVariant> &) const;
    QMap<int,QVariant> itemData(const QModelIndex &) const;

    int count() const { return rowCount(); }
    bool contains(const QModelIndex &) const;
    bool contains(const QUniqueId &) const;

    QModelIndex parent(const QModelIndex &) const;
    QModelIndex index(int, int = 0, const QModelIndex & = QModelIndex()) const;
    QModelIndex index(const QUniqueId &) const;
    QModelIndex index(const QOccurrence &) const;
    QUniqueId id(const QModelIndex &) const;
    QOccurrence occurrence(const QModelIndex &index) const;
    QOccurrence occurrence(const QUniqueId &, const QDate &) const;
    QOccurrence occurrence(int) const;
    QAppointment appointment(const QUniqueId &id) const;
    QAppointment appointment(const QModelIndex &index) const { return occurrence(index).appointment(); }
    QAppointment appointment(int index) const { return occurrence(index).appointment(); }


    void setRange(const QDateTime &, int);
    void setRange(const QDateTime &, const QDateTime &);

    void setDurationType(QAppointmentModel::DurationType f);
    QAppointmentModel::DurationType durationType() const;

    QDateTime rangeStart() const;
    QDateTime rangeEnd() const;

    // TODO extra passthrough functions
    void setCategoryFilter(const QCategoryFilter &);
    QCategoryFilter categoryFilter() const;
    QSet<QPimSource> visibleSources() const;
    void setVisibleSources(const QSet<QPimSource> &);
    QSet<QPimSource> availableSources() const;
    bool sourceExists(const QPimSource &source, const QUniqueId &id) const;
    bool editable(const QModelIndex &index) const;
    bool editable(const QUniqueId &) const;

    void refresh();

    // will want similar fetch calls for contacts:sim
    bool fetching() const;
    void completeFetch(); // can be expensive
signals:
    void fetchCompleted();

public slots:
    void rebuildCache();
private slots:
    void voidCache();

private:
    void init(QAppointmentModel *appointmentModel);

    QOccurrenceModelData *od;
};

class QFont;
class QTOPIAPIM_EXPORT QAppointmentDelegate : public QAbstractItemDelegate
{
public:
    QAppointmentDelegate( QObject * parent = 0 );
    virtual ~QAppointmentDelegate();

    virtual void paint(QPainter *painter, const QStyleOptionViewItem & option,
            const QModelIndex & index ) const;

    virtual QSize sizeHint(const QStyleOptionViewItem & option,
            const QModelIndex &index) const;

    virtual QFont mainFont(const QStyleOptionViewItem &) const;
    virtual QFont secondaryFont(const QStyleOptionViewItem &) const;
private:
    QFont differentFont(const QFont& start, int step) const;

    int iconSize;
};

#endif // APPOINTMENTLISTMODEL_H
