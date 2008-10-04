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

#ifndef Patternist_AbstractFloatMathematician_H
#define Patternist_AbstractFloatMathematician_H

#include "qabstractfloat_p.h"
#include "qatomicmathematician_p.h"
#include "qinteger_p.h"
#include "qnumeric_p.h"
#include "qpatternistlocale_p.h"
#include "qsourcelocationreflection_p.h"

/**
 * @file
 * @short Contains classes performing arithemetic operations between atomic values, such as
 * substracting two dates.
 */

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

namespace QPatternist
{
    /**
     * @short Performs arithmetics between AbstractFloat values (Float and Double classes).
     *
     * @ingroup Patternist_xdm
     * @author Vincent Ricard <magic@magicninja.org>
     */
    template <const bool isDouble>
    class AbstractFloatMathematician : public AtomicMathematician
                                     , public DelegatingSourceLocationReflection
    {
    public:
        inline AbstractFloatMathematician(const SourceLocationReflection *const r) : DelegatingSourceLocationReflection(r)
        {
        }

        virtual Item calculate(const Item &o1,
                                    const Operator op,
                                    const Item &o2,
                                    const QExplicitlySharedDataPointer<DynamicContext> &context) const;
    };

#include "qabstractfloatmathematician.cpp"

    /**
     * An instantiation of AbstractFloatMathematician that handles @c xs:double.
     */
    typedef AbstractFloatMathematician<true> DoubleMathematician;

    /**
     * An instantiation of AbstractFloatMathematician that handles @c xs:float.
     */
    typedef AbstractFloatMathematician<false> FloatMathematician;
}

QT_END_NAMESPACE

QT_END_HEADER

#endif
