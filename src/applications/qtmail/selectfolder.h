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
#ifndef SELECTFOLDER_H
#define SELECTFOLDER_H

#include <qdialog.h>
#include <qstring.h>
#include <qstringlist.h>

class QListWidget;

class SelectFolderDialog : public QDialog
{
    Q_OBJECT

public:
    SelectFolderDialog(const QStringList list,
                       QWidget *parent = 0);
    virtual ~SelectFolderDialog();
    int folder();
    void getFolders();

private slots:
    void selected();
private:
    QListWidget *mFolderList;
    QStringList mMailboxList;
};

#endif
