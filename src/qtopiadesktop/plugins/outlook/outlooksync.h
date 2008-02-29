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
#ifndef OUTLOOKSYNC_H
#define OUTLOOKSYNC_H

#include "qoutlook.h"
#include "sp.h"
#include "qmapi.h"

#include <qdplugin.h>
#include <trace.h>

#include <QVariant>

class OutlookThread;
class OutlookThreadObject;
class OTSyncObject;

QD_LOG_OPTION(OutlookSyncPlugin)
// All plugins related to synchronization inherit from this plugin
class OutlookSyncPlugin : public QDServerSyncPlugin
{
    Q_OBJECT
public:
    OutlookSyncPlugin( QObject *parent = 0 );
    virtual ~OutlookSyncPlugin();

    // QDPlugin
    QString id();
    virtual QString displayName() = 0;

    // QDSyncPlugin
    virtual QString dataset() = 0;
    virtual QByteArray referenceSchema() = 0;
    void prepareForSync();
    void finishSync();

    // QDServerSyncPlugin
    void fetchChangesSince(const QDateTime &timestamp);
    void createClientRecord(const QByteArray &record);
    void replaceClientRecord(const QByteArray &record);
    void removeClientRecord(const QString &identifier);
    void beginTransaction(const QDateTime &timestamp);
    void abortTransaction();
    void commitTransaction();

    QString dateToString( const QDate &date );
    QDate stringToDate( const QString &string );
    QString dateTimeToString( const QDateTime &datetime, bool utc = false );
    QDateTime stringToDateTime( const QString &string, bool utc = false );
    QString escape( const QString &string );
    QString unescape( const QString &string );

    bool getIdentifier( const QByteArray &record, QString &id, bool &local );

    virtual Outlook::OlDefaultFolders folderEnum() = 0;
    virtual Outlook::OlItemType itemEnum() = 0;
    virtual void getProperties( IDispatchPtr dispatch, QString &entryid, QDateTime &lastModified ) = 0;
    virtual void dump_item( IDispatchPtr dispatch, QTextStream &stream ) = 0;
    virtual QString read_item( IDispatchPtr dispatch, const QByteArray &record ) = 0;
    virtual void delete_item( IDispatchPtr dispatch ) = 0;
    virtual void init_item( IDispatchPtr dispatch );

signals:
    void t_fetchChangesSince(const QDateTime &timestamp);
    void t_updateClientRecord(const QByteArray &record);
    void t_removeClientRecord(const QString &identifier);

private slots:
    void logonDone( bool ok );

protected:
    OutlookThread *thread;
    OTSyncObject *so;
};

// =====================================================================

// This object lives on the Outlook thread
class OTSyncObject : public QObject
{
    Q_OBJECT
public:
    OTSyncObject();
    ~OTSyncObject();

    IDispatchPtr findItem( const QString &entryid );

signals:
    void logonDone(bool ok);
    void mappedId(const QString &serverId, const QString &clientId);
    void createServerRecord(const QByteArray &record);
    void replaceServerRecord(const QByteArray &record);
    void removeServerRecord(const QString &identifier);
    void serverChangesCompleted();

public slots:
    void logon();
    void fetchChangesSince( const QDateTime &timestamp );
    void updateClientRecord(const QByteArray &record);
    void removeClientRecord(const QString &identifier);
    void waitForAbort();

public:
    OutlookSyncPlugin *q;
    OutlookThreadObject *o;
    QMAPI::Session *mapi;
    QStringList rememberedIds;
    bool abort;
};

// =====================================================================

