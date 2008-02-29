/****************************************************************************
**
** Copyright (C) 1992-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_COMMERCIAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qsql_oci.h"

#include <qcoreapplication.h>
#include <qvariant.h>
#include <qdatetime.h>
#include <qmetatype.h>
#include <qregexp.h>
#include <qshareddata.h>
#include <qsqlerror.h>
#include <qsqlfield.h>
#include <qsqlindex.h>
#include <qsqlquery.h>
#include <qstringlist.h>
#include <qvarlengtharray.h>
#include <qvector.h>
#include <qdebug.h>

#include <oci.h>
#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif

#include <stdlib.h>

#ifdef OCI_UTF16
// for Oracle >= 9
#define QOCI_UNICODE_API
#endif

#ifdef OCI_ATTR_RESERVED_19
// for a bug in CLOB handling in oracle 10g
# define QOCI_ORACLE10_WORKAROUND
#endif

#define QOCI_DYNAMIC_CHUNK_SIZE 65535
#define QOCI_PREFETCH_MEM  10240

Q_DECLARE_METATYPE(OCIEnv*)
Q_DECLARE_METATYPE(OCIStmt*)

#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
enum { QOCIEncoding = 2002 }; // AL16UTF16LE
#else
enum { QOCIEncoding = 2000 }; // AL16UTF16
#endif

static const ub1 CSID_NCHAR = SQLCS_NCHAR;
static const ub2 qOraCharset = OCI_UCS2ID;

typedef QVarLengthArray<sb2, 32> IndicatorArray;
typedef QVarLengthArray<ub2, 32> SizeArray;

static QByteArray qMakeOraDate(const QDateTime& dt);
static QDateTime qMakeDate(const char* oraDate);
static QString qOraWarn(const QOCIPrivate* d);
#ifndef Q_CC_SUN
static // for some reason, Sun CC can't use qOraWarning when it's declared static
#endif
void qOraWarning(const char* msg, const QOCIPrivate* d);
static QSqlError qMakeError(const QString& err, QSqlError::ErrorType type, const QOCIPrivate* p);

class QOCIRowId: public QSharedData
{
public:
    QOCIRowId(OCIEnv *env);
    ~QOCIRowId();

    OCIRowid *id;

private:
    QOCIRowId(const QOCIRowId &other): QSharedData(other) { Q_ASSERT(false); }
};

QOCIRowId::QOCIRowId(OCIEnv *env)
    : id(0)
{
    OCIDescriptorAlloc ((dvoid *) env, (dvoid **) &id, (ub4) OCI_DTYPE_ROWID,
                        (size_t) 0, (dvoid **) 0);
}

QOCIRowId::~QOCIRowId()
{
    if (id)
        OCIDescriptorFree((dvoid *)id, (ub4) OCI_DTYPE_ROWID);
}

typedef QSharedDataPointer<QOCIRowId> QOCIRowIdPointer;
Q_DECLARE_METATYPE(QOCIRowIdPointer)

class QOCIPrivate
{
public:
    QOCIPrivate();
    ~QOCIPrivate();

    QOCIResult *q;
    OCIEnv *env;
    OCIError *err;
    OCISvcCtx *svc;
#ifdef QOCI_UNICODE_API
    OCIServer *srvhp;
    OCISession *authp;
#endif
    OCIStmt *sql;
    bool transaction;
    int serverVersion;
    QString user;
    int prefetchRows, prefetchMem;

    void setCharset(OCIBind* hbnd);
    void setStatementAttributes();
    int bindValue(OCIStmt *sql, OCIBind **hbnd, OCIError *err, int pos,
                  const QVariant &val, dvoid *indPtr, ub2 *tmpSize, QList<QByteArray> &tmpStorage);
    int bindValues(QVector<QVariant> &values, IndicatorArray &indicators, SizeArray &tmpSizes,
                   QList<QByteArray> &tmpStorage);
    void outValues(QVector<QVariant> &values, IndicatorArray &indicators,
                   QList<QByteArray> &tmpStorage);
    inline bool isOutValue(int i) const
    { return q->bindValueType(i) & QSql::Out; }
    inline bool isBinaryValue(int i) const
    { return q->bindValueType(i) & QSql::Binary; }
};

QOCIPrivate::QOCIPrivate(): q(0), env(0), err(0), svc(0),
#ifdef QOCI_UNICODE_API
        srvhp(0), authp(0),
#endif
        sql(0), transaction(false), serverVersion(-1), prefetchRows(-1),
        prefetchMem(QOCI_PREFETCH_MEM)
{
}

QOCIPrivate::~QOCIPrivate()
{
}

void QOCIPrivate::setStatementAttributes()
{
    Q_ASSERT(sql);

    int r = 0;

    if (prefetchRows >= 0) {
        r = OCIAttrSet((void*)sql,
                       OCI_HTYPE_STMT,
                       (void*) &prefetchRows,
                       (ub4) 0,
                       (ub4) OCI_ATTR_PREFETCH_ROWS,
                       err);
        if (r != 0)
            qOraWarning("QOCIPrivate::setStatementAttributes:"
                        " Couldn't set OCI_ATTR_PREFETCH_ROWS: ", this);
    }
    if (prefetchMem >= 0) {
        r = OCIAttrSet((void*)sql,
                       OCI_HTYPE_STMT,
                       (void*) &prefetchMem,
                       (ub4) 0,
                       (ub4) OCI_ATTR_PREFETCH_MEMORY,
                       err);
        if (r != 0)
            qOraWarning("QOCIPrivate::setStatementAttributes:"
                        " Couldn't set OCI_ATTR_PREFETCH_MEMORY: ", this);
    }
}

void QOCIPrivate::setCharset(OCIBind* hbnd)
{
    int r = 0;

    Q_ASSERT(hbnd);

    r = OCIAttrSet((void*)hbnd,
                    OCI_HTYPE_BIND,
                    (void*) &qOraCharset,
                    (ub4) 0,
                    (ub4) OCI_ATTR_CHARSET_ID,
                    err);
    if (r != 0)
        qOraWarning("QOCIPrivate::setCharset: Couldn't set OCI_ATTR_CHARSET_ID: ", this);
}

int QOCIPrivate::bindValue(OCIStmt *sql, OCIBind **hbnd, OCIError *err, int pos,
                   const QVariant &val, dvoid *indPtr, ub2 *tmpSize, QList<QByteArray> &tmpStorage)
{
    int r = OCI_SUCCESS;
    void *data = const_cast<void *>(val.constData());

    switch (val.type()) {
    case QVariant::ByteArray:
        r = OCIBindByPos(sql, hbnd, err,
                         pos + 1,
                         (dvoid *) ((QByteArray*)data)->constData(),
                         ((QByteArray*)data)->size(),
                         SQLT_BIN, indPtr, (ub2 *) 0, (ub2 *) 0,
                         (ub4) 0, (ub4 *) 0, OCI_DEFAULT);
        break;
    case QVariant::Time:
    case QVariant::Date:
    case QVariant::DateTime: {
        QByteArray ba = qMakeOraDate(val.toDateTime());
        r = OCIBindByPos(sql, hbnd, err,
                         pos + 1,
                         (dvoid *) ba.constData(),
                         ba.size(),
                         SQLT_DAT, indPtr, (ub2 *) 0, (ub2*) 0,
                         (ub4) 0, (ub4 *) 0, OCI_DEFAULT);
        tmpStorage.append(ba);
        break; }
    case QVariant::Int:
        r = OCIBindByPos(sql, hbnd, err,
                         pos + 1,
                         (dvoid *) data,
                         sizeof(int),
                         SQLT_INT, indPtr, (ub2 *) 0, (ub2*) 0,
                         (ub4) 0, (ub4 *) 0, OCI_DEFAULT);
        break;
    case QVariant::UInt:
        r = OCIBindByPos(sql, hbnd, err,
                         pos + 1,
                         (dvoid *) data,
                         sizeof(uint),
                         SQLT_UIN, indPtr, (ub2 *) 0, (ub2*) 0,
                         (ub4) 0, (ub4 *) 0, OCI_DEFAULT);
        break;
    case QVariant::Double:
        r = OCIBindByPos(sql, hbnd, err,
                         pos + 1,
                         (dvoid *) data,
                         sizeof(double),
                         SQLT_FLT, indPtr, (ub2 *) 0, (ub2*) 0,
                         (ub4) 0, (ub4 *) 0, OCI_DEFAULT);
        break;
    case QVariant::UserType:
        if (qVariantCanConvert<QOCIRowIdPointer>(val) && !isOutValue(pos)) {
            // use a const pointer to prevent a detach
            const QOCIRowIdPointer rptr = qVariantValue<QOCIRowIdPointer>(val);
            r = OCIBindByPos(sql, hbnd, err,
                             pos + 1,
                             // it's an IN value, so const_cast is ok
                             const_cast<OCIRowid **>(&rptr->id),
                             -1,
                             SQLT_RDD, indPtr, (ub2 *) 0, (ub2 *) 0,
                             (ub4) 0, (ub4 *) 0, OCI_DEFAULT);
        } else {
            qWarning("Unknown bind variable");
            r = OCI_ERROR;
        }
        break;
    case QVariant::String: {
        const QString s = val.toString();
        if (isBinaryValue(pos)) {
            r = OCIBindByPos(sql, hbnd, err,
                             pos + 1,
                             (dvoid *) s.utf16(),
                             s.length() * sizeof(QChar),
                             SQLT_LNG, indPtr, (ub2 *) 0, (ub2*) 0,
                             (ub4) 0, (ub4 *) 0, OCI_DEFAULT);
            break;
        } else if (!isOutValue(pos)) {
            // don't detach the string
            r = OCIBindByPos(sql, hbnd, err,
                             pos + 1,
                             (dvoid *)s.utf16(),
                             (s.length() + 1) * sizeof(QChar),
                             SQLT_STR, indPtr, (ub2 *) 0, (ub2*) 0,
                             (ub4) 0, (ub4 *) 0, OCI_DEFAULT);
            break;
        }
    } // fall through for OUT values
    default: {
        const QString s = val.toString();
        // create a deep-copy
        QByteArray ba((char*)s.utf16(), (s.length() + 1) * sizeof(QChar));
        if (isOutValue(pos)) {
            ba.reserve((s.capacity() + 1) * sizeof(QChar));
            *tmpSize = ba.size();
            r = OCIBindByPos(sql, hbnd, err,
                             pos + 1,
                             (dvoid *)ba.constData(),
                             ba.capacity(),
                             SQLT_STR, indPtr, tmpSize, (ub2*) 0,
                             (ub4) 0, (ub4 *) 0, OCI_DEFAULT);
        } else {
            r = OCIBindByPos(sql, hbnd, err,
                             pos + 1,
                             (dvoid *)ba.constData(),
                             ba.size(),
                             SQLT_STR, indPtr, (ub2 *) 0, (ub2*) 0,
                             (ub4) 0, (ub4 *) 0, OCI_DEFAULT);
        }
        if (r == OCI_SUCCESS)
            setCharset(*hbnd);
            tmpStorage.append(ba);
            break; }
    }
    if (r != OCI_SUCCESS)
        qOraWarning("QOCIPrivate::bindValue:", this);
    return r;
}

int QOCIPrivate::bindValues(QVector<QVariant> &values, IndicatorArray &indicators,
                            SizeArray &tmpSizes, QList<QByteArray> &tmpStorage)
{
    int r = OCI_SUCCESS;
    for (int i = 0; i < values.count(); ++i) {
        if (isOutValue(i))
            values[i].detach();
        const QVariant &val = values.at(i);

        OCIBind * hbnd = 0; // Oracle handles these automatically
        sb2 *indPtr = &indicators[i];
        *indPtr = val.isNull() ? -1 : 0;

        bindValue(sql, &hbnd, err, i, val, indPtr, &tmpSizes[i], tmpStorage);
    }
    return r;
}

// will assign out value and remove its temp storage.
static void qOraOutValue(QVariant &value, QList<QByteArray> &storage)
{
    switch (value.type()) {
    case QVariant::Time:
        value = qMakeDate(storage.takeFirst()).time();
        break;
    case QVariant::Date:
        value = qMakeDate(storage.takeFirst()).date();
        break;
    case QVariant::DateTime:
        value = qMakeDate(storage.takeFirst());
        break;
    case QVariant::String:
        value = QString::fromUtf16(
                reinterpret_cast<const ushort *>(storage.takeFirst().constData()));
        break;
    default:
        break; //nothing
    }
}

void QOCIPrivate::outValues(QVector<QVariant> &values, IndicatorArray &indicators,
                            QList<QByteArray> &tmpStorage)
{
    for (int i = 0; i < values.count(); ++i) {

        if (!isOutValue(i))
            continue;

        qOraOutValue(values[i], tmpStorage);

        QVariant::Type typ = values.at(i).type();
        if (indicators[i] == -1) // NULL
            values[i] = QVariant(typ);
        else
            values[i] = QVariant(typ, values.at(i).constData());
    }
}

struct OraFieldInfo
{
    QString name;
    QVariant::Type type;
    ub1 oraIsNull;
    ub4 oraType;
    sb1 oraScale;
    ub4 oraLength; // size in bytes
    ub4 oraFieldLength; // amount of characters
    sb2 oraPrecision;
};

QString qOraWarn(const QOCIPrivate* d)
{
    sb4 errcode;
    text errbuf[1024];
    errbuf[0] = 0;
    errbuf[1] = 0;

    OCIErrorGet((dvoid *)d->err,
                (ub4) 1,
                (text *)NULL,
                &errcode,
                errbuf,
                (ub4)(sizeof(errbuf)),
                OCI_HTYPE_ERROR);
#ifdef QOCI_UNICODE_API
    return QString::fromUtf16((const unsigned short *)errbuf);
#else
    return QString::fromLocal8Bit((const char *)errbuf);
#endif
}

void qOraWarning(const char* msg, const QOCIPrivate* d)
{
    qWarning("%s %s", msg, qOraWarn(d).toLocal8Bit().constData());
}

static int qOraErrorNumber(const QOCIPrivate* d)
{
    sb4 errcode;
    OCIErrorGet((dvoid *)d->err,
                (ub4) 1,
                (text *) NULL,
                &errcode,
                NULL,
                0,
                OCI_HTYPE_ERROR);
    return errcode;
}

QSqlError qMakeError(const QString& err, QSqlError::ErrorType type, const QOCIPrivate* p)
{
    return QSqlError(QLatin1String("QOCI: ") + err, qOraWarn(p), type, qOraErrorNumber(p));
}

static QVariant::Type qDecodeOCIType(const QString& ocitype, int ocilen, int ociprec, int ociscale)
{
    QVariant::Type type = QVariant::Invalid;
    if (ocitype == QLatin1String("VARCHAR2") || ocitype == QLatin1String("VARCHAR")
         || ocitype.startsWith(QLatin1String("INTERVAL"))
         || ocitype == QLatin1String("CHAR") || ocitype == QLatin1String("NVARCHAR2")
         || ocitype == QLatin1String("NCHAR"))
        type = QVariant::String;
    else if (ocitype == QLatin1String("NUMBER"))
        type = QVariant::Int;
    else if (ocitype == QLatin1String("FLOAT"))
        type = QVariant::Double;
    else if (ocitype == QLatin1String("LONG") || ocitype == QLatin1String("NCLOB")
             || ocitype == QLatin1String("CLOB"))
        type = QVariant::ByteArray;
    else if (ocitype == QLatin1String("RAW") || ocitype == QLatin1String("LONG RAW")
             || ocitype == QLatin1String("ROWID") || ocitype == QLatin1String("BLOB")
             || ocitype == QLatin1String("CFILE") || ocitype == QLatin1String("BFILE"))
        type = QVariant::ByteArray;
    else if (ocitype == QLatin1String("DATE") ||  ocitype.startsWith(QLatin1String("TIME")))
        type = QVariant::DateTime;
    else if (ocitype == QLatin1String("UNDEFINED"))
        type = QVariant::Invalid;
    if (type == QVariant::Int) {
        if (ocilen == 22 && ociprec == 0 && ociscale == 0)
            type = QVariant::Double;
        if (ociscale > 0)
            type = QVariant::Double;
    }
    if (type == QVariant::Invalid)
        qWarning("qDecodeOCIType: unknown type: %s", ocitype.toLocal8Bit().constData());
    return type;
}

static QVariant::Type qDecodeOCIType(int ocitype)
{
    QVariant::Type type = QVariant::Invalid;
    switch (ocitype) {
    case SQLT_STR:
    case SQLT_VST:
    case SQLT_CHR:
    case SQLT_AFC:
    case SQLT_VCS:
    case SQLT_AVC:
    case SQLT_RDD:
    case SQLT_LNG:
#ifdef SQLT_INTERVAL_YM
    case SQLT_INTERVAL_YM:
#endif
#ifdef SQLT_INTERVAL_DS
    case SQLT_INTERVAL_DS:
#endif
        type = QVariant::String;
        break;
    case SQLT_INT:
        type = QVariant::Int;
        break;
    case SQLT_FLT:
    case SQLT_NUM:
    case SQLT_VNU:
    case SQLT_UIN:
        type = QVariant::String;
        break;
    case SQLT_VBI:
    case SQLT_BIN:
    case SQLT_LBI:
    case SQLT_LVC:
    case SQLT_LVB:
    case SQLT_BLOB:
    case SQLT_FILE:
    case SQLT_NTY:
    case SQLT_REF:
    case SQLT_RID:
    case SQLT_CLOB:
        type = QVariant::ByteArray;
        break;
    case SQLT_DAT:
    case SQLT_ODT:
#ifdef SQLT_TIMESTAMP
    case SQLT_TIMESTAMP:
    case SQLT_TIMESTAMP_TZ:
    case SQLT_TIMESTAMP_LTZ:
#endif
        type = QVariant::DateTime;
        break;
    default:
        type = QVariant::Invalid;
        qWarning("qDecodeOCIType: unknown OCI datatype: %d", ocitype);
        break;
    }
        return type;
}

static QSqlField qFromOraInf(const OraFieldInfo &ofi)
{
    QSqlField f(ofi.name, ofi.type);
    f.setRequired(ofi.oraIsNull == 0);
    f.setLength(ofi.oraPrecision == 0 ? 38 : int(ofi.oraPrecision));
    f.setPrecision(ofi.oraScale);
    f.setSqlType(int(ofi.oraType));
    return f;
}

static OraFieldInfo qMakeOraField(const QOCIPrivate* p, OCIParam* param)
{
    OraFieldInfo ofi;
    ub2 colType(0);
    text *colName = 0;
    ub4 colNameLen(0);
    sb1 colScale(0);
    ub2 colLength(0);
    ub2 colFieldLength(0);
    sb2 colPrecision(0);
    ub1 colIsNull(0);
    int r(0);
    QVariant::Type type(QVariant::Invalid);

    r = OCIAttrGet((dvoid*)param,
                    OCI_DTYPE_PARAM,
                    &colType,
                    0,
                    OCI_ATTR_DATA_TYPE,
                    p->err);
    if (r != 0)
        qOraWarning("qMakeOraField:", p);

    r = OCIAttrGet((dvoid*) param,
                    OCI_DTYPE_PARAM,
                    (dvoid**) &colName,
                    (ub4 *) &colNameLen,
                    (ub4) OCI_ATTR_NAME,
                    p->err);
    if (r != 0)
        qOraWarning("qMakeOraField:", p);

    r = OCIAttrGet((dvoid*) param,
    OCI_DTYPE_PARAM,
                    &colLength,
                    0,
                    OCI_ATTR_DATA_SIZE, /* in bytes */
                    p->err);
    if (r != 0)
        qOraWarning("qMakeOraField:", p);

