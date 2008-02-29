/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the QtNetwork module of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://trolltech.com/products/qt/licenses/licensing/opensource/
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** In addition, as a special exception, Trolltech gives you certain
** additional rights. These rights are described in the Trolltech GPL
** Exception version 1.0, which can be found at
** http://www.trolltech.com/products/qt/gplexception/ and in the file
** GPL_EXCEPTION.txt in this package.
**
** In addition, as a special exception, Trolltech, as the sole copyright
** holder for Qt Designer, grants users of the Qt/Eclipse Integration
** plug-in the right for the Qt/Eclipse Integration to link to
** functionality provided by Qt Designer and its related libraries.
**
** Trolltech reserves all rights not expressly granted herein.
** 
** Trolltech ASA (c) 2007
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

/****************************************************************************
**
** In addition, as a special exception, Trolltech gives permission to link
** the code of its release of Qt with the OpenSSL project's "OpenSSL" library
** (or modified versions of the "OpenSSL" library that use the same license
** as the original version), and distribute the linked executables.
**
** You must comply with the GNU General Public License version 2 in all
** respects for all of the code used other than the "OpenSSL" code.  If you
** modify this file, you may extend this exception to your version of the file,
** but you are not obligated to do so.  If you do not wish to do so, delete
** this exception statement from your version of this file.
**
****************************************************************************/

#ifndef QSSLCERTIFICATE_P_H
#define QSSLCERTIFICATE_P_H

#include "qsslcertificate.h"

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of the QLibrary class.  This header file may change from
// version to version without notice, or even be removed.
//
// We mean it.
//

#include "qsslsocket_p.h"
#include <QtCore/qdatetime.h>
#include <QtCore/qmap.h>

#include <openssl/x509.h>

class QSslCertificatePrivate
{
public:
    QSslCertificatePrivate()
        : null(true), x509(0)
    { 
        QSslSocketPrivate::ensureInitialized();
        ref = 1;
    }

    ~QSslCertificatePrivate()
    {
        if (x509)
            q_X509_free(x509);
    }
    
    bool null;
    QByteArray versionString;
    QByteArray serialNumberString;

    QMap<QString, QString> issuerInfo;
    QMap<QString, QString> subjectInfo;
    QDateTime notValidAfter;
    QDateTime notValidBefore;

    X509 *x509;

    void init(const QByteArray &data, QSsl::EncodingFormat format);

    static QByteArray QByteArray_from_X509(X509 *x509, QSsl::EncodingFormat format);
    static QSslCertificate QSslCertificate_from_X509(X509 *x509);
    static QList<QSslCertificate> certificatesFromPem(const QByteArray &pem, int count = -1);
    static QList<QSslCertificate> certificatesFromDer(const QByteArray &der, int count = -1);

    friend class QSslSocketBackendPrivate;

    QAtomic ref;
};

#endif
