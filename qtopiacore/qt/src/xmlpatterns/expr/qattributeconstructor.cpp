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

#include <QUrl>

#include "qcommonsequencetypes_p.h"
#include "qnodebuilder_p.h"
#include "qqnamevalue_p.h"

#include "qattributeconstructor_p.h"

QT_BEGIN_NAMESPACE

using namespace QPatternist;

AttributeConstructor::AttributeConstructor(const Expression::Ptr &op1,
                                           const Expression::Ptr &op2) : PairContainer(op1, op2)
{
}

QString AttributeConstructor::processValue(const QXmlName name,
                                           const QString &value)
{
    if(name == QXmlName(StandardNamespaces::xml, StandardLocalNames::id))
        return value.simplified();
    else
        return value;
}

Item AttributeConstructor::evaluateSingleton(const DynamicContext::Ptr &context) const
{
    const Item nameItem(m_operand1->evaluateSingleton(context));
    const Item content(m_operand2->evaluateSingleton(context));

    const QXmlName name(nameItem.as<QNameValue>()->qName());
    const QString value(processValue(name, content ? content.stringValue() : QString()));
    const NodeBuilder::Ptr nodeBuilder(context->nodeBuilder(QUrl()));

    nodeBuilder->attribute(name, QStringRef(&value));

    const QAbstractXmlNodeModel::Ptr nm(nodeBuilder->builtDocument());
    context->addNodeModel(nm);
    return nm->root(QXmlNodeModelIndex());
}

void AttributeConstructor::evaluateToSequenceReceiver(const DynamicContext::Ptr &context) const
{
    QAbstractXmlReceiver *const receiver = context->outputReceiver();
    const Item nameItem(m_operand1->evaluateSingleton(context));

    const Item content(m_operand2->evaluateSingleton(context));
    const QXmlName name(nameItem.as<QNameValue>()->qName());
    const QString value(processValue(name, content ? content.stringValue() : QString()));

    receiver->attribute(name, QStringRef(&value));
}

SequenceType::Ptr AttributeConstructor::staticType() const
{
    return CommonSequenceTypes::ExactlyOneAttribute;
}

SequenceType::List AttributeConstructor::expectedOperandTypes() const
{
    SequenceType::List result;
    result.append(CommonSequenceTypes::ExactlyOneQName);
    result.append(CommonSequenceTypes::ZeroOrMoreItems);
    return result;
}

Expression::Properties AttributeConstructor::properties() const
{
    return DisableElimination | IsNodeConstructor;
}

ExpressionVisitorResult::Ptr
AttributeConstructor::accept(const ExpressionVisitor::Ptr &visitor) const
{
    return visitor->visit(this);
}

Expression::ID AttributeConstructor::id() const
{
    return IDAttributeConstructor;
}

QT_END_NAMESPACE