#ifdef OCI_ATTR_CHAR_SIZE
    r = OCIAttrGet((dvoid*) param,
                    OCI_DTYPE_PARAM,
                    &colFieldLength,
                    0,
                    OCI_ATTR_CHAR_SIZE,
                    p->err);
    if (r != 0)
        qOraWarning("qMakeOraField:", p);
#else
    // for Oracle8.
    colFieldLength = colLength;
#endif

    r = OCIAttrGet((dvoid*) param,
                    OCI_DTYPE_PARAM,
                    &colPrecision,
                    0,
                    OCI_ATTR_PRECISION,
                    p->err);
    if (r != 0)
        qOraWarning("qMakeOraField:", p);

    r = OCIAttrGet((dvoid*) param,
                    OCI_DTYPE_PARAM,
                    &colScale,
                    0,
                    OCI_ATTR_SCALE,
                    p->err);
    if (r != 0)
        qOraWarning("qMakeOraField:", p);
    r = OCIAttrGet((dvoid*)param,
                    OCI_DTYPE_PARAM,
                    (dvoid*)&colType,
                    0,
                    OCI_ATTR_DATA_TYPE,
                    p->err);
    if (r != 0)
        qOraWarning("qMakeOraField:", p);
    r = OCIAttrGet((dvoid*)param,
                    OCI_DTYPE_PARAM,
                    (dvoid*)&colIsNull,
                    0,
                    OCI_ATTR_IS_NULL,
                    p->err);
    if (r != 0)
        qOraWarning("qMakeOraField:", p);

    type = qDecodeOCIType(colType);

    if (type == QVariant::Int) {
        if (colLength == 22 && colPrecision == 0 && colScale == 0)
            type = QVariant::String;
        if (colScale > 0)
            type = QVariant::String;
    }
    if (colType == SQLT_BLOB)
        colLength = 0;

    // colNameLen is length in bytes
