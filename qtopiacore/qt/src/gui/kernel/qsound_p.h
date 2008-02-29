/****************************************************************************
**
** Copyright (C) 1992-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QSOUND_P_H
#define QSOUND_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of other Qt classes.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "QtCore/qobject.h"

#ifndef QT_NO_SOUND

class QSound;
/*
  QAuServer is an INTERNAL class.  If you wish to provide support for
  additional audio servers, you can make a subclass of QAuServer to do
  so, HOWEVER, your class may need to be re-engineered to some degree
  with each new Qt release, including minor releases.

  QAuBucket is whatever you want.
*/

class QAuBucket {
public:
    virtual ~QAuBucket();
};

class QAuServer : public QObject {
    Q_OBJECT

public:
    explicit QAuServer(QObject* parent);
    ~QAuServer();

    virtual void init(QSound*);
    virtual void play(const QString& filename);
    virtual void play(QSound*)=0;
    virtual void stop(QSound*)=0;
    virtual bool okay()=0;

protected:
    void setBucket(QSound*, QAuBucket*);
    QAuBucket* bucket(QSound*);
    int decLoop(QSound*);
};

#endif // QT_NO_SOUND

#endif // QSOUND_P_H
