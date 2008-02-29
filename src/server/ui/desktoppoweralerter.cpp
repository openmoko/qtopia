/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
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

#include "desktoppoweralerter.h"
#include <QTimer>
#include <limits.h>

/*!
  \class DesktopPowerAlerter
  \ingroup QtopiaServer
  \brief The DesktopPowerAlerter class displays a message when power is low.
  \internal

  Priority is defined as the number of alerts that are needed to pop-up
an alert.
 */
DesktopPowerAlerter::DesktopPowerAlerter( QWidget *parent )
: QMessageBox( tr("Battery Status"), tr("Low Battery"),
               QMessageBox::Critical,
               QMessageBox::Ok | QMessageBox::Default,
               QMessageBox::NoButton, QMessageBox::NoButton,
               parent )
{
    currentPriority = lastBlockedPriority = INT_MAX;
    alertCount = 0;
    blockTimer = new QTimer( this );
    blockTimer->setSingleShot(true);
    connect(blockTimer, SIGNAL( timeout() ), this, SLOT( enableAlert() ));
}

void DesktopPowerAlerter::alert( const QString &text, int priority )
{
    if (blockTimer->isActive() && priority >= lastBlockedPriority)
        return;
    alertCount++;
    if ( alertCount < priority )
        return;
    if ( priority > currentPriority )
        return;
    currentPriority = priority;
    setText( "<qt>" + text + "</qt>" );
    show();
    raise();
}


void DesktopPowerAlerter::hideEvent( QHideEvent *e )
{
    lastBlockedPriority = currentPriority;
    blockTimer->start(5 * 60 * 1000);
    QMessageBox::hideEvent( e );
    alertCount = 0;
    currentPriority = INT_MAX;
}

void DesktopPowerAlerter::enableAlert()
{
    lastBlockedPriority = INT_MAX;
}