#ifdef QOCI_UNICODE_API
    ofi.name = QString((const QChar*)colName, colNameLen / 2);
#else
    ofi.name = QString::fromLocal8Bit(reinterpret_cast<char *>(colName), colNameLen);
#endif
    ofi.type = type;
    ofi.oraType = colType;
    ofi.oraFieldLength = colFieldLength;
    ofi.oraLength = colLength;
    ofi.oraScale = colScale;
    ofi.oraPrecision = colPrecision;
    ofi.oraIsNull = colIsNull;

    return ofi;
}


/*!
    \internal

    Convert QDateTime to the internal Oracle DATE format NB!
    It does not handle BCE dates.
*/
QByteArray qMakeOraDate(const QDateTime& dt)
{
    QByteArray ba;
    ba.resize(7);
    int year = dt.date().year();
    ba[0]= (year / 100) + 100; // century
    ba[1]= (year % 100) + 100; // year
    ba[2]= dt.date().month();
    ba[3]= dt.date().day();
    ba[4]= dt.time().hour() + 1;
    ba[5]= dt.time().minute() + 1;
    ba[6]= dt.time().second() + 1;
    return ba;
}

QDateTime qMakeDate(const char* oraDate)
{
    int century = oraDate[0];
    if(century >= 100){
        int year    = (unsigned char)oraDate[1];
        year = ((century-100)*100) + (year-100);
        int month = oraDate[2];
        int day   = oraDate[3];
        int hour  = oraDate[4] - 1;
        int min   = oraDate[5] - 1;
        int sec   = oraDate[6] - 1;
        return QDateTime(QDate(year,month,day), QTime(hour,min,sec));
    }
    return QDateTime();
}

class QOCIResultPrivate
{
public:
    QOCIResultPrivate(int size, QOCIPrivate* dp);
    ~QOCIResultPrivate();
    void setCharset(OCIDefine* dfn);
    int readPiecewise(QVector<QVariant> &values, int index = 0);
    int readLOBs(QVector<QVariant> &values, int index = 0);
    int fieldFromDefine(OCIDefine* d);
    void getValues(QVector<QVariant> &v, int index);
    inline int size() { return fieldInf.size(); }
    static bool execBatch(QOCIPrivate *d, QVector<QVariant> &boundValues, bool arrayBind);

    QSqlRecord rec;

private:
    char* create(int position, int size);
    OCILobLocator ** createLobLocator(int position, OCIEnv* env);

    class OraFieldInf
    {
    public:
        OraFieldInf(): data(0), len(0), ind(0), typ(QVariant::Invalid), oraType(0), def(0), lob(0)
        {}
        ~OraFieldInf();
        char *data;
        int len;
        sb2 ind;
        QVariant::Type typ;
        ub4 oraType;
        OCIDefine *def;
        OCILobLocator *lob;
    };

    QVector<OraFieldInf> fieldInf;
    QOCIPrivate* d;
};

QOCIResultPrivate::OraFieldInf::~OraFieldInf()
{
    delete [] data;
    if (lob) {
        int r = OCIDescriptorFree(lob, (ub4) OCI_DTYPE_LOB);
        if (r != 0)
            qWarning("QOCIResultPrivate: Cannot free LOB descriptor");
    }
}

QOCIResultPrivate::QOCIResultPrivate(int size, QOCIPrivate* dp)
    : fieldInf(size), d(dp)
{
    ub4 dataSize = 0;
    OCIDefine* dfn = 0;
    int r;

    OCIParam* param = 0;
    sb4 parmStatus = 0;
    ub4 count = 1;
    int idx = 0;
    parmStatus = OCIParamGet(d->sql,
                              OCI_HTYPE_STMT,
                              d->err,
                              (void**)&param,
                              count);

    while (parmStatus == OCI_SUCCESS) {
        OraFieldInfo ofi = qMakeOraField(d, param);
        if (ofi.oraType == SQLT_RDD)
            dataSize = 50;
#ifdef SQLT_INTERVAL_YM
#ifdef SQLT_INTERVAL_DS
        else if (ofi.oraType == SQLT_INTERVAL_YM || ofi.oraType == SQLT_INTERVAL_DS)
            // since we are binding interval datatype as string,
            // we are not interested in the number of bytes but characters.
            dataSize = 50;  // magic number
#endif //SQLT_INTERVAL_DS
#endif //SQLT_INTERVAL_YM
        else if (ofi.oraType == SQLT_NUM || ofi.oraType == SQLT_VNU){
            if (ofi.oraPrecision > 0)
                dataSize = (ofi.oraPrecision + 1) * sizeof(utext);
            else
                dataSize = (38 + 1) * sizeof(utext);
        }
        else
            dataSize = ofi.oraLength;

        fieldInf[idx].typ = ofi.type;
        fieldInf[idx].oraType = ofi.oraType;
        rec.append(qFromOraInf(ofi));

        switch (ofi.type) {
        case QVariant::DateTime:
            r = OCIDefineByPos(d->sql,
                               &dfn,
                               d->err,
                               count,
                               create(idx, dataSize+1),
                               dataSize+1,
                               SQLT_DAT,
                               (dvoid *) &(fieldInf[idx].ind),
                               0, 0, OCI_DEFAULT);
            break;
        case QVariant::ByteArray:
            // RAW and LONG RAW fields can't be bound to LOB locators
            if (ofi.oraType == SQLT_BIN) {
//                                qDebug("binding SQLT_BIN");
                r = OCIDefineByPos(d->sql,
                                   &dfn,
                                   d->err,
                                   count,
                                   create(idx, dataSize),
                                   dataSize,
                                   SQLT_BIN,
                                   (dvoid *) &(fieldInf[idx].ind),
                                   0, 0, OCI_DYNAMIC_FETCH);
            } else if (ofi.oraType == SQLT_LBI) {
//                                    qDebug("binding SQLT_LBI");
                r = OCIDefineByPos(d->sql,
                                    &dfn,
                                    d->err,
                                    count,
                                    0,
                                    SB4MAXVAL,
                                    SQLT_LBI,
                                    (dvoid *) &(fieldInf[idx].ind),
                                    0, 0, OCI_DYNAMIC_FETCH);
            } else if (ofi.oraType == SQLT_CLOB) {
                r = OCIDefineByPos(d->sql, &dfn, d->err, count, createLobLocator(idx, d->env),
                                   (sb4)-1, SQLT_CLOB, (dvoid *) &(fieldInf[idx].ind), 0,
                                   0, OCI_DEFAULT);
            } else {
//                 qDebug("binding SQLT_BLOB");
                r = OCIDefineByPos(d->sql,
                                    &dfn,
                                    d->err,
                                    count,
                                    createLobLocator(idx, d->env),
                                    (sb4)-1,
                                    SQLT_BLOB,
                                    (dvoid *) &(fieldInf[idx].ind),
                                    0, 0, OCI_DEFAULT);
            }
            break;
        case QVariant::String:
            if (ofi.oraType == SQLT_LNG) {
                r = OCIDefineByPos(d->sql,
                        &dfn,
                        d->err,
                        count,
                        0,
                        SB4MAXVAL,
                        SQLT_LNG,
                        (dvoid *) &(fieldInf[idx].ind),
                        0, 0, OCI_DYNAMIC_FETCH);
            } else {
                dataSize += dataSize + sizeof(QChar);
                //qDebug("OCIDefineByPosStr(%d): %d", count, dataSize);
                r = OCIDefineByPos(d->sql,
                        &dfn,
                        d->err,
                        count,
                        create(idx, dataSize),
                        dataSize,
                        SQLT_STR,
                        (dvoid *) &(fieldInf[idx].ind),
                        0, 0, OCI_DEFAULT);
                if (r == 0)
                    setCharset(dfn);
            }
           break;
        default:
            // this should make enough space even with character encoding
            dataSize = (dataSize + 1) * sizeof(utext) ;
            //qDebug("OCIDefineByPosDef(%d): %d", count, dataSize);
            r = OCIDefineByPos(d->sql,
                                &dfn,
                                d->err,
                                count,
                                create(idx, dataSize),
                                dataSize+1,
                                SQLT_STR,
                                (dvoid *) &(fieldInf[idx].ind),
                                0, 0, OCI_DEFAULT);
            break;
        }
        if (r != 0)
            qOraWarning("QOCIResultPrivate::bind:", d);
        fieldInf[idx].def = dfn;
        ++count;
        ++idx;
        parmStatus = OCIParamGet(d->sql,
                                  OCI_HTYPE_STMT,
                                  d->err,
                                  (void**)&param,
                                  count);
    }
}

