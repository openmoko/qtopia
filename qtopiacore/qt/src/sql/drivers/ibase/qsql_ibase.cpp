/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the QtSql module of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qsql_ibase.h"

#include <qcoreapplication.h>
#include <qdatetime.h>
#include <qvariant.h>
#include <qsqlerror.h>
#include <qsqlfield.h>
#include <qsqlindex.h>
#include <qsqlquery.h>
#include <qstringlist.h>
#include <qlist.h>
#include <qvector.h>

#include <qdebug.h>

#include <ibase.h>

#include <stdlib.h>
#include <limits.h>
#include <math.h>

#define FBVERSION SQL_DIALECT_V6

enum { QIBaseChunkSize = SHRT_MAX / 2 };

static bool getIBaseError(QString& msg, ISC_STATUS* status, long &sqlcode)
{
    if (status[0] != 1 || status[1] <= 0)
        return false;

    sqlcode = isc_sqlcode(status);
    char buf[512];
    isc_sql_interprete(sqlcode, buf, 512);
    msg = QString::fromUtf8(buf);
    return true;
}

static void createDA(XSQLDA *&sqlda)
{
    sqlda = (XSQLDA *) malloc(XSQLDA_LENGTH(1));
    sqlda->sqln = 1;
    sqlda->sqld = 0;
    sqlda->version = SQLDA_VERSION1;
    sqlda->sqlvar[0].sqlind = 0;
    sqlda->sqlvar[0].sqldata = 0;
}

static void enlargeDA(XSQLDA *&sqlda, int n)
{
    free(sqlda);
    sqlda = (XSQLDA *) malloc(XSQLDA_LENGTH(n));
    sqlda->sqln = n;
    sqlda->version = SQLDA_VERSION1;
}

static void initDA(XSQLDA *sqlda)
{
    for (int i = 0; i < sqlda->sqld; ++i) {
        switch (sqlda->sqlvar[i].sqltype & ~1) {
        case SQL_INT64:
        case SQL_LONG:
        case SQL_SHORT:
        case SQL_FLOAT:
        case SQL_DOUBLE:
        case SQL_TIMESTAMP:
        case SQL_TYPE_TIME:
        case SQL_TYPE_DATE:
        case SQL_TEXT:
        case SQL_BLOB:
            sqlda->sqlvar[i].sqldata = (char*)malloc(sqlda->sqlvar[i].sqllen);
            break;
        case SQL_ARRAY:
            sqlda->sqlvar[i].sqldata = (char*)malloc(sizeof(ISC_QUAD));
            memset(sqlda->sqlvar[i].sqldata, 0, sizeof(ISC_QUAD));
            break;
        case SQL_VARYING:
            sqlda->sqlvar[i].sqldata = (char*)malloc(sqlda->sqlvar[i].sqllen + sizeof(short));
            break;
        default:
            // not supported - do not bind.
            sqlda->sqlvar[i].sqldata = 0;
            break;
        }
        if (sqlda->sqlvar[i].sqltype & 1) {
            sqlda->sqlvar[i].sqlind = (short*)malloc(sizeof(short));
            *(sqlda->sqlvar[i].sqlind) = 0;
        } else {
            sqlda->sqlvar[i].sqlind = 0;
        }
    }
}

static void delDA(XSQLDA *&sqlda)
{
    if (!sqlda)
        return;
    for (int i = 0; i < sqlda->sqld; ++i) {
        free(sqlda->sqlvar[i].sqlind);
        free(sqlda->sqlvar[i].sqldata);
    }
    free(sqlda);
    sqlda = 0;
}

static QVariant::Type qIBaseTypeName(int iType)
{
    switch (iType) {
    case blr_varying:
    case blr_varying2:
    case blr_text:
    case blr_cstring:
    case blr_cstring2:
        return QVariant::String;
    case blr_sql_time:
        return QVariant::Time;
    case blr_sql_date:
        return QVariant::Date;
    case blr_timestamp:
        return QVariant::DateTime;
    case blr_blob:
        return QVariant::ByteArray;
    case blr_quad:
    case blr_short:
    case blr_long:
        return QVariant::Int;
    case blr_int64:
        return QVariant::LongLong;
    case blr_float:
    case blr_d_float:
    case blr_double:
        return QVariant::Double;
    }
    qWarning("qIBaseTypeName: unknown datatype: %d", iType);
    return QVariant::Invalid;
}

static QVariant::Type qIBaseTypeName2(int iType)
{
    switch(iType & ~1) {
    case SQL_VARYING:
    case SQL_TEXT:
        return QVariant::String;
    case SQL_LONG:
    case SQL_SHORT:
        return QVariant::Int;
    case SQL_INT64:
        return QVariant::LongLong;
    case SQL_FLOAT:
    case SQL_DOUBLE:
        return QVariant::Double;
    case SQL_TIMESTAMP:
        return QVariant::DateTime;
    case SQL_TYPE_TIME:
        return QVariant::Time;
    case SQL_TYPE_DATE:
        return QVariant::Date;
    case SQL_ARRAY:
        return QVariant::List;
    case SQL_BLOB:
        return QVariant::ByteArray;
    default:
        return QVariant::Invalid;
    }
}

static ISC_TIMESTAMP toTimeStamp(const QDateTime &dt)
{
    static const QTime midnight(0, 0, 0, 0);
    static const QDate basedate(1858, 11, 17);
    ISC_TIMESTAMP ts;
    ts.timestamp_time = midnight.msecsTo(dt.time()) * 10;
    ts.timestamp_date = basedate.daysTo(dt.date());
    return ts;
}

static QDateTime fromTimeStamp(char *buffer)
{
    static const QDate bd(1858, 11, 17);
    QTime t;
    QDate d;

    // have to demangle the structure ourselves because isc_decode_time
    // strips the msecs
    t = t.addMSecs(int(((ISC_TIMESTAMP*)buffer)->timestamp_time / 10));
    d = bd.addDays(int(((ISC_TIMESTAMP*)buffer)->timestamp_date));

    return QDateTime(d, t);
}

