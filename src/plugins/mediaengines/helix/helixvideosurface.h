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

#ifndef HELIXVIDEOSURFACE_H
#define HELIXVIDEOSURFACE_H

#include <QtGui>

#include <config.h>
#include <hxcom.h>
#include <hxcore.h>
#include <hxwin.h>
#include <hxwintyp.h> // ### hxcore && hxwin && hxwintyp ?
#include <hxvsurf.h>
#include <hxcolor.h>


typedef LPHXCOLORCONVERTER (HXEXPORT_PTR FPGETCOLORCONVERTER) (INT32 cidIn, INT32 cidOut);
typedef void (HXEXPORT_PTR FPINITCOLORCONVERTER) (void);

struct HelixColorLibrary
{
    FPGETCOLORCONVERTER GetColorConverter;
    FPINITCOLORCONVERTER InitColorConverter;
};


class PaintObserver
{
public:
    virtual ~PaintObserver() {}
    virtual void setVideoSize(QSize const& size) = 0;
    virtual void paint(QImage const& frame) = 0;
};


class GenericVideoSurface : public IHXVideoSurface
{
public:
    GenericVideoSurface();
    virtual  ~GenericVideoSurface() {}
    QImage const& buffer() const { return m_buffer; }

    // IHXVideoSurface
    STDMETHOD(BeginOptimizedBlt) (THIS_ HXBitmapInfoHeader *pBitmapInfo);
    STDMETHOD(Blt) (THIS_
        UCHAR* pImageBits,
        HXBitmapInfoHeader* pBitmapInfo,
        REF(HXxRect) rDestRect,
        REF(HXxRect) rSrcRect);
    STDMETHOD(EndOptimizedBlt) (THIS);
    STDMETHOD(GetOptimizedFormat) (THIS_ REF(HX_COMPRESSION_TYPE) ulType);
    STDMETHOD(GetPreferredFormat) (THIS_ REF(HX_COMPRESSION_TYPE) ulType);
    STDMETHOD(OptimizedBlt) (THIS_
        UCHAR* pImageBits,
        REF(HXxRect) rDestRect,
        REF(HXxRect) rSrcRect);

    // IUnknown
    STDMETHOD(QueryInterface) (THIS_
        REFIID ID,
        void **object);
    STDMETHOD_(UINT32, AddRef) (THIS);
    STDMETHOD_(UINT32, Release) (THIS);

    void setPaintObserver(PaintObserver* paintObserver);

private:
    INT32 m_refCount;

    HelixColorLibrary m_library;

    QSize               m_videoSize;
    QImage              m_buffer;
    LPHXCOLORCONVERTER  Converter;
    int                 m_bufferPitch;
    int                 m_inPitch;
    int                 m_bufferWidth;
    int                 m_bufferHeight;
    PaintObserver*      m_paintObserver;
};


#endif // HELIXVIDEOSURFACE_H