QOCIResultPrivate::~QOCIResultPrivate()
{
}

char* QOCIResultPrivate::create(int position, int size)
{
    char* c = new char[size+1];
    // Oracle may not fill fixed width fields
    memset(c, 0, size+1);
    fieldInf[position].data = c;
    fieldInf[position].len = size;
    return c;
}

OCILobLocator **QOCIResultPrivate::createLobLocator(int position, OCIEnv* env)
{
    OCILobLocator *& lob = fieldInf[position].lob;
    int r = OCIDescriptorAlloc((dvoid *)env,
                                (dvoid **)&lob,
                                (ub4)OCI_DTYPE_LOB,
                                (size_t) 0,
                                (dvoid **) 0);
    if (r != 0) {
        qWarning("QOCIResultPrivate: Cannot create LOB locator");
        lob = 0;
    }
    return &lob;
}

void QOCIResultPrivate::setCharset(OCIDefine* dfn)
{
    int r = 0;

    Q_ASSERT(dfn);

    r = OCIAttrSet((void*)dfn,
                   OCI_HTYPE_DEFINE,
                   (void*) &qOraCharset,
                   (ub4) 0,
                   (ub4) OCI_ATTR_CHARSET_ID,
                   d->err);
        if (r != 0)
            qOraWarning("QOCIResultPrivate::setCharset: Couldn't set OCI_ATTR_CHARSET_ID: ", d);
}

int QOCIResultPrivate::readPiecewise(QVector<QVariant> &values, int index)
{
    OCIDefine*     dfn;
    ub4            typep;
    ub1            in_outp;
    ub4            iterp;
    ub4            idxp;
    ub1            piecep;
    sword          status;
    text           col [QOCI_DYNAMIC_CHUNK_SIZE+1];
    int            fieldNum = -1;
    int            r = 0;
    bool           nullField;

    do {
        r = OCIStmtGetPieceInfo(d->sql, d->err, (dvoid**) &dfn, &typep,
                                 &in_outp, &iterp, &idxp, &piecep);
        if (r != OCI_SUCCESS)
            qOraWarning("OCIResultPrivate::readPiecewise: unable to get piece info:", d);
        fieldNum = fieldFromDefine(dfn);
        bool isStringField = fieldInf.at(fieldNum).oraType == SQLT_LNG;
        int chunkSize = QOCI_DYNAMIC_CHUNK_SIZE;
        nullField = false;
        r  = OCIStmtSetPieceInfo(dfn, OCI_HTYPE_DEFINE,
                                  d->err, (void *)col,
                                  (ub4 *)&chunkSize, piecep, NULL, NULL);
        if (r != OCI_SUCCESS)
            qOraWarning("OCIResultPrivate::readPiecewise: unable to set piece info:", d);
        status = OCIStmtFetch (d->sql, d->err, 1, OCI_FETCH_NEXT, OCI_DEFAULT);
        if (status == -1) {
            sb4 errcode;
            OCIErrorGet((dvoid *)d->err, (ub4) 1, (text *) NULL,
                        &errcode, NULL, 0,OCI_HTYPE_ERROR);
            switch (errcode) {
            case 1405: /* NULL */
                nullField = true;
                break;
            default:
                qOraWarning("OCIResultPrivate::readPiecewise: unable to fetch next:", d);
                break;
            }
        }
        if (status == OCI_NO_DATA)
            break;
        if (nullField || !chunkSize) {
            fieldInf[fieldNum].ind = -1;
        } else {
            if (isStringField) {
                QString str = values.at(fieldNum + index).toString();
                str += QString::fromUtf16((const unsigned short *)col, chunkSize / 2);
                values[fieldNum + index] = str;
                fieldInf[fieldNum].ind = 0;
            } else {
                QByteArray ba = values.at(fieldNum + index).toByteArray();
                int sz = ba.size();
                ba.resize(sz + chunkSize);
                memcpy(ba.data() + sz, (char*)col, chunkSize);
                values[fieldNum + index] = ba;
                fieldInf[fieldNum].ind = 0;
            }
        }
    } while (status == OCI_SUCCESS_WITH_INFO || status == OCI_NEED_DATA);
    return r;
}

struct QOCIBatchColumn
{
    inline QOCIBatchColumn()
        : bindh(0), bindAs(0), maxLen(0), recordCount(0),
          data(0), lengths(0), indicators(0), maxarr_len(0), curelep(0) {}

    OCIBind* bindh;
    ub2 bindAs;
    ub4 maxLen;
    ub4 recordCount;
    char* data;
    ub2* lengths;
    sb2* indicators;
    ub4 maxarr_len;
    ub4 curelep;
};

struct QOCIBatchCleanupHandler
{
    inline QOCIBatchCleanupHandler(QVector<QOCIBatchColumn> &columns)
        : col(columns) {}

    ~QOCIBatchCleanupHandler()
    {
        // deleting storage, length and indicator arrays
        for ( int j = 0; j < col.count(); ++j){
            delete[] col[j].lengths;
            delete[] col[j].indicators;
            delete[] col[j].data;
        }
    }

    QVector<QOCIBatchColumn> &col;
};

