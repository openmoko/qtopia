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

#include "helixsite.h"

#include <reporterror.h>

#include <hxccf.h>
#include <ihxpckts.h>

HelixSite::HelixSite( IUnknown* unknown )
    : m_unknown( unknown ), m_refCount( 0 ), m_values( NULL ), m_siteUser( NULL )
{
    HX_ADDREF( m_unknown );

    IHXCommonClassFactory *factory;
    m_unknown->QueryInterface( IID_IHXCommonClassFactory, (void**)&factory ); // ### check!

    factory->CreateInstance( CLSID_IHXValues, (void**)&m_values ); // ### check!
    HX_ADDREF( m_values );

    HX_RELEASE( factory );

    m_surface = new GenericVideoSurface;
    HX_ADDREF( m_surface );
}

HelixSite::~HelixSite()
{
    HX_RELEASE( m_surface );
    HX_RELEASE( m_values );
    HX_RELEASE( m_unknown );
}

STDMETHODIMP HelixSite::AttachUser( IHXSiteUser *pUser )
{
    if( m_siteUser ) {
        return HXR_UNEXPECTED;
    }

    m_siteUser = pUser;
    if( m_siteUser ) {
        m_siteUser->AddRef();
        m_siteUser->AttachSite( this );
    }

    return HXR_OK;
}

STDMETHODIMP HelixSite::AttachWatcher( IHXSiteWatcher *pWatcher )
{
    Q_UNUSED(pWatcher);

    return HXR_NOTIMPL;
}

STDMETHODIMP HelixSite::CreateChild( REF(IHXSite*) pChildSite )
{
    Q_UNUSED(pChildSite);

    return HXR_NOTIMPL;
}

STDMETHODIMP HelixSite::DamageRect( HXxRect rect )
{
    Q_UNUSED(rect);

    return HXR_NOTIMPL;
}

STDMETHODIMP HelixSite::DamageRegion( HXxRegion region )
{
    Q_UNUSED(region);

    return HXR_NOTIMPL;
}

STDMETHODIMP HelixSite::DestroyChild( IHXSite *pChildSite )
{
    Q_UNUSED(pChildSite);

    return HXR_NOTIMPL;
}

STDMETHODIMP HelixSite::DetachUser()
{
    HRESULT result = HXR_OK;

    if( !m_siteUser ) {
        return HXR_UNEXPECTED;
    }

    result = m_siteUser->DetachSite();
    if( HXR_OK == result ) {
        m_siteUser->Release();
        m_siteUser = NULL;
    }

    return result;
}

STDMETHODIMP HelixSite::DetachWatcher()
{
    return HXR_NOTIMPL;
}

STDMETHODIMP HelixSite::ForceRedraw()
{
    HXxEvent event = { HX_SURFACE_UPDATE, NULL, m_surface, NULL, 0, 0 };
    m_siteUser->HandleEvent( &event );

    return HXR_OK;
}

STDMETHODIMP HelixSite::GetPosition( REF(HXxPoint) position )
{
    position = m_position;

    return HXR_OK;
}

STDMETHODIMP HelixSite::GetSize( REF(HXxSize) size )
{
    size = m_size;

    return HXR_OK;
}

STDMETHODIMP HelixSite::GetUser( REF(IHXSiteUser*) pUser )
{
    if( !m_siteUser ) {
        return HXR_UNEXPECTED;
    }

    pUser = m_siteUser;
    pUser->AddRef();

    return HXR_OK;
}

STDMETHODIMP HelixSite::SetPosition( HXxPoint position )
{
    m_position = position;

    return HXR_OK;
}

STDMETHODIMP HelixSite::SetSize( HXxSize size )
{
    m_size = size;

    return HXR_OK;
}

STDMETHODIMP HelixSite::AddPassiveSiteWatcher( IHXPassiveSiteWatcher *pWatcher )
{
    Q_UNUSED(pWatcher);

    return HXR_NOTIMPL;
}

STDMETHODIMP_(UINT32) HelixSite::GetNumberOfChildSites()
{
    return HXR_NOTIMPL;
}

STDMETHODIMP HelixSite::GetVideoSurface( REF(IHXVideoSurface*) pSurface )
{
    Q_UNUSED(pSurface);

    return HXR_NOTIMPL;
}

STDMETHODIMP HelixSite::GetZOrder( REF(INT32) lZOrder )
{
    Q_UNUSED(lZOrder);

    return HXR_NOTIMPL;
}

STDMETHODIMP_(BOOL) HelixSite::IsSiteVisible()
{
    return true;
}

STDMETHODIMP HelixSite::MoveSiteToTop()
{
    return HXR_NOTIMPL;
}

STDMETHODIMP HelixSite::RemovePassiveSiteWatcher( IHXPassiveSiteWatcher *pWatcher )
{
    Q_UNUSED(pWatcher);

    return HXR_NOTIMPL;
}

STDMETHODIMP HelixSite::SetCursor( HXxCursor ulCursor, REF(HXxCursor) ulOldCursor )
{
    Q_UNUSED(ulCursor);
    Q_UNUSED(ulOldCursor);

    return HXR_NOTIMPL;
}

STDMETHODIMP HelixSite::SetZOrder( INT32 lZOrder )
{
    Q_UNUSED(lZOrder);

    return HXR_NOTIMPL;
}

STDMETHODIMP HelixSite::ShowSite( BOOL bShow )
{
    Q_UNUSED(bShow);

    return HXR_NOTIMPL;
}

STDMETHODIMP HelixSite::UpdateSiteWindow( HXxWindow *pWindow )
{
    Q_UNUSED(pWindow);

    return HXR_NOTIMPL;
}

