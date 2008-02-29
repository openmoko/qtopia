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
/* [emulation.h]          Fundamental Terminal Emulation                      */
/*                                                                            */
/* -------------------------------------------------------------------------- */
/*                                                                            */
/* Copyright (c) 1997,1998 by Lars Doelle <lars.doelle@on-line.de>            */
/*                                                                            */
/* This file is part of Konsole - an X terminal for KDE                       */
/*                                                                            */
/* -------------------------------------------------------------------------- */
/*									      */
/* Konsole ported to Qt/Embedded by Trolltech                                 */
/*									      */
/* -------------------------------------------------------------------------- */

#ifndef EMULATION_H
#define EMULATION_H

#include "TEWidget.h"
#include "TEScreen.h"
#include <qtimer.h>
#include <stdio.h>
#include <qtextcodec.h>
#include "keytrans.h"

class TEmulation : public QObject
{ Q_OBJECT

public:

  TEmulation(TEWidget* gui);
  ~TEmulation();

public:
  virtual void setHistory(bool on);
  virtual bool history();

public slots: // signals incoming from TEWidget

  virtual void onImageSizeChange(int lines, int columns);
  virtual void onHistoryCursorChange(int cursor);
  virtual void onKeyPress(QKeyEvent*);
 
  virtual void clearSelection();
  virtual void onSelectionBegin(const int x, const int y);
  virtual void onSelectionExtend(const int x, const int y);
  virtual void setSelection(const BOOL preserve_line_breaks);

public slots: // signals incoming from data source

  void onRcvBlock(const char* txt,int len);

signals:

  void sndBlock(const char* txt,int len);
  void ImageSizeChanged(int lines, int columns);
  void changeColumns(int columns);
  void changeTitle(int arg, const char* str);

public:

  virtual void onRcvChar(int);

  virtual void setMode  (int) = 0;
  virtual void resetMode(int) = 0;

  virtual void sendString(const char*) = 0;

  virtual void setConnect(bool r);
  void setColumns(int columns);

  void setKeytrans(int no);
  void setKeytrans(const char * no);

protected:

  TEWidget* gui;
  TEScreen* scr;         // referes to one `screen'
  TEScreen* screen[2];   // 0 = primary, 1 = alternate
  void setScreen(int n); // set `scr' to `screen[n]'

  bool   connected;    // communicate with widget

  void setCodec(int c); // codec number, 0 = locale, 1=utf8

  QTextCodec* codec;
  QTextCodec* localeCodec;
  QTextDecoder* decoder;

  KeyTrans* keytrans;

// refreshing related material.
// this is localized in the class.
private slots: // triggered by timer

  void showBulk();

private:

  void bulkNewline();
  void bulkStart();
  void bulkEnd();

private:

  QTimer bulk_timer;
  int    bulk_nlcnt;   // bulk newline counter
  char*  SelectedText;
  int    bulk_incnt;   // bulk counter


};

#endif // ifndef EMULATION_H
