/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#ifndef HEXKEY_VALIDATOR
#define HEXKEY_VALIDATOR

#include <QValidator>
#include <QWidget>
#include <qtopiaglobal.h>

class QTOPIACOMM_EXPORT HexKeyValidator : public QValidator {
public:
    explicit HexKeyValidator( QWidget* parent = 0, int numDigits = 0);
    ~HexKeyValidator() {};

    QValidator::State validate( QString& key, int& curs ) const;
private:
    const int neededNumDigits;
};

#endif //HEXKEY_VALIDATOR
