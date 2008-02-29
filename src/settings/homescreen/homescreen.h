/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
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
#ifndef HOMESCREENSETTINGS_H
#define HOMESCREENSETTINGS_H

#include <QDialog>
#include <QContent>
#include <QPowerStatus>

class QCheckBox;
class QComboBox;
class QLabel;
class QPushButton;
class QValueSpaceItem;

class HomescreenSettings : public QDialog
{
    Q_OBJECT
public:
    HomescreenSettings(QWidget* parent = 0, Qt::WindowFlags fl = 0);
    ~HomescreenSettings() {}
    
protected:
    void accept();
    void reject();
    
protected slots:
    void editPhoto();
    void editSecondaryPhoto();
    void homeScreenActivated(int index = -1);
    void appMessage(const QString&,const QByteArray&);

private:
    QPushButton *image;
    QComboBox *imageMode;
    QPushButton *secondaryImage;
    QComboBox *secondaryImageMode;
    QComboBox *homeScreen;
    QCheckBox *lock;
    QCheckBox *time;
    QCheckBox *date;
    QCheckBox *op;
    QCheckBox *profile;
    QCheckBox *location;
    QLabel* powerNote;
    QPowerStatus powerStatus;
    QValueSpaceItem* screenSaver_vsi;
    
    QContent hsImage;
    QContent secondaryHsImage;
};

#endif
