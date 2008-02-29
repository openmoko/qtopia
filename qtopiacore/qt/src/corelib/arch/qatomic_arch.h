/****************************************************************************
**
** Copyright (C) 1992-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QATOMIC_ARCH_H
#define QATOMIC_ARCH_H

#include "QtCore/qglobal.h"

QT_BEGIN_HEADER

#if defined(QT_ARCH_ALPHA)
#  include "QtCore/qatomic_alpha.h"
#elif defined(QT_ARCH_ARM)
#  include "QtCore/qatomic_arm.h"
#elif defined(QT_ARCH_BOUNDSCHECKER)
#  include "QtCore/qatomic_boundschecker.h"
#elif defined(QT_ARCH_GENERIC)
#  include "QtCore/qatomic_generic.h"
#elif defined(QT_ARCH_I386)
#  include "QtCore/qatomic_i386.h"
#elif defined(QT_ARCH_IA64)
#  include "QtCore/qatomic_ia64.h"
#elif defined(QT_ARCH_MACOSX)
#  include "QtCore/qatomic_macosx.h"
#elif defined(QT_ARCH_MIPS)
#  include "QtCore/qatomic_mips.h"
#elif defined(QT_ARCH_PARISC)
#  include "QtCore/qatomic_parisc.h"
#elif defined(QT_ARCH_POWERPC)
#  include "QtCore/qatomic_powerpc.h"
#elif defined(QT_ARCH_S390)
#  include "QtCore/qatomic_s390.h"
#elif defined(QT_ARCH_SPARC)
#  include "QtCore/qatomic_sparc.h"
#elif defined(QT_ARCH_WINDOWS)
#  include "QtCore/qatomic_windows.h"
#elif defined(QT_ARCH_X86_64)
#  include "QtCore/qatomic_x86_64.h"
#else
#  error "Qt has not been ported to this architecture"
#endif

QT_END_HEADER

#endif // QATOMIC_ARCH_H

