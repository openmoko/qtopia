/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
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

#ifndef _DESKTOPPOWERALERTER_H_
#define _DESKTOPPOWERALERTER_H_

#include <QMessageBox>

class DesktopPowerAlerter : public QMessageBox
{
    Q_OBJECT
public:
    DesktopPowerAlerter( QWidget *parent );
    void alert( const QString &text, int priority );
    void hideEvent( QHideEvent * );
private slots:
    void enableAlert();
private:
    QTimer *blockTimer;
    int lastBlockedPriority;
    int currentPriority;
    int alertCount;
};


#endif // _DESKTOPPOWERALERTER_H_

