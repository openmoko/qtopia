/****************************************************************************
**
** Copyright(C) 2007-2008 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef NOKIASCREEN_H
#define NOKIASCREEN_H

#include <QLinuxFbScreen>

//class NokiaScreenPrivate;
class NokiaScreen : public QLinuxFbScreen
{
public:
    NokiaScreen(int displayId);
    ~NokiaScreen();

    void exposeRegion(QRegion region, int windowIndex);

/*      bool useOffscreen() { return false; } */
/*      bool initDevice(); */
/*      bool connect(const QString &displaySpec); */
/*      void disconnect(); */
/*      void shutdownDevice(); */
/*      void setMode(int,int,int); */

/*      void save(); */
/*      void restore(); */
/*      void blank(bool on); */
/*      void set(unsigned int,unsigned int,unsigned int,unsigned int); */
/*      uchar * cache(int); */
/*      void uncache(uchar *); */
/*      int sharedRamSize(void *); */
    
private:
      //   NokiaScreenPrivate *d;
        QScreen * screen;    
};

#endif // NOKIASCREEN_H