static ISC_TIME toTime(const QTime &t)
{
    static const QTime midnight(0, 0, 0, 0);
    return (ISC_TIME)midnight.msecsTo(t) * 10;
}

static QTime fromTime(char *buffer)
{
    QTime t;
    // have to demangle the structure ourselves because isc_decode_time
    // strips the msecs
    t = t.addMSecs(int((*(ISC_TIME*)buffer) / 10));

    return t;
}
static ISC_DATE toDate(const QDate &t)
{
    static const QDate basedate(1858, 11, 17);
    ISC_DATE date;

    date = basedate.daysTo(t);
    return date;
}

static QDate fromDate(char *buffer)
{
    static const QDate bd(1858, 11, 17);
    QDate d;

    // have to demangle the structure ourselves because isc_decode_time
    // strips the msecs
    d = bd.addDays(int(((ISC_TIMESTAMP*)buffer)->timestamp_date));

    return d;
}

class QIBaseDriverPrivate
{
public:
    QIBaseDriverPrivate(QIBaseDriver *d) : q(d)
    {
        ibase = 0;
        trans = 0;
    }

    bool isError(const char *msg, QSqlError::ErrorType typ = QSqlError::UnknownError)
    {
        QString imsg;
        long sqlcode;
        if (!getIBaseError(imsg, status, sqlcode))
            return false;

        //qDebug() << "ERROR" << msg << imsg << typ;
        q->setLastError(QSqlError(QCoreApplication::translate("QIBaseDriver", msg),
                        imsg, typ, int(sqlcode)));
        return true;
    }

public:
    QIBaseDriver* q;
    isc_db_handle ibase;
    isc_tr_handle trans;
    ISC_STATUS status[20];
};

class QIBaseResultPrivate
{
public:
    QIBaseResultPrivate(QIBaseResult *d, const QIBaseDriver *ddb);
    ~QIBaseResultPrivate() { cleanup(); }

    void cleanup();
    bool isError(const char *msg, QSqlError::ErrorType typ = QSqlError::UnknownError)
    {
        QString imsg;
        long sqlcode;
        if (!getIBaseError(imsg, status, sqlcode))
            return false;

        q->setLastError(QSqlError(QCoreApplication::translate("QIBaseResult", msg),
                        imsg, typ, int(sqlcode)));
        return true;
    }

    bool transaction();
    bool commit();

    bool isSelect();
    QVariant fetchBlob(ISC_QUAD *bId);
    bool writeBlob(int i, const QByteArray &ba);
    QVariant fetchArray(int pos, ISC_QUAD *arr);
    bool writeArray(int i, const QList<QVariant> &list);

public:
    QIBaseResult *q;
    const QIBaseDriver *db;
    ISC_STATUS status[20];
    isc_tr_handle trans;
    //indicator whether we have a local transaction or a transaction on driver level
    bool localTransaction;
    isc_stmt_handle stmt;
    isc_db_handle ibase;
    XSQLDA *sqlda; // output sqlda
    XSQLDA *inda; // input parameters
    int queryType;
};


QIBaseResultPrivate::QIBaseResultPrivate(QIBaseResult *d, const QIBaseDriver *ddb):
    q(d), db(ddb), trans(0), stmt(0), ibase(ddb->d->ibase), sqlda(0), inda(0), queryType(-1)
{
    localTransaction = (ddb->d->ibase == 0);
}

void QIBaseResultPrivate::cleanup()
{
    commit();
    if (!localTransaction)
        trans = 0;

    if (stmt) {
        isc_dsql_free_statement(status, &stmt, DSQL_drop);
        stmt = 0;
    }

    delDA(sqlda);
    delDA(inda);

    queryType = -1;
    q->cleanup();
}

bool QIBaseResultPrivate::writeBlob(int i, const QByteArray &ba)
{
    isc_blob_handle handle = 0;
    ISC_QUAD *bId = (ISC_QUAD*)inda->sqlvar[i].sqldata;
    isc_create_blob2(status, &ibase, &trans, &handle, bId, 0, 0);
    if (!isError(QT_TRANSLATE_NOOP("QIBaseResult", "Unable to create BLOB"),
                 QSqlError::StatementError)) {
        int i = 0;
        while (i < ba.size()) {
            isc_put_segment(status, &handle, qMin(ba.size() - i, int(QIBaseChunkSize)),
                            const_cast<char*>(ba.data()) + i);
            if (isError(QT_TRANSLATE_NOOP("QIBaseResult", "Unable to write BLOB")))
                return false;
            i += qMin(ba.size() - i, int(QIBaseChunkSize));
        }
    }
    isc_close_blob(status, &handle);
    return true;
}

QVariant QIBaseResultPrivate::fetchBlob(ISC_QUAD *bId)
{
    isc_blob_handle handle = 0;

    isc_open_blob2(status, &ibase, &trans, &handle, bId, 0, 0);
    if (isError(QT_TRANSLATE_NOOP("QIBaseResult", "Unable to open BLOB"),
                QSqlError::StatementError))
        return QVariant();

    unsigned short len = 0;
    QByteArray ba;
    int chunkSize = QIBaseChunkSize;
    ba.resize(chunkSize);
    int read = 0;
    while ( isc_get_segment(status, &handle, &len, chunkSize, ba.data() + read) == 0 || status[1] == isc_segment ){
	read += len;
	if ( len < chunkSize  )
	    break;
	ba.resize(ba.size() + chunkSize);
    }

    bool isErr = isError(QT_TRANSLATE_NOOP("QIBaseResult", "Unable to read BLOB"),
                         QSqlError::StatementError);
    isc_close_blob(status, &handle);

    if (isErr)
        return QVariant();

    ba.resize(read);
    return ba;
}

template<typename T>
static QList<QVariant> toList(char** buf, int count, T* = 0)
{
    QList<QVariant> res;
    for (int i = 0; i < count; ++i) {
        res.append(*(T*)(*buf));
        *buf += sizeof(T);
    }
    return res;
}
/* char** ? seems like bad influence from oracle ... */
template<>
static QList<QVariant> toList<long>(char** buf, int count, long*)
{
    QList<QVariant> res;
    for (int i = 0; i < count; ++i) {
        if (sizeof(int) == sizeof(long))
            res.append(int((*(long*)(*buf))));
        else
            res.append((qint64)(*(long*)(*buf)));
        *buf += sizeof(long);
    }
    return res;
}

