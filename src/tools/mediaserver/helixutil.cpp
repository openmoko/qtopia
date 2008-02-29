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

#include "helixutil.h"

#include <qtopianamespace.h>

QString helix_library_path()
{
    static QString library_path;
    static bool resolve = true;

    if( resolve ) {
        static const QString CORELIB_PATH = "lib/clntcore.so";

        resolve = false;
        foreach( QString preface, Qtopia::installPaths() ) {
            if( QFile::exists( preface + CORELIB_PATH ) ) {
                return library_path = preface + QString( "lib" );
            }
        }
    }

    return library_path;
}

GenericContext::GenericContext( const QList<IUnknown*>& classes )
    : m_refCount( 0 ), m_classes( classes )
{
    foreach( IUnknown* unknown, m_classes ) {
        HX_ADDREF( unknown );
    }
}

GenericContext::~GenericContext()
{
    foreach( IUnknown* unknown, m_classes ) {
        HX_RELEASE( unknown );
    }
}

STDMETHODIMP_(ULONG32) GenericContext::AddRef()
{
    return InterlockedIncrement( &m_refCount );
}

STDMETHODIMP_(ULONG32) GenericContext::Release()
{
    if( InterlockedDecrement( &m_refCount ) > 0 )
    {
        return m_refCount;
    }

    delete this;
    return 0;
}

STDMETHODIMP GenericContext::QueryInterface( REFIID riid, void** object )
{
    if( IsEqualIID( riid, IID_IUnknown ) ) {
        AddRef();
        *object = (IUnknown*)this;
        return HXR_OK;
    } else {
        foreach( IUnknown* unknown, m_classes ) {
            if( unknown->QueryInterface( riid, object ) == HXR_OK ) {
                return HXR_OK;
            }
        }
    }

    *object = NULL;
    return HXR_NOINTERFACE;
}
