/****************************************************************************
**
** Copyright (C) 1992-2008 Trolltech ASA. All rights reserved.
**
** This file is part of the QtSql module of the Qt Toolkit.
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
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QSQLFIELD_H
#define QSQLFIELD_H

#include <QtCore/qvariant.h>
#include <QtCore/qstring.h>

QT_BEGIN_HEADER

QT_MODULE(Sql)

class QSqlFieldPrivate;

class Q_SQL_EXPORT QSqlField
{
public:
    enum RequiredStatus { Unknown = -1, Optional = 0, Required = 1 };

    QSqlField(const QString& fieldName = QString(),
              QVariant::Type type = QVariant::Invalid);

    QSqlField(const QSqlField& other);
    QSqlField& operator=(const QSqlField& other);
    bool operator==(const QSqlField& other) const;
    inline bool operator!=(const QSqlField &other) const { return !operator==(other); }
    ~QSqlField();

    void setValue(const QVariant& value);
    inline QVariant value() const
    { return val; }
    void setName(const QString& name);
    QString name() const;
    bool isNull() const;
    void setReadOnly(bool readOnly);
    bool isReadOnly() const;
    void clear();
    QVariant::Type type() const;
    bool isAutoValue() const;

    void setType(QVariant::Type type);
    void setRequiredStatus(RequiredStatus status);
    inline void setRequired(bool required)
    { setRequiredStatus(required ? Required : Optional); }
    void setLength(int fieldLength);
    void setPrecision(int precision);
    void setDefaultValue(const QVariant &value);
    void setSqlType(int type);
    void setGenerated(bool gen);
    void setAutoValue(bool autoVal);

    RequiredStatus requiredStatus() const;
    int length() const;
    int precision() const;
    QVariant defaultValue() const;
    int typeID() const;
    bool isGenerated() const;
    bool isValid() const;

#ifdef QT3_SUPPORT
    inline QT3_SUPPORT void setNull() { clear(); }
#endif

private:
    void detach();
    QVariant val;
    QSqlFieldPrivate* d;
};

#ifndef QT_NO_DEBUG_STREAM
Q_SQL_EXPORT QDebug operator<<(QDebug, const QSqlField &);
#endif

QT_END_HEADER

#endif // QSQLFIELD_H
