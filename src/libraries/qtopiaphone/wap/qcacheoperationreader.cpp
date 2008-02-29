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

#include <qcacheoperationreader.h>

/*!
    \class QCacheOperationReader
    \mainclass
    \brief The QCacheOperationReader class reads WAP Cache Operation (CO) documents

    The QCacheOperationReader class extends QWbXmlReader to provide support
    for reading documents in the WAP Cache Operation (CO) language.
    This language is defined in the WAP standard
    \l{http://www.openmobilealliance.org/tech/affiliates/wap/wap-175-cacheop-20010731-a.pdf}{wap-175-cacheop-20010731-a.pdf}.

    \ingroup telephony
    \sa QWbXmlReader
*/

static void set(QWbXmlTagSet& s, int i, const char* v)
{
    s[i] = v;
}

/*!
    Construct a WBXML reader and initialize it to process WAP Cache
    Operation (CO) documents.
*/
QCacheOperationReader::QCacheOperationReader()
{
    QWbXmlTagSet tags;
    QWbXmlTagSet attrs;

    set(tags, 0x05, "co");
    set(tags, 0x06, "invalidate-object");
    set(tags, 0x07, "invalidate-service");

    set(attrs, 0x05, "uri");
    set(attrs, 0x06, "uri=http://");
    set(attrs, 0x07, "uri=http://www.");
    set(attrs, 0x08, "uri=https://");
    set(attrs, 0x09, "uri=https://www.");
    set(attrs, 0x85, ".com/");
    set(attrs, 0x86, ".edu/");
    set(attrs, 0x87, ".net/");
    set(attrs, 0x88, ".org/");

    setTagSets( tags, attrs );
}

/*!
    Destruct a WAP Service Loading reader.
*/
QCacheOperationReader::~QCacheOperationReader()
{
    // Nothing to do here.
}
