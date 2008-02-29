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

#ifndef QSIZE_H
#define QSIZE_H

#include "qglobal.h"

#ifndef QT_H
#include "qpoint.h" // ### change to qwindowdefs.h?
#endif // QT_H

class Q_EXPORT QSize
// ### Make QSize inherit Qt in Qt 4.0
{
public:
    // ### Move this enum to qnamespace.h in Qt 4.0
    enum ScaleMode {
	ScaleFree,
	ScaleMin,
	ScaleMax
    };

    QSize();
    QSize( int w, int h );

    bool isNull() const;
    bool isEmpty() const;
    bool isValid() const;

    int width() const;
    int height() const;
    void setWidth( int w );
    void setHeight( int h );
    void transpose();

    void scale( int w, int h, ScaleMode mode );
    void scale( const QSize &s, ScaleMode mode );

    QSize expandedTo( const QSize & ) const;
    QSize boundedTo( const QSize & ) const;

    QCOORD &rwidth();
    QCOORD &rheight();

    QSize &operator+=( const QSize & );
    QSize &operator-=( const QSize & );
    QSize &operator*=( int c );
    QSize &operator*=( double c );
    QSize &operator/=( int c );
    QSize &operator/=( double c );

    friend inline bool operator==( const QSize &, const QSize & );
    friend inline bool operator!=( const QSize &, const QSize & );
    friend inline const QSize operator+( const QSize &, const QSize & );
    friend inline const QSize operator-( const QSize &, const QSize & );
    friend inline const QSize operator*( const QSize &, int );
    friend inline const QSize operator*( int, const QSize & );
    friend inline const QSize operator*( const QSize &, double );
    friend inline const QSize operator*( double, const QSize & );
    friend inline const QSize operator/( const QSize &, int );
    friend inline const QSize operator/( const QSize &, double );

private:
    static void warningDivByZero();

    QCOORD wd;
    QCOORD ht;
};


/*****************************************************************************
  QSize stream functions
 *****************************************************************************/

Q_EXPORT QDataStream &operator<<( QDataStream &, const QSize & );
Q_EXPORT QDataStream &operator>>( QDataStream &, QSize & );


/*****************************************************************************
  QSize inline functions
 *****************************************************************************/

inline QSize::QSize()
{ wd = ht = -1; }

inline QSize::QSize( int w, int h )
{ wd=(QCOORD)w; ht=(QCOORD)h; }

inline bool QSize::isNull() const
{ return wd==0 && ht==0; }

inline bool QSize::isEmpty() const
{ return wd<1 || ht<1; }

inline bool QSize::isValid() const
{ return wd>=0 && ht>=0; }

inline int QSize::width() const
{ return wd; }

inline int QSize::height() const
{ return ht; }

inline void QSize::setWidth( int w )
{ wd=(QCOORD)w; }

inline void QSize::setHeight( int h )
{ ht=(QCOORD)h; }

inline QCOORD &QSize::rwidth()
{ return wd; }

inline QCOORD &QSize::rheight()
{ return ht; }

inline QSize &QSize::operator+=( const QSize &s )
{ wd+=s.wd; ht+=s.ht; return *this; }

inline QSize &QSize::operator-=( const QSize &s )
{ wd-=s.wd; ht-=s.ht; return *this; }

inline QSize &QSize::operator*=( int c )
{ wd*=(QCOORD)c; ht*=(QCOORD)c; return *this; }

inline QSize &QSize::operator*=( double c )
{ wd=(QCOORD)(wd*c); ht=(QCOORD)(ht*c); return *this; }

inline bool operator==( const QSize &s1, const QSize &s2 )
{ return s1.wd == s2.wd && s1.ht == s2.ht; }

inline bool operator!=( const QSize &s1, const QSize &s2 )
{ return s1.wd != s2.wd || s1.ht != s2.ht; }

inline const QSize operator+( const QSize & s1, const QSize & s2 )
{ return QSize(s1.wd+s2.wd, s1.ht+s2.ht); }

inline const QSize operator-( const QSize &s1, const QSize &s2 )
{ return QSize(s1.wd-s2.wd, s1.ht-s2.ht); }

inline const QSize operator*( const QSize &s, int c )
{ return QSize(s.wd*c, s.ht*c); }

inline const QSize operator*( int c, const QSize &s )
{  return QSize(s.wd*c, s.ht*c); }

inline const QSize operator*( const QSize &s, double c )
{ return QSize((QCOORD)(s.wd*c), (QCOORD)(s.ht*c)); }

inline const QSize operator*( double c, const QSize &s )
{ return QSize((QCOORD)(s.wd*c), (QCOORD)(s.ht*c)); }

inline QSize &QSize::operator/=( int c )
{
#if defined(QT_CHECK_MATH)
    if ( c == 0 )
	warningDivByZero();
#endif
    wd/=(QCOORD)c; ht/=(QCOORD)c;
    return *this;
}

inline QSize &QSize::operator/=( double c )
{
#if defined(QT_CHECK_MATH)
    if ( c == 0.0 )
	warningDivByZero();
#endif
    wd=(QCOORD)(wd/c); ht=(QCOORD)(ht/c);
    return *this;
}

inline const QSize operator/( const QSize &s, int c )
{
#if defined(QT_CHECK_MATH)
    if ( c == 0 )
	QSize::warningDivByZero();
#endif
    return QSize(s.wd/c, s.ht/c);
}

inline const QSize operator/( const QSize &s, double c )
{
#if defined(QT_CHECK_MATH)
    if ( c == 0.0 )
	QSize::warningDivByZero();
#endif
    return QSize((QCOORD)(s.wd/c), (QCOORD)(s.ht/c));
}

inline QSize QSize::expandedTo( const QSize & otherSize ) const
{
    return QSize( QMAX(wd,otherSize.wd), QMAX(ht,otherSize.ht) );
}

inline QSize QSize::boundedTo( const QSize & otherSize ) const
{
    return QSize( QMIN(wd,otherSize.wd), QMIN(ht,otherSize.ht) );
}


#endif // QSIZE_H
