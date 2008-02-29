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

#ifndef QIMPENSIG_H_
#define QIMPENSIG_H_

#include <qobject.h>
#include <qbytearray.h>
#include <qvector.h>
#include <qlist.h>
#include <qtopiaglobal.h>

struct QTOPIAHW_EXPORT Q_PACKED QIMPenGlyphLink
{
    signed char dx;
    signed char dy;
};

class QIMPenStroke;

class QTOPIAHW_EXPORT QIMPenSignature : public QVector<int>
{
public:
    QIMPenSignature();
    virtual ~QIMPenSignature(){}
    explicit QIMPenSignature(const QVector<int> &o);
    QIMPenSignature(const QIMPenSignature &o);
    void setStroke(const QIMPenStroke &links);
    virtual QByteArray name() const = 0;
    virtual int maxError() const = 0;
    virtual int weight() const = 0;

    virtual int calcError(const QIMPenSignature &other, const QVector<int> &weight = QVector<int>()) const;

protected:
    virtual void calcSignature(const QIMPenStroke &links) = 0;
    // assist if using normal calc error.
    void scale(unsigned int, bool);
    static QVector<int> createBase(const QVector<int>&, int e);
    static int calcError(const QVector<int> &, const QVector<int> &, int offset, bool t, const QVector<int> &weight = QVector<int>());
    virtual bool loops() const { return true; }
    virtual bool slides() const { return true; }
};

class QTOPIAHW_EXPORT TanSignature : public QIMPenSignature
{
public:
    TanSignature() : QIMPenSignature() {}
    explicit TanSignature(const QIMPenStroke &);
    virtual ~TanSignature();

    QByteArray name() const { return "tan"; }
    int maxError() const { return 40; }
    int weight() const { return 1; }

protected:
    void calcSignature(const QIMPenStroke &links);
};

class QTOPIAHW_EXPORT AngleSignature : public QIMPenSignature
{
public:
    AngleSignature() : QIMPenSignature() {}
    explicit AngleSignature(const QIMPenStroke &);
    virtual ~AngleSignature();

    QByteArray name() const { return "tan"; }
    int maxError() const { return 60; }
    int weight() const { return 40; }

protected:
    void calcSignature(const QIMPenStroke &links);
};

class QTOPIAHW_EXPORT DistSignature : public QIMPenSignature
{
public:
    DistSignature() : QIMPenSignature() {}
    explicit DistSignature(const QIMPenStroke &);
    virtual ~DistSignature();

    QByteArray name() const { return "tan"; }
    int maxError() const { return 100; }
    int weight() const { return 60; }

protected:
    void calcSignature(const QIMPenStroke &links);
    bool loops() const { return false; }
};

#endif

