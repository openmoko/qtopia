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

#ifdef QT_HAVE_IWMMXT

#include <mmintrin.h>
#if defined(Q_OS_WINCE)
#  include "qplatformdefs.h"
#endif
#if !defined(__IWMMXT__) && !defined(Q_OS_WINCE)
#  include <xmmintrin.h>
#elif defined(Q_OS_WINCE_STD) && defined(_X86_)
#  pragma warning(disable: 4391)
#  include <xmmintrin.h>
#endif

#include <private/qdrawhelper_sse_p.h>

QT_BEGIN_NAMESPACE

#ifndef _MM_SHUFFLE
#define _MM_SHUFFLE(fp3,fp2,fp1,fp0) \
 (((fp3) << 6) | ((fp2) << 4) | ((fp1) << 2) | (fp0))
#endif

struct QIWMMXTIntrinsics : public QMMXCommonIntrinsics
{
    static inline m64 alpha(m64 x) {
        return _mm_shuffle_pi16 (x, _MM_SHUFFLE(3, 3, 3, 3));
    }

    static inline m64 _load_alpha(uint x, const m64 &mmx_0x0000) {
        m64 t = _mm_unpacklo_pi8(_mm_cvtsi32_si64(x), mmx_0x0000);
        return _mm_shuffle_pi16(t, _MM_SHUFFLE(0, 0, 0, 0));
    }

    static inline void end() {
    }
};

CompositionFunctionSolid qt_functionForModeSolid_IWMMXT[numCompositionFunctions] = {
    comp_func_solid_SourceOver<QIWMMXTIntrinsics>,
    comp_func_solid_DestinationOver<QIWMMXTIntrinsics>,
    comp_func_solid_Clear<QIWMMXTIntrinsics>,
    comp_func_solid_Source<QIWMMXTIntrinsics>,
    0,
    comp_func_solid_SourceIn<QIWMMXTIntrinsics>,
    comp_func_solid_DestinationIn<QIWMMXTIntrinsics>,
    comp_func_solid_SourceOut<QIWMMXTIntrinsics>,
    comp_func_solid_DestinationOut<QIWMMXTIntrinsics>,
    comp_func_solid_SourceAtop<QIWMMXTIntrinsics>,
    comp_func_solid_DestinationAtop<QIWMMXTIntrinsics>,
    comp_func_solid_XOR<QIWMMXTIntrinsics>
};

CompositionFunction qt_functionForMode_IWMMXT[] = {
    comp_func_SourceOver<QIWMMXTIntrinsics>,
    comp_func_DestinationOver<QIWMMXTIntrinsics>,
    comp_func_Clear<QIWMMXTIntrinsics>,
    comp_func_Source<QIWMMXTIntrinsics>,
    0,
    comp_func_SourceIn<QIWMMXTIntrinsics>,
    comp_func_DestinationIn<QIWMMXTIntrinsics>,
    comp_func_SourceOut<QIWMMXTIntrinsics>,
    comp_func_DestinationOut<QIWMMXTIntrinsics>,
    comp_func_SourceAtop<QIWMMXTIntrinsics>,
    comp_func_DestinationAtop<QIWMMXTIntrinsics>,
    comp_func_XOR<QIWMMXTIntrinsics>
};

void qt_blend_color_argb_iwmmxt(int count, const QSpan *spans, void *userData)
{
    qt_blend_color_argb_x86<QIWMMXTIntrinsics>(count, spans, userData,
                                               (CompositionFunctionSolid*)qt_functionForModeSolid_IWMMXT);
}

#endif // QT_HAVE_IWMMXT

QT_END_NAMESPACE