bool QOCIResultPrivate::execBatch(QOCIPrivate *d, QVector<QVariant> &boundValues, bool arrayBind)
{
    int columnCount = boundValues.count();
    if (boundValues.isEmpty() || columnCount == 0)
        return false;

#ifdef BATCH_DEBUG
    qDebug() << "columnCount:" << columnCount << boundValues;
#endif

    int i;
    sword r;

    QVarLengthArray<QVariant::Type> fieldTypes;
    for (i = 0; i < columnCount; ++i) {
        QVariant::Type tp = boundValues.at(i).type();
        fieldTypes.append(tp == QVariant::List ? boundValues.at(i).toList().value(0).type()
                                               : tp);
    }

    QList<QByteArray> tmpStorage;
    SizeArray tmpSizes(columnCount);
    QVector<QOCIBatchColumn> columns(columnCount);
    QOCIBatchCleanupHandler cleaner(columns);

    // figuring out buffer sizes
    for (i = 0; i < columnCount; ++i) {

        if (boundValues.at(i).type() != QVariant::List) {

            // not a list - create a deep-copy of the single value
            QOCIBatchColumn &singleCol = columns[i];
            singleCol.indicators = new sb2[1];
            *singleCol.indicators = boundValues.at(i).isNull() ? -1 : 0;

            r = d->bindValue(d->sql, &singleCol.bindh, d->err, i,
                             boundValues.at(i), singleCol.indicators, &tmpSizes[i], tmpStorage);

            if (r != OCI_SUCCESS && r != OCI_SUCCESS_WITH_INFO) {
                qOraWarning("QOCIPrivate::execBatch: unable to bind column:", d);
                d->q->setLastError(qMakeError(QCoreApplication::translate("QOCIResult",
                         "Unable to bind column for batch execute"), QSqlError::StatementError, d));
                return false;
            }
            continue;
        }

        QOCIBatchColumn &col = columns[i];
        col.recordCount = boundValues.at(i).toList().count();

        col.lengths = new ub2[col.recordCount];
        col.indicators = new sb2[col.recordCount];
        col.maxarr_len = col.recordCount;
        col.curelep = col.recordCount;

        switch (fieldTypes[i]) {
            case QVariant::Time:
            case QVariant::Date:
            case QVariant::DateTime:
                col.bindAs = SQLT_DAT;
                col.maxLen = 7;
                break;

            case QVariant::Int:
                col.bindAs = SQLT_INT;
                col.maxLen = sizeof(int);
                break;

            case QVariant::UInt:
                col.bindAs = SQLT_UIN;
                col.maxLen = sizeof(uint);
                break;

            case QVariant::Double:
                col.bindAs = SQLT_FLT;
                col.maxLen = sizeof(double);
                break;

            case QVariant::UserType:
                col.bindAs = SQLT_RDD;
                col.maxLen = sizeof(OCIRowid*);
                break;

            case QVariant::String: {
                col.bindAs = SQLT_STR;
                for (uint j = 0; j < col.recordCount; ++j) {
                    uint len = boundValues.at(i).toList().at(j).toString().length() + 1;
                    if (len > col.maxLen)
                        col.maxLen = len;
                }
#ifdef QOCI_UNICODE_API
                col.maxLen *= sizeof(QChar);
#endif
                break; }

            case QVariant::ByteArray:
            default: {
                col.bindAs = SQLT_LBI;
                for (uint j = 0; j < col.recordCount; ++j) {
                    col.lengths[j] = boundValues.at(i).toList().at(j).toByteArray().size();
                    if (col.lengths[j] > col.maxLen)
                        col.maxLen = col.lengths[j];
                }
                break; }
        }

        col.data = new char[col.maxLen * col.recordCount];
        memset(col.data, 0, col.maxLen * col.recordCount);

        // we may now populate column with data
        for (uint row = 0; row < col.recordCount; ++row) {
            const QVariant &val = boundValues.at(i).toList().at(row);

            if (val.isNull()){
                columns[i].indicators[row] = -1;
                columns[i].lengths[row] = 0;
            } else {
                columns[i].indicators[row] = 0;
                char *dataPtr = columns[i].data + (columns[i].maxLen * row);
                switch (fieldTypes[i]) {
                    case QVariant::Time:
                    case QVariant::Date:
                    case QVariant::DateTime:{
                        columns[i].lengths[row] = columns[i].maxLen;
                        const QByteArray ba = qMakeOraDate(val.toDateTime());
                        Q_ASSERT(ba.size() == int(columns[i].maxLen));
                        memcpy(dataPtr, ba.constData(), columns[i].maxLen);
                        break;
                    }
                    case QVariant::Int:
                        columns[i].lengths[row] = columns[i].maxLen;
                        *reinterpret_cast<int*>(dataPtr) = val.toInt();
                        break;

                    case QVariant::UInt:
                        columns[i].lengths[row] = columns[i].maxLen;
                        *reinterpret_cast<uint*>(dataPtr) = val.toUInt();
                        break;

                    case QVariant::Double:
                         columns[i].lengths[row] = columns[i].maxLen;
                         *reinterpret_cast<double*>(dataPtr) = val.toDouble();
                         break;

                    case QVariant::String: {
                        const QString s = val.toString();
                        columns[i].lengths[row] = (s.length() + 1) * sizeof(QChar);
                        memcpy(dataPtr, s.utf16(), columns[i].lengths[row]);
                        break;
                    }
                    case QVariant::UserType:
                        if (qVariantCanConvert<QOCIRowIdPointer>(val)) {
                            const QOCIRowIdPointer rptr = qVariantValue<QOCIRowIdPointer>(val);
                            *reinterpret_cast<OCIRowid**>(dataPtr) = rptr->id;
                            columns[i].lengths[row] = 0;
                            break;
                        }
                    case QVariant::ByteArray:
                    default: {
                        const QByteArray ba = val.toByteArray();
                        columns[i].lengths[row] = ba.size();
                        memcpy(dataPtr, ba.constData(), ba.size());
                        break;
                    }
                }
            }
        }

        QOCIBatchColumn &bindColumn = columns[i];

#ifdef BATCH_DEBUG
        qDebug("OCIBindByPos(%p, %p, %p, %d, %p, %d, %d, %p, %p, 0, %d, %p, OCI_DEFAULT)",
            d->sql, &bindColumn.bindh, d->err, i + 1, bindColumn.data,
            bindColumn.maxLen, bindColumn.bindAs, bindColumn.indicators, bindColumn.lengths,
            arrayBind ? bindColumn.maxarr_len : 0, arrayBind ? &bindColumn.curelep : 0);

        for (int ii = 0; ii < (int)bindColumn.recordCount; ++ii) {
            qDebug(" record %d: indicator %d, length %d", ii, bindColumn.indicators[ii],
                    bindColumn.lengths[ii]);
        }
#endif


        // binding the column
        r = OCIBindByPos(
                d->sql, &bindColumn.bindh, d->err, i + 1,
                (dvoid *) bindColumn.data,
                bindColumn.maxLen,
                bindColumn.bindAs,
                (dvoid *)bindColumn.indicators,
                (ub2 *) bindColumn.lengths,
                (ub2*) 0,
                arrayBind ? bindColumn.maxarr_len : 0,
                arrayBind ? &bindColumn.curelep : 0,
                OCI_DEFAULT);

#ifdef BATCH_DEBUG
        qDebug("After OCIBindByPos: r = %d, bindh = %p", r, bindColumn.bindh);
#endif

        if (r != OCI_SUCCESS && r != OCI_SUCCESS_WITH_INFO) {
            qOraWarning("QOCIPrivate::execBatch: unable to bind column:", d);
            d->q->setLastError(qMakeError(QCoreApplication::translate("QOCIResult",
                     "Unable to bind column for batch execute"), QSqlError::StatementError, d));
            return false;
        }

        r = OCIBindArrayOfStruct (
                columns[i].bindh, d->err,
                columns[i].maxLen,
                sizeof(columns[i].indicators[0]),
                sizeof(columns[i].lengths[0]),
                0);

        if (r != OCI_SUCCESS && r != OCI_SUCCESS_WITH_INFO) {
            qOraWarning("QOCIPrivate::execBatch: unable to bind column:", d);
            d->q->setLastError(qMakeError(QCoreApplication::translate("QOCIResult",
                     "Unable to bind column for batch execute"), QSqlError::StatementError, d));
            return false;
        }
    }

    //finaly we can execute
    r = OCIStmtExecute(d->svc, d->sql, d->err,
                       arrayBind ? 1 : columns[0].recordCount,
                       0, NULL, NULL,
                       d->transaction ? OCI_DEFAULT : OCI_COMMIT_ON_SUCCESS);

    if (r != OCI_SUCCESS && r != OCI_SUCCESS_WITH_INFO) {
        qOraWarning("QOCIPrivate::execBatch: unable to execute batch statement:", d);
        d->q->setLastError(qMakeError(QCoreApplication::translate("QOCIResult",
                        "Unable to execute batch statement"), QSqlError::StatementError, d));
        return false;
    }

    // for out parameters we copy data back to value vector
    for (i = 0; i < columnCount; ++i) {

        if (!d->isOutValue(i))
            continue;

        QVariant::Type tp = boundValues.at(i).type();
        if (tp != QVariant::List) {
            qOraOutValue(boundValues[i], tmpStorage);
            if (*columns[i].indicators == -1)
                boundValues[i] = QVariant(tp);
            continue;
        }

        QVariantList *list = static_cast<QVariantList *>(const_cast<void*>(boundValues.at(i).data()));

        char* data = columns[i].data;
        for (uint r = 0; r < columns[i].recordCount; ++r){

            if (columns[i].indicators[r] == -1) {
                (*list)[r] = QVariant();
                continue;
            }

            switch(columns[i].bindAs) {

                case SQLT_DAT:
                    (*list)[r] =  qMakeDate(data + r * columns[i].maxLen);
                    break;

                case SQLT_INT:
                    (*list)[r] =  *reinterpret_cast<int*>(data + r * columns[i].maxLen);
                    break;

                case SQLT_UIN:
                    (*list)[r] =  *reinterpret_cast<uint*>(data + r * columns[i].maxLen);
                    break;

                case SQLT_FLT:
                    (*list)[r] =  *reinterpret_cast<double*>(data + r * columns[i].maxLen);
                    break;

                case SQLT_STR:
                    (*list)[r] =  QString::fromUtf16((ushort*)(data + r * columns[i].maxLen));
                    break;

                default:
                    (*list)[r] =  QByteArray(data + r * columns[i].maxLen, columns[i].maxLen);
                break;
            }
        }
    }

    d->q->setSelect(false);
    d->q->setAt(QSql::BeforeFirstRow);
    d->q->setActive(true);

    return true;
}

#ifndef Q_CC_SUN
static // for some reason, Sun CC can't use qInitialLobSize when it's declared static
#endif
int qInitialLobSize(QOCIPrivate *d, OCILobLocator *lob)
{
    ub4 i;
    int r = OCILobGetChunkSize(d->svc, d->err, lob, &i);
    if (r != OCI_SUCCESS) {
        qOraWarning("OCIResultPrivate::readLobs: Couldn't get LOB chunk size: ", d);
        i = 1024 * 10;
    }
    return i;
}

template<class T, int sz>
int qReadLob(T &buf, QOCIPrivate *d, OCILobLocator *lob)
{
    ub4 read = 0;
    int r;

    buf.resize(qInitialLobSize(d, lob));

    while (true) {
        ub4 amount = ub4(-1); // read maximum amount of data
        r = OCILobRead(d->svc, d->err, lob, &amount, read + 1, (char*)(buf.constData()) + read * sz,
                (buf.size() - read) * sz, 0, 0, sz == 1 ? ub2(0) : ub2(QOCIEncoding), 0);

#ifdef QOCI_ORACLE10_WORKAROUND
        /* Hack, because Oracle suddently returns the amount in bytes when not reading from 0
           offset. */
        if (read && amount)
            amount /= sz;
#endif

        read += amount;
        if (r == OCI_NEED_DATA)
            buf.resize(buf.size() * 3);
        else
            break;
    }
    if (r == OCI_SUCCESS) {
        buf.resize(read);
    } else {
        qOraWarning("OCIResultPrivate::readLOBs: Cannot read LOB: ", d);
    }
    return r;
}

int QOCIResultPrivate::readLOBs(QVector<QVariant> &values, int index)
{
    OCILobLocator *lob;
    int r = OCI_SUCCESS;

    for (int i = 0; i < size(); ++i) {
        const OraFieldInf &fi = fieldInf.at(i);
        if (fi.ind == -1 || !(lob = fi.lob))
            continue;

        bool isClob = fi.oraType == SQLT_CLOB;
        QVariant var;

        if (isClob) {
            QString str;
            r = qReadLob<QString, sizeof(QChar)>(str, d, lob);
            var = str;
        } else {
            QByteArray buf;
            r = qReadLob<QByteArray, sizeof(char)>(buf, d, lob);
            var = buf;
        }
        if (r == OCI_SUCCESS)
            values[index + i] = var;
        else
            break;
    }
    return r;
}

int QOCIResultPrivate::fieldFromDefine(OCIDefine* d)
{
    for (int i = 0; i < fieldInf.count(); ++i) {
        if (fieldInf.at(i).def == d)
            return i;
    }
    return -1;
}