static char* readArrayBuffer(QList<QVariant>& list, char *buffer, short curDim,
                             short* numElements, ISC_ARRAY_DESC *arrayDesc)
{
    const short dim = arrayDesc->array_desc_dimensions - 1;
    const unsigned char dataType = arrayDesc->array_desc_dtype;
    QList<QVariant> valList;
    unsigned short strLen = arrayDesc->array_desc_length;

    if (curDim != dim) {
        for(int i = 0; i < numElements[curDim]; ++i)
            buffer = readArrayBuffer(list, buffer, curDim + 1, numElements, arrayDesc);
    } else {
        switch(dataType) {
            case blr_varying:
            case blr_varying2:
                 strLen += 2; // for the two terminating null values
            case blr_text:
            case blr_text2: {
                int o;
                for(int i = 0; i < numElements[dim]; ++i) {
                    for(o = 0; o < strLen && buffer[o]!=0; ++o );
                    valList.append(QString::fromUtf8(buffer, o));
                    buffer += strLen;
                }
                break; }
            case blr_long:
                valList = toList<long>(&buffer, numElements[dim], static_cast<long *>(0));
                break;
            case blr_short:
                valList = toList<short>(&buffer, numElements[dim]);
                break;
            case blr_int64:
                valList = toList<qint64>(&buffer, numElements[dim]);
                break;
            case blr_float:
                valList = toList<float>(&buffer, numElements[dim]);
                break;
            case blr_double:
                valList = toList<double>(&buffer, numElements[dim]);
                break;
            case blr_timestamp:
                for(int i = 0; i < numElements[dim]; ++i) {
                    valList.append(fromTimeStamp(buffer));
                    buffer += sizeof(ISC_TIMESTAMP);
                }
                break;
            case blr_sql_time:
                for(int i = 0; i < numElements[dim]; ++i) {
                    valList.append(fromTime(buffer));
                    buffer += sizeof(ISC_TIME);
                }
                break;
            case blr_sql_date:
                for(int i = 0; i < numElements[dim]; ++i) {
                    valList.append(fromDate(buffer));
                    buffer += sizeof(ISC_DATE);
                }
                break;
        }
    }
    if (dim > 0)
        list.append(valList);
    else
        list += valList;
    return buffer;
}

QVariant QIBaseResultPrivate::fetchArray(int pos, ISC_QUAD *arr)
{
    QList<QVariant> list;
    ISC_ARRAY_DESC desc;

    if (!arr)
        return list;

    QByteArray relname(sqlda->sqlvar[pos].relname, sqlda->sqlvar[pos].relname_length);
    QByteArray sqlname(sqlda->sqlvar[pos].aliasname, sqlda->sqlvar[pos].aliasname_length);

    isc_array_lookup_bounds(status, &ibase, &trans, relname.data(), sqlname.data(), &desc);
    if (isError(QT_TRANSLATE_NOOP("QIBaseResult", "Could not find array"),
                QSqlError::StatementError))
        return list;


    int arraySize = 1, subArraySize;
    short dimensions = desc.array_desc_dimensions;
    short *numElements = new short[dimensions];

    for(int i = 0; i < dimensions; ++i) {
        subArraySize = (desc.array_desc_bounds[i].array_bound_upper -
                      desc.array_desc_bounds[i].array_bound_lower + 1);
        numElements[i] = subArraySize;
        arraySize = subArraySize * arraySize;
    }

    ISC_LONG bufLen;
    QByteArray ba;
    /* varying arrayelements are stored with 2 trailing null bytes
       indicating the length of the string
     */
    if (desc.array_desc_dtype == blr_varying
        || desc.array_desc_dtype == blr_varying2) {
        desc.array_desc_length += 2;
        bufLen = desc.array_desc_length * arraySize * sizeof(short);
    } else {
        bufLen = desc.array_desc_length *  arraySize;
    }


    ba.resize(int(bufLen));
    isc_array_get_slice(status, &ibase, &trans, arr, &desc, ba.data(), &bufLen);
    if (isError(QT_TRANSLATE_NOOP("QIBaseResult", "Could not get array data"),
                QSqlError::StatementError))
        return list;

    readArrayBuffer(list, ba.data(), 0, numElements, &desc);

    delete[] numElements;

    return QVariant(list);
}

template<typename T>
static char* fillList(char *buffer, const QList<QVariant> &list, T* = 0)
{
    for (int i = 0; i < list.size(); ++i) {
        T val;
        val = qvariant_cast<T>(list.at(i));
        memcpy(buffer, &val, sizeof(T));
        buffer += sizeof(T);
    }
    return buffer;
}

template<>
static char* fillList<float>(char *buffer, const QList<QVariant> &list, float*)
{
    for (int i = 0; i < list.size(); ++i) {
        double val;
        float val2 = 0;
        val = qvariant_cast<double>(list.at(i));
        val2 = (float)val;
        memcpy(buffer, &val2, sizeof(float));
        buffer += sizeof(float);
    }
    return buffer;
}

static char* qFillBufferWithString(char *buffer, const QString& string, short buflen, bool varying, bool array)
{
    QByteArray str = string.toUtf8(); // keep a copy of the string alive in this scope
    if (varying) {
        short tmpBuflen = buflen;
        if (str.length() < buflen)
            buflen = str.length();
        if (array) { // interbase stores varying arrayelements different than normal varying elements
            memcpy(buffer, str.data(), buflen);
            memset(buffer + buflen, 0, tmpBuflen - buflen);
        } else {
            *(short*)buffer = buflen; // first two bytes is the length
            memcpy(buffer + sizeof(short), str.data(), buflen);
        }
        buffer += tmpBuflen;
    } else {
        str = str.leftJustified(buflen, ' ', true);
        memcpy(buffer, str.data(), buflen);
        buffer += buflen;
    }
    return buffer;
}

