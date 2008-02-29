/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** Licensees holding valid Qtopia Developer license may use this
** file in accordance with the Qtopia Developer License Agreement
** provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING
** THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
** PURPOSE.
**
** email sales@trolltech.com for information about Qtopia License
** Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef QUICKEXEC_H
#define QUICKEXEC_H

#define HAVE_QUICKEXEC
/*
  A special version of execl that opens a shared library and executes the main() function in
  the lib. Requires the quickexec daemon to run.
  
  Note: You do not need to call fork() before calling quickexec, the function actually returns.

  WARNING: path has to be a shared library, otherwise quickexec will hang 
*/

extern int quickexecl( const char *path, const char *, ...);
extern int quickexecv( const char *path, const char *argv[] );

#endif
