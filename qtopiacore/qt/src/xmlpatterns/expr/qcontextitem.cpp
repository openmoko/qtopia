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

#include "qbuiltintypes_p.h"
#include "qcommonsequencetypes_p.h"
#include "qgenericsequencetype_p.h"

#include "qcontextitem_p.h"

QT_BEGIN_NAMESPACE

using namespace QPatternist;

Item ContextItem::evaluateSingleton(const DynamicContext::Ptr &context) const
{
    return context->contextItem();
}

Expression::Ptr ContextItem::compress(const StaticContext::Ptr &context)
{
    m_itemType = context->contextItemType();
    return EmptyContainer::compress(context);
}

Expression::Ptr ContextItem::typeCheck(const StaticContext::Ptr &context,
                                       const SequenceType::Ptr &reqType)
{
    m_itemType = context->contextItemType();
    return EmptyContainer::typeCheck(context, reqType);
}

SequenceType::Ptr ContextItem::staticType() const
{
    /* We test m_itemType here because Patternist View calls staticType() before the typeCheck()
     * stage. */
    if(m_itemType)
        return makeGenericSequenceType(m_itemType, Cardinality::exactlyOne());
    else
        return CommonSequenceTypes::ExactlyOneItem;
}

Expression::Properties ContextItem::properties() const
{
    return DisableElimination | RequiresContextItem | EvaluationCacheRedundant;
}

ExpressionVisitorResult::Ptr ContextItem::accept(const ExpressionVisitor::Ptr &visitor) const
{
    return visitor->visit(this);
}

Expression::ID ContextItem::id() const
{
    return IDContextItem;
}

ItemType::Ptr ContextItem::expectedContextItemType() const
{
    return BuiltinTypes::item;
}

const SourceLocationReflection *ContextItem::actualReflection() const
{
    if(m_expr)
        return m_expr.data();
    else
        return this;
}

void ContextItem::announceFocusType(const ItemType::Ptr &type)
{
    Q_ASSERT(type);
    m_itemType = type;
}

QT_END_NAMESPACE
