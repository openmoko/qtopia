/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the Qt Designer of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "defs.h"

namespace qdesigner_internal {

int size_type_to_int( QSizePolicy::Policy t )
{
    if ( t == QSizePolicy::Fixed )
	return 0;
    if ( t == QSizePolicy::Minimum )
	return 1;
    if ( t == QSizePolicy::Maximum )
	return 2;
    if ( t == QSizePolicy::Preferred )
	return 3;
    if ( t == QSizePolicy::MinimumExpanding )
	return 4;
    if ( t == QSizePolicy::Expanding )
	return 5;
    if ( t == QSizePolicy::Ignored )
	return 6;
    return 0;
}

QString size_type_to_string( QSizePolicy::Policy t )
{
    if ( t == QSizePolicy::Fixed )
	return QString::fromUtf8("Fixed");
    if ( t == QSizePolicy::Minimum )
	return QString::fromUtf8("Minimum");
    if ( t == QSizePolicy::Maximum )
	return QString::fromUtf8("Maximum");
    if ( t == QSizePolicy::Preferred )
	return QString::fromUtf8("Preferred");
    if ( t == QSizePolicy::MinimumExpanding )
	return QString::fromUtf8("MinimumExpanding");
    if ( t == QSizePolicy::Expanding )
	return QString::fromUtf8("Expanding");
    if ( t == QSizePolicy::Ignored )
	return QString::fromUtf8("Ignored");
    return QString();
}

QSizePolicy::Policy int_to_size_type( int i )
{
    if ( i == 0 )
	return QSizePolicy::Fixed;
    if ( i == 1 )
	return QSizePolicy::Minimum;
    if ( i == 2 )
	return QSizePolicy::Maximum;
    if ( i == 3 )
	return QSizePolicy::Preferred;
    if ( i == 4 )
	return QSizePolicy::MinimumExpanding;
    if ( i == 5 )
	return QSizePolicy::Expanding;
    if ( i == 6 )
	return QSizePolicy::Ignored;
    return QSizePolicy::Preferred;
}

}  // namespace qdesigner_internal
