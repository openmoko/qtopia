/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
** 
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
** 
** A copy of the GNU GPL license version 2 is included in this package as 
** LICENSE.GPL.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
** See the GNU General Public License for more details.
**
** In addition, as a special exception Trolltech gives permission to link
** the code of this program with Qtopia applications copyrighted, developed
** and distributed by Trolltech under the terms of the Qtopia Personal Use
** License Agreement. You must comply with the GNU General Public License
** in all respects for all of the code used other than the applications
** licensed under the Qtopia Personal Use License Agreement. If you modify
** this file, you may extend this exception to your version of the file,
** but you are not obligated to do so. If you do not wish to do so, delete
** this exception statement from your version.
** 
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef FIELDMAPDIALOG_H
#define FIELDMAPDIALOG_H

#include <qtopia/qpeglobal.h>
#include <qmap.h>
#include <qstringlist.h>
#include <qwidget.h>

class QLabel;
class QListBox;
class QListBoxItem;
class QPushButton;

class QTOPIA_EXPORT FieldMap : public QWidget
{
    Q_OBJECT

public:
    FieldMap(QWidget *parent = 0, const char* name = 0);

    void setFields( const QMap<int,QString> &, const QValueList<int> &);
    QValueList<int> fields();

protected slots:
    void addClicked();
    void removeClicked();

    void moveUp();
    void moveDown();

    void enableButtons();
private:
    void init();
    bool containsSelection(QListBox *b);

private:
    QPushButton* addButton;
    QListBox* displayBox;
    QListBox* fieldBox;
    QPushButton* removeButton;
    QPushButton* moveUpButton;
    QPushButton* moveDownButton;

    QMap<QString,int> keys;
};

#endif

