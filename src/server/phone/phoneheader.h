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

#ifndef _PHONEHEADER_H_
#define _PHONEHEADER_H_

#include <themedview.h>
#include <QString>
#include "phonethemeview.h"
class InputMethods;
class QRect;
class PhoneHeader : public PhoneThemedView
{
    Q_OBJECT
public:
    PhoneHeader(QWidget *parent = 0);

    QSize reservedSize() const;

private slots:
    void updateIM();

private:
    virtual void themeLoaded(const QString &theme);
    InputMethods *inputMethods;
    QString title;
};

#endif
