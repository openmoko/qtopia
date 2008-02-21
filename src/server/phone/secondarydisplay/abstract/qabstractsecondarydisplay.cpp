/****************************************************************************
**
** Copyright (C) 2008-2008 TROLLTECH ASA. All rights reserved.
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

#include "qabstractsecondarydisplay.h"

/*!
  \class QAbstractSecondaryDisplay
  \brief The QAbstractSecondaryDisplay class allows developers to replace the "secondary screen" portion of the Phone UI.


  The QAbstractSecondaryDisplay interface is part of is part of the 
  \l {QtopiaServerApplication#qtopia-server-widgets}{server widgets framework}
  and represents the portion of the phone UI that is shown on a smaller
  utility screen such as on clam shell phones.
  
  A small tutorial on how to develop new server widgets using one of the abstract widgets as base can
  be found in QAbstractServerInterface class documentation.
  
  This class is part of the Qtopia server and cannot be used by other Qtopia applications.

  \ingroup QtopiaServer::PhoneUI::TTSmartPhone

  */

/*! \fn QAbstractSecondaryDisplay::QAbstractSecondaryDisplay(QWidget *parent, Qt::WFlags flags)

  Construct a new QAbstractSecondaryDisplay with the specified \a parent and
  widget \a flags.
 */

/*!
  \fn void QAbstractSecondaryDisplay::applyBackgroundImage()

  Applies a background image to the secondary display
  */

/*!
  \fn void QAbstractSecondaryDisplay::updateBackground()

   Persist the new background, after applying it.
*/
