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

#ifndef QIMPENMAINWINDOW_H
#define QIMPENMAINWINDOW_H

#include <QDialog>
#include <QList>
#include <qtopia/mstroke/profile.h>

class QIMPenProfile;
class CharSetDlg;
class QDialog;
class GeneralPref;
class QTreeWidget;
class QTreeWidgetItem;

class QIMPenProfileEdit : public QDialog
{
    Q_OBJECT
public:
    QIMPenProfileEdit(QWidget *parent, Qt::WFlags f = 0);
    ~QIMPenProfileEdit();

private slots:
    void editItem(QTreeWidgetItem *);

private:
    bool loadProfiles();
    bool saveProfiles();

    QList<QIMPenProfile *> profileList;

    QTreeWidget *lv;

    CharSetDlg *cdiag;
    QDialog *gdiag;
    GeneralPref *gpb;
};

#endif
