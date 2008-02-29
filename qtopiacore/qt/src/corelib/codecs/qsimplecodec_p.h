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

#ifndef QSIMPLECODEC_P_H
#define QSIMPLECODEC_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "QtCore/qtextcodec.h"

#ifndef QT_NO_TEXTCODEC

class QSimpleTextCodec: public QTextCodec
{
public:
    enum { numSimpleCodecs = 30 };
    explicit QSimpleTextCodec(int);
    ~QSimpleTextCodec();

    QString convertToUnicode(const char *, int, ConverterState *) const;
    QByteArray convertFromUnicode(const QChar *, int, ConverterState *) const;

    QByteArray name() const;
    QList<QByteArray> aliases() const;
    int mibEnum() const;

private:
    void buildReverseMap() const;

    int forwardIndex;
#ifndef Q_WS_QWS
    mutable QByteArray reverseMap;
#endif
};

#endif // QT_NO_TEXTCODEC

#endif // QSIMPLECODEC_P_H
