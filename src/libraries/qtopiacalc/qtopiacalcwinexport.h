
/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#ifndef QTOPIACALCWINEXP_H
#define QTOPIALCALCWINEXP_H

#ifdef QTOPIA_TEMPLATEDLL
//MOC_SKIP_BEGIN

#if defined (CALCULATOR_SYSTEM_H)
#   include <qstack.h>
#   include <qlist.h>
#   include <qtopia/calc/instruction.h>
    QTOPIA_TEMPLATE_EXTERN template class QTOPIA_EXPORT QStack<InstructionDescription>;
    QTOPIA_TEMPLATE_EXTERN template class QTOPIA_EXPORT QStack<QString>;
    QTOPIA_TEMPLATE_EXTERN template class QTOPIA_EXPORT QStack<Data>;
    QTOPIA_TEMPLATE_EXTERN template class QTOPIA_EXPORT QList<InstructionDescription>;
#endif

//MOC_SKIP_END
#endif

#endif //QTOPIACALCWINEXP_H
