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

#ifndef _HAVE_QFIXEDPOINTNUMBER_H_
#define _HAVE_QFIXEDPOINTNUMBER_H_

#include <QString>
#include <QtGlobal>

const int MAX_FIXED_PRECISION = 15;
const int MAX_FIXED_LENGTH = 28;



typedef qint64 QFixedPointNumberType;

inline int qfixedpointnumber_cast_to_integer(QFixedPointNumberType x) {return (int) x;}

class qfixedpointnumber_scale
{
  public:
      QFixedPointNumberType x[MAX_FIXED_PRECISION+1];
    qfixedpointnumber_scale();
};
static const qfixedpointnumber_scale qfixedpointnumber_scale_inst;

class QFixedPointNumber
{
  private:
  public:
    QFixedPointNumberType value;
    unsigned char precision;
    void equalize_precision(QFixedPointNumber &);

  public:
    QFixedPointNumber(int x, int p) {value = x; precision = p;}
    explicit QFixedPointNumber(const char *);
    QFixedPointNumber();
    QFixedPointNumber operator [] (int) const;
    QFixedPointNumber operator = (QFixedPointNumber);
    QFixedPointNumber operator = (int);
    QFixedPointNumber operator = (const char*);
    friend QFixedPointNumber operator + (QFixedPointNumber, QFixedPointNumber);
    friend QFixedPointNumber operator - (QFixedPointNumber, QFixedPointNumber);
    friend QFixedPointNumber operator * (QFixedPointNumber, QFixedPointNumber);
    friend QFixedPointNumber operator / (QFixedPointNumber, QFixedPointNumber);
    friend QFixedPointNumber operator - (QFixedPointNumber);
    friend bool operator == (QFixedPointNumber, QFixedPointNumber);
    friend bool operator < (QFixedPointNumber, QFixedPointNumber);
    friend QFixedPointNumber operator - (int, QFixedPointNumber);
    friend QFixedPointNumber operator / (int, QFixedPointNumber);
    friend bool operator == (int, QFixedPointNumber);
    friend bool operator < (int, QFixedPointNumber);
    friend QFixedPointNumber operator + (QFixedPointNumber, int);
    friend QFixedPointNumber operator - (QFixedPointNumber, int);
    friend QFixedPointNumber operator * (QFixedPointNumber, int);
    friend QFixedPointNumber operator / (QFixedPointNumber, int);
    friend bool operator == (QFixedPointNumber, int);
    friend bool operator < (QFixedPointNumber, int);
    operator bool ();
    bool operator ! ();
    bool operator == ( bool t );
    enum {ALIGN=1, COMMAS=2, DECIMAL=4};
    QString toString(int = COMMAS) const;
    int whole(void) const;
};

inline bool operator != (QFixedPointNumber x, QFixedPointNumber y) {return !(x == y);}
inline bool operator != (QFixedPointNumber x, int y) {return !(x == y);}
inline bool operator != (int x, QFixedPointNumber y) {return !(x == y);}
inline bool operator >= (QFixedPointNumber x, QFixedPointNumber y) {return !(x < y);}
inline bool operator >= (QFixedPointNumber x, int y) {return !(x < y);}
inline bool operator >= (int x, QFixedPointNumber y) {return !(x < y);}
inline bool operator > (QFixedPointNumber x, QFixedPointNumber y) {return y < x;}
inline bool operator > (QFixedPointNumber x, int y) {return y < x;}
inline bool operator > (int x, QFixedPointNumber y) {return y < x;}
inline bool operator <= (QFixedPointNumber x, QFixedPointNumber y) {return !(y < x);}
inline bool operator <= (QFixedPointNumber x, int y) {return !(y < x);}
inline bool operator <= (int x, QFixedPointNumber y) {return !(y < x);}
inline QFixedPointNumber::operator bool () { return value != 0; }
inline bool QFixedPointNumber::operator ! () { return value == 0; }
inline bool QFixedPointNumber::operator == ( bool t ) { return (value != 0) == t; }

/*
QFixedPointNumber operator + (int x, QFixedPointNumber y) {return y + x;}
QFixedPointNumber operator * (int x, QFixedPointNumber y) {return y * x;}
*/

#endif


#if 0


#include <stdio.h>
#include "QFixedPointNumber.h"

void main(int argc, char **argv)
{
  int i;
  QFixedPointNumber a(argv[1]);
  QFixedPointNumber b(argv[2]);
  printf("a = %s\n", a.edit());
  printf("b = %s\n", b.edit());
  printf("a+b = %s\n", (a+b).edit());
  printf("a-b = %s\n", (a-b).edit());
  printf("a*b = %s\n", (a*b).edit());
  printf("-a = %s\n", (-a).edit());
  printf("a[4] = %s\n", a[4].edit());
  if (b != 0)
    printf("a/b = %s\n", (a/b).edit());
  printf("a.whole() = %d\n", a.whole());
  printf("is a == b? %s\n", a == b ? "yes" : "no");
  printf("is a != b? %s\n", a != b ? "yes" : "no");
  printf("is a <  b? %s\n", a <  b ? "yes" : "no");
  printf("is a <= b? %s\n", a <= b ? "yes" : "no");
  printf("is a >  b? %s\n", a >  b ? "yes" : "no");
  printf("is a >= b? %s\n", a >= b ? "yes" : "no");
}



#endif
