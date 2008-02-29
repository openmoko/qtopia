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

#ifndef DEVICE_CONFIGURE_H
#define DEVICE_CONFIGURE_H

#include "ui_configurebase.h"

class ConfigureData;

class Configure : public QDialog, public Ui::ConfigureBase
{
    Q_OBJECT
public:
    const ConfigureData &data() { return *mData; }
    static Configure *dialog( QWidget *parent = 0, Qt::WFlags f = 0 );
protected:
    Configure( QWidget *parent = 0, Qt::WFlags f = 0 );
    virtual ~Configure();
private slots:
    void fileDialog();
    void textChanged( const QString & );
private:
    ConfigureData *mData;
};

#endif
