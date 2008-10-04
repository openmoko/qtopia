/****************************************************************************
**
** This file is part of the Qt Extended Opensource Package.
**
** Copyright (C) 2008 Trolltech ASA.
**
** Contact: Qt Extended Information (info@qtextended.org)
**
** This file may be used under the terms of the GNU General Public License
** version 2.0 as published by the Free Software Foundation and appearing
** in the file LICENSE.GPL included in the packaging of this file.
**
** Please review the following information to ensure GNU General Public
** Licensing requirements will be met:
**     http://www.fsf.org/licensing/licenses/info/GPLv2.html.
**
**
****************************************************************************/

#ifndef CONFIGURE_H
#define CONFIGURE_H

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
