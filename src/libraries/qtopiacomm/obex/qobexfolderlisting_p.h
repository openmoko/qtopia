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

#ifndef __QOBEXFOLDERLISTING_H__
#define __QOBEXFOLDERLISTING_H__

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

#include <QXmlDefaultHandler>
#include <QObject>

#include "qobexfolderlistingentryinfo.h"

class QObexFolderListingHandler : public QObject, public QXmlDefaultHandler
{
    Q_OBJECT

public:
    QObexFolderListingHandler();

    bool startElement(const QString &namespaceURI,
                      const QString &localName,
                      const QString &qName,
                      const QXmlAttributes &attributes);

    bool endElement(const QString &namespaceURI,
                    const QString &localName,
                    const QString &qName);

    bool characters(const QString &str);
    bool fatalError(const QXmlParseException &exception);
    bool warning(const QXmlParseException &exception);
    bool error(const QXmlParseException &exception);

signals:
    void info(const QObexFolderListingEntryInfo &info);

private:
    QObexFolderListingEntryInfo m_info;
    bool m_valid_elem;
};

#endif