STDMETHODIMP HelixSite::EventOccurred( HXxEvent *pEvent )
{
    Q_UNUSED(pEvent);

    return HXR_NOTIMPL;
}

STDMETHODIMP_(HXxWindow*) HelixSite::GetParentWindow()
{
    return NULL;
}

STDMETHODIMP_(ULONG32) HelixSite::AddRef()
{
    return InterlockedIncrement( &m_refCount );
}

STDMETHODIMP_(ULONG32) HelixSite::Release()
{
    if( InterlockedDecrement( &m_refCount ) > 0 )
    {
        return m_refCount;
    }

    delete this;
    return 0;
}

STDMETHODIMP HelixSite::QueryInterface( REFIID riid, void** object )
{
    if( IsEqualIID( riid, IID_IUnknown ) ) {
        AddRef();
        *object = (IUnknown*)(IHXSite*)this;
        return HXR_OK;
    } else if( IsEqualIID( riid, IID_IHXSite ) ) {
        AddRef();
        *object = (IHXSite*)this;
        return HXR_OK;
    } else if( IsEqualIID( riid, IID_IHXSite2 ) ) {
        AddRef();
        *object = (IHXSite2*)this;
        return HXR_OK;
    } else if( IsEqualIID( riid, IID_IHXSiteWindowless ) ) {
        AddRef();
        *object = (IHXSiteWindowless*)this;
        return HXR_OK;
    } else if( m_values->QueryInterface( riid, object ) == HXR_OK ) {
        return HXR_OK;
    }

    *object = NULL;
    return HXR_NOINTERFACE;
}

HelixSiteSupplier::HelixSiteSupplier( IUnknown *unknown )
    : m_unknown( unknown ), m_refCount( 0 ), m_manager( NULL ), m_site( NULL )
{
    HX_ADDREF( m_unknown );

    // Get handle for site manager
    m_unknown->QueryInterface( IID_IHXSiteManager, (void**)&m_manager );
}

HelixSiteSupplier::~HelixSiteSupplier()
{
    HX_RELEASE( m_manager );
    HX_RELEASE( m_unknown );
}

STDMETHODIMP HelixSiteSupplier::SitesNeeded( UINT32 uRequestID, IHXValues *pRequestProps )
{
    Q_UNUSED(uRequestID);

    // ### Currently only one site supported
    if( m_site ) {
        return HXR_FAIL;
    }

    HRESULT hres = HXR_OK;

    if( !pRequestProps ) {
        return HXR_INVALID_PARAMETER;
    }

    // Create windowless site
    m_site = new HelixSite( m_unknown );
    HX_ADDREF( m_site );

    // Add site to site manager
    IHXSite *ihxsite;
    m_site->QueryInterface( IID_IHXSite, (void**)&ihxsite );

    // Set site properties
    IHXValues *properties;
    hres = m_site->QueryInterface( IID_IHXValues, (void**)&properties );
    if( HXR_OK != hres ) {
        REPORT_ERROR( ERR_UNKNOWN );
    }

    IHXBuffer *buffer;
    hres = pRequestProps->GetPropertyCString( "playto", buffer );
    if( HXR_OK == hres ) {
        properties->SetPropertyCString( "channel", buffer );
        HX_RELEASE( buffer );
    } else {
        hres = pRequestProps->GetPropertyCString( "name", buffer );
        if( HXR_OK == hres ) {
            properties->SetPropertyCString( "LayoutGroup", buffer );
            HX_RELEASE( buffer );
        }
    }
    HX_RELEASE( properties );

    hres = m_manager->AddSite( ihxsite );
    if( hres != HXR_OK ) {
        REPORT_ERROR( ERR_HELIX );
    }
    HX_RELEASE( ihxsite );

    // Notify observers
    notify();

    return hres;
}

STDMETHODIMP HelixSiteSupplier::SitesNotNeeded( UINT32 uRequestID )
{
    Q_UNUSED(uRequestID);

    if( !m_site ) {
        return HXR_FAIL;
    }

    HRESULT result = HXR_OK;

    // Remove site
    IHXSite *ihxsite;
    m_site->QueryInterface( IID_IHXSite, (void**)&ihxsite );
    if( ihxsite ) {
        result = m_manager->RemoveSite( ihxsite );
    } else {
        return HXR_UNEXPECTED;
    }

    HX_RELEASE( ihxsite );

    HX_RELEASE( m_site );
    m_site = NULL;

    // Notify observers
    notify();

    return result;
}

STDMETHODIMP HelixSiteSupplier::BeginChangeLayout()
{
    return HXR_OK;
}

STDMETHODIMP HelixSiteSupplier::DoneChangeLayout()
{
    return HXR_OK;
}

STDMETHODIMP_(ULONG32) HelixSiteSupplier::AddRef()
{
    return InterlockedIncrement( &m_refCount );
}

STDMETHODIMP_(ULONG32) HelixSiteSupplier::Release()
{
    if( InterlockedDecrement( &m_refCount ) > 0 )
    {
        return m_refCount;
    }

    delete this;
    return 0;
}

STDMETHODIMP HelixSiteSupplier::QueryInterface( REFIID riid, void** object )
{
    if( IsEqualIID( riid, IID_IUnknown ) ) {
        AddRef();
        *object = (IUnknown*)(IHXSiteSupplier*)this;
        return HXR_OK;
    } else if( IsEqualIID( riid, IID_IHXSiteSupplier ) ) {
        AddRef();
        *object = (IHXSiteSupplier*)this;
        return HXR_OK;
    }

    *object = NULL;
    return HXR_NOINTERFACE;
}
