/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtXMLPatterns module of the Qt Toolkit.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.  In addition, as a special
** exception, Nokia gives you certain additional rights. These rights
** are described in the Nokia Qt GPL Exception version 1.2, included in
** the file GPL_EXCEPTION.txt in this package.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/

#include <QHash>

#include "qbuiltintypes_p.h"
#include "qitem_p.h"
#include "qitem_p.h"

#include "qnamespacenametest_p.h"

QT_BEGIN_NAMESPACE

using namespace QPatternist;

NamespaceNameTest::NamespaceNameTest(const ItemType::Ptr &primaryType,
                                     const QXmlName::NamespaceCode namespaceURI) : AbstractNodeTest(primaryType),
                                                                                m_namespaceURI(namespaceURI)
{
}

ItemType::Ptr NamespaceNameTest::create(const ItemType::Ptr &primaryType, const QXmlName::NamespaceCode namespaceURI)
{
    Q_ASSERT(primaryType);

    return ItemType::Ptr(new NamespaceNameTest(primaryType, namespaceURI));
}

bool NamespaceNameTest::itemMatches(const Item &item) const
{
    Q_ASSERT(item.isNode());
    return m_primaryType->itemMatches(item) &&
           item.asNode().name().namespaceURI() == m_namespaceURI;
}

QString NamespaceNameTest::displayName(const NamePool::Ptr &np) const
{
    return QLatin1Char('{') + np->stringForNamespace(m_namespaceURI) + QLatin1String("}:*");
}

ItemType::InstanceOf NamespaceNameTest::instanceOf() const
{
    return ClassNamespaceNameTest;
}

bool NamespaceNameTest::operator==(const ItemType &other) const
{
    return other.instanceOf() == ClassNamespaceNameTest &&
           static_cast<const NamespaceNameTest &>(other).m_namespaceURI == m_namespaceURI;
}

QT_END_NAMESPACE
