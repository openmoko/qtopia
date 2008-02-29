/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#include "helixvideosurface.h"

#include "helixutil.h"
#include "reporterror.h"

#include <QDirectPainter>
#include <QtGui/qscreen_qws.h>

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

static int NullConverter( unsigned char*, int, int, int, int, int, int, int,
    unsigned char*, int, int, int, int, int, int, int )
{
    return 0;
}

GenericVideoSurface::GenericVideoSurface()
    : m_refCount( 0 ), Converter( 0 )
{
    m_library = load_color_library();

    if( m_library.InitColorConverter ) {
        m_library.InitColorConverter();
    } else {
        REPORT_ERROR( ERR_HELIX );
    }
}

STDMETHODIMP GenericVideoSurface::BeginOptimizedBlt( HXBitmapInfoHeader *pBitmapInfo )
{
    Q_UNUSED(pBitmapInfo);

    return HXR_NOTIMPL;
}

STDMETHODIMP GenericVideoSurface::Blt( UCHAR* pImageBits, HXBitmapInfoHeader* pBitmapInfo, REF(HXxRect) rDestRect, REF(HXxRect) rSrcRect )
{
    if( m_buffer.isNull() ) {
        // Assume rDestRect does not change
        m_buffer = QImage( rDestRect.right - rDestRect.left,
            rDestRect.bottom - rDestRect.top,
            QImage::Format_RGB32 );
    }

    // Obtain color converter
    if( !Converter ) {
        if( m_library.GetColorConverter ) {
            HXBitmapInfoHeader bufferInfo;
            memset( &bufferInfo, 0, sizeof(HXBitmapInfoHeader) );

            bufferInfo.biWidth = m_buffer.width();
            bufferInfo.biHeight = m_buffer.height();
            bufferInfo.biPlanes = 1;
            bufferInfo.biCompression = HX_RGB;
            bufferInfo.biBitCount = 32;
            bufferInfo.biSizeImage = bufferInfo.biHeight * (( (bufferInfo.biWidth * 4) + 3) & ~3);

            m_bufferPitch = (((bufferInfo.biWidth * 4) + 3) & ~3);
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

    Converter( m_buffer.bits(), m_buffer.width(), m_buffer.height(), m_bufferPitch,
        0, 0, m_buffer.width(), m_buffer.height(),
        pImageBits, pBitmapInfo->biWidth, pBitmapInfo->biHeight, m_inPitch,
        rSrcRect.left, rSrcRect.top, rSrcRect.right - rSrcRect.left, rSrcRect.bottom - rSrcRect.top );

    // Notify observers
    notify();

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

STDMETHODIMP GenericVideoSurface::QueryInterface( REFIID riid, void** object )
{
    if( IsEqualIID( riid, IID_IUnknown ) ) {
        AddRef();
        *object = (IUnknown*)(IHXSite*)this;
        return HXR_OK;
    } else if( IsEqualIID( riid, IID_IHXVideoSurface ) ) {
        REPORT_ERROR( ERR_TEST );
        AddRef();
        *object = (IHXVideoSurface*)this;
        return HXR_OK;
    }

    *object = NULL;
    return HXR_NOINTERFACE;
}

GenericVideoWidget::GenericVideoWidget( GenericVideoSurface* surface, QWidget* parent )
    : QWidget( parent ), m_surface( surface )
{
    HX_ADDREF( m_surface );

    m_surface->attach( this );

    // Optimize paint event
    setAttribute( Qt::WA_NoSystemBackground );
}

GenericVideoWidget::~GenericVideoWidget()
{
    m_surface->detach( this );

    HX_RELEASE( m_surface );
}

void GenericVideoWidget::update( Subject* )
{
    QWidget::update();
}

void GenericVideoWidget::paintEvent( QPaintEvent* )
{
    QPainter painter( this );
    painter.fillRect( rect(), Qt::black );

    QImage buffer = m_surface->buffer();
    if( !buffer.isNull() ) {
        QSize scaled = buffer.size();
        scaled.scale( width(), height(), Qt::KeepAspectRatio ); // ### optimize

        painter.drawImage( QRect( QPoint( (width() - scaled.width()) / 2, (height() - scaled.height()) / 2  ), scaled ),
             buffer, buffer.rect() );
    }
}


DirectPainterVideoWidget::DirectPainterVideoWidget( GenericVideoSurface* surface, QWidget* parent )
    : QWidget( parent ), m_surface( surface )
{
    HX_ADDREF( m_surface );

    m_surface->attach( this );

    // Optimize paint event
    setAttribute( Qt::WA_NoSystemBackground );
}

DirectPainterVideoWidget::~DirectPainterVideoWidget()
{
    QDirectPainter::reserveRegion( QRegion() );

    m_surface->detach( this );

    HX_RELEASE( m_surface );
}

void DirectPainterVideoWidget::update( Subject* )
{
    QImage buffer = m_surface->buffer();

    if( m_buffer.isNull() ||
        m_buffer.width() != buffer.width() ||
        m_buffer.height() != buffer.height() ) {
            m_buffer = buffer;
            calcDestRect();
    } else {
            m_buffer = buffer;
    }

    QWidget::update();
}

int DirectPainterVideoWidget::isSupported()
{
    return QDirectPainter::screenDepth() == 32 && !qt_screen->isTransformed();
}

static bool isQVFb()
{
    static bool result, test = true;

    if( test ) {
        QByteArray display_spec( getenv( "QWS_DISPLAY" ) );

        test = false;
        if( display_spec.startsWith( "QVFb" ) ) {
            return result = true;
        }
        return result = false;
    }

    return result;
}

void DirectPainterVideoWidget::paintEvent( QPaintEvent* )
{
    static const int PRECISION = 16;

    if( !m_buffer.isNull() ) {
        // Clear widget
        QRgb *o = ( QRgb* )QDirectPainter::frameBuffer();
        QRect clearrect = QRect( mapToGlobal( QPoint( 0, 0 ) ), size() );
        for( int j = clearrect.top(); j <= clearrect.bottom(); ++j ) {
            for( int i = clearrect.left(); i <= clearrect.right(); ++i ) {
                *(o + j*QDirectPainter::screenWidth() + i) = qRgb( 0, 0, 0 );
            }
        }

        // Paint buffer
        // Calculate mapping factors
        uint factor_x = ( m_buffer.width() << PRECISION ) / m_destrect.width();
        uint factor_y = ( m_buffer.height() << PRECISION ) / m_destrect.height();

        QRgb *src = ( QRgb* )m_buffer.bits();
        QRgb *dest = ( QRgb* )QDirectPainter::frameBuffer();
        // For each pixel in buffer
        uint y = 0;
        for( int j = m_destrect.top(); j <= m_destrect.bottom(); ++j ) {
            uint yd = y >> PRECISION;
            uint x = 0;
            for( int i = m_destrect.left(); i <= m_destrect.right(); ++i ) {
                // Calculate position in image
                // Copy pixel value from image into buffer
                *(dest + j*QDirectPainter::screenWidth() + i) = *(src + yd*m_buffer.width() + (x >> PRECISION));
                x += factor_x;
            }
            y += factor_y;
        }

        if( isQVFb() ) {
            // Inform QVFb of change in framebuffer
            QScreen::instance()->setDirty( m_destrect );
        }
    }
}

void DirectPainterVideoWidget::resizeEvent( QResizeEvent* )
{
    calcDestRect();

    QDirectPainter::reserveRegion( QRect( mapToGlobal( QPoint( 0, 0 ) ), size() ) );
}

void DirectPainterVideoWidget::showEvent( QShowEvent* )
{
    QDirectPainter::reserveRegion( QRect( mapToGlobal( QPoint( 0, 0 ) ), size() ) );
}

void DirectPainterVideoWidget::hideEvent( QHideEvent* )
{
    QDirectPainter::reserveRegion( QRegion() );
}

void DirectPainterVideoWidget::calcDestRect()
{
    if( !m_buffer.isNull() ) {
        QSize destsize = m_buffer.size();
        destsize.scale( size(), Qt::KeepAspectRatio );

        QPoint desttopleft = QPoint( (width() - destsize.width()) / 2,
            (height() - destsize.height()) / 2 );

        m_destrect = QRect( mapToGlobal( desttopleft ), destsize );
    }
}
