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

#include "qanyuri_p.h"
#include "qboolean_p.h"
#include "qcommonsequencetypes_p.h"
#include "qcommonvalues_p.h"
#include "qemptysequence_p.h"
#include "qitemmappingiterator_p.h"
#include "qnodesortexpression_p.h"
#include "qpatternistlocale_p.h"
#include "private/qxmlutils_p.h"

#include "qsequencegeneratingfns_p.h"

QT_BEGIN_NAMESPACE

using namespace QPatternist;

IdFN::IdFN() : m_hasCreatedSorter(false)
{
}

Item IdFN::mapToItem(const Item &id,
                     const IDContext &context) const
{
    const QString ncName(id.stringValue());

    if(QXmlUtils::isNCName(ncName))
        return context.second->elementById(context.first->namePool()->allocateQName(QString(), ncName));
    else
        return Item();
}

Item::Iterator::Ptr IdFN::evaluateSequence(const DynamicContext::Ptr &context) const
{
    const Item::Iterator::Ptr idrefs(m_operands.first()->evaluateSequence(context));
    const Item node(m_operands.last()->evaluateSingleton(context));

    checkTargetNode(node.asNode(), context);

    return makeItemMappingIterator<Item,
                                   Item,
                                   IdFN::ConstPtr,
                                   IDContext>(ConstPtr(this),
                                              idrefs,
                                              qMakePair(context, node.asNode().model()));
}


Expression::Ptr IdFN::typeCheck(const StaticContext::Ptr &context,
                                const SequenceType::Ptr &reqType)
{
    if(m_hasCreatedSorter)
        return FunctionCall::typeCheck(context, reqType);
    else
    {
        const Expression::Ptr newMe(new NodeSortExpression(Expression::Ptr(this)));
        // TODO
        m_hasCreatedSorter = true;
        return newMe->typeCheck(context, reqType);
    }
}

void IdFN::checkTargetNode(const QXmlNodeModelIndex &node, const DynamicContext::Ptr &context) const
{
    if(node.root().kind() != QXmlNodeModelIndex::Document)
    {
        context->error(QtXmlPatterns::tr("The root node of the second argument "
                                         "to function %1 must be a document "
                                         "node. %2 is not a document node.")
                       .arg(formatFunction(context->namePool(), signature()),
                            formatData(node)),
                       ReportContext::FODC0001, this);
    }
}

Item::Iterator::Ptr IdrefFN::evaluateSequence(const DynamicContext::Ptr &context) const
{
    const Item::Iterator::Ptr ids(m_operands.first()->evaluateSequence(context));

    Item mId(ids->next());
    if(!mId)
        return CommonValues::emptyIterator;

    const Item node(m_operands.last()->evaluateSingleton(context));
    checkTargetNode(node.asNode(), context);

    return CommonValues::emptyIterator; /* TODO Haven't implemented further. */
}

Item DocFN::evaluateSingleton(const DynamicContext::Ptr &context) const
{
    const Item itemURI(m_operands.first()->evaluateSingleton(context));

    if(!itemURI)
        return Item();

    /* These two lines were previously in a separate function but are now duplicated
     * in DocAvailableFN::evaluateEBV() and DocFN::typeCheck(),
     * as part of a workaround for solaris-cc-64. DocFN::typeCheck() is in qsequencefns.cpp
     * as part of that workaround. */
    const QUrl mayRela(AnyURI::toQUrl<ReportContext::FODC0005>(itemURI.stringValue(), context, this));
    const QUrl uri(context->resolveURI(mayRela, staticBaseURI()));

    Q_ASSERT(uri.isValid());
    Q_ASSERT(!uri.isRelative());

    const Item doc(context->resourceLoader()->openDocument(uri, context));

    return doc;
}

SequenceType::Ptr DocFN::staticType() const
{
    if(m_type)
        return m_type;
    else
        return CommonSequenceTypes::ZeroOrOneDocumentNode;
}

bool DocAvailableFN::evaluateEBV(const DynamicContext::Ptr &context) const
{
    const Item itemURI(m_operands.first()->evaluateSingleton(context));

    /* 15.5.4 fn:doc reads: "If $uri is the empty sequence, the result is an empty sequence."
     * Hence, we return false for the empty sequence, because this doesn't hold true:
     * "If this function returns true, then calling fn:doc($uri) within
     * the same execution scope must return a document node."(15.5.5 fn:doc-available) */
    if(!itemURI)
        return false;

    /* These two lines are duplicated in DocFN::evaluateSingleton(), as part
     * of a workaround for solaris-cc-64. */
    const QUrl mayRela(AnyURI::toQUrl<ReportContext::FODC0005>(itemURI.stringValue(), context, this));
    const QUrl uri(context->resolveURI(mayRela, staticBaseURI()));

    Q_ASSERT(!uri.isRelative());
    return context->resourceLoader()->isDocumentAvailable(uri);
}

Expression::Ptr DocAvailableFN::typeCheck(const StaticContext::Ptr &context,
                                          const SequenceType::Ptr &reqType)
{
    prepareStaticBaseURI(context);
    return FunctionCall::typeCheck(context, reqType);
}

Item::Iterator::Ptr CollectionFN::evaluateSequence(const DynamicContext::Ptr &context) const
{
    // TODO resolve with URI resolve
    if(m_operands.isEmpty())
    {
        // TODO check default collection
        context->error(QtXmlPatterns::tr("The default collection is undefined"),
                       ReportContext::FODC0002, this);
        return CommonValues::emptyIterator;
    }
    else
    {
        const Item itemURI(m_operands.first()->evaluateSingleton(context));

        if(itemURI)
        {
            const QUrl uri(AnyURI::toQUrl<ReportContext::FODC0004>(itemURI.stringValue(), context, this));

            // TODO 2. Resolve against static context base URI(store base URI at compile time)
            context->error(QtXmlPatterns::tr("%1 cannot be retrieved").arg(formatURI(uri)),
                           ReportContext::FODC0004, this);
            return CommonValues::emptyIterator;
        }
        else
        {
            /* This is out default collection currently, */
            return CommonValues::emptyIterator;
        }
    }
}

QT_END_NAMESPACE