static char* createArrayBuffer(char *buffer, const QList<QVariant> &list,
                               QVariant::Type type, short curDim, ISC_ARRAY_DESC *arrayDesc,
                               QString& error)
{
    int i;
    ISC_ARRAY_BOUND *bounds = arrayDesc->array_desc_bounds;
    short dim = arrayDesc->array_desc_dimensions - 1;

    int elements = (bounds[curDim].array_bound_upper -
                    bounds[curDim].array_bound_lower + 1);

    if (list.size() != elements) { // size mismatch
        error = QLatin1String("Expected size: %1. Supplied size: %2");
        error = QLatin1String("Array size mismatch. Fieldname: %1 ")
                + error.arg(elements).arg(list.size());
        return 0;
    }

    if (curDim != dim) {
        for(i = 0; i < list.size(); ++i) {

          if (list.at(i).type() != QVariant::List) { // dimensions mismatch
              error = QLatin1String("Array dimensons mismatch. Fieldname: %1");
              return 0;
          }

          buffer = createArrayBuffer(buffer, list.at(i).toList(), type, curDim + 1, arrayDesc,
                                     error);
          if (!buffer)
              return 0;
        }
    } else {
        switch(type) {
        case QVariant::Int:
        case QVariant::UInt:
            if (arrayDesc->array_desc_dtype == blr_short)
                buffer = fillList<short>(buffer, list);
            else
                buffer = fillList<int>(buffer, list);
            break;
        case QVariant::Double:
            if (arrayDesc->array_desc_dtype == blr_float)
                buffer = fillList<float>(buffer, list, static_cast<float *>(0));
            else
                buffer = fillList<double>(buffer, list);
            break;
        case QVariant::LongLong:
            buffer = fillList<qint64>(buffer, list);
            break;
        case QVariant::ULongLong:
            buffer = fillList<quint64>(buffer, list);
            break;
        case QVariant::String:
            for (i = 0; i < list.size(); ++i)
                buffer = qFillBufferWithString(buffer, list.at(i).toString(),
                                               arrayDesc->array_desc_length,
                                               arrayDesc->array_desc_dtype == blr_varying, true);
            break;
        case QVariant::Date:
            for (i = 0; i < list.size(); ++i) {
                *((ISC_DATE*)buffer) = toDate(list.at(i).toDate());
                buffer += sizeof(ISC_DATE);
            }
            break;
        case QVariant::Time:
            for (i = 0; i < list.size(); ++i) {
                *((ISC_TIME*)buffer) = toTime(list.at(i).toTime());
                buffer += sizeof(ISC_TIME);
            }
            break;

        case QVariant::DateTime:
            for (i = 0; i < list.size(); ++i) {
                *((ISC_TIMESTAMP*)buffer) = toTimeStamp(list.at(i).toDateTime());
                buffer += sizeof(ISC_TIMESTAMP);
            }
            break;
        default:
            break;
        }
    }
    return buffer;
}

bool QIBaseResultPrivate::writeArray(int column, const QList<QVariant> &list)
{
    QString error;
    ISC_QUAD *arrayId = (ISC_QUAD*) inda->sqlvar[column].sqldata;
    ISC_ARRAY_DESC desc;

    QByteArray relname(inda->sqlvar[column].relname, inda->sqlvar[column].relname_length);
    QByteArray sqlname(inda->sqlvar[column].aliasname, inda->sqlvar[column].aliasname_length);


    isc_array_lookup_bounds(status, &ibase, &trans, relname.data(), sqlname.data(), &desc);
    if (isError(QT_TRANSLATE_NOOP("QIBaseResult", "Could not find array"),
                QSqlError::StatementError))
        return false;

    short arraySize = 1;
    ISC_LONG bufLen;
    QList<QVariant> subList = list;

    short dimensions = desc.array_desc_dimensions;
    for(int i = 0; i < dimensions; ++i) {
        arraySize *= (desc.array_desc_bounds[i].array_bound_upper -
                      desc.array_desc_bounds[i].array_bound_lower + 1);
    }

    /* varying arrayelements are stored with 2 trailing null bytes
       indicating the length of the string
     */
    if (desc.array_desc_dtype == blr_varying ||
       desc.array_desc_dtype == blr_varying2)
        desc.array_desc_length += 2;

    bufLen = desc.array_desc_length * arraySize;
    QByteArray ba;
    ba.resize(int(bufLen));

    if (list.size() > arraySize) {
        error = QLatin1String("Array size missmatch: size of %1 is %2, size of provided list is %3");
        error = error.arg(QLatin1String(sqlname)).arg(arraySize).arg(list.size());
        q->setLastError(QSqlError(error, QLatin1String(""), QSqlError::StatementError));
        return false;
    }

    if (!createArrayBuffer(ba.data(), list, qIBaseTypeName(desc.array_desc_dtype),
                           0, &desc, error)) {
        q->setLastError(QSqlError(error.arg(QLatin1String(sqlname)), QLatin1String(""),
                        QSqlError::StatementError));
        return false;
    }

    /* readjust the buffer size*/
    if (desc.array_desc_dtype == blr_varying
        || desc.array_desc_dtype == blr_varying2)
        desc.array_desc_length -= 2;

    isc_array_put_slice(status, &ibase, &trans, arrayId, &desc, ba.data(), &bufLen);
    return true;
}


bool QIBaseResultPrivate::isSelect()
{
    char acBuffer[9];
    char qType = isc_info_sql_stmt_type;
    isc_dsql_sql_info(status, &stmt, 1, &qType, sizeof(acBuffer), acBuffer);
    if (isError(QT_TRANSLATE_NOOP("QIBaseResult", "Could not get query info"),
                QSqlError::StatementError))
        return false;
    int iLength = isc_vax_integer(&acBuffer[1], 2);
    queryType = isc_vax_integer(&acBuffer[3], iLength);
    return (queryType == isc_info_sql_stmt_select);
}

