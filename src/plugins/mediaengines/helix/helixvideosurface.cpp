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

#include "helixvideosurface.h"

#include "helixutil.h"
#include "reporterror.h"

#if defined (Q_WS_X11)
#include <QX11Info>
#elif defined (Q_WS_QWS)
#include <QtGui/qscreen_qws.h>
#endif

#include <colormap.h>


#define GETBITMAPCOLOR(x) GetBitmapColor( (HXBitmapInfo*)(x) )
#define GETBITMAPPITCH(x) GetBitmapPitch( (HXBitmapInfo*)(x) )


static HelixColorLibrary load_color_library()
{
    QLibrary library( helix_library_path() + QLatin1String("/hxltcolor.so") );

    HelixColorLibrary symbols;
    symbols.GetColorConverter = (FPGETCOLORCONVERTER)library.resolve( "GetColorConverter" );
    symbols.InitColorConverter = (FPINITCOLORCONVERTER)library.resolve( "InitColorConverter" );

    return symbols;
}

static int NullConverter(unsigned char*, int, int, int, int, int, int, int,
                         unsigned char*, int, int, int, int, int, int, int)
{
    return 0;
}

GenericVideoSurface::GenericVideoSurface():
    m_refCount(0),
    Converter(0),
    m_paintObserver(0)
{
    m_library = load_color_library();

    if (m_library.InitColorConverter) 
    {
        m_library.InitColorConverter();
    }
    else
    {
        REPORT_ERROR(ERR_HELIX);
    }
}

STDMETHODIMP GenericVideoSurface::BeginOptimizedBlt(HXBitmapInfoHeader *pBitmapInfo)
{
    Q_UNUSED(pBitmapInfo);

    return HXR_NOTIMPL;
}

static inline bool is16Bit()
{
#if defined(Q_WS_QWS)
    return qt_screen->depth() == 16;
#elif defined(Q_WS_X11)
    return QX11Info::appDepth() == 16;
#else
    return false;
#endif
}

STDMETHODIMP GenericVideoSurface::Blt( UCHAR* pImageBits, HXBitmapInfoHeader* pBitmapInfo, REF(HXxRect) rDestRect, REF(HXxRect) rSrcRect )
{
    // Init
    if (m_buffer.isNull())
    {
        // Assume rDestRect does not change
        m_bufferWidth = rDestRect.right - rDestRect.left;
        m_bufferHeight = rDestRect.bottom - rDestRect.top;

        m_buffer = QImage(m_bufferWidth, m_bufferHeight, is16Bit() ? QImage::Format_RGB16 : QImage::Format_RGB32);
    }

    // Obtain color converter
    if (!Converter)
    {
        if (m_library.GetColorConverter)
        {
            HXBitmapInfoHeader bufferInfo;
            memset( &bufferInfo, 0, sizeof(HXBitmapInfoHeader) );

            bufferInfo.biWidth = m_bufferWidth;
            bufferInfo.biHeight = m_bufferHeight;
            bufferInfo.biPlanes = 1;
            if( is16Bit() ) {
                bufferInfo.biCompression = HXCOLOR_RGB565_ID;
                bufferInfo.biBitCount = 16;
            } else {
                bufferInfo.biCompression = HX_RGB;
                bufferInfo.biBitCount = 32;
            }
            bufferInfo.biSizeImage = bufferInfo.biWidth * bufferInfo.biHeight * bufferInfo.biBitCount / 8;

            m_bufferPitch = GETBITMAPPITCH( &bufferInfo );
            m_inPitch = GETBITMAPPITCH( pBitmapInfo );

            int bufferCID = GETBITMAPCOLOR( &bufferInfo );
            int inCID = GETBITMAPCOLOR( pBitmapInfo );

            Converter = m_library.GetColorConverter( inCID, bufferCID );
        }

        if( !Converter ) {
            REPORT_ERROR( ERR_UNSUPPORTED );
            // Assign null converter if no converter available
            Converter = &NullConverter;
        }
    }

    Converter(m_buffer.bits(),
              m_bufferWidth,
              m_bufferHeight,
              m_bufferPitch,
              0,
              0,
              m_bufferWidth,
              m_bufferHeight,

              pImageBits,
              pBitmapInfo->biWidth,
              pBitmapInfo->biHeight,
              m_inPitch,
              rSrcRect.left,
              rSrcRect.top,
              rSrcRect.right - rSrcRect.left,
              rSrcRect.bottom - rSrcRect.top);

    // Notify observer
    if (m_paintObserver != 0)
        m_paintObserver->paintNotification();

    return HXR_OK;
}

STDMETHODIMP GenericVideoSurface::EndOptimizedBlt()
{
    return HXR_NOTIMPL;
}

STDMETHODIMP GenericVideoSurface::GetOptimizedFormat( REF(HX_COMPRESSION_TYPE) ulType )
{
    Q_UNUSED(ulType);

    return HXR_NOTIMPL;
}

STDMETHODIMP GenericVideoSurface::GetPreferredFormat( REF(HX_COMPRESSION_TYPE) ulType )
{
    ulType = HX_RGB;

    return HXR_OK;
}

STDMETHODIMP GenericVideoSurface::OptimizedBlt( UCHAR* pImageBits, REF(HXxRect) rDestRect, REF(HXxRect) rSrcRect )
{
    Q_UNUSED(pImageBits);
    Q_UNUSED(rDestRect);
    Q_UNUSED(rSrcRect);

    return HXR_NOTIMPL;
}

STDMETHODIMP_(ULONG32) GenericVideoSurface::AddRef()
{
    return InterlockedIncrement( &m_refCount );
}

STDMETHODIMP_(ULONG32) GenericVideoSurface::Release()
{
    if( InterlockedDecrement( &m_refCount ) > 0 )
    {
        return m_refCount;
    }

    delete this;
    return 0;
}

STDMETHODIMP GenericVideoSurface::QueryInterface(REFIID riid, void** object)
{
    if (IsEqualIID( riid, IID_IUnknown)) 
    {
        AddRef();
        *object = (IUnknown*)(IHXSite*)this;
        return HXR_OK;
    }
    else if (IsEqualIID(riid, IID_IHXVideoSurface))
    {
        REPORT_ERROR( ERR_TEST );
        AddRef();
        *object = (IHXVideoSurface*)this;
        return HXR_OK;
    }

    *object = NULL;

    return HXR_NOINTERFACE;
}

void GenericVideoSurface::addPaintObserver(PaintObserver* paintObserver)
{
    m_paintObserver = paintObserver;
}


