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

#ifndef __QTOPIA_MEDIALIBRARY_AUDIODOMAIN_H
#define __QTOPIA_MEDIALIBRARY_AUDIODOMAIN_H


#include <QString>
#include <QList>

#include <qtopiaglobal.h>
#include <qtopiaipcmarshal.h>


enum QAudioCapabilities { Input = 1, Output = 2 };

class QTOPIAMEDIA_EXPORT QAudioDomain
{
public:
    QString     name;
    int         priority;

    // {{{ Serialization
    template <typename Stream> void serialize(Stream &stream) const;
    template <typename Stream> void deserialize(Stream &stream);
    // }}}
};

typedef QList<QAudioDomain> QAudioDomainList;

Q_DECLARE_USER_METATYPE(QAudioDomain);
Q_DECLARE_USER_METATYPE_NO_OPERATORS(QAudioDomainList);

#endif  // __QTOPIA_MEDIALIBRARY_AUDIODOMAIN_H
