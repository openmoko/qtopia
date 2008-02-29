/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#ifndef BT_GUI_PAGE_H
#define BT_GUI_PAGE_H

#include <QWidget>
#include <qtopianetworkinterface.h>

#include "ui_advancedbtbase.h"

class AdvancedBTPage : public QWidget
{
    Q_OBJECT
public:
    AdvancedBTPage( const QtopiaNetworkProperties& cfg, QWidget* parent = 0,
            Qt::WFlags flags = 0 );

    QtopiaNetworkProperties properties();
private:
    void init();
    void readConfig( const QtopiaNetworkProperties& prop );

private slots:
    void timeoutSelected( int state );

private:
    Ui::AdvancedBtBase ui;
};

#include "ui_dialingbtbase.h"

class DialingBTPage : public QWidget
{
    Q_OBJECT
public:
    DialingBTPage( const QtopiaNetworkProperties& cfg, QWidget* parent = 0,
            Qt::WFlags flags = 0 );

    QtopiaNetworkProperties properties();
private:
    void init();
    void readConfig( const QtopiaNetworkProperties& prop );

private:
    Ui::DialingBtBase ui;
};

#endif //BT_GUI_PAGE_H
