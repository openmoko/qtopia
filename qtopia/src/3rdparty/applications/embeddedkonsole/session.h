/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS and its licensors.
** All rights reserved.
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
** See below for additional copyright and license information
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
/* -------------------------------------------------------------------------- */
/*                                                                            */
/* [session.h]              Testbed for TE framework                          */
/*                                                                            */
/* -------------------------------------------------------------------------- */
/*                                                                            */
/* Copyright (c) 1997,1998 by Lars Doelle <lars.doelle@on-line.de>            */
/*                                                                            */
/* This file is part of Konsole, an X terminal.                               */
/*                                                                            */
/* -------------------------------------------------------------------------- */
/*									      */
/* Konsole ported to Qt/Embedded by Trolltech                                 */
/*									      */
/* -------------------------------------------------------------------------- */

#ifndef SESSION_H
#define SESSION_H

#include <qapplication.h>
#include <qmainwindow.h>
#include <qstrlist.h>

#include "MyPty.h"
#include "TEWidget.h"
#include "TEmuVt102.h"

class TESession : public QObject
{ Q_OBJECT

public:

  TESession(QMainWindow* main, TEWidget* w,
            const char* pgm, QStrList & _args,
	    const char* term);
  ~TESession();

public:

  void        setConnect(bool r);
  TEmulation* getEmulation();      // to control emulation
  bool        isSecure();

public:

  int schemaNo();
  int fontNo();
  const char* emuName();
  const QString& Title();
  bool history();
  int keymap();

  void setHistory(bool on);
  void setSchemaNo(int sn);
  void setKeymapNo(int kn);
  void setFontNo(int fn);
  void setTitle(const QString& title);
  void kill(int signal);

public slots:

  void run();
  void done(int status);
  void terminate();

signals:

  void done(TESession*, int);

private:

  // TEPty*         sh;
  MyPty*         sh;
  TEWidget*      te;
  TEmulation*    em;

  //FIXME: using the indices here
  // is propably very bad. We should
  // use a persistent reference instead.
  int            schema_no;
  int            font_no;
  int            keymap_no;
  QString        title;

  const char*    pgm;
  QStrList       args;

  QCString       term;
};

#endif
