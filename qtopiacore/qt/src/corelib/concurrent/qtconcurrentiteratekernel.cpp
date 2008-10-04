/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtCore module of the Qt Toolkit.
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

#include "qtconcurrentiteratekernel.h"

#if defined(Q_OS_MAC)

#include <mach/mach.h>
#include <mach/mach_time.h>
#include <unistd.h>

#elif defined(Q_OS_UNIX)

#include <time.h>
#include <unistd.h>

#elif defined(Q_OS_WIN)

#include <windows.h>

#endif


#ifndef QT_NO_CONCURRENT

QT_BEGIN_NAMESPACE

enum {
    TargetRatio = 100,
    MedianSize = 7
};

#if defined(Q_OS_MAC)

static qint64 getticks()
{
    return mach_absolute_time();
}

#elif defined(Q_OS_UNIX)


static qint64 getticks()
{
    clockid_t clockId;

#ifndef _POSIX_THREAD_CPUTIME
    clockId = CLOCK_REALTIME;
#elif (_POSIX_THREAD_CPUTIME-0 <= 0)
    // if we don't have CLOCK_THREAD_CPUTIME_ID, we have to just use elapsed realtime instead
    clockId = CLOCK_REALTIME;

#  if (_POSIX_THREAD_CPUTIME-0 == 0)
    // detect availablility of CLOCK_THREAD_CPUTIME_ID
    static long useThreadCpuTime = -2;
    if (useThreadCpuTime == -2) {
        // sysconf() will return either -1 or _POSIX_VERSION (don't care about thread races here)
        useThreadCpuTime = sysconf(_SC_THREAD_CPUTIME);
    }
    if (useThreadCpuTime != -1)
        clockId = CLOCK_THREAD_CPUTIME_ID;
#  endif
#else
    clockId = CLOCK_THREAD_CPUTIME_ID;
#endif

    struct timespec ts;
    if (clock_gettime(clockId, &ts) == -1)
        return 0;
    return (ts.tv_sec * 1000000000) + ts.tv_nsec;
}

#elif defined(Q_OS_WIN)

static qint64 getticks()
{
    LARGE_INTEGER x;
    if (!QueryPerformanceCounter(&x))
        return 0;
    return x.QuadPart;
}

#endif

static double elapsed(qint64 after, qint64 before)
{
    return double(after - before);
}

namespace QtConcurrent {

/*! \internal

*/
BlockSizeManager::BlockSizeManager(int iterationCount)
: maxBlockSize(iterationCount / (QThreadPool::globalInstance()->maxThreadCount() * 2)),
  beforeUser(0), afterUser(0),
  controlPartElapsed(MedianSize), userPartElapsed(MedianSize),
  m_blockSize(1)
{ }

// Records the time before user code.
void BlockSizeManager::timeBeforeUser()
{
    if (blockSizeMaxed())
        return;

    beforeUser = getticks();
    controlPartElapsed.addValue(elapsed(beforeUser, afterUser));
}

 // Records the time after user code and adjust the block size if we are spending
 // to much time in the for control code compared with the user code.
void BlockSizeManager::timeAfterUser()
{
    if (blockSizeMaxed())
        return;

    afterUser = getticks();
    userPartElapsed.addValue(elapsed(afterUser, beforeUser));

    if (controlPartElapsed.isMedianValid() == false)
        return;

    if (controlPartElapsed.median() * TargetRatio < userPartElapsed.median())
        return;

    m_blockSize = qMin(m_blockSize * 2,  maxBlockSize);

#ifdef QTCONCURRENT_FOR_DEBUG
    qDebug() << QThread::currentThread() << "adjusting block size" << controlPartElapsed.median() << userPartElapsed.median() << m_blockSize;
#endif

    // Reset the medians after adjusting the block size so we get
    // new measurements with the new block size.
    controlPartElapsed.reset();
    userPartElapsed.reset();
}

int BlockSizeManager::blockSize()
{
    return m_blockSize;
}

} // namespace QtConcurrent

QT_END_NAMESPACE

#endif // QT_NO_CONCURRENT
