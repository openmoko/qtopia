/****************************************************************************
**
** Copyright (C) 1992-2008 Trolltech ASA. All rights reserved.
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License versions 2.0 or 3.0 as published by the Free Software
** Foundation and appearing in the files LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file.  Alternatively you may (at
** your option) use any later version of the GNU General Public
** License if such license has been publicly approved by Trolltech ASA
** (or its successors, if any) and the KDE Free Qt Foundation. In
** addition, as a special exception, Trolltech gives you certain
** additional rights. These rights are described in the Trolltech GPL
** Exception version 1.1, which can be found at
** http://www.trolltech.com/products/qt/gplexception/ and in the file
** GPL_EXCEPTION.txt in this package.
**
** Please review the following information to ensure GNU General
** Public Licensing requirements will be met:
** http://trolltech.com/products/qt/licenses/licensing/opensource/. If
** you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** In addition, as a special exception, Trolltech, as the sole
** copyright holder for Qt Designer, grants users of the Qt/Eclipse
** Integration plug-in the right for the Qt/Eclipse Integration to
** link to functionality provided by Qt Designer and its related
** libraries.
**
** This file is provided "AS IS" with NO WARRANTY OF ANY KIND,
** INCLUDING THE WARRANTIES OF DESIGN, MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE. Trolltech reserves all rights not expressly
** granted herein.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#if defined(QT_HAVE_MMX)

#include <private/qdrawhelper_mmx_p.h>

const CompositionFunctionSolid qt_functionForModeSolid_MMX[] = {
    comp_func_solid_SourceOver<QMMXIntrinsics>,
    comp_func_solid_DestinationOver<QMMXIntrinsics>,
    comp_func_solid_Clear<QMMXIntrinsics>,
    comp_func_solid_Source<QMMXIntrinsics>,
    0,
    comp_func_solid_SourceIn<QMMXIntrinsics>,
    comp_func_solid_DestinationIn<QMMXIntrinsics>,
    comp_func_solid_SourceOut<QMMXIntrinsics>,
    comp_func_solid_DestinationOut<QMMXIntrinsics>,
    comp_func_solid_SourceAtop<QMMXIntrinsics>,
    comp_func_solid_DestinationAtop<QMMXIntrinsics>,
    comp_func_solid_XOR<QMMXIntrinsics>,
    comp_func_solid_Plus,
    comp_func_solid_Multiply,
    comp_func_solid_Screen,
    comp_func_solid_Overlay,
    comp_func_solid_Darken,
    comp_func_solid_Lighten,
    comp_func_solid_ColorDodge,
    comp_func_solid_ColorBurn,
    comp_func_solid_HardLight,
    comp_func_solid_SoftLight,
    comp_func_solid_Difference,
    comp_func_solid_Exclusion
};

const CompositionFunction qt_functionForMode_MMX[] = {
    comp_func_SourceOver<QMMXIntrinsics>,
    comp_func_DestinationOver<QMMXIntrinsics>,
    comp_func_Clear<QMMXIntrinsics>,
    comp_func_Source<QMMXIntrinsics>,
    0,
    comp_func_SourceIn<QMMXIntrinsics>,
    comp_func_DestinationIn<QMMXIntrinsics>,
    comp_func_SourceOut<QMMXIntrinsics>,
    comp_func_DestinationOut<QMMXIntrinsics>,
    comp_func_SourceAtop<QMMXIntrinsics>,
    comp_func_DestinationAtop<QMMXIntrinsics>,
    comp_func_XOR<QMMXIntrinsics>,
    comp_func_Plus,
    comp_func_Multiply,
    comp_func_Screen,
    comp_func_Overlay,
    comp_func_Darken,
    comp_func_Lighten,
    comp_func_ColorDodge,
    comp_func_ColorBurn,
    comp_func_HardLight,
    comp_func_SoftLight,
    comp_func_Difference,
    comp_func_Exclusion
};

void qt_blend_color_argb_mmx(int count, const QSpan *spans, void *userData)
{
    qt_blend_color_argb_x86<QMMXIntrinsics>(count, spans, userData,
                                            (CompositionFunctionSolid*)qt_functionForModeSolid_MMX);
}

#endif // QT_HAVE_MMX
