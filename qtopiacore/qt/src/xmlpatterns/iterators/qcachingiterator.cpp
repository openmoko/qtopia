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



#include "qcachingiterator_p.h"

QT_BEGIN_NAMESPACE

using namespace QPatternist;

CachingIterator::CachingIterator(ItemSequenceCacheCell::Vector &cacheCells,
                                 const VariableSlotID slot,
                                 const DynamicContext::Ptr &context) : m_position(0),
                                                                       m_varSlot(slot),
                                                                       m_context(context),
                                                                       m_cacheCells(cacheCells),
                                                                       m_usingCache(true)
{
    Q_ASSERT(m_varSlot > -1);
    Q_ASSERT(m_context);
    Q_ASSERT(m_cacheCells.at(m_varSlot).sourceIterator);
    Q_ASSERT_X((m_cacheCells.at(m_varSlot).cachedItems.isEmpty() && m_cacheCells.at(m_varSlot).cacheState == ItemSequenceCacheCell::Empty) ||
               m_cacheCells.at(m_varSlot).cacheState == ItemSequenceCacheCell::PartiallyPopulated,
               Q_FUNC_INFO,
               "It makes no sense to construct a CachingIterator for a cache that is ItemSequenceCacheCell::Full.");
}

Item CachingIterator::next()
{
    ItemSequenceCacheCell &cell = m_cacheCells[m_varSlot];
    if(m_position == -1)
        return Item();

    if(m_usingCache)
    {
        ++m_position;

        /* QAbstractXmlForwardIterator::position() starts at 1, while Qt's container classes
         * starts at 0. */
        if(m_position - 1 < cell.cachedItems.count())
        {
            m_current = cell.cachedItems.at(m_position - 1);
            return m_current;
        }
        else
        {
            cell.cacheState = ItemSequenceCacheCell::PartiallyPopulated;
            m_usingCache = false;
            /* We decrement here so we don't have to add a branch for this
             * when using the source QAbstractXmlForwardIterator below. */
            --m_position;
        }
    }

    m_current = cell.sourceIterator->next();

    if(m_current)
    {
        cell.cachedItems.append(m_current);
        Q_ASSERT(cell.cacheState == ItemSequenceCacheCell::PartiallyPopulated);
        ++m_position;
        return m_current;
    }
    else
    {
        m_position = -1;
        cell.cacheState = ItemSequenceCacheCell::Full;
        return Item();
    }
}

Item CachingIterator::current() const
{
    return m_current;
}

xsInteger CachingIterator::position() const
{
    return m_position;
}

Item::Iterator::Ptr CachingIterator::copy() const
{
    const ItemSequenceCacheCell &cell = m_cacheCells.at(m_varSlot);
    if(cell.cacheState == ItemSequenceCacheCell::Full)
        return makeListIterator(cell.cachedItems);
    else
        return Item::Iterator::Ptr(new CachingIterator(m_cacheCells, m_varSlot, m_context));
}

QT_END_NAMESPACE
