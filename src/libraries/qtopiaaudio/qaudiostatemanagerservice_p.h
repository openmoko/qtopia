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

#ifndef __QAUDIOSTATEMANAGERSERVICE_P_H__
#define __QAUDIOSTATEMANAGERSERVICE_P_H__

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qtopia API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <qtopiaglobal.h>
#include <QtopiaIpcAdaptor>

class QAudioStateManager;
class QAudioStateManagerServicePrivate;

class QAudioStateManagerService : public QtopiaIpcAdaptor
{
    Q_OBJECT

public:
    QAudioStateManagerService(QAudioStateManager *parent);
    ~QAudioStateManagerService();

public slots:
     void setProfile(const QByteArray &profile);
     void setDomain(const QByteArray &domain, int capability);

signals:
    void profileFailed(const QByteArray &profile);
    void domainFailed(const QByteArray &domain, int capability);

private:
    QAudioStateManagerServicePrivate *m_data;
};

#endif
