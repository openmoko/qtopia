/****************************************************************************
**
** Copyright (C) 1992-2008 Trolltech ASA. All rights reserved.
**
** This file is part of the tools applications of the Qt Toolkit.
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

/*
  command.cpp
*/

#include <QProcess>

#include "command.h"

void executeCommand( const Location& location, const QString& format,
		     const QStringList& args )
{
    QString actualCommand;
    for ( int i = 0; i < (int) format.length(); i++ ) {
	int ch = format[i].unicode();
	if ( ch > 0 && ch < 8 ) {
	    actualCommand += args[ch - 1];
	} else {
	    actualCommand += format[i];
	}
    }

    QString toolName = actualCommand;
    int space = toolName.indexOf( " " );
    if ( space != -1 )
	toolName.truncate( space );

    QProcess process;
    process.start("sh", QStringList() << "-c" << actualCommand );
    process.waitForFinished();

    if (process.exitCode() == 127)
	location.fatal( tr("Couldn't launch the '%1' tool")
			.arg(toolName),
			tr("Make sure the tool is installed and in the"
			   " path.") );

    QString errors = process.readAllStandardError();
    while ( errors.endsWith("\n") )
	errors.truncate( errors.length() - 1 );
    if ( !errors.isEmpty() )
	location.fatal( tr("The '%1' tool encountered some problems")
			.arg(toolName),
			tr("The tool was invoked like this:\n%1\n"
			   "It emitted these errors:\n%2")
			.arg(actualCommand).arg(errors) );
}
