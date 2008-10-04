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

#include "qebvtype_p.h"
#include "qgenericsequencetype_p.h"
#include "qnonetype_p.h"

#include "qcommonsequencetypes_p.h"

/* To avoid the static initialization fiasco, we put the builtin types in this compilation unit, since
 * the sequence types depends on them. */
#include "qbuiltintypes.cpp"

QT_BEGIN_NAMESPACE

using namespace QPatternist;

// STATIC DATA
#define st(var, type, card)                                             \
const SequenceType::Ptr                                                 \
CommonSequenceTypes::var(new GenericSequenceType(BuiltinTypes::type,    \
                                                 Cardinality::card()))

/* Alphabetically. */
st(ExactlyOneAnyURI,                xsAnyURI,               exactlyOne);
st(ExactlyOneAtomicType,            xsAnyAtomicType,        exactlyOne);
st(ExactlyOneAttribute,             attribute,              exactlyOne);
st(ExactlyOneBase64Binary,          xsBase64Binary,         exactlyOne);
st(ExactlyOneBoolean,               xsBoolean,              exactlyOne);
st(ExactlyOneComment,               comment,                exactlyOne);
st(ExactlyOneDate,                  xsDate,                 exactlyOne);
st(ExactlyOneDateTime,              xsDateTime,             exactlyOne);
st(ExactlyOneDayTimeDuration,       xsDayTimeDuration,      exactlyOne);
st(ExactlyOneDecimal,               xsDecimal,              exactlyOne);
st(ExactlyOneDocumentNode,          document,               exactlyOne);
st(ExactlyOneDouble,                xsDouble,               exactlyOne);
st(ExactlyOneDuration,              xsDuration,             exactlyOne);
st(ExactlyOneElement,               element,                exactlyOne);
st(ExactlyOneFloat,                 xsFloat,                exactlyOne);
st(ExactlyOneGDay,                  xsGDay,                 exactlyOne);
st(ExactlyOneGMonth,                xsGMonth,               exactlyOne);
st(ExactlyOneGMonthDay,             xsGMonthDay,            exactlyOne);
st(ExactlyOneGYear,                 xsGYear,                exactlyOne);
st(ExactlyOneGYearMonth,            xsGYearMonth,           exactlyOne);
st(ExactlyOneHexBinary,             xsHexBinary,            exactlyOne);
st(ExactlyOneInteger,               xsInteger,              exactlyOne);
st(ExactlyOneItem,                  item,                   exactlyOne);
st(ExactlyOneNode,                  node,                   exactlyOne);
st(ExactlyOneNumeric,               numeric,                exactlyOne);
st(ExactlyOneProcessingInstruction, pi,                     exactlyOne);
st(ExactlyOneQName,                 xsQName,                exactlyOne);
st(ExactlyOneString,                xsString,               exactlyOne);
st(ExactlyOneTextNode,              text,                   exactlyOne);
st(ExactlyOneTime,                  xsTime,                 exactlyOne);
st(ExactlyOneUntypedAtomic,         xsUntypedAtomic,        exactlyOne);
st(ExactlyOneYearMonthDuration,     xsYearMonthDuration,    exactlyOne);
st(OneOrMoreItems,                  item,                   oneOrMore);
st(ZeroOrMoreAtomicTypes,           xsAnyAtomicType,        zeroOrMore);
st(ZeroOrMoreElements,              element,                zeroOrMore);
st(ZeroOrMoreIntegers,              xsInteger,              zeroOrMore);
st(ZeroOrMoreItems,                 item,                   zeroOrMore);
st(ZeroOrMoreNodes,                 node,                   zeroOrMore);
st(ZeroOrMoreStrings,               xsString,               zeroOrMore);
st(ZeroOrOneAnyURI,                 xsAnyURI,               zeroOrOne);
st(ZeroOrOneAtomicType,             xsAnyAtomicType,        zeroOrOne);
st(ZeroOrOneBoolean,                xsBoolean,              zeroOrOne);
st(ZeroOrOneDate,                   xsDate,                 zeroOrOne);
st(ZeroOrOneDateTime,               xsDateTime,             zeroOrOne);
st(ZeroOrOneDayTimeDuration,        xsDayTimeDuration,      zeroOrOne);
st(ZeroOrOneDecimal,                xsDecimal,              zeroOrOne);
st(ZeroOrOneDocumentNode,           document,               zeroOrOne);
st(ZeroOrOneDuration,               xsDuration,             zeroOrOne);
st(ZeroOrOneInteger,                xsInteger,              zeroOrOne);
st(ZeroOrOneItem,                   item,                   zeroOrOne);
st(ZeroOrOneNCName,                 xsNCName,               zeroOrOne);
st(ZeroOrOneNode,                   node,                   zeroOrOne);
st(ZeroOrOneNumeric,                numeric,                zeroOrOne);
st(ZeroOrOneQName,                  xsQName,                zeroOrOne);
st(ZeroOrOneString,                 xsString,               zeroOrOne);
st(ZeroOrOneTextNode,               text,                   zeroOrOne);
st(ZeroOrOneTime,                   xsTime,                 zeroOrOne);
st(ZeroOrOneYearMonthDuration,      xsYearMonthDuration,    zeroOrOne);

#undef st

/* Special cases. */
const EmptySequenceType::Ptr    CommonSequenceTypes::Empty  (new EmptySequenceType());
const NoneType::Ptr             CommonSequenceTypes::None   (new NoneType());
const SequenceType::Ptr         CommonSequenceTypes::EBV    (new EBVType());


QT_END_NAMESPACE
