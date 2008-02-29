/****************************************************************************
**
** Copyright (C) 1992-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef Q3SQLFIELDINFO_H
#define Q3SQLFIELDINFO_H

#ifndef QT_NO_SQL

#include <QtCore/qglobal.h>
#include <QtSql/qsqlfield.h>

QT_BEGIN_HEADER

QT_MODULE(Qt3Support)

/* Q3SqlFieldInfo Class
   obsoleted, use QSqlField instead
*/

class Q_COMPAT_EXPORT Q3SqlFieldInfo
{
    // class is obsoleted, won't change anyways,
    // so no d pointer
    int req, len, prec, tID;
    uint gen: 1;
    uint trim: 1;
    uint calc: 1;
    QString nm;
    QVariant::Type typ;
    QVariant defValue;

public:
    Q3SqlFieldInfo(const QString& name = QString(),
                   QVariant::Type typ = QVariant::Invalid,
                   int required = -1,
                   int len = -1,
                   int prec = -1,
                   const QVariant& defValue = QVariant(),
                   int sqlType = 0,
                   bool generated = true,
                   bool trim = false,
                   bool calculated = false) :
        req(required), len(len), prec(prec), tID(sqlType),
        gen(generated), trim(trim), calc(calculated),
        nm(name), typ(typ), defValue(defValue) {}

    virtual ~Q3SqlFieldInfo() {}

    Q3SqlFieldInfo(const QSqlField & other)
    {
        nm = other.name();
        typ = other.type();
        switch (other.requiredStatus()) {
        case QSqlField::Unknown: req = -1; break;
        case QSqlField::Required: req = 1; break;
        case QSqlField::Optional: req = 0; break;
        }
        len = other.length();
        prec = other.precision();
        defValue = other.defaultValue();
        tID = other.typeID();
        gen = other.isGenerated();
        calc = false;
        trim = false;
    }

    bool operator==(const Q3SqlFieldInfo& f) const
    {
        return (nm == f.nm &&
                typ == f.typ &&
                req == f.req &&
                len == f.len &&
                prec == f.prec &&
                defValue == f.defValue &&
                tID == f.tID &&
                gen == f.gen &&
                trim == f.trim &&
                calc == f.calc);
    }

    QSqlField toField() const
    { QSqlField f(nm, typ);
      f.setRequiredStatus(QSqlField::RequiredStatus(req));
      f.setLength(len);
      f.setPrecision(prec);
      f.setDefaultValue(defValue);
      f.setSqlType(tID);
      f.setGenerated(gen);
      return f;
    }
    int isRequired() const
    { return req; }
    QVariant::Type type() const
    { return typ; }
    int length() const
    { return len; }
    int precision() const
    { return prec; }
    QVariant defaultValue() const
    { return defValue; }
    QString name() const
    { return nm; }
    int typeID() const
    { return tID; }
    bool isGenerated() const
    { return gen; }
    bool isTrim() const
    { return trim; }
    bool isCalculated() const
    { return calc; }

    virtual void setTrim(bool trim)
    { this->trim = trim; }
    virtual void setGenerated(bool generated)
    { gen = generated; }
    virtual void setCalculated(bool calculated)
    { calc = calculated; }

};

QT_END_HEADER

#endif // QT_NO_SQL

#endif // Q3SQLFIELDINFO_H
