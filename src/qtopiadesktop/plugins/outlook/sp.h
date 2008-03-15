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
#ifndef SP_H
#define SP_H

/*
    Smart Pointer class.

    This is a rather primitive class. Really, it's just something created
    to manage deleting pointers at the end of the scope they're obtained in.
*/
template <class T>
class SP
{
public:
    // SP<T> foo;
    inline SP()
        : o(0) {}

    // SP<T> foo(bar);
    // SP<T> foo = bar;
    inline SP(T *p)
        : o(p) {}

    // Stack-based cleanup (delete the pointer)
    // You should assign 0 if you don't want the pointer deleted
    inline ~SP()
        { delete o; }

    // foo = bar;
    inline SP<T> &operator=(T *p)
        { o = p; return *this; }

    // foo->bar();
    inline T *operator->() const
        { return o; }

    // if ( !foo )
    inline bool operator!() const
        { return (o == 0); }

    // if ( foo ) // SAFE
private:
   class Tester
   {
      void operator delete(void*);
   };
public:
   operator Tester*() const
   {
      if (!o) return 0;
      static Tester test;
      return &test;
   }

private:
   T *o;
};

#endif