bool QIBaseResultPrivate::transaction()
{
    if (trans)
        return true;
    if (db->d->trans) {
        localTransaction = false;
        trans = db->d->trans;
        return true;
    }
    localTransaction = true;

    isc_start_transaction(status, &trans, 1, &ibase, 0, NULL);
    if (isError(QT_TRANSLATE_NOOP("QIBaseResult", "Could not start transaction"),
                QSqlError::TransactionError))
        return false;

    return true;
}

// does nothing if the transaction is on the
// driver level
bool QIBaseResultPrivate::commit()
{
    if (!trans)
        return false;
    // don't commit driver's transaction, the driver will do it for us
    if (!localTransaction)
        return true;

    isc_commit_transaction(status, &trans);
    trans = 0;
    return !isError(QT_TRANSLATE_NOOP("QIBaseResult", "Unable to commit transaction"),
                    QSqlError::TransactionError);
}

//////////

QIBaseResult::QIBaseResult(const QIBaseDriver* db):
    QSqlCachedResult(db)
{
    d = new QIBaseResultPrivate(this, db);
}

QIBaseResult::~QIBaseResult()
{
    delete d;
}

bool QIBaseResult::prepare(const QString& query)
{
    //qDebug("prepare: %s\n", qPrintable(query));
    if (!driver() || !driver()->isOpen() || driver()->isOpenError())
        return false;
    d->cleanup();
    setActive(false);
    setAt(QSql::BeforeFirstRow);

    createDA(d->sqlda);
    createDA(d->inda);

    if (!d->transaction())
        return false;

    isc_dsql_allocate_statement(d->status, &d->ibase, &d->stmt);
    if (d->isError(QT_TRANSLATE_NOOP("QIBaseResult", "Could not allocate statement"),
                   QSqlError::StatementError))
        return false;
    isc_dsql_prepare(d->status, &d->trans, &d->stmt, 0,
                     const_cast<char*>(query.toUtf8().constData()), FBVERSION, d->sqlda);
    if (d->isError(QT_TRANSLATE_NOOP("QIBaseResult", "Could not prepare statement"),
                   QSqlError::StatementError))
        return false;

    isc_dsql_describe_bind(d->status, &d->stmt, FBVERSION, d->inda);
    if (d->isError(QT_TRANSLATE_NOOP("QIBaseResult",
                    "Could not describe input statement"), QSqlError::StatementError))
        return false;
    if (d->inda->sqld > d->inda->sqln) {
        enlargeDA(d->inda, d->inda->sqld);

        isc_dsql_describe_bind(d->status, &d->stmt, FBVERSION, d->inda);
        if (d->isError(QT_TRANSLATE_NOOP("QIBaseResult",
                        "Could not describe input statement"), QSqlError::StatementError))
            return false;
    }
    initDA(d->inda);
    if (d->sqlda->sqld > d->sqlda->sqln) {
        // need more field descriptors
        enlargeDA(d->sqlda, d->sqlda->sqld);

        isc_dsql_describe(d->status, &d->stmt, FBVERSION, d->sqlda);
        if (d->isError(QT_TRANSLATE_NOOP("QIBaseResult", "Could not describe statement"),
                       QSqlError::StatementError))
            return false;
    }
    initDA(d->sqlda);

    setSelect(d->isSelect());
    if (!isSelect()) {
        free(d->sqlda);
        d->sqlda = 0;
    }

    return true;
}


bool QIBaseResult::exec()
{
    bool ok = true;

    if (!d->trans)
        d->transaction();

    if (!driver() || !driver()->isOpen() || driver()->isOpenError())
        return false;
    setActive(false);
    setAt(QSql::BeforeFirstRow);

    if (d->inda) {
        QVector<QVariant>& values = boundValues();
        int i;
        if (values.count() > d->inda->sqld) {
            qWarning("QIBaseResult::exec: Parameter mismatch, expected %d, got %d parameters",
                     d->inda->sqld, values.count());
            return false;
        }
        int para = 0;
        for (i = 0; i < values.count(); ++i) {
            para = i;
            if (!d->inda->sqlvar[para].sqldata)
                // skip unknown datatypes
                continue;
            const QVariant val(values[i]);
            if (d->inda->sqlvar[para].sqltype & 1) {
                if (val.isNull()) {
                    // set null indicator
                    *(d->inda->sqlvar[para].sqlind) = -1;
                    // and set the value to 0, otherwise it would count as empty string.
                    // it seems to be working with just setting sqlind to -1
                    //*((char*)d->inda->sqlvar[para].sqldata) = 0;
                    continue;
                }
                // a value of 0 means non-null.
                *(d->inda->sqlvar[para].sqlind) = 0;
            }
            switch(d->inda->sqlvar[para].sqltype & ~1) {
            case SQL_INT64:
                if (d->inda->sqlvar[para].sqlscale < 0)
                    *((qint64*)d->inda->sqlvar[para].sqldata) =
                        qint64(val.toDouble() * pow(10.0, d->inda->sqlvar[para].sqlscale * -1));
                else
                    *((qint64*)d->inda->sqlvar[para].sqldata) = val.toLongLong();
                break;
            case SQL_LONG:
                *((long*)d->inda->sqlvar[para].sqldata) = (long)val.toLongLong();
                break;
            case SQL_SHORT:
                *((short*)d->inda->sqlvar[para].sqldata) = (short)val.toInt();
                break;
            case SQL_FLOAT:
                *((float*)d->inda->sqlvar[para].sqldata) = (float)val.toDouble();
                break;
            case SQL_DOUBLE:
                *((double*)d->inda->sqlvar[para].sqldata) = val.toDouble();
                break;
            case SQL_TIMESTAMP:
                *((ISC_TIMESTAMP*)d->inda->sqlvar[para].sqldata) = toTimeStamp(val.toDateTime());
                break;
            case SQL_TYPE_TIME:
                *((ISC_TIME*)d->inda->sqlvar[para].sqldata) = toTime(val.toTime());
                break;
            case SQL_TYPE_DATE:
                *((ISC_DATE*)d->inda->sqlvar[para].sqldata) = toDate(val.toDate());
                break;
            case SQL_VARYING:
            case SQL_TEXT:
                qFillBufferWithString(d->inda->sqlvar[para].sqldata, val.toString(),
                                      d->inda->sqlvar[para].sqllen,
                                      (d->inda->sqlvar[para].sqltype & ~1) == SQL_VARYING, false);
                break;
            case SQL_BLOB:
                    ok &= d->writeBlob(para, val.toByteArray());
                    break;
            case SQL_ARRAY:
                    ok &= d->writeArray(para, val.toList());
                    break;
            default:
                    qWarning("QIBaseResult::exec: Unknown datatype %d",
                             d->inda->sqlvar[para].sqltype & ~1);
                    break;
            }
        }
    }

    if (ok) {
        if (colCount()) {
            isc_dsql_free_statement(d->status, &d->stmt, DSQL_close);
            if (d->isError(QT_TRANSLATE_NOOP("QIBaseResult", "Unable to close statement")))
                return false;
            cleanup();
        }
        isc_dsql_execute(d->status, &d->trans, &d->stmt, FBVERSION, d->inda);
        if (d->isError(QT_TRANSLATE_NOOP("QIBaseResult", "Unable to execute query")))
            return false;

        if (d->sqlda)
            init(d->sqlda->sqld);

        if (d->queryType != isc_info_sql_stmt_select)
             d->commit();

        setActive(true);
        return true;
    }
    return false;
}

