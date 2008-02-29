/****************************************************************************
**
** Copyright (C) 1992-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QDBUSXMLPARSER_H
#define QDBUSXMLPARSER_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of the QLibrary class.  This header file may change from
// version to version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/qmap.h>
#include <QtXml/qdom.h>
#include <qdbusmacros.h>
#include <qdbusintrospection_p.h>

/*!
    \internal
*/
class QDBusXmlParser
{
    QString m_service;
    QString m_path;
    QDomElement m_node;

public:
    QDBusXmlParser(const QString& service, const QString& path,
                   const QString& xmlData);
    QDBusXmlParser(const QString& service, const QString& path,
                   const QDomElement& node);

    QDBusIntrospection::Interfaces interfaces() const;
    QSharedDataPointer<QDBusIntrospection::Object> object() const;
    QSharedDataPointer<QDBusIntrospection::ObjectTree> objectTree() const;
};

#endif
