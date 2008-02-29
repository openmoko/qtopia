/****************************************************************************
**
** Copyright (C) 2004-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the qmake application of the Qt Toolkit.
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

#ifndef QMAKE_PCH_H
#define QMAKE_PCH_H
#include <qglobal.h>
#ifdef Q_WS_WIN
# define _POSIX_
# include <limits.h>
# undef _POSIX_
#endif

#include <stdio.h>
//#include "makefile.h"
//#include "meta.h"
#include <qfile.h>
//#include "winmakefile.h"
#include <qtextstream.h>
//#include "project.h"
#include <qstring.h>
#include <qstringlist.h>
#include <qhash.h>
#include <time.h>
#include <stdlib.h>
#include <qregexp.h>
//#include <qdir.h>
//#include "option.h"

#endif
