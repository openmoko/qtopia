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

#ifndef Patternist_SortTuple_H
#define Patternist_SortTuple_H

#include "qitem_p.h"
#include "qitem_p.h"
#include "qitemtype_p.h"

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

namespace QPatternist
{
    /**
     * @short Represents a value and its sort keys
     * in FLOWR's <tt>order by</tt> clause.
     *
     * SortTuple doesn't correspond to anything in the XPath Data Model and
     * can therefore well be described as an exotic implementation detail.
     * Most of its functions asserts because it makes no sense to
     * call them.
     *
     * SortTuple exclusively exists for use with the expressions OrderBy and
     * ReturnOrderBy, and acts as a carrier between those two for sort keys and
     * source values.
     *
     * @ingroup Patternist_xdm
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class SortTuple : public AtomicValue
    {
    public:
        /**
         * @p aSortKeys may be empty.
         */
        inline SortTuple(const Item::Iterator::Ptr &aValue,
                         const Item::Vector &aSortKeys) : m_sortKeys(aSortKeys),
                                                          m_value(aValue)
        {
            Q_ASSERT(m_value);
            Q_ASSERT(!m_sortKeys.isEmpty());
        }

        /**
         * A smart pointer wrapping SortTuple instances.
         */
        typedef QExplicitlySharedDataPointer<SortTuple> Ptr;

        /**
         * This function is sometimes called by Literal::description().
         * This function simply returns "SortTuple".
         */
        virtual QString stringValue() const;

        /**
         * @short Always asserts.
         */
        virtual Item::Iterator::Ptr typedValue() const;

        /**
         * @short Always asserts.
         */
        virtual bool isAtomicValue() const;

        /**
         * @short Always asserts.
         */
        virtual bool isNode() const;

        /**
         * @short Always asserts.
         */
        virtual bool hasError() const;

        virtual ItemType::Ptr type() const;

        inline const Item::Vector &sortKeys() const
        {
            return m_sortKeys;
        }

        inline const Item::Iterator::Ptr &value() const
        {
            return m_value;
        }

    private:
        const Item::Vector          m_sortKeys;
        const Item::Iterator::Ptr   m_value;
    };
}

QT_END_NAMESPACE

QT_END_HEADER

#endif