bool QIBaseResult::reset (const QString& query)
{
    if (!prepare(query))
        return false;
    return exec();
}

bool QIBaseResult::gotoNext(QSqlCachedResult::ValueCache& row, int rowIdx)
{
    ISC_STATUS stat = isc_dsql_fetch(d->status, &d->stmt, FBVERSION, d->sqlda);

    if (stat == 100) {
        // no more rows
        setAt(QSql::AfterLastRow);
        return false;
    }
    if (d->isError(QT_TRANSLATE_NOOP("QIBaseResult", "Could not fetch next item"),
                   QSqlError::StatementError))
        return false;
    if (rowIdx < 0) // not interested in actual values
        return true;

    for (int i = 0; i < d->sqlda->sqld; ++i) {
        int idx = rowIdx + i;
        char *buf = d->sqlda->sqlvar[i].sqldata;
        int size = d->sqlda->sqlvar[i].sqllen;
        Q_ASSERT(buf);

        if ((d->sqlda->sqlvar[i].sqltype & 1) && *d->sqlda->sqlvar[i].sqlind) {
            // null value
            QVariant v;
            v.convert(qIBaseTypeName2(d->sqlda->sqlvar[i].sqltype));
            row[idx] = v;
            continue;
        }

        switch(d->sqlda->sqlvar[i].sqltype & ~1) {
        case SQL_VARYING:
            // pascal strings - a short with a length information followed by the data
            row[idx] = QString::fromUtf8(buf + sizeof(short), *(short*)buf);
            break;
        case SQL_INT64:
            if (d->sqlda->sqlvar[i].sqlscale < 0)
                row[idx] = *(qint64*)buf * pow(10.0, d->sqlda->sqlvar[i].sqlscale);
            else
                row[idx] = QVariant(*(qint64*)buf);
            break;
        case SQL_LONG:
            if (d->sqlda->sqlvar[i].sqllen == 4)
                row[idx] = QVariant(int((*(long*)buf)));
            else
                row[idx] = QVariant(qint64((*(long*)buf)));
            break;
        case SQL_SHORT:
            row[idx] = QVariant(int((*(short*)buf)));
            break;
        case SQL_FLOAT:
            row[idx] = QVariant(double((*(float*)buf)));
            break;
        case SQL_DOUBLE:
            row[idx] = QVariant(*(double*)buf);
            break;
        case SQL_TIMESTAMP:
            row[idx] = fromTimeStamp(buf);
            break;
        case SQL_TYPE_TIME:
            row[idx] = fromTime(buf);
            break;
        case SQL_TYPE_DATE:
            row[idx] = fromDate(buf);
            break;
        case SQL_TEXT:
            row[idx] = QString::fromUtf8(buf, size);
            break;
        case SQL_BLOB:
            row[idx] = d->fetchBlob((ISC_QUAD*)buf);
            break;
        case SQL_ARRAY:
            row[idx] = d->fetchArray(i, (ISC_QUAD*)buf);
            break;
        default:
            // unknown type - don't even try to fetch
            row[idx] = QVariant();
            break;
        }
    }

    return true;
}

int QIBaseResult::size()
{
    return -1;

#if 0 /// ### FIXME
    static char sizeInfo[] = {isc_info_sql_records};
    char buf[64];

    //qDebug() << sizeInfo;
    if (!isActive() || !isSelect())
        return -1;

        char ct;
        short len;
        int val = 0;
//    while(val == 0) {
        isc_dsql_sql_info(d->status, &d->stmt, sizeof(sizeInfo), sizeInfo, sizeof(buf), buf);
//        isc_database_info(d->status, &d->ibase, sizeof(sizeInfo), sizeInfo, sizeof(buf), buf);

        for(int i = 0; i < 66; ++i)
            qDebug() << QString::number(buf[i]);

        for (char* c = buf + 3; *c != isc_info_end; /*nothing*/) {
            ct = *(c++);
            len = isc_vax_integer(c, 2);
            c += 2;
            val = isc_vax_integer(c, len);
            c += len;
            qDebug() << "size" << val;
            if (ct == isc_info_req_select_count)
                return val;
        }
        //qDebug() << "size -1";
        return -1;

        unsigned int i, result_size;
        if (buf[0] == isc_info_sql_records) {
            i = 3;
            result_size = isc_vax_integer(&buf[1],2);
            while (buf[i] != isc_info_end && i < result_size) {
                len = (short)isc_vax_integer(&buf[i+1],2);
                if (buf[i] == isc_info_req_select_count)
                     return (isc_vax_integer(&buf[i+3],len));
                i += len+3;
           }
        }
//    }
    return -1;
#endif
}

