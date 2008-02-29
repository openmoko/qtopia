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
#ifndef QDOCUMENTSERVERCATEGORYSTORE_P_H
#define QDOCUMENTSERVERCATEGORYSTORE_P_H

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

class QDocumentServerCategoryStorePrivate;

class QDocumentServerCategoryStore : public QCategoryStore
{
public:
    QDocumentServerCategoryStore( QObject *parent = 0 );

    virtual ~QDocumentServerCategoryStore();

    virtual bool addCategory( const QString &categoryId, const QString &scope, const QString &label, const QString &icon, bool isSystem );

    virtual bool categoryExists( const QString &categoryId );

    virtual QCategoryData categoryFromId( const QString &categoryId );

    virtual QCategoryDataMap scopeCategories( const QString &scope );

    virtual bool removeCategory( const QString &categoryId );

    virtual bool setCategoryScope( const QString &categoryId, const QString &scope );

    virtual bool setCategoryIcon( const QString &categoryId, const QString &icon );

    virtual bool setCategoryRingTone( const QString &id, const QString &fileName );

    virtual bool setCategoryLabel( const QString &categoryId, const QString &label );

    virtual bool setSystemCategory( const QString &categoryId );

private:
    QDocumentServerCategoryStorePrivate *d;
};

#endif
