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

#ifndef __QAUDIOSTATEINFO_H__
#define __QAUDIOSTATEINFO_H__

#include <qtopiaglobal.h>
#include <qglobal.h>

class QAudioStateInfoPrivate;

class QTOPIAAUDIO_EXPORT QAudioStateInfo
{
public:
    QAudioStateInfo(const QByteArray &domain, const QByteArray &profile,
                    const QString &displayName, int priority);
    QAudioStateInfo();
    QAudioStateInfo(const QAudioStateInfo &state);
    ~QAudioStateInfo();

    bool isValid() const;

    QAudioStateInfo &operator=(const QAudioStateInfo &state);

    bool operator==(const QAudioStateInfo &state) const;
    inline bool operator!=(const QAudioStateInfo &state) const { return !operator==(state); }

    QByteArray domain() const;
    void setDomain(const QByteArray &domain);

    QByteArray profile() const;
    void setProfile(const QByteArray &profile);

    QString displayName() const;
    void setDisplayName(const QString &displayName);

    int priority() const;
    void setPriority(int priority);

private:
    QAudioStateInfoPrivate *m_data;
};

QTOPIAMEDIA_EXPORT uint qHash(const QAudioStateInfo &key);
QTOPIAMEDIA_EXPORT QDebug operator<<(QDebug, const QAudioStateInfo &);

#endif
