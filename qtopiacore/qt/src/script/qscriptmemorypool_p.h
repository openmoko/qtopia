/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtScript module of the Qt Toolkit.
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

#ifndef QSCRIPTMEMORYPOOL_P_H
#define QSCRIPTMEMORYPOOL_P_H

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

#include <QtCore/qglobal.h>
#include <QtCore/qshareddata.h>
#include <string.h>

QT_BEGIN_NAMESPACE

namespace QScript {

class MemoryPool : public QSharedData
{
public:
    enum { maxBlockCount = -1 };
    enum { defaultBlockSize = 1 << 16 };

    MemoryPool() {
        m_blockIndex = maxBlockCount;
        m_currentIndex = 0;
        m_storage = 0;
        m_currentBlock = 0;
    }

    virtual ~MemoryPool() {
        for (int index = 0; index < m_blockIndex + 1; ++index)
            qFree(m_storage[index]);

        qFree(m_storage);
    }

    char *allocate(int bytes) {
        bytes += (8 - bytes) & 7; // ensure multiple of 8 bytes (maintain alignment)
        if (m_currentBlock == 0 || defaultBlockSize < m_currentIndex + bytes) {
            ++m_blockIndex;

            m_storage = reinterpret_cast<char**>(qRealloc(m_storage, sizeof(char*) * (1 + m_blockIndex)));
            m_currentBlock = m_storage[m_blockIndex] = reinterpret_cast<char*>(qMalloc(defaultBlockSize));
            ::memset(m_currentBlock, 0, defaultBlockSize);

            m_currentIndex = (8 - quintptr(m_currentBlock)) & 7; // ensure first chunk is 64-bit aligned
            Q_ASSERT(m_currentIndex + bytes <= defaultBlockSize);
        }

        char *p = reinterpret_cast<char *>
            (m_currentBlock + m_currentIndex);

        m_currentIndex += bytes;

        return p;
    }

private:
    int m_blockIndex;
    int m_currentIndex;
    char *m_currentBlock;
    char **m_storage;

private:
    Q_DISABLE_COPY(MemoryPool)
};

} // namespace QScript

QT_END_NAMESPACE

#endif
