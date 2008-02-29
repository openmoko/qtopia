/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
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

