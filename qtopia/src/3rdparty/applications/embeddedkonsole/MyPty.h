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
/* [MyPty.h]                 Pseudo Terminal Device                           */
/*                                                                            */
/* -------------------------------------------------------------------------- */
/*                                                                            */
/* Copyright (c) 1997,1998 by Lars Doelle <lars.doelle@on-line.de>            */
/*                                                                            */
/* This file is part of Konsole - an X terminal for KDE                       */
/*                                                                            */
/* -------------------------------------------------------------------------- */
/*									      */
/* Konsole is ported to Qt/Embedded by Trolltech                              */
/*									      */
/* -------------------------------------------------------------------------- */

/* \file
*/

#ifndef MY_PTY_H
#define MY_PTY_H

#include <qglobal.h>
#include <qobject.h>
#include <qstrlist.h>
#if defined (_OS_WIN32_)
#include "../../../server/qprocess.h"
#include <qarray.h>
#endif


class MyPty : public QObject
{
Q_OBJECT

  public:

    MyPty();
    ~MyPty();

    /*
        having a `run' separate from the constructor makes it possible to create        the necessary connections to the signals and slots of the
        instance before starting the execution of the client.
    */
    int run(const char* pgm, QStrList & args, const char* term, int addutmp);

  public slots:

    void send_bytes(const char* s, int len);
    void setSize(int lines, int columns);
    void error();

  signals:

    /*
        emitted when the client program terminates.
        \param status the wait(2) status code of the terminated client program.
    */
    void done(int status);

    /*
        emitted when a new block of data is received.
        \param s - the data
        \param len - the length of the block
    */
    void block_in(const char* s, int len);

  public:

    void send_byte(char s);
//    void send_string(const char* s);

    const char* deviceName();

  protected slots:
      void readPty();
      void donePty();

  private:
    int  openPty();

  private:
#if !defined (_OS_WIN32_)
    char ptynam[16]; // "/dev/ptyxx" | "/dev/ptmx"
    char ttynam[16]; // "/dev/ttyxx" | "/dev/pts/########..."
    int fd;
    int cpid;
#else
    QProcess terminalProcess;
    QByteArray stdInBuffer;
#endif
};

#endif
