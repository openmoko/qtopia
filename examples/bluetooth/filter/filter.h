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

#ifndef __FILTER_H__
#define __FILTER_H__

#include <QWidget>

class QComboBox;
class QBluetoothRemoteDeviceDialogFilter;

class Filter : public QWidget
{
    Q_OBJECT

public:
    Filter(QWidget *parent = 0, Qt::WFlags f = 0);
    ~Filter();

private slots:
    void showDialog();

private:
    QBluetoothRemoteDeviceDialogFilter *greenphoneFilter;
    QComboBox *filterChoice;
    QComboBox *serviceChoice;
};

#endif
