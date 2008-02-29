/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** Licensees holding valid Qtopia Developer license may use this
** file in accordance with the Qtopia Developer License Agreement
** provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING
** THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
** PURPOSE.
**
** email sales@trolltech.com for information about Qtopia License
** Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef CALCULATOR_DATA_H
#define CALCULATOR_DATA_H

#include <qstring.h>

class Data {
public:
    Data(){};
    virtual ~Data(){};

    virtual void push(char){};
    virtual void del(){};
    virtual void clear(){formattedOutput.truncate(0);formattedOutput.append("0");};

    virtual QString getType(){return QString("NONE");};
    virtual QString getFormattedOutput(){return formattedOutput;};
protected:
    QString formattedOutput;
};

#endif
