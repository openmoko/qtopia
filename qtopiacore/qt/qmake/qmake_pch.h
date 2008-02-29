/****************************************************************************
**
** Copyright (C) 2004-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
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
