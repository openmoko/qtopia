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

#ifndef HELIXSITE_H
#define HELIXSITE_H

#include <observer.h>
#include <helixvideosurface.h>

#include <config.h>
#include <hxcom.h>
#include <hxcore.h>
#include <hxwin.h>
#include <hxwintyp.h>
#include <hxsite2.h>
#include <hxevent.h>

class HelixSite : public IHXSite,
    public IHXSite2,
    public IHXSiteWindowless
{
public:
    HelixSite( IUnknown *unknown );
    virtual ~HelixSite();

    GenericVideoSurface* surface() const { return m_surface; }

    // IHXSite
    STDMETHOD(AttachUser) (THIS_ IHXSiteUser *pUser);
    STDMETHOD(AttachWatcher) (THIS_ IHXSiteWatcher *pWatcher);
    STDMETHOD(CreateChild) (THIS_ REF(IHXSite*) pChildSite);
    STDMETHOD(DamageRect) (THIS_ HXxRect rect);
    STDMETHOD(DamageRegion) (THIS_ HXxRegion region );
    STDMETHOD(DestroyChild) (THIS_ IHXSite *pChildSite);
    STDMETHOD(DetachUser) (THIS);
    STDMETHOD(DetachWatcher) (THIS);
    STDMETHOD(ForceRedraw) (THIS);
    STDMETHOD(GetPosition) (THIS_ REF(HXxPoint) position);
    STDMETHOD(GetSize) (THIS_ REF(HXxSize) size);
    STDMETHOD(GetUser) (THIS_ REF(IHXSiteUser*) pUser);
    STDMETHOD(SetPosition) (THIS_ HXxPoint position);
    STDMETHOD(SetSize) (THIS_ HXxSize size);

    // IHXSite2
    STDMETHOD(AddPassiveSiteWatcher) (THIS_ IHXPassiveSiteWatcher *pWatcher);
    STDMETHOD_(UINT32,GetNumberOfChildSites) (THIS);
    STDMETHOD(GetVideoSurface) (THIS_ REF(IHXVideoSurface*) pSurface);
    STDMETHOD(GetZOrder) (THIS_ REF(INT32) lZOrder);
    STDMETHOD_(BOOL,IsSiteVisible) (THIS);
    STDMETHOD(MoveSiteToTop) (THIS);
    STDMETHOD(RemovePassiveSiteWatcher) (THIS_ IHXPassiveSiteWatcher *pWatcher);
    STDMETHOD(SetCursor) (THIS_
        HXxCursor ulCursor,
        REF(HXxCursor) ulOldCursor);
    STDMETHOD(SetZOrder) (THIS_ INT32 lZOrder);
    STDMETHOD(ShowSite) (THIS_ BOOL bShow);
    STDMETHOD(UpdateSiteWindow) (THIS_ HXxWindow *pWindow);

    // IHXSiteWindowless
    STDMETHOD(EventOccurred) (THIS_ HXxEvent *pEvent);
    STDMETHOD_(HXxWindow*,GetParentWindow) (THIS);

    // IUnknown
    STDMETHOD(QueryInterface) (THIS_
        REFIID ID,
        void **object);
    STDMETHOD_(UINT32, AddRef) (THIS);
    STDMETHOD_(UINT32, Release) (THIS);

private:
    IUnknown *m_unknown;
    INT32 m_refCount;

    IHXValues *m_values;

    IHXSiteUser *m_siteUser;
    IHXSiteWatcher *m_siteWatcher;

    HXxPoint m_position;
    HXxSize m_size;

    GenericVideoSurface *m_surface;
};

class HelixSiteSupplier : public IHXSiteSupplier,
    public Subject
{
public:
    HelixSiteSupplier( IUnknown *unknown );
    ~HelixSiteSupplier();

    bool hasVideo() const { return m_site != NULL; }
    HelixSite* site() const { return m_site; }

    // IHXSiteSupplier
    STDMETHOD(SitesNeeded) (THIS_
        UINT32 uRequestID,
        IHXValues *pSiteProps);
    STDMETHOD(SitesNotNeeded) (THIS_ UINT32 uRequestID);
    STDMETHOD(BeginChangeLayout) (THIS);
    STDMETHOD(DoneChangeLayout) (THIS);

    // IUnknown
    STDMETHOD(QueryInterface) (THIS_
        REFIID ID,
        void **object);
    STDMETHOD_(UINT32, AddRef) (THIS);
    STDMETHOD_(UINT32, Release) (THIS);

private:
    IUnknown *m_unknown;
    INT32 m_refCount;

    IHXSiteManager *m_manager;
    HelixSite *m_site;
};

#endif // HELIXSITE_H
