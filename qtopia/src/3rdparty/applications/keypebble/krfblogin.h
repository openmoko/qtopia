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

#ifndef KRFBLOGIN_H
#define KRFBLOGIN_H

#include <qobject.h>

class KRFBConnection;

/**
 * Handles logging into the RFB server.
 *
 * @version $Id: krfblogin.h,v 1.1 2001/09/07 02:15:52 martinj Exp $
 * @author Richard Moore, rich@kde.org
 */
class KRFBLogin : QObject
{
  Q_OBJECT

public:
  enum State {
    AwaitingServerVersion,
    AwaitingAuthScheme,
    AwaitingChallenge,
    WaitingForResponse,
    Authenticated,
    Ready,
    Failed,
    Error
  };

  KRFBLogin( KRFBConnection *con );
  ~KRFBLogin();

  KRFBLogin::State state() const;

protected:
  void sendClientVersion();

protected slots:
  void getPassword();
  void gotServerVersion();
  void gotAuthScheme();
  void gotChallenge();
  void gotAuthResult();
  void gotFailureReasonSize();

signals:
  void passwordRequired( KRFBConnection * );
  void error( const QString & );
  void status( const QString & );

private:
  KRFBConnection *con;
  State currentState;
  QCString versionString;
  int serverMajor;
  int serverMinor;
};

#endif // KRFBLOGIN_H

