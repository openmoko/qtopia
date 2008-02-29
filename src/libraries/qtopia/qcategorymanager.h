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

#ifndef QTPALMTOP_CATEGORIES_H
#define QTPALMTOP_CATEGORIES_H

#include <qtopiaglobal.h>
#include <QStringList>
#include <QString>
#include <QIcon>
#include <QList>
#include <QSharedData>

class QSettings;
class QCategoryFilterData;
class QTOPIA_EXPORT QCategoryFilter
{
public:
    enum FilterType {
        List,
        Unfiled,
        All
    };
    QCategoryFilter();
    QCategoryFilter(FilterType t);
    explicit QCategoryFilter(const QList<QString>&);
    explicit QCategoryFilter(const QString &);
    QCategoryFilter(const QCategoryFilter &other);
    ~QCategoryFilter();

    QCategoryFilter &operator=(const QCategoryFilter &other);

    bool operator==(const QCategoryFilter &o) const;
    bool operator!=(const QCategoryFilter &o) const;

    bool accepted(const QList<QString> &) const;
    bool acceptAll() const;
    bool acceptUnfiledOnly() const;

    // might be a bad choice.
    QList<QString> requiredCategories() const;

    QString label(const QString &scope = QString()) const;

    void writeConfig( QSettings &, const QString &key) const;
    void readConfig( const QSettings &, const QString &key);

private:
    QSharedDataPointer<QCategoryFilterData> d;
};

class QCategoryManagerData;
class QTOPIA_EXPORT QCategoryManager : public QObject
{
    Q_OBJECT
public:
    explicit QCategoryManager(QObject *parent = 0);
    explicit QCategoryManager(const QString &scope, QObject *parent = 0);
    ~QCategoryManager();


    QString label(const QString &) const;
    QList<QString> labels(const QList<QString> &) const;
    QIcon icon(const QString &) const;
    QString iconFile(const QString &) const;

    static QString unfiledLabel();
    static QString allLabel();
    static QString multiLabel();

    bool isSystem(const QString &id) const;
    bool setSystem(const QString &id);

    bool isGlobal(const QString &id) const;
    bool setGlobal(const QString &id, bool);

    QString add( const QString &trLabel, const QString &icon=QString(), bool forceGlobal=false );
    bool addCategory( const QString &id, const QString &trLabel, const QString &icon=QString(), bool forceGlobal=false, bool isSystem=false );

    bool remove( const QString &id );
    bool setLabel( const QString &id, const QString &trLabel );
    bool setIcon( const QString &id, const QString &icon );

    bool contains(const QString &id) const;
    bool containsLabel(const QString &trLabel, bool forceGlobal=false) const;
    QString idForLabel( const QString &trLabel ) const;
    QList<QString> categoryIds() const;

    bool exists( const QString &id ) const;

signals:
    void categoriesChanged();

private slots:
    void reloadCategories();

private:
    static void diff(
            const QMap<QString,QString> &orig,
            const QMap<QString,QString> &current,
            QList<QString> &added,
            QList<QString> &removed,
            QList<QString> &updated);

    QCategoryManagerData *d;
};

#endif