#define DUMP_STRING(Field,Method)\
    do {\
        stream << "<" #Field ">";\
        try {\
            QString value = bstr_to_qstring(item->Get##Method());\
            LOG() << "item->Get" #Method << value;\
            stream << escape(value);\
        } catch (...) {\
            WARNING() << "item->Get" #Method << "!!! EXCEPTION !!!";\
        }\
        stream << "</" #Field ">\n";\
    } while ( 0 )

#define DUMP_STRING_ATTRIB(Field,Method,attrib,expression)\
    do {\
        try {\
            QString value = bstr_to_qstring(item->Get##Method());\
            LOG() << "item->Get" #Method << value;\
            stream << "<" #Field;\
            QString attr = QString("\"%1\"").arg(expression);\
            stream << " " #attrib "=" << attr;\
            stream << ">" << escape(value) << "</" #Field ">\n";\
        } catch (...) {\
            WARNING() << "item->Get" #Method << "!!! EXCEPTION !!!";\
            stream << "<" #Field "></" #Field ">\n";\
        }\
    } while ( 0 )

#define DUMP_INT(Field,Method)\
    do {\
        stream << "<" #Field ">";\
        try {\
            int value = item->Get##Method();\
            LOG() << "item->Get" #Method << value;\
            stream << value;\
        } catch (...) {\
            WARNING() << "item->Get" #Method << "!!! EXCEPTION !!!";\
        }\
        stream << "</" #Field ">\n";\
    } while ( 0 )

#define DUMP_DATE(Field,Method)\
    do {\
        stream << "<" #Field ">";\
        try {\
            QDateTime value = date_to_qdatetime(item->Get##Method());\
            LOG() << "item->Get" #Method << value;\
            if ( value.isValid() )\
                stream << escape(dateToString(value.date()));\
        } catch (...) {\
            WARNING() << "item->Get" #Method << "!!! EXCEPTION !!!";\
        }\
        stream << "</" #Field ">\n";\
    } while ( 0 )

#define DUMP_DATE_ITEM(Field,Method,item)\
    do {\
        stream << "<" #Field ">";\
        try {\
            QDateTime value = date_to_qdatetime(item->Get##Method());\
            LOG() << "item->Get" #Method << value;\
            if ( value.isValid() )\
                stream << escape(dateToString(value.date()));\
        } catch (...) {\
            WARNING() << "item->Get" #Method << "!!! EXCEPTION !!!";\
        }\
        stream << "</" #Field ">\n";\
    } while ( 0 )

#define DUMP_DATETIME(Field,Method)\
    do {\
        stream << "<" #Field ">";\
        try {\
            QDateTime value = date_to_qdatetime(item->Get##Method());\
            LOG() << "item->Get" #Method << value;\
            if ( value.isValid() )\
                stream << escape(dateTimeToString(value));\
        } catch (...) {\
            WARNING() << "item->Get" #Method << "!!! EXCEPTION !!!";\
        }\
        stream << "</" #Field ">\n";\
    } while ( 0 )

#define DUMP_EXPR(Field,expression)\
    do {\
        stream << "<" #Field ">";\
        try {\
            QString value = expression;\
            LOG() << #expression << value;\
            stream << escape(value);\
        } catch (...) {\
            WARNING() << #expression << "!!! EXCEPTION !!!";\
        }\
        stream << "</" #Field ">\n";\
    } while ( 0 )

#define DUMP_CUSTOM(Field,CustomField)\
    do {\
        stream << "<" #Field ">";\
        try {\
            Outlook::UserPropertyPtr up = props->Find(#CustomField);\
            if ( up ) {\
                QString value = variant_to_qstring(up->GetValue());\
                stream << escape(value);\
            }\
        } catch (...) {\
            WARNING() << "props->Find" #CustomField << "!!! EXCEPTION !!!";\
        }\
        stream << "</" #Field ">\n";\
    } while ( 0 )

#define DUMP_MAPI(Field,Method)\
    do {\
        stream << "<" #Field ">";\
        try {\
            QString value = mc?mc->##Method():bstr_to_qstring(item->Get##Method());\
            LOG() << "item->Get" #Method << value;\
            stream << escape(value);\
        } catch (...) {\
            WARNING() << "item->Get" #Method << "!!! EXCEPTION !!!";\
        }\
        stream << "</" #Field ">\n";\
    } while ( 0 )

#define PREPARE_CUSTOM_MAP(qtopiaUserProps)\
    Outlook::UserPropertiesPtr props = item->GetUserProperties();\
    Q_ASSERT(props);\
    QMap<QString,QString> qtopiaUserProps

#define DUMP_CUSTOM_MAP(qtopiaUserProps)\
    do {\
        stream << "<CustomFields>\n";\
        for ( QMap<QString,QString>::const_iterator it = qtopiaUserProps.constBegin(); it != qtopiaUserProps.constEnd(); ++it )\
            stream << "<CustomField>\n<Key>" << escape(it.key()) << "</Key>\n<Value>" << escape(it.value()) << "</Value>\n</CustomField>\n";\
        stream << "</CustomFields>\n";\
    } while ( 0 )

#define PREPARE_MAPI(type)\
    SP<QMAPI::##type> mc;\
    if ( so->mapi ) {\
        IUnknownPtr obj;\
        try {\
            obj = item->GetMAPIOBJECT();\
        } catch (...) {\
            WARNING() << "item->GetMAPIOBJECT" << "!!! EXCEPTION !!!";\
            obj = 0;\
        }\
        if ( obj ) {\
            mc = so->mapi->open##type(obj);\
            if ( !mc )\
                WARNING() << "Could not open MAPI object for item with EntryID" << bstr_to_qstring(item->GetEntryID());\
        }\
    }

#define DUMP_ENUM(Field,Method,OValue,EValue)\
    do {\
        try {\
            if ( item->Get##Method() == OValue ) {\
                LOG() << "item->Get" #Method << OValue;\
                stream << #EValue;\
            }\
        } catch (...) {\
            WARNING() << "item->Get" #Method << "!!! EXCEPTION !!!";\
        }\
    } while ( 0 )

#define DUMP_ENUM_V(Field,Method,OValue,EValue)\
    do {\
        try {\
            if ( item->Get##Method() == OValue ) {\
                LOG() << "item->Get" #Method << OValue;\
                stream << EValue;\
            }\
        } catch (...) {\
            WARNING() << "item->Get" #Method << "!!! EXCEPTION !!!";\
        }\
    } while ( 0 )

// =====================================================================

#define READ_STRING(Field,Method)\
    do {\
        if ( key == #Field ) {\
            LOG() << "item->Put" #Method << value;\
            try {\
                item->Put##Method( qstring_to_bstr(value) );\
            } catch (...) {\
                WARNING() << "item->Put" #Method << "!!! EXCEPTION !!!";\
            }\
        }\
    } while ( 0 )

#define READ_STRING_ATTRIB(Field,Method,attrib,attrMethod)\
    do {\
        if ( key == #Field ) {\
            LOG() << "item->Put" #Method << value;\
            try {\
                item->Put##Method( qstring_to_bstr(value) );\
            } catch (...) {\
                WARNING() << "item->Put" #Method << "!!! EXCEPTION !!!";\
            }\
            QStringRef v = attributes.value( #attrib );\
            if ( !v.isNull() ) {\
                LOG() << "item->Put" #attrMethod << v.toString();\
                try {\
                    item->Put##attrMethod( qstring_to_bstr(v.toString()) );\
                } catch (...) {\
                    WARNING() << "item->Put" #attrMethod << "!!! EXCEPTION !!!";\
                }\
            }\
        }\
    } while ( 0 )

#define READ_DATE(Field,Method)\
    READ_DATE_ITEM(Field,Method,item)

#define READ_DATE_ITEM(Field,Method,item)\
    do {\
        if ( key == #Field ) {\
            LOG() << #item "->Put" #Method << stringToDate(value);\
            try {\
                item->Put##Method( qdatetime_to_date(QDateTime(stringToDate(value))) );\
            } catch (...) {\
                WARNING() << "item->Put" #Method << "!!! EXCEPTION !!!";\
            }\
        }\
    } while ( 0 )

#define READ_DATETIME(Field,Method)\
    do {\
        if ( key == #Field ) {\
            LOG() << "item->Put" #Method << stringToDateTime(value);\
            try {\
                item->Put##Method( qdatetime_to_date(stringToDateTime(value)) );\
            } catch (...) {\
                WARNING() << "item->Put" #Method << "!!! EXCEPTION !!!";\
            }\
        }\
    } while ( 0 )

#define READ_INT(Field,Method)\
    READ_INT_ITEM(Field,Method,item)

#define READ_INT_ITEM(Field,Method,item)\
    do {\
        if ( key == #Field ) {\
            LOG() << #item "->Put" #Method << QVariant(value).toInt();\
            try {\
                item->Put##Method( QVariant(value).toInt() );\
            } catch (...) {\
                WARNING() << "item->Put" #Method << "!!! EXCEPTION !!!";\
            }\
        }\
    } while ( 0 )

#define READ_CUSTOM(Field,CustomField)\
    do {\
        if ( key == #Field ) {\
            Outlook::UserPropertyPtr up = props->Find(#CustomField);\
            if ( up );else\
                up = props->Add(#CustomField,Outlook::olText);\
            up->PutValue( qstring_to_variant(value) );\
        }\
    } while ( 0 )

#define READ_ENUM(Field,Method,OValue,EValue)\
    READ_ENUM_ITEM(Field,Method,OValue,EValue,item)

#define READ_ENUM_ITEM(Field,Method,OValue,EValue,item)\
    do {\
        if ( key == #Field ) {\
            if ( value == #EValue ) {\
                LOG() << #item "->Put" #Method << #OValue << "(" << value << ")";\
                try {\
                    item->Put##Method( OValue );\
                } catch (...) {\
                    WARNING() << "item->Put" #Method << "!!! EXCEPTION !!!";\
                }\
            }\
        }\
    } while ( 0 )

#endif
