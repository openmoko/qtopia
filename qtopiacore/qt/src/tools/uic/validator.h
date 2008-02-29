/****************************************************************************
**
** Copyright (C) 1992-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef VALIDATOR_H
#define VALIDATOR_H

#include "treewalker.h"

class QTextStream;
class Driver;
class Uic;

struct Option;

struct Validator : public TreeWalker
{
    Validator(Uic *uic);

    void acceptUI(DomUI *node);
    void acceptWidget(DomWidget *node);

    void acceptLayoutItem(DomLayoutItem *node);
    void acceptLayout(DomLayout *node);

    void acceptActionGroup(DomActionGroup *node);
    void acceptAction(DomAction *node);

private:
    Driver *driver;
    QTextStream &output;
    const Option &option;
    Uic *uic;
};

#endif // VALIDATOR_H