int QIBaseResult::numRowsAffected()
{
    static char acCountInfo[] = {isc_info_sql_records};
    char cCountType;

    switch (d->queryType) {
    case isc_info_sql_stmt_select:
        cCountType = isc_info_req_select_count;
        break;
    case isc_info_sql_stmt_update:
        cCountType = isc_info_req_update_count;
        break;
    case isc_info_sql_stmt_delete:
        cCountType = isc_info_req_delete_count;
        break;
    case isc_info_sql_stmt_insert:
        cCountType = isc_info_req_insert_count;
        break;
    }

    char acBuffer[33];
    int iResult = -1;
    isc_dsql_sql_info(d->status, &d->stmt, sizeof(acCountInfo), acCountInfo, sizeof(acBuffer), acBuffer);
    if (d->isError(QT_TRANSLATE_NOOP("QIBaseResult", "Could not get statement info"),
                   QSqlError::StatementError))
        return -1;
    for (char *pcBuf = acBuffer + 3; *pcBuf != isc_info_end; /*nothing*/) {
        char cType = *pcBuf++;
        short sLength = isc_vax_integer (pcBuf, 2);
        pcBuf += 2;
        int iValue = isc_vax_integer (pcBuf, sLength);
        pcBuf += sLength;

        if (cType == cCountType) {
            iResult = iValue;
            break;
        }
    }
    return iResult;
}

QSqlRecord QIBaseResult::record() const
{
    QSqlRecord rec;
    if (!isActive() || !d->sqlda)
        return rec;

    XSQLVAR v;
    for (int i = 0; i < d->sqlda->sqld; ++i) {
        v = d->sqlda->sqlvar[i];
        QSqlField f(QString::fromLatin1(v.aliasname, v.aliasname_length).simplified(),
                    qIBaseTypeName2(d->sqlda->sqlvar[i].sqltype));
        f.setLength(v.sqllen);
        f.setPrecision(v.sqlscale);
        f.setSqlType(v.sqltype);
        rec.append(f);
    }
    return rec;
}

QVariant QIBaseResult::handle() const
{
    return QVariant(qRegisterMetaType<isc_stmt_handle>("isc_stmt_handle"), &d->stmt);
}

/*********************************/

QIBaseDriver::QIBaseDriver(QObject * parent)
    : QSqlDriver(parent)
{
    d = new QIBaseDriverPrivate(this);
}

QIBaseDriver::QIBaseDriver(void *connection, QObject *parent)
    : QSqlDriver(parent)
{
    d = new QIBaseDriverPrivate(this);
    d->ibase = (isc_db_handle)connection;
    setOpen(true);
    setOpenError(false);
}

QIBaseDriver::~QIBaseDriver()
{
    delete d;
}

bool QIBaseDriver::hasFeature(DriverFeature f) const
{
    switch (f) {
    case QuerySize:
    case NamedPlaceholders:
    case LastInsertId:
    case BatchOperations:
        return false;
    case Transactions:
    case PreparedQueries:
    case PositionalPlaceholders:
    case Unicode:
    case BLOB:
        return true;
    }
    return false;
}

bool QIBaseDriver::open(const QString & db,
          const QString & user,
          const QString & password,
          const QString & host,
          int /*port*/,
          const QString & /* connOpts */)
{
    if (isOpen())
        close();

    static const char enc[8] = "UTF_FSS";
    QByteArray usr = user.toLocal8Bit();
    QByteArray pass = password.toLocal8Bit();
    usr.truncate(255);
    pass.truncate(255);

    QByteArray ba;
    ba.resize(usr.length() + pass.length() + sizeof(enc) + 6);
    int i = -1;
    ba[++i] = isc_dpb_version1;
    ba[++i] = isc_dpb_user_name;
    ba[++i] = usr.length();
    memcpy(ba.data() + ++i, usr.data(), usr.length());
    i += usr.length();
    ba[i] = isc_dpb_password;
    ba[++i] = pass.length();
    memcpy(ba.data() + ++i, pass.data(), pass.length());
    i += pass.length();
    ba[i] = isc_dpb_lc_ctype;
    ba[++i] = sizeof(enc) - 1;
    memcpy(ba.data() + ++i, enc, sizeof(enc) - 1);
    i += sizeof(enc) - 1;

    QString ldb;
    if (!host.isEmpty())
        ldb += host + QLatin1Char(':');
    ldb += db;
    isc_attach_database(d->status, 0, const_cast<char *>(ldb.toLatin1().constData()),
                        &d->ibase, i, ba.data());
    if (d->isError(QT_TRANSLATE_NOOP("QIBaseDriver", "Error opening database"),
                   QSqlError::ConnectionError)) {
        setOpenError(true);
        return false;
    }

    setOpen(true);
    return true;
}

void QIBaseDriver::close()
{
    if (isOpen()) {
        isc_detach_database(d->status, &d->ibase);
        d->ibase = 0;
        setOpen(false);
        setOpenError(false);
    }
}

QSqlResult *QIBaseDriver::createResult() const
{
    return new QIBaseResult(this);
}

bool QIBaseDriver::beginTransaction()
{
    if (!isOpen() || isOpenError())
        return false;
    if (d->trans)
        return false;

    isc_start_transaction(d->status, &d->trans, 1, &d->ibase, 0, NULL);
    return !d->isError(QT_TRANSLATE_NOOP("QIBaseDriver", "Could not start transaction"),
                       QSqlError::TransactionError);
}

bool QIBaseDriver::commitTransaction()
{
    if (!isOpen() || isOpenError())
        return false;
    if (!d->trans)
        return false;

    isc_commit_transaction(d->status, &d->trans);
    d->trans = 0;
    return !d->isError(QT_TRANSLATE_NOOP("QIBaseDriver", "Unable to commit transaction"),
                       QSqlError::TransactionError);
}

bool QIBaseDriver::rollbackTransaction()
{
    if (!isOpen() || isOpenError())
        return false;
    if (!d->trans)
        return false;

    isc_rollback_transaction(d->status, &d->trans);
    d->trans = 0;
    return !d->isError(QT_TRANSLATE_NOOP("QIBaseDriver", "Unable to rollback transaction"),
                       QSqlError::TransactionError);
}

