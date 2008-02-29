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
#ifndef QCONTENTFILTERMODEL_H
#define QCONTENTFILTERMODEL_H

#include <QAbstractItemModel>
#include <qcontentset.h>
#include <QListView>
#include <QDialog>

class QContentFilterModelPrivate;
class QContentFilterModelTemplatePrivate;

class QTOPIA_EXPORT QContentFilterModel : public QAbstractItemModel
{
    Q_OBJECT
public:

    enum TemplateOption
    {
        NoTemplateOptions = 0x00,
        SelectAll         = 0x01,
        ForceAll          = 0x02,
        CheckList         = 0x04,
        AndCheckedFilters = 0x08,
        ShowEmptyLabels   = 0x10
    };

    Q_DECLARE_FLAGS( TemplateOptions, TemplateOption );

    class QTOPIA_EXPORT Template
    {
    public:
        Template();
        explicit Template( QContentFilter::FilterType type,
                           const QString &subType = QString(),
                           TemplateOptions options = NoTemplateOptions,
                           const QStringList &checked = QStringList() );
        explicit Template( QContent::Property property,
                           TemplateOptions options = NoTemplateOptions,
                           const QStringList &checked = QStringList() );
        Template( const Template &target, QContentFilter::FilterType type, const QString &subType = QString(), TemplateOptions options = NoTemplateOptions, const QStringList &checked = QStringList() );
        Template( const Template &target, QContent::Property property, TemplateOptions options = NoTemplateOptions, const QStringList &defaultSelection = QStringList() );
        Template( const Template &other );
        ~Template();

        Template &operator =( const Template &other );

        bool isValid() const;

        TemplateOptions options() const;

        void setOptions( TemplateOptions options );

        void addLabel( const QString &title, bool checked = false );
        void addLabel( const Template &target, const QString &title, bool checked = false );
        void addLabel( const QString &title, const QContentFilter &filter, bool checked = false );
        void addLabel( const Template &target, const QString &title, const QContentFilter &filter, bool checked = false );
        void addList( QContentFilter::FilterType type, const QString &subType = QString(), const QStringList &checked = QStringList() );
        void addList( const Template &target, QContentFilter::FilterType type, const QString &subType = QString(), const QStringList &checked = QStringList() );
        void addList( const QContentFilter &filter, QContentFilter::FilterType type, const QString &subType = QString(), const QStringList &checked = QStringList() );
        void addList( const Template &target, const QContentFilter &filter, QContentFilter::FilterType type, const QString &subType = QString(), const QStringList &checked = QStringList() );
        void addList( QContent::Property property, const QStringList &checked = QStringList() );
        void addList( const Template &target, QContent::Property property, const QStringList &checked = QStringList() );
        void addList( const QContentFilter &filter, QContent::Property property, const QStringList &checked = QStringList() );
        void addList( const Template &target, const QContentFilter &filter, QContent::Property property, const QStringList &checked = QStringList() );

        QContentFilter filter() const;

        void setFilter( const QContentFilter &filter );

    private:
        QSharedDataPointer< QContentFilterModelTemplatePrivate > d;

        friend class QContentFilterModelPrivate;
    };

    explicit QContentFilterModel( QObject* parent = 0 );

    explicit QContentFilterModel( QContent::Property property,
                                  TemplateOptions options = SelectAll,
                                  const QStringList &checked = QStringList(),
                                  QObject *parent = 0 );

    explicit QContentFilterModel( QContentFilter::FilterType type,
                                  const QString &subType = QString(),
                                  TemplateOptions = SelectAll,
                                  const QStringList &checked = QStringList(),
                                  QObject *parent = 0 );

    virtual ~QContentFilterModel();

    Template modelTemplate() const;

    void setModelTemplate( const Template &modelTemplate );

    QContentFilter baseFilter() const;
    void setBaseFilter( const QContentFilter &filter );

    QContentFilter filter( const QModelIndex &index ) const;

    QContentFilter checkedFilter( const QModelIndex &parent = QModelIndex() ) const;

    virtual int columnCount ( const QModelIndex &parent = QModelIndex() ) const;

    virtual QVariant data ( const QModelIndex &index, int role = Qt::DisplayRole ) const;

    virtual bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole );

    virtual Qt::ItemFlags flags( const QModelIndex &index ) const;

    virtual QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const;

    virtual QModelIndex parent( const QModelIndex &index ) const;

    virtual int rowCount( const QModelIndex &parent = QModelIndex() ) const;

    virtual bool hasChildren( const QModelIndex &parent = QModelIndex() ) const;

private slots:
    void contentChanged( const QContentIdList &ids, QContent::ChangeType type );

private:
    QContentFilterModelPrivate *child( const QModelIndex &parent ) const;

    void beginRemoveRows( QContentFilterModelPrivate *parent, int start, int end );
    void beginInsertRows( QContentFilterModelPrivate *parent, int start, int end );

    QContentFilterModelPrivate *d;

    friend class QContentFilterModelPrivate;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QContentFilterModel::TemplateOptions );

#endif
