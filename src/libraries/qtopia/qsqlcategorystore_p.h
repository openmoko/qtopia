/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
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
#ifndef QSQLCATEGORYSTORE_P_H
#define QSQLCATEGORYSTORE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qtopia API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <qtopia/private/qcategorystore_p.h>
#include <QCache>

class QSqlRecord;

class QSqlCategoryStore : public QCategoryStore
{
    Q_OBJECT
public:
    QSqlCategoryStore();
    virtual ~QSqlCategoryStore();

    virtual bool addCategory( const QString &id, const QString &scope, const QString &label, const QString &icon, bool isSystem );
    virtual bool categoryExists( const QString &id );
    virtual QCategoryData categoryFromId( const QString &id );
    virtual QMap< QString, QCategoryData > scopeCategories( const QString &scope );
    virtual bool removeCategory( const QString &id );
    virtual bool setCategoryScope( const QString &id, const QString &scope );
    virtual bool setCategoryIcon( const QString &id, const QString &icon );
    virtual bool setCategoryRingTone( const QString &id, const QString &icon );
    virtual bool setCategoryLabel( const QString &id, const QString &label );
    virtual bool setSystemCategory( const QString &id );

private slots:
    void reloadCategories();

private:
    QCategoryData categoryFromRecord( const QString &id, const QSqlRecord &record );

    void categoryAddedLocal( const QString &id );
    void categoryRemovedLocal( const QString &id );
    void categoryChangedLocal( const QString &id );

    QCache< QString, QCategoryData > m_cache;
};

#endif
