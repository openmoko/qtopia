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

#ifndef Patternist_DateTimeFNs_H
#define Patternist_DateTimeFNs_H

#include "qatomiccomparator_p.h"
#include "qcommonvalues_p.h"
#include "qdatetime_p.h"
#include "qdaytimeduration_p.h"
#include "qdecimal_p.h"
#include "qinteger_p.h"
#include "qfunctioncall_p.h"

/**
 * @file
 * @short Contains classes implementing the functions found in
 * <a href="http://www.w3.org/TR/xpath-functions/#component-exraction-functions">XQuery 1.0 and
 * XPath 2.0 Functions and Operators, 10.5 Component Extraction Functions on Durations, Dates and Times</a>.
 *
 * @ingroup Patternist_functions
 */

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

namespace QPatternist
{
    /**
     * @short Helper class for implementing functions extracting components from durations.
     *
     * Each sub-class must implement this function:
     *
     * @code
     * Item extract(const AbstractDuration *const duration) const;
     * @endcode
     *
     * This function performs the actual component extraction from the argument, that
     * is guaranteed to never be @c null.
     *
     * @ingroup Patternist_functions
     * @author Frans Englich <fenglich@trolltech.com>
     */
    template<typename TSubClass>
    class ExtractFromDurationFN : public FunctionCall
    {
    public:
        /**
         * Takes care of the argument handling, and, if applicable,
         * calls extract() with the value of the operand.
         */
        virtual Item evaluateSingleton(const DynamicContext::Ptr &context) const;
    };

    /**
     * @short Implements the function <tt>fn:years-from-duration()</tt>.
     *
     * @ingroup Patternist_functions
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class YearsFromDurationFN : public ExtractFromDurationFN<YearsFromDurationFN>
    {
    public:
        inline Item extract(const AbstractDuration *const duration) const;
    };

    /**
     * @short Implements the function <tt>fn:months-from-duration()</tt>.
     *
     * @ingroup Patternist_functions
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class MonthsFromDurationFN : public ExtractFromDurationFN<MonthsFromDurationFN>
    {
    public:
        inline Item extract(const AbstractDuration *const duration) const;
    };

    /**
     * @short Implements the function <tt>fn:days-from-duration()</tt>.
     *
     * @ingroup Patternist_functions
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class DaysFromDurationFN : public ExtractFromDurationFN<DaysFromDurationFN>
    {
    public:
        inline Item extract(const AbstractDuration *const duration) const;
    };

    /**
     * @short Implements the function <tt>fn:hours-from-duration()</tt>.
     *
     * @ingroup Patternist_functions
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class HoursFromDurationFN : public ExtractFromDurationFN<HoursFromDurationFN>
    {
    public:
        inline Item extract(const AbstractDuration *const duration) const;
    };

    /**
     * @short Implements the function <tt>fn:minutes-from-duration()</tt>.
     *
     * @ingroup Patternist_functions
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class MinutesFromDurationFN : public ExtractFromDurationFN<MinutesFromDurationFN>
    {
    public:
        inline Item extract(const AbstractDuration *const duration) const;
    };

    /**
     * @short Implements the function <tt>fn:seconds-from-duration()</tt>.
     *
     * @ingroup Patternist_functions
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class SecondsFromDurationFN : public ExtractFromDurationFN<SecondsFromDurationFN>
    {
    public:
        inline Item extract(const AbstractDuration *const duration) const;
    };

    /**
     * @short Helper class for implementing functions extracting components
     * from date/time values.
     *
     * Each sub-class must implement this function:
     *
     * @code
     * Item extract(const AbstractDuration *const duration) const;
     * @endcode
     *
     * This function performs the actual component extraction from the argument, that
     * is guaranteed to never be @c null.
     *
     * @ingroup Patternist_functions
     * @author Frans Englich <fenglich@trolltech.com>
     */
    template<typename TSubClass>
    class ExtractFromDateTimeFN : public FunctionCall
    {
    public:
        /**
         * Takes care of the argument handling, and, if applicable,
         * calls extract() with the value of the operand.
         */
        virtual Item evaluateSingleton(const DynamicContext::Ptr &context) const;
    };

    /**
     * @short Extracts the year property from a sub-class of AbstractDateTime such as DateTime or Date.
     * This function implements <tt>fn:year-from-dateTime()</tt> and <tt>fn:year-from-date()</tt>.
     *
     * @ingroup Patternist_functions
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class YearFromAbstractDateTimeFN : public ExtractFromDateTimeFN<YearFromAbstractDateTimeFN>
    {
    public:
        inline Item extract(const QDateTime &dt) const;
    };

    /**
     * @short Extracts the day property from a sub-class of AbstractDateTime such as DateTime or Date.
     * This function implements <tt>fn:day-from-dateTime()</tt> and <tt>fn:day-from-date()</tt>.
     *
     * @ingroup Patternist_functions
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class DayFromAbstractDateTimeFN : public ExtractFromDateTimeFN<DayFromAbstractDateTimeFN>
    {
    public:
        inline Item extract(const QDateTime &dt) const;
    };

    /**
     * @short Extracts the minute property from a sub-class of AbstractDateTime such as DateTime or SchemaTime.
     * Implements the functions <tt>fn:hours-from-dateTime()</tt> and
     * <tt>fn:hours-from-time()</tt>.
     *
     * @ingroup Patternist_functions
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class HoursFromAbstractDateTimeFN : public ExtractFromDateTimeFN<HoursFromAbstractDateTimeFN>
    {
    public:
        inline Item extract(const QDateTime &dt) const;
    };

    /**
     * @short Extracts the minutes property from a sub-class of AbstractDateTime such as DateTime or Date.
     * Implements the functions <tt>fn:minutes-from-dateTime()</tt> and
     * <tt>fn:minutes-from-time()</tt>.
     *
     * @ingroup Patternist_functions
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class MinutesFromAbstractDateTimeFN : public ExtractFromDateTimeFN<MinutesFromAbstractDateTimeFN>
    {
    public:
        inline Item extract(const QDateTime &dt) const;
    };

    /**
     * @short Extracts the seconds property from a sub-class of AbstractDateTime such as DateTime or Date.
     * Implements the functions <tt>fn:seconds-from-dateTime()</tt> and
     * <tt>fn:seconds-from-time()</tt>.
     *
     * @ingroup Patternist_functions
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class SecondsFromAbstractDateTimeFN : public ExtractFromDateTimeFN<SecondsFromAbstractDateTimeFN>
    {
    public:
        inline Item extract(const QDateTime &dt) const;
    };

    /**
     * @short Extracts the timezone property from a sub-class of AbstractDateTime such as DateTime or Date.
     * Implements the functions <tt>fn:timezone-from-dateTime()</tt>,
     * <tt>fn:timezone-from-time()</tt> and <tt>fn:timezone-from-date()</tt>.
     *
     * @ingroup Patternist_functions
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class TimezoneFromAbstractDateTimeFN : public ExtractFromDateTimeFN<TimezoneFromAbstractDateTimeFN>
    {
    public:
        inline Item extract(const QDateTime &dt) const;
    };

    /**
     * @short implements the functions <tt>fn:month-from-dateTime()</tt> and <tt>fn:month-from-date()</tt>.
     *
     * @ingroup Patternist_functions
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class MonthFromAbstractDateTimeFN : public ExtractFromDateTimeFN<MonthFromAbstractDateTimeFN>
    {
    public:
        inline Item extract(const QDateTime &dt) const;
    };

#include "qdatetimefns.cpp"

}

QT_END_NAMESPACE

QT_END_HEADER

#endif
