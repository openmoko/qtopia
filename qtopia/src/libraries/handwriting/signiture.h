/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
** 
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
** 
** A copy of the GNU GPL license version 2 is included in this package as 
** LICENSE.GPL.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
** See the GNU General Public License for more details.
**
** In addition, as a special exception Trolltech gives permission to link
** the code of this program with Qtopia applications copyrighted, developed
** and distributed by Trolltech under the terms of the Qtopia Personal Use
** License Agreement. You must comply with the GNU General Public License
** in all respects for all of the code used other than the applications
** licensed under the Qtopia Personal Use License Agreement. If you modify
** this file, you may extend this exception to your version of the file,
** but you are not obligated to do so. If you do not wish to do so, delete
** this exception statement from your version.
** 
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef QIMPENSIG_H_
#define QIMPENSIG_H_

#include <qobject.h>
#include <qarray.h>
#include <qlist.h>

struct Q_PACKED QIMPenGlyphLink
{
    signed char dx;
    signed char dy;
};

class QIMPenStroke;

class QIMPenSigniture : public QArray<int>
{
public:
    QIMPenSigniture();
    ~QIMPenSigniture(){}
    QIMPenSigniture(const QArray<int> &o);
    QIMPenSigniture(const QIMPenSigniture &o);
    void setStroke(const QIMPenStroke &links);
    virtual QCString name() const = 0;
    virtual int maxError() const = 0;
    virtual int weight() const = 0;

    virtual int calcError(const QIMPenSigniture &other) const;

protected:
    virtual void calcSigniture(const QIMPenStroke &links) = 0;
    // assist if using normal calc error.
    void scale(unsigned int, bool);
    static QArray<int> createBase(const QArray<int>&, int e);
    static int calcError(const QArray<int> &, const QArray<int> &, int offset, bool t);
    virtual bool loops() const { return TRUE; }
    virtual bool slides() const { return TRUE; }
};

class TanSigniture : public QIMPenSigniture
{
public:
    TanSigniture() : QIMPenSigniture() {}
    TanSigniture(const QIMPenStroke &);
    ~TanSigniture();

    QCString name() const { return "tan"; }
    int maxError() const { return 40; }
    int weight() const { return 1; }

protected:
    void calcSigniture(const QIMPenStroke &links);
};

class AngleSigniture : public QIMPenSigniture
{
public:
    AngleSigniture() : QIMPenSigniture() {}
    AngleSigniture(const QIMPenStroke &);
    ~AngleSigniture();

    QCString name() const { return "tan"; }
    int maxError() const { return 60; }
    int weight() const { return 20; }

protected:
    void calcSigniture(const QIMPenStroke &links);
};

class DistSigniture : public QIMPenSigniture
{
public:
    DistSigniture() : QIMPenSigniture() {}
    DistSigniture(const QIMPenStroke &);
    ~DistSigniture();

    QCString name() const { return "tan"; }
    int maxError() const { return 100; }
    int weight() const { return 60; }

protected:
    void calcSigniture(const QIMPenStroke &links);
    bool loops() { return FALSE; }
};

#endif

