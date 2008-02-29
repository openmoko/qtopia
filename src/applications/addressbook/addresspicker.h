/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
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
#ifndef AddressPicker_H
#define AddressPicker_H

#include <qdialog.h>

class AbTable;

class AddressPicker : public QDialog {
public:
    AddressPicker(AbTable* table, QWidget* parent, const char* name=0, bool modal=false);

    void setChoiceNames(const QStringList&);
    void setSelection(int index, const QStringList&);
    QStringList selection(int index) const;

private:
    AbTable* table;
};

#endif
