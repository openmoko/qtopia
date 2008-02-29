/****************************************************************************
** $Id: qt/examples/listbox/listbox.h   2.3.12   edited 2005-10-27 $
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#ifndef LISTBOX_H
#define LISTBOX_H


class QSpinBox;
class QListBox;
class QButtonGroup;

#include <qwidget.h>


class ListBoxDemo: public QWidget
{
    Q_OBJECT
public:
    ListBoxDemo();
    ~ListBoxDemo();

private slots:
    void setNumRows();
    void setNumCols();
    void setRowsByHeight();
    void setColsByWidth();
    void setVariableWidth( bool );
    void setVariableHeight( bool );
    void setMultiSelection( bool );
    void sortAscending();
    void sortDescending();
    
private:
    QListBox * l;
    QSpinBox * columns;
    QSpinBox * rows;
    QButtonGroup * bg;
};


#endif
