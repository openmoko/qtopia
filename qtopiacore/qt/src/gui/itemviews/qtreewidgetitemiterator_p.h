/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtGui module of the Qt Toolkit.
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

#ifndef QTREEWIDGETITEMITERATOR_P_H
#define QTREEWIDGETITEMITERATOR_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/qstack.h>

#ifndef QT_NO_TREEWIDGET
#include "qtreewidgetitemiterator.h"

QT_BEGIN_NAMESPACE

class QTreeModel;
class QTreeWidgetItem;

class QTreeWidgetItemIteratorPrivate {
    Q_DECLARE_PUBLIC(QTreeWidgetItemIterator)
public:
    QTreeWidgetItemIteratorPrivate(QTreeWidgetItemIterator *q, QTreeModel *model)
        : m_currentIndex(0), m_model(model), q_ptr(q)
    {

    }

    QTreeWidgetItemIteratorPrivate(const QTreeWidgetItemIteratorPrivate& other)
        : m_currentIndex(other.m_currentIndex), m_model(other.m_model), m_parentIndex(other.m_parentIndex)
    {

    }

    QTreeWidgetItemIteratorPrivate &operator=(const QTreeWidgetItemIteratorPrivate& other)
    {
        m_currentIndex = other.m_currentIndex;
        m_parentIndex = other.m_parentIndex;
        m_model = other.m_model;
        return (*this);
    }

    ~QTreeWidgetItemIteratorPrivate()
    {
    }

    QTreeWidgetItem* nextSibling(const QTreeWidgetItem* item) const;
    void ensureValidIterator(const QTreeWidgetItem *itemToBeRemoved);

    QTreeWidgetItem *next(const QTreeWidgetItem *current);
    QTreeWidgetItem *previous(const QTreeWidgetItem *current);
private:
    int             m_currentIndex;
    QTreeModel     *m_model;        // This iterator class should not have ownership of the model.
    QStack<int>     m_parentIndex;
    QTreeWidgetItemIterator *q_ptr;
};

QT_END_NAMESPACE

#endif // QT_NO_TREEWIDGET

#endif //QTREEWIDGETITEMITERATOR_P_H
