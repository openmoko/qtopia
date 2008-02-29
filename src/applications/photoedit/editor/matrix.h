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

#ifndef MATRIX_H
#define MATRIX_H

#include <qpoint.h>
#include <qrect.h>

class Matrix {
    friend Matrix operator*( const Matrix&, const Matrix& );
public:
    Matrix()
        : a1( 1 ), a2( 0 ), b1( 0 ), b2( 1 )
    { }

    Matrix( int w, int x, int y, int z )
        : a1( w ), a2( x ), b1( y ), b2( z )
    { }

    int a() const { return a1; }
    int b() const { return a2; }
    int c() const { return b1; }
    int d() const { return b2; }

    void map( int, int, int*, int* ) const;

    QPoint map( const QPoint& ) const;

    QRect map( const QRect& ) const;

    Matrix inverse() const;

    bool operator==( const Matrix& ) const;

    bool operator!=( const Matrix& ) const;

    Matrix& operator*=( const Matrix& );

private:
    int a1, a2, b1, b2;
};

inline Matrix operator*( const Matrix& a, const Matrix& b )
{
    return Matrix( a.a1 * b.a1 + a.a2 * b.b1,
        a.a1 * b.a2 + a.a2 * b.b2,
        a.b1 * b.a1 + a.b2 * b.b1,
        a.b1 * b.a2 + a.b2 * b.b2 );
}

inline void Matrix::map( int x, int y, int* xd, int* yd ) const
{
    *xd = x * a1 + y * b1;
    *yd = x * a2 + y * b2;
}

inline QPoint Matrix::map( const QPoint& p ) const
{
    return QPoint( p.x() * a1 + p.y() * b1, p.x() * a2 + p.y() * b2 );
}

inline QRect Matrix::map( const QRect& r ) const
{
    return QRect( map( r.topLeft() ), map( r.bottomRight() ) );
}

inline Matrix Matrix::inverse() const
{
    int det = a1 * b2 - a2 * b1;
    if( det ) return Matrix( b2 / det, -a2 / det, -b1 / det, a1 / det );
    else return Matrix();
}

inline bool Matrix::operator==( const Matrix& a ) const
{
    return a1 == a.a1 && a2 == a.a2 && b1 == a.b1 && b2 == a.b2;
}

inline bool Matrix::operator!=( const Matrix& a ) const
{
    return a1 != a.a1 || a2 != a.a2 || b1 != a.b1 || b2 != a.b2;
}

inline Matrix& Matrix::operator*=( const Matrix& a )
{
    return *this = *this * a;
}

#endif
