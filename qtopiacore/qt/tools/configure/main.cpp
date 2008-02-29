/****************************************************************************
**
** Copyright (C) 1992-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

/*
** Configure tool
**
*/

#include "configureapp.h"

int main( int argc, char** argv )
{
    Configure app( argc, argv );
    if (!app.isOk())
        return 3;

    app.parseCmdLine();
#if !defined(EVAL)
    app.validateArgs();
#endif
    if( app.displayHelp() )
	return 1;

    // Read license now, and exit if it doesn't pass.
    // This lets the user see the command-line options of configure
    // without having to load and parse the license file.
    app.readLicense();
    if (!app.isOk())
        return 3;

    // Auto-detect modules and settings.
    app.autoDetection();

    // After reading all command-line arguments, and doing all the
    // auto-detection, it's time to do some last minute validation.
    // If the validation fails, we cannot continue.
    if (!app.verifyConfiguration())
        return 3;

    app.generateOutputVars();

#if !defined(EVAL)
    if( !app.isDone() )
	app.generateCachefile();
    if( !app.isDone() )
        app.generateBuildKey();
    if( !app.isDone() )
	app.generateConfigfiles();
    if( !app.isDone() )
	app.displayConfig();
    if( !app.isDone() )
	app.buildQmake();
    if( !app.isOk() )
	return 2;
#endif
    if( !app.isDone() )
	app.generateMakefiles();
    if( !app.isDone() )
	app.showSummary();

    return 0;
}
