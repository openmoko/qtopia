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

#ifndef Patternist_UnaryExpression_H
#define Patternist_UnaryExpression_H

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

#include "qarithmeticexpression_p.h"

namespace QPatternist
{
    /**
     * @short Implements XPath 2.0 unary expression, <tt>(-|+)ValueExpr</tt>.
     *
     * UnaryExpression is implemented by rewriting the expression <tt>operator [expr]</tt>
     * to the ArithmeticExpression <tt>0 operator [expr]</tt>. For example, the expression
     * <tt>+3</tt> becomes <tt>0 + 3</tt>, and <tt>-nodetest</tt> becomes <tt>0 - nodetest</tt>.
     *
     * On top of that expression ArithmeticExpression does the usual type
     * checking conversion. The only thing this class do, is to overide
     * evaluateSingleton() and calls Numeric::toNegated(). The reason this
     * UnaryExpression is needed at all and that <tt>0 - [expr]</tt> is
     * insufficent is that <tt>0 - xs:double(0)</tt> needs to return -0,
     * instead of 0. I know no other difference.
     *
     * In most cases the constant propagation optimization rewrites UnaryExpression into
     * a value, an instance of a sub-class of the Numeric class, wrapped with
     * Literal.
     *
     * Beyond the mathematical implication the unary expression have, it also
     * have the significant effect that it may invoke type promotion or that an expression
     * may contain a type error. For example, the expression "+'a string'" contains a type error, since
     * no unary operator is defined for @c xs:string. This is the reason why the '+' unary
     * operator isn't ignored.
     *
     * @see <a href="http://www.w3.org/TR/xpath20/#id-arithmetic">XML Path Language
     * (XPath) 2.0, 3.4 Arithmetic Expressions</a>
     * @see <a href="http://www.w3.org/TR/xpath-functions/#func-numeric-unary-plus">XQuery 1.0 and XPath
     * 2.0 Functions and Operators, 6.2.7 op:numeric-unary-plus</a>
     * @see <a href="http://www.w3.org/TR/xpath-functions/#func-numeric-unary-minus">XQuery 1.0 and XPath
     * 2.0 Functions and Operators, 6.2.8 op:numeric-unary-minus</a>
     * @author Frans Englich <fenglich@trolltech.com>
     * @ingroup Patternist_expressions
     */
    class UnaryExpression : public ArithmeticExpression
    {
    public:
        UnaryExpression(const AtomicMathematician::Operator op,
                        const Expression::Ptr &operand,
                        const StaticContext::Ptr &context);

        virtual Item evaluateSingleton(const DynamicContext::Ptr &context) const;

    private:
        Q_DISABLE_COPY(UnaryExpression)
    };
}

QT_END_NAMESPACE

QT_END_HEADER

#endif
