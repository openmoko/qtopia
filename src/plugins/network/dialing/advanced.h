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

#ifndef ADVANCEDCONFIG_H
#define ADVANCEDCONFIG_H

#include <QWidget>
#include <qtopianetworkinterface.h>

#include "ui_advancedbase.h"

class AdvancedPage : public QWidget
{
    Q_OBJECT
public:
    AdvancedPage( const QtopiaNetworkProperties& cfg, QWidget* parent = 0, Qt::WFlags flags = 0 );
    virtual ~AdvancedPage();

    QtopiaNetworkProperties properties();

protected:
    bool eventFilter( QObject*, QEvent* );
private slots:
    void externalState( int state );

private:
    void init();
    void readConfig( const QtopiaNetworkProperties& prop);
    void selectCombo( QComboBox* combo, const QString& item );

    bool GPRS;

private:
    Ui::AdvancedBase ui;

};
#endif // ADVANCEDCONFIG_H

