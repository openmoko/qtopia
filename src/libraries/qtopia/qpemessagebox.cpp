/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
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
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qpemessagebox.h"
#include <qmessagebox.h>

#include <qapplication.h>

/*!
  \class QPEMessageBox qpemessagebox.h
  \brief A message box that provides yes, no and cancel options.

  \ingroup qtopiaemb
 */

/*!
  Displays a QMessageBox with parent \a parent and caption \a caption.
  The message displayed is "Are you sure you want to delete: ",
  followed by \a object.  
 */
bool QPEMessageBox::confirmDelete( QWidget *parent, const QString & caption,
			       const QString & object )
{
    QString msg = "<qt>" + qApp->translate( "QPEMessageBox", "Are you sure you want to delete: %1?").arg( object ) + "</qt>";
    int r = QMessageBox::warning( parent, caption, msg, QMessageBox::Yes,
				  QMessageBox::No|QMessageBox::Default|
				  QMessageBox::Escape, 0 );

    return r == QMessageBox::Yes;
}
