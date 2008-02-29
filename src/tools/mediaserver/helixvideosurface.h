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
    virtual void paintNotification() = 0;
};

class GenericVideoSurface : public IHXVideoSurface
{
public:
    GenericVideoSurface();

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

    void addPaintObserver(PaintObserver* paintObserver);

private:
    INT32 m_refCount;

    HelixColorLibrary m_library;

    QImage              m_buffer;
    LPHXCOLORCONVERTER  Converter;
    int                 m_bufferPitch;
    int                 m_inPitch;
    int                 m_bufferWidth;
    int                 m_bufferHeight;
    PaintObserver*      m_paintObserver;
};

class GenericVideoWidget :
    public QWidget,
    public PaintObserver
{
public:
    GenericVideoWidget( GenericVideoSurface* surface, QWidget* parent = 0 );
    ~GenericVideoWidget();

    // Observer
    void paintNotification();

protected:
    // QWidget
    void paintEvent( QPaintEvent* e );

private:
    GenericVideoSurface *m_surface;
};

class DirectPainterVideoWidget :
    public QWidget,
    public PaintObserver
{
public:
    DirectPainterVideoWidget( GenericVideoSurface* surface, QWidget* parent = 0 );
    ~DirectPainterVideoWidget();

    // Observer
    void paintNotification();

    static int isSupported();

protected:
    // QWidget
    void paintEvent( QPaintEvent* e );
    void resizeEvent( QResizeEvent* e );

    void showEvent( QShowEvent* e );
    void hideEvent( QHideEvent* e );

private:
    void paint();

    void calcDestRect();

    GenericVideoSurface *m_surface;

    QRect       m_destrect;
    QRegion     m_bufferRegion;
    QPoint      m_destTopLeft;
    QSize       m_destSize;
    QRegion     m_reservedRegion;
    bool        m_clear;
    bool        m_isVisible;
    bool        m_firstPaintCalc;
};

#endif // HELIXVIDEOSURFACE_H
