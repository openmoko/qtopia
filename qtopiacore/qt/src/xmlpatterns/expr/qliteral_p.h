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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.

#ifndef Patternist_Literal_H
#define Patternist_Literal_H

#include "qemptycontainer_p.h"

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

namespace QPatternist
{
    /**
     * @short Houses an AtomicValue, making it available as an Expression.
     *
     * This is not only literals that can be created via the XQuery syntax(strings and numbers), but
     * all other atomic values, such as <tt>xs:date</tt> or <tt>xs:time</tt>.
     *
     * @see <a href="http://www.w3.org/TR/xquery/#id-literals">XQuery 1.0: An XML Query Language,
     * 3.1.1 Literals</a>
     * @author Frans Englich <fenglich@trolltech.com>
     * @ingroup Patternist_expressions
     */
    class Literal : public EmptyContainer
    {
    public:
        /**
         * Creates a Literal that represents @p item.
         *
         * @param item must be non-null and cannot be a QXmlNodeModelIndex.
         */
        Literal(const Item &item);

        virtual Item evaluateSingleton(const DynamicContext::Ptr &context) const;
        virtual bool evaluateEBV(const DynamicContext::Ptr &context) const;
        void evaluateToSequenceReceiver(const DynamicContext::Ptr &context) const;

        virtual SequenceType::Ptr staticType() const;
        virtual ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const;
        virtual ID id() const;
        virtual QString description() const;

        /**
         * @returns Expression::DisableElimination and Expression::IsEvaluated
         */
        virtual Properties properties() const;

        /**
         * Declaring the return value of this function a const reference, leads
         * to crashes in patternistview, for a to me unknown reason.
         */
        inline Item item() const
        {
            return m_item;
        }

    private:
        const Item m_item;
    };

    /**
     * @short Creates a Literal that wraps @p item, and returns it.
     *
     * This simplifies code. Instead of writing:
     *
     * @code
     * Expression::Ptr(new Literal(item));
     * @endcode
     *
     * One can write:
     *
     * @code
     * wrapLiteral(item);
     * @endcode
     *
     * This function is not declared static, because it breaks the build on
     * at least aix-xlc-64.
     *
     * @relates Literal
     */
    inline Expression::Ptr wrapLiteral(const Item &item,
                                       const StaticContext::Ptr &context,
                                       const SourceLocationReflection *const r)
    {
        Q_ASSERT(item);

        const Expression::Ptr retval(new Literal(item));
        context->addLocation(retval.data(), context->locationFor(r));

        return retval;
    }
}

QT_END_NAMESPACE

QT_END_HEADER

#endif
