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

#ifndef Patternist_AtomicMathematician_H
#define Patternist_AtomicMathematician_H

#include <QFlags>

#include "qdynamiccontext_p.h"
#include "qitem_p.h"
#include "qatomictypedispatch_p.h"

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

namespace QPatternist
{
    /**
     * @short Base class for classes that performs arithmetic operations between atomic values.
     *
     * @ingroup Patternist_xdm
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class Q_AUTOTEST_EXPORT AtomicMathematician : public AtomicTypeVisitorResult
    {
    public:
        virtual ~AtomicMathematician();

        typedef QExplicitlySharedDataPointer<AtomicMathematician> Ptr;

        enum Operator
        {
            /**
             * @see <a href="http://www.w3.org/TR/xpath-functions/#func-numeric-divide">XQuery 1.0
             * and XPath 2.0 Functions and Operators, 6.2.4 op:numeric-divide</a>
             */
            Div         = 1,

            /**
             * @see <a href="http://www.w3.org/TR/xpath-functions/#func-numeric-integer-divide">XQuery 1.0
             * and XPath 2.0 Functions and Operators, 6.2.5 op:numeric-integer-divide</a>
             */
            IDiv        = 2,

            /**
             * @see <a href="http://www.w3.org/TR/xpath-functions/#func-numeric-subtract">XQuery 1.0
             * and XPath 2.0 Functions and Operators, 6.2.2 op:numeric-subtract</a>
             */
            Substract   = 4,

            /**
             * @see <a href="http://www.w3.org/TR/xpath-functions/#func-numeric-mod">XQuery 1.0
             * and XPath 2.0 Functions and Operators, 6.2.6 op:numeric-mod</a>
             */
            Mod         = 8,

            /**
             * @see <a href="http://www.w3.org/TR/xpath-functions/#func-numeric-multiply">XQuery 1.0
             * and XPath 2.0 Functions and Operators, 6.2.3 op:numeric-multiply</a>
             */
            Multiply    = 16,

            /**
             * @see <a href="http://www.w3.org/TR/xpath-functions/#func-numeric-add">XQuery 1.0
             * and XPath 2.0 Functions and Operators, 6.2.1 op:numeric-add</a>
             */
            Add         = 32
        };

        typedef QFlags<Operator> Operators;

        virtual Item calculate(const Item &operand1,
                                    const Operator op,
                                    const Item &operand2,
                                    const QExplicitlySharedDataPointer<DynamicContext> &context) const = 0;

        static QString displayName(const AtomicMathematician::Operator op);

    };
    Q_DECLARE_OPERATORS_FOR_FLAGS(AtomicMathematician::Operators)
}

QT_END_NAMESPACE

QT_END_HEADER

#endif
