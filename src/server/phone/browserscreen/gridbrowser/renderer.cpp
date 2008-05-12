/****************************************************************************
**
** Copyright (C) 2008-2008 TROLLTECH ASA. All rights reserved.
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

#include "renderer.h"
#include <QSvgRenderer>
#include <QPicture>
#include <QPainter>
#include <QFileInfo>
#include <QDebug>

class SvgRenderer : public Renderer
{
public:
    bool load(const QString &filename);
    void render(QPainter *painter, const QRectF &bounds);

private:
    QSvgRenderer renderer;
};

bool SvgRenderer::load(const QString &filename)
{
    if (renderer.load(filename) && renderer.isValid())
        return true;

    return false;
}

void SvgRenderer::render(QPainter *painter, const QRectF &bounds)
{
    renderer.render(painter, bounds);
}

//===========================================================================
#ifndef QT_NO_PICTURE
class PictureRenderer : public Renderer
{
public:
    bool load(const QString &filename);
    void render(QPainter *painter, const QRectF &bounds);

private:
    QPicture picture;
    QString fname;
};

bool PictureRenderer::load(const QString &filename)
{
    fname = filename;
    if (picture.load(filename) && picture.size() > 0)
        return true;

    return false;
}

void PictureRenderer::render(QPainter *painter, const QRectF &bounds)
{
    QRect br = picture.boundingRect();
    QTransform worldTransform = painter->worldTransform();
    painter->translate(bounds.topLeft());
    painter->scale(bounds.width()/br.width(), bounds.height()/br.height());
    painter->drawPicture(br.topLeft(), picture);
    painter->setWorldTransform(worldTransform);
}
#endif
//===========================================================================

Renderer *Renderer::rendererFor(const QString &filename)
{
    QFileInfo fi(filename);
    Renderer *r = 0;
    if (fi.suffix() == "svg") {
        r = new SvgRenderer;
#ifndef QT_NO_PICTURE
    } else if (fi.suffix() == "pic") {
        r = new PictureRenderer;
#endif
    }

    if (r && !r->load(fi.filePath())) {
        delete r;
        r = 0;
    }

    return r;
}

