/****************************************************************************
**
** Copyright (C) 1992-2008 Trolltech ASA. All rights reserved.
**
** This file is part of the QtScript module of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://trolltech.com/products/qt/licenses/licensing/opensource/
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** In addition, as a special exception, Trolltech gives you certain
** additional rights. These rights are described in the Trolltech GPL
** Exception version 1.0, which can be found at
** http://www.trolltech.com/products/qt/gplexception/ and in the file
** GPL_EXCEPTION.txt in this package.
**
** In addition, as a special exception, Trolltech, as the sole copyright
** holder for Qt Designer, grants users of the Qt/Eclipse Integration
** plug-in the right for the Qt/Eclipse Integration to link to
** functionality provided by Qt Designer and its related libraries.
**
** Trolltech reserves all rights not expressly granted herein.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
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

#endif