void QOCIResultPrivate::getValues(QVector<QVariant> &v, int index)
{
    for (int i = 0; i < fieldInf.size(); ++i) {
        const OraFieldInf &fld = fieldInf.at(i);

        if (fld.ind == -1) {
            // got a NULL value
            v[index + i] = QVariant(fld.typ);
            continue;
        }

        if (fld.oraType == SQLT_BIN || fld.oraType == SQLT_LBI || fld.oraType == SQLT_LNG)
            continue; // already fetched piecewise

        switch (fld.typ) {
        case QVariant::DateTime:
            v[index + i] = QVariant(qMakeDate(fld.data));
            break;
        case QVariant::String:
        case QVariant::Double: // when converted to strings
        case QVariant::Int:    // keep these as strings so that we do not lose precision
            v[index + i] = QVariant(QString::fromUtf16((const short unsigned int*)fld.data));
            break;
        case QVariant::ByteArray:
            if (fld.len > 0)
                v[index + i] = QByteArray(fld.data, fld.len);
            else
                v[index + i] = QVariant(QVariant::ByteArray);
            break;
        default:
            qWarning("QOCIResultPrivate::value: unknown data type");
            break;
        }
    }
}

////////////////////////////////////////////////////////////////////////////

QOCIResult::QOCIResult(const QOCIDriver * db, QOCIPrivate* p)
: QSqlCachedResult(db),
  cols(0)
{
    d = new QOCIPrivate();
    (*d) = (*p);
    d->q = this;
}

QOCIResult::~QOCIResult()
{
    if (d->sql) {
        int r = OCIHandleFree(d->sql, OCI_HTYPE_STMT);
        if (r != 0)
            qOraWarning("~QOCIResult: unable to free statement handle:", d);
    }
    delete d;
    delete cols;
}

QVariant QOCIResult::handle() const
{
    return qVariantFromValue(d->sql);
}

bool QOCIResult::reset (const QString& query)
{
    if (!prepare(query))
        return false;
    return exec();
}

bool QOCIResult::gotoNext(QSqlCachedResult::ValueCache &values, int index)
{
    if (at() == QSql::AfterLastRow)
        return false;

    bool piecewise = false;
    int r = OCI_SUCCESS;
    r = OCIStmtFetch(d->sql, d->err, 1, OCI_FETCH_NEXT, OCI_DEFAULT);

    if (index < 0) //not interested in values
        return r == OCI_SUCCESS || r == OCI_SUCCESS_WITH_INFO;

    switch (r) {
    case OCI_SUCCESS:
        break;
    case OCI_SUCCESS_WITH_INFO:
        qOraWarning("QOCIResult::gotoNext: SuccessWithInfo: ", d);
        r = OCI_SUCCESS; //ignore it
        break;
    case OCI_NO_DATA:
        // end of rowset
        return false;
    case OCI_NEED_DATA:
        piecewise = true;
        r = OCI_SUCCESS;
        break;
    case OCI_ERROR:
        if (qOraErrorNumber(d) == 1406) {
            qWarning("QOCI Warning: data truncated for %s", lastQuery().toLocal8Bit().constData());
            r = OCI_SUCCESS; /* ignore it */
            break;
        }
        // fall through
    default:
        qOraWarning("QOCIResult::gotoNext: ", d);
        setLastError(qMakeError(QCoreApplication::translate("QOCIResult",
                                "Unable to goto next"), QSqlError::StatementError, d));
        break;
    }

    // need to read piecewise before assigning values
    if (r == OCI_SUCCESS && piecewise)
        r = cols->readPiecewise(values, index);

    if (r == OCI_SUCCESS)
        cols->getValues(values, index);
    if (r == OCI_SUCCESS)
        r = cols->readLOBs(values, index);
    if (r != OCI_SUCCESS)
        setAt(QSql::AfterLastRow);
    return r == OCI_SUCCESS || r == OCI_SUCCESS_WITH_INFO;
}

int QOCIResult::size()
{
    return -1;
}

int QOCIResult::numRowsAffected()
{
    int rowCount;
    OCIAttrGet(d->sql,
                OCI_HTYPE_STMT,
                &rowCount,
                NULL,
                OCI_ATTR_ROW_COUNT,
                d->err);
    return rowCount;
}

bool QOCIResult::prepare(const QString& query)
{
    int r = 0;

    delete cols;
    cols = 0;
    QSqlCachedResult::cleanup();

    if (d->sql) {
        r = OCIHandleFree(d->sql, OCI_HTYPE_STMT);
        if (r != OCI_SUCCESS)
            qOraWarning("QOCIResult::prepare: unable to free statement handle:", d);
    }
    if (query.isEmpty())
        return false;
    r = OCIHandleAlloc((dvoid *) d->env,
                        (dvoid **) &d->sql,
                        OCI_HTYPE_STMT,
                        0,
                        0);
    if (r != OCI_SUCCESS) {
        qOraWarning("QOCIResult::prepare: unable to alloc statement:", d);
        setLastError(qMakeError(QCoreApplication::translate("QOCIResult",
                     "Unable to alloc statement"), QSqlError::StatementError, d));
        return false;
    }
    d->setStatementAttributes();
#ifdef QOCI_UNICODE_API
    const OraText *txt = (const OraText *)query.utf16();
    const int len = query.length() * sizeof(QChar);
#else
    const QByteArray tmp = query.toAscii();
    OraText *txt = (OraText *)tmp.constData();
    const int len = tmp.length();
#endif
    r = OCIStmtPrepare(d->sql,
                       d->err,
                       txt,
                       len,
                       OCI_NTV_SYNTAX,
                       OCI_DEFAULT);
    if (r != OCI_SUCCESS) {
        qOraWarning("QOCIResult::prepare: unable to prepare statement:", d);
        setLastError(qMakeError(QCoreApplication::translate("QOCIResult",
                                "Unable to prepare statement"), QSqlError::StatementError, d));
        return false;
    }
    return true;
}

bool QOCIResult::exec()
{
    int r = 0;
    ub2 stmtType;
    QList<QByteArray> tmpStorage;
    IndicatorArray indicators(boundValueCount());
    SizeArray tmpSizes(boundValueCount());

    // bind placeholders
    if (boundValueCount() > 0
         && d->bindValues(boundValues(), indicators, tmpSizes, tmpStorage) != OCI_SUCCESS) {
        qOraWarning("QOCIResult::exec: unable to bind value: ", d);
        setLastError(qMakeError(QCoreApplication::translate("QOCIResult", "Unable to bind value"),
                     QSqlError::StatementError, d));
        return false;
    }

    r = OCIAttrGet(d->sql,
                    OCI_HTYPE_STMT,
                    (dvoid*)&stmtType,
                    NULL,
                    OCI_ATTR_STMT_TYPE,
                    d->err);
    // execute
    if (stmtType == OCI_STMT_SELECT)
    {
        r = OCIStmtExecute(d->svc,
                            d->sql,
                            d->err,
                            0,
                            0,
                            (CONST OCISnapshot *) NULL,
                            (OCISnapshot *) NULL,
                            OCI_DEFAULT);
        if (r != OCI_SUCCESS && r != OCI_SUCCESS_WITH_INFO) {
            qOraWarning("QOCIResult::exec: unable to execute select statement:", d);
            setLastError(qMakeError(QCoreApplication::translate("QOCIResult",
                         "Unable to execute select statement"), QSqlError::StatementError, d));
            return false;
        }
        ub4 parmCount = 0;
        int r = OCIAttrGet(d->sql, OCI_HTYPE_STMT, (dvoid*)&parmCount, NULL, OCI_ATTR_PARAM_COUNT, d->err);
        if (r == 0 && !cols)
            cols = new QOCIResultPrivate(parmCount, d);
        setSelect(true);
        QSqlCachedResult::init(parmCount);
    } else { /* non-SELECT */
        r = OCIStmtExecute(d->svc, d->sql, d->err, 1,0,
                                (CONST OCISnapshot *) NULL,
                                (OCISnapshot *) NULL,
                                d->transaction ? OCI_DEFAULT : OCI_COMMIT_ON_SUCCESS );
        if (r != OCI_SUCCESS && r != OCI_SUCCESS_WITH_INFO) {
            qOraWarning("QOCIResult::exec: unable to execute statement:", d);
            setLastError(qMakeError(QCoreApplication::translate("QOCIResult",
                         "Unable to execute statement"), QSqlError::StatementError, d));
            return false;
        }
        setSelect(false);
    }
    setAt(QSql::BeforeFirstRow);
    setActive(true);

    if (hasOutValues())
        d->outValues(boundValues(), indicators, tmpStorage);

    return true;
}

QSqlRecord QOCIResult::record() const
{
    QSqlRecord inf;
    if (!isActive() || !isSelect() || !cols)
        return inf;
    return cols->rec;
}

QVariant QOCIResult::lastInsertId() const
{
    if (isActive()) {
        QOCIRowIdPointer ptr(new QOCIRowId(d->env));

        int r = OCIAttrGet((dvoid*) d->sql, OCI_HTYPE_STMT, ptr.constData()->id,
                           (ub4 *) 0, OCI_ATTR_ROWID, (OCIError *) d->err);
        if (r == OCI_SUCCESS)
            return qVariantFromValue(ptr);
    }
    return QVariant();
}

void QOCIResult::virtual_hook(int id, void *data)
{
    Q_ASSERT(id == QSqlResult::BatchOperation);
    Q_ASSERT(data);
    Q_UNUSED(id);

    bool *op = reinterpret_cast<bool *>(data);
    QOCIResultPrivate::execBatch(d, boundValues(), *op);
}

////////////////////////////////////////////////////////////////////////////


