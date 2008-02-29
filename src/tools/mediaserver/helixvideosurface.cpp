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

GenericVideoSurface::GenericVideoSurface():
    m_refCount(0),
    Converter(0),
    m_paintObserver(0)
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

static inline bool is16Bit()
{
    return qt_screen->depth() == 16;
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

void GenericVideoSurface::addPaintObserver(PaintObserver* paintObserver)
{
    m_paintObserver = paintObserver;
}

GenericVideoWidget::GenericVideoWidget( GenericVideoSurface* surface, QWidget* parent )
    : QWidget( parent ), m_surface( surface )
{
    HX_ADDREF( m_surface );

    m_surface->addPaintObserver(this);

    // Optimize paint event
    setAttribute(Qt::WA_NoSystemBackground);

    QPalette pal(palette());
    pal.setBrush(QPalette::Window, Qt::black);

    setPalette(pal);
}

GenericVideoWidget::~GenericVideoWidget()
{
    HX_RELEASE(m_surface);
}

void GenericVideoWidget::paintNotification()
{
    QWidget::update();
}

void GenericVideoWidget::paintEvent( QPaintEvent* )
{
    QPainter painter( this );
    painter.setCompositionMode( QPainter::CompositionMode_Source );

    QImage const& buffer = m_surface->buffer();
    if( !buffer.isNull() ) {
        QSize scaled = buffer.size();
        scaled.scale( width(), height(), Qt::KeepAspectRatio ); // ### optimize

        painter.drawImage( QRect( QPoint( (width() - scaled.width()) / 2, (height() - scaled.height()) / 2  ), scaled ),
             buffer, buffer.rect() );
    }
}


DirectPainterVideoWidget::DirectPainterVideoWidget( GenericVideoSurface* surface, QWidget* parent ):
    QWidget(parent),
    m_surface(surface),
    m_clear(true),
    m_isVisible(false),
    m_firstPaintCalc(true)
{
    HX_ADDREF(m_surface);

    m_surface->addPaintObserver(this);

    // Optimize paint event
    setAttribute(Qt::WA_NoSystemBackground);
}

DirectPainterVideoWidget::~DirectPainterVideoWidget()
{
    QDirectPainter::reserveRegion(QRegion());

    HX_RELEASE(m_surface);
}

void DirectPainterVideoWidget::paintNotification()
{
    if (m_isVisible)
    {
        if (m_firstPaintCalc)
        {
            calcDestRect();
            m_firstPaintCalc = false;
        }

        paint();
    }
}

int DirectPainterVideoWidget::isSupported()
{
    return true;
}

static bool isQVFb()
{
    static bool result, test = true;

    if (test)
    {
        test = false;

        QByteArray display_spec(getenv("QWS_DISPLAY"));

        result = display_spec.startsWith("QVFb");
    }

    return result;
}

void DirectPainterVideoWidget::paintEvent( QPaintEvent* )
{
    m_clear = true;
    paint();
}

void DirectPainterVideoWidget::resizeEvent(QResizeEvent*)
{
    calcDestRect();

    m_reservedRegion = QDirectPainter::reserveRegion(QRect(mapToGlobal(QPoint(0, 0)), size()));
}

void DirectPainterVideoWidget::showEvent(QShowEvent*)
{
    m_isVisible = true;
    m_reservedRegion = QDirectPainter::reserveRegion(QRect(mapToGlobal(QPoint(0, 0)), size()));
}

void DirectPainterVideoWidget::hideEvent( QHideEvent* )
{
    m_isVisible = false;
    m_reservedRegion = QDirectPainter::reserveRegion(QRegion());
}

void DirectPainterVideoWidget::paint()
{
    // Clear widget
    if (m_clear)
    {
        m_clear = false;

        qt_screen->solidFill(Qt::black, m_reservedRegion);
    }

    QImage const& buffer = m_surface->buffer();

    if (!buffer.isNull())
    {
        qt_screen->blit(buffer.scaled(m_destSize),
                                      m_destTopLeft,
                                      m_bufferRegion);

        if (isQVFb())
        {
            // Inform QVFb of change in framebuffer
            QScreen::instance()->setDirty(m_destrect);
        }
    }
}

void DirectPainterVideoWidget::calcDestRect()
{
    QImage const& buffer = m_surface->buffer();
    if (!buffer.isNull())
    {
        m_destSize = buffer.size();

        m_destSize.scale(size(), Qt::KeepAspectRatio);

        m_destTopLeft = mapToGlobal(QPoint((width() - m_destSize.width()) / 2,
                                           (height() - m_destSize.height()) / 2));

        m_destrect = QRect(m_destTopLeft, m_destSize);

        m_bufferRegion = QRegion(m_destrect);
    }
}
