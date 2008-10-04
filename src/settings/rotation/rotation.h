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

#ifndef ROTATION_H
#define ROTATION_H

#include <QDialog>
#include "ui_rotationsettingsbase.h"

class RotationSettings : public QDialog, private Ui::RotationSettingsBase
{
    Q_OBJECT

public:
    RotationSettings( QWidget* parent = 0, Qt::WFlags fl = 0 );
    ~RotationSettings();
protected:
    void reject();
    void done( int r );
    int getCurrentOrientation();
    int getDefaultOrientation();
    
private slots:
    void applyRotation();
    void reset();
    void setRotation();
    
private:
    int changeRotTo;
    void setRotation( int r );
    void keyPressEvent(QKeyEvent * event);
    int transformDirKey(int key);

};

#endif