QOCIDriver::QOCIDriver(QObject* parent)
    : QSqlDriver(parent)
{
    d = new QOCIPrivate();

#ifdef QOCI_UNICODE_API
    static const ub4 mode = OCI_UTF16 | OCI_OBJECT;
#else
    static const ub4 mode = OCI_OBJECT;
#endif
    int r = OCIEnvCreate(&d->env,
                         mode,
                         NULL,
                         NULL,
                         NULL,
                         NULL,
                         0,
                         NULL);
    if (r != 0)
        qOraWarning("QOCIDriver: unable to create environment:", d);
    r = OCIHandleAlloc((dvoid *) d->env,
                        (dvoid **) &d->err,
                        OCI_HTYPE_ERROR,
                        (size_t) 0,
                        (dvoid **) 0);
    if (r != 0) {
        qOraWarning("QOCIDriver: unable to alloc error handle:", d);
        setLastError(qMakeError(tr("Unable to initialize", "QOCIDriver"),
                     QSqlError::ConnectionError, d));
    }
}

QOCIDriver::QOCIDriver(OCIEnv* env, OCISvcCtx* ctx, QObject* parent)
    : QSqlDriver(parent)
{
    d = new QOCIPrivate();
    d->env = env;
    d->svc = ctx;

    int r = OCIHandleAlloc((dvoid *) d->env,
                           (dvoid **) &d->err,
                           OCI_HTYPE_ERROR,
                           (size_t) 0,
                           (dvoid **) 0);
    if (r != 0)
        qOraWarning("QOCIDriver: unable to alloc error handle:", d);
    if (env && ctx) {
        setOpen(true);
        setOpenError(false);
    }
}

QOCIDriver::~QOCIDriver()
{
    if (isOpen())
        close();
    int r = OCIHandleFree((dvoid *) d->err, OCI_HTYPE_ERROR);
    if (r != OCI_SUCCESS)
        qWarning("Unable to free Error handle: %d", r);
    r = OCIHandleFree((dvoid *) d->env, OCI_HTYPE_ENV);
    if (r != OCI_SUCCESS)
        qWarning("Unable to free Environment handle: %d", r);

    delete d;
}

bool QOCIDriver::hasFeature(DriverFeature f) const
{
    switch (f) {
    case Transactions:
    case LastInsertId:
    case BLOB:
    case PreparedQueries:
    case NamedPlaceholders:
    case BatchOperations:
        return true;
    case QuerySize:
    case PositionalPlaceholders:
        return false;
    case Unicode:
        return d->serverVersion >= 9;
    }
    return false;
}

static void qParseOpts(const QString &options, QOCIPrivate *d)
{
    const QStringList opts(options.split(QLatin1Char(';'), QString::SkipEmptyParts));
    for (int i = 0; i < opts.count(); ++i) {
        const QString tmp(opts.at(i));
        int idx;
        if ((idx = tmp.indexOf(QLatin1Char('='))) == -1) {
            qWarning("QOCIDriver::parseArgs: Invalid parameter: '%s'",
                     tmp.toLocal8Bit().constData());
            continue;
        }
        const QString opt = tmp.left(idx);
        const QString val = tmp.mid(idx + 1).simplified();
        bool ok;
        if (opt == QLatin1String("OCI_ATTR_PREFETCH_ROWS")) {
            d->prefetchRows = val.toInt(&ok);
            if (!ok)
                d->prefetchRows = -1;
        } else if (opt == QLatin1String("OCI_ATTR_PREFETCH_MEMORY")) {
            d->prefetchMem = val.toInt(&ok);
            if (!ok)
                d->prefetchMem = -1;
        } else {
            qWarning ("QOCIDriver::parseArgs: Invalid parameter: '%s'",
                      opt.toLocal8Bit().constData());
        }
    }
}

bool QOCIDriver::open(const QString & db,
                       const QString & user,
                       const QString & password,
                       const QString & ,
                       int,
                       const QString &opts)
{
    int r;

    if (isOpen())
        close();

    qParseOpts(opts, d);

#ifdef QOCI_UNICODE_API
    r = OCIHandleAlloc(d->env, (dvoid **)&d->srvhp, OCI_HTYPE_SERVER, 0, 0);
    if (r == OCI_SUCCESS)
        r = OCIServerAttach(d->srvhp, d->err, reinterpret_cast<const OraText *>(db.utf16()),
                            db.length() * sizeof(QChar), OCI_DEFAULT);
    if (r == OCI_SUCCESS || r == OCI_SUCCESS_WITH_INFO)
        r = OCIHandleAlloc(d->env, (dvoid **)&d->svc, OCI_HTYPE_SVCCTX, 0, 0);
    if (r == OCI_SUCCESS)
        r = OCIAttrSet(d->svc, OCI_HTYPE_SVCCTX, d->srvhp, 0, OCI_ATTR_SERVER, d->err);
    if (r == OCI_SUCCESS)
        r = OCIHandleAlloc(d->env, (dvoid **)&d->authp, OCI_HTYPE_SESSION, 0, 0);
    if (r == OCI_SUCCESS)
        r = OCIAttrSet(d->authp, OCI_HTYPE_SESSION, (dvoid *)user.utf16(),
                       user.length() * sizeof(QChar), OCI_ATTR_USERNAME, d->err);
    if (r == OCI_SUCCESS)
        r = OCIAttrSet(d->authp, OCI_HTYPE_SESSION, (dvoid *)password.utf16(),
                       password.length() * sizeof(QChar), OCI_ATTR_PASSWORD, d->err);
    if (r == OCI_SUCCESS) {
        if (user.isEmpty() && password.isEmpty())
            r = OCISessionBegin(d->svc, d->err, d->authp, OCI_CRED_EXT, OCI_DEFAULT);
        else
            r = OCISessionBegin(d->svc, d->err, d->authp, OCI_CRED_RDBMS, OCI_DEFAULT);
    }
    if (r == OCI_SUCCESS || r == OCI_SUCCESS_WITH_INFO)
        r = OCIAttrSet(d->svc, OCI_HTYPE_SVCCTX, d->authp, 0, OCI_ATTR_SESSION, d->err);

    if (r != OCI_SUCCESS) {
        setLastError(qMakeError(tr("Unable to logon"), QSqlError::ConnectionError, d));
        setOpenError(true);
        if (d->authp)
            OCIHandleFree(d->authp, OCI_HTYPE_SESSION);
        d->authp = 0;
        if (d->srvhp)
            OCIHandleFree(d->srvhp, OCI_HTYPE_SERVER);
        d->srvhp = 0;
        return false;
    }
#else
    QByteArray tmpUser = user.toAscii();
    QByteArray tmpPassword = password.toAscii();
    QByteArray tmpDb = db.toAscii();
    r = OCILogon(d->env,
                 d->err,
                 &d->svc,
                 (OraText *)tmpUser.data(),
                 tmpUser.length(),
                 (OraText *)tmpPassword.data(),
                 tmpPassword.length(),
                 (OraText *)tmpDb.data(),
                 tmpDb.length());

    if (r != 0) {
        setLastError(qMakeError(tr("Unable to logon"), QSqlError::ConnectionError, d));
        setOpenError(true);
        OCIHandleFree((dvoid *) d->svc, OCI_HTYPE_SVCCTX);
        d->svc = 0;
        return false;
    }
#endif

    // get server version
    char vertxt[512];
    r = OCIServerVersion(d->svc,
                          d->err,
                          reinterpret_cast<OraText *>(vertxt),
                          sizeof(vertxt),
                          OCI_HTYPE_SVCCTX);
    if (r != 0) {
        qWarning("QOCIDriver::open: could not get Oracle server version.");
    } else {
        QString versionStr;
#ifdef QOCI_UNICODE_API
        versionStr = QString::fromUtf16(reinterpret_cast<unsigned short *>(vertxt));
#else
        versionStr = QString::fromLocal8Bit(static_cast<char *>(vertxt), sizeof(vertxt));
#endif
        QRegExp vers(QLatin1String("([0-9]+)\\.[0-9\\.]+[0-9]"));
        if (vers.indexIn(versionStr) >= 0)
            d->serverVersion = vers.cap(1).toInt();
        if (d->serverVersion == 0)
            d->serverVersion = -1;
    }

    setOpen(true);
    setOpenError(false);
    d->user = user.toUpper();

    return true;
}

void QOCIDriver::close()
{
    if (!isOpen())
        return;

#ifdef QOCI_UNICODE_API
    OCISessionEnd(d->svc, d->err, d->authp, OCI_DEFAULT);
    OCIHandleFree(d->authp, OCI_HTYPE_SESSION);
    d->authp = 0;
    OCIHandleFree(d->srvhp, OCI_HTYPE_SERVER);
    d->srvhp = 0;
    OCIHandleFree(d->svc, OCI_HTYPE_SVCCTX);
#else
    OCILogoff(d->svc, d->err); // will deallocate svc
#endif
    d->svc = 0;
    setOpen(false);
    setOpenError(false);
}

QSqlResult *QOCIDriver::createResult() const
{
    return new QOCIResult(this, d);
}

bool QOCIDriver::beginTransaction()
{
    if (!isOpen()) {
        qWarning("QOCIDriver::beginTransaction: Database not open");
        return false;
    }
    d->transaction = true;
    int r = OCITransStart (d->svc,
                            d->err,
                            2,
                            OCI_TRANS_READWRITE);
    if (r == OCI_ERROR) {
        qOraWarning("QOCIDriver::beginTransaction: ", d);
        return false;
    }
    return true;
}

bool QOCIDriver::commitTransaction()
{
    if (!isOpen()) {
        qWarning("QOCIDriver::commitTransaction: Database not open");
        return false;
    }
    d->transaction = false;
    int r = OCITransCommit (d->svc,
                             d->err,
                             0);
    if (r == OCI_ERROR) {
        qOraWarning("QOCIDriver::commitTransaction:", d);
        return false;
    }
    return true;
}

