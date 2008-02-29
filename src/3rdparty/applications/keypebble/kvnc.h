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
// -*- c++ -*-

#ifndef KVNC_H
#define KVNC_H

#include <qmainwindow.h>
#include <qurl.h>

class QAction;
class KRFBCanvas;
class QPushButton;
class QToolBar;

/**
 * Top level window for Keystone.
 *
 * @author Richard Moore, rich@kde.org
 * @version $Id: kvnc.h,v 1.1 2001/09/07 02:15:52 martinj Exp $
 */
class KVNC : public QMainWindow
{
  Q_OBJECT
public:
  KVNC( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );  
  ~KVNC();

public slots:
  void toggleFullScreen(bool);
  void openURL( const QUrl & );
  void connectToServer();
  void showOptions();

protected:
  void setupActions();

protected slots:
  void showMenu();
  void connected();
  void loggedIn();
  void disconnected();
  void statusMessage( const QString & );
  void error( const QString & );
  void zoom( bool );

private:
  bool fullscreen;
  bool isConnected;
  KRFBCanvas *canvas;
  QPopupMenu *cornerMenu;
  QPushButton *cornerButton;
  QAction *fullScreenAction;
  QAction *optionsAction;
  QAction *connectAction;
  QAction *zoomAction;
};

#endif // KVNC_H
