/****************************************************************************
** $Id: qt/examples/rangecontrols/rangecontrols.h   2.3.12   edited 2005-10-27 $
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#ifndef RANGECONTROLS_H
#define RANGECONTROLS_H

#include <qvbox.h>

class QCheckBox;

class RangeControls : public QVBox
{
    Q_OBJECT

public:
    RangeControls( QWidget *parent = 0, const char *name = 0 );

private:
    QCheckBox *notches, *wrapping;
};

#endif
