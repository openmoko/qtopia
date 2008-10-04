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

#ifndef Patternist_Primitives_H
#define Patternist_Primitives_H

#include <QtGlobal>

/**
 * @file
 * @short Contains enumerators and typedefs applying
 * for Patternist on a global scale, as well as central documentation.
 */

/**
 * @short Contains Patternist, an XPath 2.0, XQuery 1.0 and XSL-T 2.0 implementation.
 *
 * @author Frans Englich <fenglich@trolltech.com>
 */
QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

class QString;

namespace QPatternist
{

    /**
     * @defgroup Patternist_cppWXSTypes C++ Primitives for W3C XML Schema Number Types
     *
     * The implementations of W3C XML Schema's(WXS) number types, more specifically
     * their value spaces, must in the end be represented by primitive C++ types.
     * In addition, there is an extensive range of functions and classes that in
     * different ways deals with data that will end up as instances of the WXS
     * types. For this reason, a set of typedefs for these primitives exists, that
     * are used throughout the API. This ensures consistency, reduces the amount
     * of conversions, and potentially precision loss in conversions.
     *
     * @author Frans Englich <fenglich@trolltech.com>
     */

    /**
     * This is the native C++ scalar type holding the value space
     * for atomic values of type xs:double. Taking this type, xsDouble,
     * as parameter, is the most efficient way to integrate with xs:double.
     *
     * @ingroup Patternist_cppWXSTypes
     */
    typedef qreal xsDouble;

    /**
     * This is the native C++ scalar type holding the value space
     * for atomic values of type xs:float. Taking this type, xsFloat,
     * as parameter, is the most efficient way to integrate with xs:float.
     *
     * @ingroup Patternist_cppWXSTypes
     */
    typedef xsDouble xsFloat;

    /**
     * This is the native C++ scalar type holding the value space
     * for atomic values of type xs:decimal. Taking this type, xsDecimal,
     * as parameter, is the most efficient way to integrate with xs:decimal.
     *
     * @ingroup Patternist_cppWXSTypes
     */
    typedef xsDouble xsDecimal;

    /**
     * This is the native C++ scalar type holding the value space
     * for atomic values of type xs:integer. Taking this type, xsInteger,
     * as parameter, is the most efficient way to integrate with xs:integer.
     *
     * @ingroup Patternist_cppWXSTypes
     */
    typedef qint64 xsInteger;

    /**
     * This is the native C++ scalar type holding the value space
     * for atomic values of type xs:integer. Taking this type, xsInteger,
     * as parameter, is the most efficient way to integrate with xs:integer.
     *
     * @ingroup Patternist_cppWXSTypes
     */
    typedef qint32 VariableSlotID;

    typedef qint32  DayCountProperty;
    typedef qint32  HourCountProperty;
    typedef qint32  MinuteCountProperty;
    typedef qint32  MonthCountProperty;
    typedef qint32  SecondCountProperty;
    typedef qint64  MSecondCountProperty;
    typedef qint32  SecondProperty;
    typedef qint32  YearProperty;
    typedef qint8   DayProperty;
    typedef qint8   HourProperty;
    typedef qint8   MinuteProperty;
    typedef qint8   MonthProperty;

    /**
     * Milliseconds. 1 equals 0.001 SecondProperty.
     */
    typedef qint16  MSecondProperty;

    /**
     * The hour property of a zone offset. For example, -13 in the
     * zone offset "-13:08".
     */
    typedef qint8   ZOHourProperty;

    /**
     * The minute property of a zone offset. For example, -08 in the
     * zone offset "-13:08".
     */
    typedef qint8   ZOMinuteProperty;

    /**
     * The full zone offset in minutes.
     */
    typedef qint32  ZOTotal;

    /**
     * @short Identical to Qt::escape() but since Qt::escape() is in QtGui, using
     * it creates a dependency on that library. This function does not.
     *
     * @see Qt::escape()
     */
    QString escape(const QString &input);
}

QT_END_NAMESPACE

QT_END_HEADER

#endif
