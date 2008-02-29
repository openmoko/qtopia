/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QTOPIACALCWINEXP_H
#define QTOPIALCALCWINEXP_H

#ifdef QTOPIA_TEMPLATEDLL
//MOC_SKIP_BEGIN

#if defined (CALCULATOR_SYSTEM_H)
#   include <qstack.h>
#   include <qlist.h>
#   include <qtopia/calc/instruction.h>
    QTOPIA_TEMPLATE_EXTERN template class QTOPIACALC_EXPORT QStack<InstructionDescription>;
    QTOPIA_TEMPLATE_EXTERN template class QTOPIACALC_EXPORT QStack<QString>;
    QTOPIA_TEMPLATE_EXTERN template class QTOPIACALC_EXPORT QStack<Data>;
    QTOPIA_TEMPLATE_EXTERN template class QTOPIACALC_EXPORT QList<InstructionDescription>;
#endif

//MOC_SKIP_END
#endif

#endif //QTOPIACALCWINEXP_H