QStringList QIBaseDriver::tables(QSql::TableType type) const
{
    QStringList res;
    if (!isOpen())
        return res;

    QString typeFilter;

    if (type == QSql::SystemTables) {
        typeFilter += QLatin1String("RDB$SYSTEM_FLAG != 0");
    } else if (type == (QSql::SystemTables | QSql::Views)) {
        typeFilter += QLatin1String("RDB$SYSTEM_FLAG != 0 OR RDB$VIEW_BLR NOT NULL");
    } else {
        if (!(type & QSql::SystemTables))
            typeFilter += QLatin1String("RDB$SYSTEM_FLAG = 0 AND ");
        if (!(type & QSql::Views))
            typeFilter += QLatin1String("RDB$VIEW_BLR IS NULL AND ");
        if (!(type & QSql::Tables))
            typeFilter += QLatin1String("RDB$VIEW_BLR IS NOT NULL AND ");
        if (!typeFilter.isEmpty())
            typeFilter.chop(5);
    }
    if (!typeFilter.isEmpty())
        typeFilter.prepend(QLatin1String("where "));

    QSqlQuery q(createResult());
    q.setForwardOnly(true);
    if (!q.exec(QLatin1String("select rdb$relation_name from rdb$relations ") + typeFilter))
        return res;
    while(q.next())
            res << q.value(0).toString().simplified();

    return res;
}

QSqlRecord QIBaseDriver::record(const QString& tablename) const
{
    QSqlRecord rec;
    if (!isOpen())
        return rec;

    QSqlQuery q(createResult());
    q.setForwardOnly(true);

    q.exec(QLatin1String("SELECT a.RDB$FIELD_NAME, b.RDB$FIELD_TYPE, b.RDB$FIELD_LENGTH, "
           "b.RDB$FIELD_SCALE, b.RDB$FIELD_PRECISION, a.RDB$NULL_FLAG "
           "FROM RDB$RELATION_FIELDS a, RDB$FIELDS b "
           "WHERE b.RDB$FIELD_NAME = a.RDB$FIELD_SOURCE "
           "AND a.RDB$RELATION_NAME = '") + tablename.toUpper() + QLatin1String("' "
           "ORDER BY a.RDB$FIELD_POSITION"));

    while (q.next()) {
        int type = q.value(1).toInt();
        QSqlField f(q.value(0).toString().simplified(), qIBaseTypeName(type));
        f.setLength(q.value(5).toInt());
        f.setPrecision(q.value(2).toInt());
        f.setRequired(q.value(4).toInt() > 0 ? true : false);
        f.setSqlType(type);

        rec.append(f);
    }
    return rec;
}

QSqlIndex QIBaseDriver::primaryIndex(const QString &table) const
{
    QSqlIndex index(table);
    if (!isOpen())
        return index;

    QSqlQuery q(createResult());
    q.setForwardOnly(true);
    q.exec(QLatin1String("SELECT a.RDB$INDEX_NAME, b.RDB$FIELD_NAME, d.RDB$FIELD_TYPE "
           "FROM RDB$RELATION_CONSTRAINTS a, RDB$INDEX_SEGMENTS b, RDB$RELATION_FIELDS c, RDB$FIELDS d "
           "WHERE a.RDB$CONSTRAINT_TYPE = 'PRIMARY KEY' "
           "AND a.RDB$RELATION_NAME = '") + table.toUpper() +
           QLatin1String(" 'AND a.RDB$INDEX_NAME = b.RDB$INDEX_NAME "
           "AND c.RDB$RELATION_NAME = a.RDB$RELATION_NAME "
           "AND c.RDB$FIELD_NAME = b.RDB$FIELD_NAME "
           "AND d.RDB$FIELD_NAME = c.RDB$FIELD_SOURCE "
           "ORDER BY b.RDB$FIELD_POSITION"));

    while (q.next()) {
        QSqlField field(q.value(1).toString().simplified(), qIBaseTypeName(q.value(2).toInt()));
        index.append(field); //TODO: asc? desc?
        index.setName(q.value(0).toString());
    }

    return index;
}

QString QIBaseDriver::formatValue(const QSqlField &field, bool trimStrings) const
{
    switch (field.type()) {
    case QVariant::DateTime: {
        QDateTime datetime = field.value().toDateTime();
        if (datetime.isValid())
            return QLatin1Char('\'') + QString::number(datetime.date().year()) + QLatin1Char('-') +
                QString::number(datetime.date().month()) + QLatin1Char('-') +
                QString::number(datetime.date().day()) + QLatin1Char(' ') +
                QString::number(datetime.time().hour()) + QLatin1Char(':') +
                QString::number(datetime.time().minute()) + QLatin1Char(':') +
                QString::number(datetime.time().second()) + QLatin1Char('.') +
                QString::number(datetime.time().msec()).rightJustified(3, QLatin1Char('0'), true) +
		QLatin1Char('\'');
        else
            return QLatin1String("NULL");
    }
    case QVariant::Time: {
        QTime time = field.value().toTime();
        if (time.isValid())
            return QLatin1Char('\'') + QString::number(time.hour()) + QLatin1Char(':') +
                QString::number(time.minute()) + QLatin1Char(':') +
                QString::number(time.second()) + QLatin1Char('.') +
                QString::number(time.msec()).rightJustified(3, QLatin1Char('0'), true) +
                QLatin1Char('\'');
        else
            return QLatin1String("NULL");
    }
    case QVariant::Date: {
        QDate date = field.value().toDate();
        if (date.isValid())
            return QLatin1Char('\'') + QString::number(date.year()) + QLatin1Char('-') +
                QString::number(date.month()) + QLatin1Char('-') +
                QString::number(date.day()) + QLatin1Char('\'');
            else
                return QLatin1String("NULL");
    }
    default:
        return QSqlDriver::formatValue(field, trimStrings);
    }
}

QVariant QIBaseDriver::handle() const
{
    return QVariant(qRegisterMetaType<isc_db_handle>("isc_db_handle"), &d->ibase);
}
