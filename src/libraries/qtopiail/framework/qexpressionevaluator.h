/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
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

#ifndef _HAVE_FRAMEWORKEXPRESSION_H_
#define _HAVE_FRAMEWORKEXPRESSION_H_

/* Qt includes */
#include <QByteArray>
#include <QVariant>
#include <QObject>

/* Qtopia includes */
#ifdef QTOPIAIL
#include <qom.h>
#endif

#include "qtopiailglobal.h"

struct QExpressionEvaluatorPrivate;
#ifdef QTOPIAIL
class QAbstractExpressionEvaluatorTerm;
#endif

//========================================
//= Expression Declaration
//=======================================
class ExpressionTokenizer;
class QTOPIAIL_EXPORT QExpressionEvaluator : public QObject // FIXME : should just use QObject, but are there problems with Qom -> QObject connections? to test.
{
    Q_OBJECT
public:
    /* Public Data */
    enum FloatingPointFormat {
        Double,
        FixedPoint
    };

    /* Public Methods */
    /* Expression Ctors */
    explicit QExpressionEvaluator( QObject* parent = 0 );
    explicit QExpressionEvaluator( const QByteArray&, QObject* parent = 0 );
    /* Expression Dtor */
    ~QExpressionEvaluator();

    bool isValid() const;
    bool evaluate();
    QVariant result();

    void setFloatingPointFormat( const FloatingPointFormat& fmt );
    FloatingPointFormat floatingPointFormat() const;

    QByteArray expression() const;
public slots:
    bool setExpression( const QByteArray& expr );
    void clear();

#ifdef QTOPIAIL // FIXME : needed because QOM says this signature and termsChanged() are incompatible signals
private slots:
    void termChanged( const QReference<QAbstractExpressionEvaluatorTerm>& );
#endif

signals:
    void termsChanged();

private:
    /* Private Data */
    QExpressionEvaluatorPrivate* d;
};

//========================================
//= ExpressionToken Declaration
//=======================================
#ifdef QTOPIAIL
class QAbstractExpressionEvaluatorTerm : public QAbstractObject
{
    QOM_OBJECT
public:
    /* Public Methods */
    /* QAbstractExpressionEvaluatorTerm Ctors */
    QAbstractExpressionEvaluatorTerm() {}
    /* QAbstractExpressionEvaluatorTerm Dtor */
    virtual ~QAbstractExpressionEvaluatorTerm() {}

    enum DataType {
        String,
        Double,
        Integer,
        Bool
    };
    virtual bool canHandle( const QString& term ) = 0;
    virtual void setTerm( const QString& term) = 0;
    virtual QVariant value() = 0;

    virtual DataType dataType() const = 0;

    virtual bool canRead() const = 0;
    virtual bool canWrite() const = 0;

signals:
    void termChanged(QReference<QAbstractExpressionEvaluatorTerm>);
};
#endif

#endif
