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

#ifndef PINYINIMPL_H
#define PINYINIMPL_H

#include <inputmethodinterface.h>
class PinyinIM;

class PinyinImpl : public QtopiaInputMethod
{

public:
    PinyinImpl(QObject *parent = 0);
    ~PinyinImpl();

    QString name() const;
    QString identifier() const;

    QIcon icon() const;
    QString version() const;

    int properties() const;

    State state() const;

    void reset();

    QWSInputMethod *inputModifier();

    void setHint(const QString &, bool);
private:
    PinyinIM *input;
    QIcon icn;
    ulong ref;
};

#endif
