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

#include <qtopiacomm/private/qobexcommon_p.h>
#include <qmimetype.h>
#include <qtopialog.h>

#include <netinet/in.h>

QObexHeader::QObexHeader()
{

}

inline void convertNBOQString(QString &str)
{
    // Convert from network order to host order ...
    for (int i = 0; i < str.length(); i++)
        str[i] = QChar(ntohs(str[i].unicode()));
}

inline void convertHBOQString(QString &str)
{
    // Convert unicode string to network order
    for(int i = 0; i < str.length() - 1; i++)
        str[i] = QChar(htons(str[i].unicode()));
}

void getHeaders( obex_t *self, obex_object_t *object, QObexHeader &header )
{
    uchar hi;
    obex_headerdata_t hv;
    unsigned int hv_size;

    QString name;
    QString mimetype;
    QString desc;

    while(OBEX_ObjectGetNextHeader(self, object, &hi, &hv, &hv_size))   {
        switch ( hi ) {
            case OBEX_HDR_DESCRIPTION:
            {
                desc.setUnicode((const QChar *)hv.bs, hv_size / 2 - 1);
                convertNBOQString(desc);
                break;
            }
            case OBEX_HDR_LENGTH:
                header.setLength(hv.bq4);
                break;

            case OBEX_HDR_NAME:
            {
                name.setUnicode((const QChar*)hv.bs, hv_size / 2 - 1);
                convertNBOQString(name);
                break;
            }
            case OBEX_HDR_TYPE:
                // Grr Qt is weird
                if ( (const char *)hv.bs[hv_size - 1] == '\0' )
                    mimetype = QString::fromLatin1( (const char *) hv.bs ); // Null terminated
                else
                    mimetype = QString::fromLatin1((const char*)hv.bs, hv_size );  // Not

                break;
        }
    }

    int slash = name.indexOf( '/', -1 );
    if ( slash > 0 )
        name = name.mid( slash+1 );

    if ( name.isEmpty() )
        name = "unnamed"; // No tr

    if ( mimetype.isNull() )
        mimetype = QMimeType(name).id();

    for( int i = 0 ; i < (int)name.length() ; ++i) {
        if(name[i] != '.' && (name[i].isPunct() || name[i].isSpace()) ) {
            name[i] = '_'; //ensure it's something valid
        }
    }

    header.setName(name);
    header.setMimeType(mimetype);
    header.setDescription(desc);
}

void setHeaders( obex_t *self, obex_object_t *object, const QObexHeader &header )
{
    obex_headerdata_t hd;

    if (header.length() != 0) {
        qLog(Obex) << "Adding length header";
        /* Add length header */
        hd.bq4 = header.length();
        OBEX_ObjectAddHeader(self, object, OBEX_HDR_LENGTH, hd, 4, 0);
    }

    if (!header.mimeType().isEmpty()) {
        qLog(Obex) << "Adding type header";
        /* Add type header */
        QByteArray latinArr = header.mimeType().toLatin1();
        hd.bs = (uchar*)latinArr.constData();
        OBEX_ObjectAddHeader(self, object,
                            OBEX_HDR_TYPE, hd, latinArr.length()+1, 0);

        /* Add PalmOS-style application id header */
        if ( latinArr == "text/x-vCalendar" ) {
            hd.bq4 = 0x64617465;
            OBEX_ObjectAddHeader(self, object, 0xcf, hd, 4, 0);
        }
    }

    if (!header.name().isEmpty()) {
        qLog(Obex) << "Adding name header";
        /* Add unicode name header*/
        QString uc = header.name() + QChar( 0x0 );
        uc = uc.mid( uc.lastIndexOf("/") + 1 );
        int name_size = uc.length() * 2;

        convertHBOQString(uc);
        hd.bs = reinterpret_cast<const uchar *>(uc.unicode());

        OBEX_ObjectAddHeader(self, object,
                            OBEX_HDR_NAME, hd, name_size, 0);
    }

    if (!header.description().isEmpty()) {
        qLog(Obex) << "Adding description header";

        QString uc = header.description() + QChar( 0x0 );
        int desc_size = uc.length() * 2;

        convertHBOQString(uc);
        hd.bs = reinterpret_cast<const uchar *>(uc.unicode());

        OBEX_ObjectAddHeader(self, object,
                             OBEX_HDR_DESCRIPTION, hd, desc_size, 0);
    }
}
