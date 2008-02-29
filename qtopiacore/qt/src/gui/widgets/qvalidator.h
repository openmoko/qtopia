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

#ifndef QVALIDATOR_H
#define QVALIDATOR_H

#include <QtCore/qobject.h>
#include <QtCore/qstring.h>
#include <QtCore/qregexp.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

#ifndef QT_NO_VALIDATOR

class Q_GUI_EXPORT QValidator : public QObject
{
    Q_OBJECT
public:
    explicit QValidator(QObject * parent);
    ~QValidator();

    enum State {
        Invalid,
        Intermediate,
        Acceptable

#if defined(QT3_SUPPORT) && !defined(Q_MOC_RUN)
        , Valid = Intermediate
#endif
    };

    virtual State validate(QString &, int &) const = 0;
    virtual void fixup(QString &) const;

#ifdef QT3_SUPPORT
public:
    QT3_SUPPORT_CONSTRUCTOR QValidator(QObject * parent, const char *name);
#endif

private:
    Q_DISABLE_COPY(QValidator)
};

class Q_GUI_EXPORT QIntValidator : public QValidator
{
    Q_OBJECT
    Q_PROPERTY(int bottom READ bottom WRITE setBottom)
    Q_PROPERTY(int top READ top WRITE setTop)

public:
    explicit QIntValidator(QObject * parent);
    QIntValidator(int bottom, int top, QObject * parent);
    ~QIntValidator();

    QValidator::State validate(QString &, int &) const;

    void setBottom(int);
    void setTop(int);
    virtual void setRange(int bottom, int top);

    int bottom() const { return b; }
    int top() const { return t; }

#ifdef QT3_SUPPORT
public:
    QT3_SUPPORT_CONSTRUCTOR QIntValidator(QObject * parent, const char *name);
    QT3_SUPPORT_CONSTRUCTOR QIntValidator(int bottom, int top, QObject * parent, const char *name);
#endif

private:
    Q_DISABLE_COPY(QIntValidator)

    int b;
    int t;
};

#ifndef QT_NO_REGEXP

class Q_GUI_EXPORT QDoubleValidator : public QValidator
{
    Q_OBJECT
    Q_PROPERTY(double bottom READ bottom WRITE setBottom)
    Q_PROPERTY(double top READ top WRITE setTop)
    Q_PROPERTY(int decimals READ decimals WRITE setDecimals)

public:
    explicit QDoubleValidator(QObject * parent);
    QDoubleValidator(double bottom, double top, int decimals, QObject * parent);
    ~QDoubleValidator();

    QValidator::State validate(QString &, int &) const;

    virtual void setRange(double bottom, double top, int decimals = 0);
    void setBottom(double);
    void setTop(double);
    void setDecimals(int);

    double bottom() const { return b; }
    double top() const { return t; }
    int decimals() const { return d; }

#ifdef QT3_SUPPORT
public:
    QT3_SUPPORT_CONSTRUCTOR QDoubleValidator(QObject * parent, const char *name);
    QT3_SUPPORT_CONSTRUCTOR QDoubleValidator(double bottom, double top, int decimals,
                                           QObject * parent, const char *name);
#endif
private:
    Q_DISABLE_COPY(QDoubleValidator)

    double b;
    double t;
    int d;
};


class Q_GUI_EXPORT QRegExpValidator : public QValidator
{
    Q_OBJECT
    Q_PROPERTY(QRegExp regExp READ regExp WRITE setRegExp)

public:
    explicit QRegExpValidator(QObject *parent);
    QRegExpValidator(const QRegExp& rx, QObject *parent);
    ~QRegExpValidator();

    virtual QValidator::State validate(QString& input, int& pos) const;

    void setRegExp(const QRegExp& rx);
    const QRegExp& regExp() const { return r; } // ### make inline for 5.0

#ifdef QT3_SUPPORT
public:
    QT3_SUPPORT_CONSTRUCTOR QRegExpValidator(QObject *parent, const char *name);
    QT3_SUPPORT_CONSTRUCTOR QRegExpValidator(const QRegExp& rx, QObject *parent, const char *name);
#endif

private:
    Q_DISABLE_COPY(QRegExpValidator)

    QRegExp r;
};

#endif // QT_NO_REGEXP

#endif // QT_NO_VALIDATOR

QT_END_HEADER

#endif // QVALIDATOR_H