bool QOCIDriver::rollbackTransaction()
{
    if (!isOpen()) {
        qWarning("QOCIDriver::rollbackTransaction: Database not open");
        return false;
    }
    d->transaction = false;
    int r = OCITransRollback (d->svc,
                               d->err,
                               0);
    if (r == OCI_ERROR) {
        qOraWarning("QOCIDriver::rollbackTransaction:", d);
        return false;
    }
    return true;
}

QStringList QOCIDriver::tables(QSql::TableType type) const
{
    QStringList tl;
    if (!isOpen())
        return tl;

    QSqlQuery t(createResult());
    t.setForwardOnly(true);
    if (type & QSql::Tables) {
        t.exec(QLatin1String("select owner, table_name from all_tables "
                "where owner != 'MDSYS' "
                "and owner != 'LBACSYS' "
                "and owner != 'SYS' "
                "and owner != 'SYSTEM' "
                "and owner != 'WKSYS'"
                "and owner != 'CTXSYS'"
                "and owner != 'WMSYS'"));
        while (t.next()) {
            if (t.value(0).toString() != d->user)
                tl.append(t.value(0).toString() + QLatin1String(".") + t.value(1).toString());
            else
                tl.append(t.value(1).toString());
        }
    }
    if (type & QSql::Views) {
        t.exec(QLatin1String("select owner, view_name from all_views "
                "where owner != 'MDSYS' "
                "and owner != 'LBACSYS' "
                "and owner != 'SYS' "
                "and owner != 'SYSTEM' "
                "and owner != 'WKSYS'"
                "and owner != 'CTXSYS'"
                "and owner != 'WMSYS'"));
        while (t.next()) {
            if (t.value(0).toString() != d->user)
                tl.append(t.value(0).toString() + QLatin1String(".") + t.value(1).toString());
            else
                tl.append(t.value(1).toString());
        }
    }
    if (type & QSql::SystemTables) {
        t.exec(QLatin1String("select table_name from dictionary"));
        while (t.next()) {
            tl.append(t.value(0).toString());
        }
    }
    return tl;
}

void qSplitTableAndOwner(const QString & tname, QString * tbl,
                          QString * owner)
{
    int i = tname.indexOf(QLatin1Char('.')); // prefixed with owner?
    if (i != -1) {
        *tbl = tname.right(tname.length() - i - 1).toUpper();
        *owner = tname.left(i).toUpper();
    } else {
        *tbl = tname.toUpper();
    }
}

QSqlRecord QOCIDriver::record(const QString& tablename) const
{
    QSqlRecord fil;
    if (!isOpen())
        return fil;

    QSqlQuery t(createResult());
    // using two separate queries for this is A LOT faster than using
    // eg. a sub-query on the sys.synonyms table
    QString stmt(QLatin1String("select column_name, data_type, data_length, "
                  "data_precision, data_scale, nullable, data_default%1"
                  "from all_tab_columns "
                  "where upper(table_name)=%2"));
    if (d->serverVersion >= 9)
        stmt = stmt.arg(QLatin1String(", char_length "));
    else
        stmt = stmt.arg(QLatin1String(" "));
    bool buildRecordInfo = false;
    QString table, owner, tmpStmt;
    qSplitTableAndOwner(tablename, &table, &owner);
    tmpStmt = stmt.arg(QLatin1Char('\'') + table + QLatin1Char('\''));
    if (owner.isEmpty()) {
        owner = d->user;
    }
    tmpStmt += QLatin1String(" and upper(owner)='") + owner + QLatin1String("'");
    t.setForwardOnly(true);
    t.exec(tmpStmt);
    if (!t.next()) { // try and see if the tablename is a synonym
        stmt= stmt.arg(QLatin1String("(select tname from sys.synonyms where sname='")
                        + table + QLatin1String("' and creator=owner)"));
        t.setForwardOnly(true);
        t.exec(stmt);
        if (t.next())
            buildRecordInfo = true;
    } else {
        buildRecordInfo = true;
    }
    if (buildRecordInfo) {
        do {
            QVariant::Type ty = qDecodeOCIType(t.value(1).toString(), t.value(2).toInt(),
                            t.value(3).toInt(), t.value(4).toInt());
            QSqlField f(t.value(0).toString(), ty);
            f.setRequired(t.value(5).toString() == QLatin1String("N"));
            f.setPrecision(t.value(4).toInt());
            if (d->serverVersion >= 9 && (ty == QVariant::String) && !t.isNull(3)) {
                // Oracle9: data_length == size in bytes, char_length == amount of characters
                f.setLength(t.value(7).toInt());
            } else {
                f.setLength(t.value(t.isNull(3) ? 2 : 3).toInt());
            }
            f.setDefaultValue(t.value(6));
            fil.append(f);
        } while (t.next());
    }
    return fil;
}

QSqlIndex QOCIDriver::primaryIndex(const QString& tablename) const
{
    QSqlIndex idx(tablename);
    if (!isOpen())
        return idx;
    QSqlQuery t(createResult());
    QString stmt(QLatin1String("select b.column_name, b.index_name, a.table_name, a.owner "
                  "from all_constraints a, all_ind_columns b "
                  "where a.constraint_type='P' "
                  "and b.index_name = a.constraint_name "
                  "and b.index_owner = a.owner"));

    bool buildIndex = false;
    QString table, owner, tmpStmt;
    qSplitTableAndOwner(tablename, &table, &owner);
    tmpStmt = stmt + QLatin1String(" and upper(a.table_name)='") + table + QLatin1String("'");
    if (owner.isEmpty()) {
        owner = d->user;
    }
    tmpStmt += QLatin1String(" and upper(a.owner)='") + owner + QLatin1String("'");
    t.setForwardOnly(true);
    t.exec(tmpStmt);

    if (!t.next()) {
        stmt += QLatin1String(" and a.table_name=(select tname from sys.synonyms "
                "where sname='") + table + QLatin1String("' and creator=a.owner)");
        t.setForwardOnly(true);
        t.exec(stmt);
        if (t.next()) {
            owner = t.value(3).toString();
            buildIndex = true;
        }
    } else {
        buildIndex = true;
    }
    if (buildIndex) {
        QSqlQuery tt(createResult());
        tt.setForwardOnly(true);
        idx.setName(t.value(1).toString());
        do {
            tt.exec(QLatin1String("select data_type from all_tab_columns where table_name='") +
                     t.value(2).toString() + QLatin1String("' and column_name='") +
                     t.value(0).toString() + QLatin1String("' and owner='") +
                     owner +QLatin1String("'"));
            if (!tt.next()) {
                return QSqlIndex();
            }
            QSqlField f(t.value(0).toString(), qDecodeOCIType(tt.value(0).toString(), 0, 0, 0));
            idx.append(f);
        } while (t.next());
        return idx;
    }
    return QSqlIndex();
}

QString QOCIDriver::formatValue(const QSqlField &field, bool) const
{
    switch (field.type()) {
    case QVariant::String: {
        if (d->serverVersion >= 9) {
            QString encStr = QLatin1String("UNISTR('");
            const QString srcStr = field.value().toString();
            for (int i = 0; i < srcStr.length(); ++i) {
                encStr += QLatin1Char('\\') +
                          QString::number(srcStr.at(i).unicode(),
                                          16).rightJustified(4, QLatin1Char('0'));
            }
            encStr += QLatin1String("')");
            return encStr;
        } else {
            return QSqlDriver::formatValue(field);
        }
        break;
    }
    case QVariant::DateTime: {
        QDateTime datetime = field.value().toDateTime();
        QString datestring;
        if (datetime.isValid()) {
            datestring = QLatin1String("TO_DATE('") + QString::number(datetime.date().year())
                         + QLatin1Char('-')
                         + QString::number(datetime.date().month()) + QLatin1Char('-')
                         + QString::number(datetime.date().day()) + QLatin1Char(' ')
                         + QString::number(datetime.time().hour()) + QLatin1Char(':')
                         + QString::number(datetime.time().minute()) + QLatin1Char(':')
                         + QString::number(datetime.time().second())
                         + QLatin1String("','YYYY-MM-DD HH24:MI:SS')");
        } else {
            datestring = QLatin1String("NULL");
        }
        return datestring;
    }
    case QVariant::Time: {
        QDateTime datetime = field.value().toDateTime();
        QString datestring;
        if (datetime.isValid()) {
            datestring = QLatin1String("TO_DATE('")
                         + QString::number(datetime.time().hour()) + QLatin1Char(':')
                         + QString::number(datetime.time().minute()) + QLatin1Char(':')
                         + QString::number(datetime.time().second())
                         + QLatin1String("','HH24:MI:SS')");
        } else {
            datestring = QLatin1String("NULL");
        }
        return datestring;
    }
    case QVariant::Date: {
        QDate date = field.value().toDate();
        QString datestring;
        if (date.isValid()) {
            datestring = QLatin1String("TO_DATE('") + QString::number(date.year()) +
                         QLatin1Char('-') +
                         QString::number(date.month()) + QLatin1Char('-') +
                         QString::number(date.day()) + QLatin1String("','YYYY-MM-DD')");
        } else {
            datestring = QLatin1String("NULL");
        }
        return datestring;
    }
    default:
        break;
    }
    return QSqlDriver::formatValue(field);
}

QVariant QOCIDriver::handle() const
{
    return qVariantFromValue(d->env);
}

QString QOCIDriver::escapeIdentifier(const QString &identifier, IdentifierType type) const
{
    QString res = identifier;
    res.replace(QLatin1Char('"'), QLatin1String("\"\""));
    res.prepend(QLatin1Char('"')).append(QLatin1Char('"'));

    if (type == QSqlDriver::TableName)
        return res;

    res.replace(QLatin1Char('.'), QLatin1String("\".\""));
    return res;
}

