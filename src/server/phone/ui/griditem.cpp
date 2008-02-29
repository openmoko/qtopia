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

#include "griditem.h"
#include "animator_p.h"

#include <QContent>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QPixmap>
#include <QApplication>
#include <QMovie>
#include <QPen>
#include <QByteArray>


const QString GridItem::MOVIE_PREFIX(":image/");
const QString GridItem::MOVIE_SUFFIX("_48_anim");
const qreal GridItem::SELECTED_IMAGE_SIZE_FACTOR = 0.4;

const QColor GridItem::DEFAULT_PEN_COLOR(0xbb,0xbb,0xbb);
const Qt::PenStyle GridItem::DEFAULT_PEN_STYLE(Qt::DotLine);


/*!
  \internal
  \class GridItem

  \brief GridItem is a graphical object used to populate the scene required by
  PhoneLauncherView.

  \mainclass Each GridItem object stores its own row and column indices for its
  position in the grid in which it will be displayed.

  In addition to using \a basicPixmap to draw itself, GridItem creates and
  maintains the pixmap used to display SelectedItem (when it is positioned over this
  GridItem object), an SVG renderer (if the GridItem can be loaded from an SVG file)
  and the movie used to animate SelectedItem (when it is positioned
  over this GridItem object).

  Note that GridItem objects cannot be copied or assigned to.

  It is possible to create an 'empty' GridItem object, i.e. one that contains no content. Such an
  object can be used in those cases where a particular version of Qtopia will not release all of
  the available applications.

  \sa PhoneLauncherView
  \sa SelectedItem
*/


/*!
  \internal
  \fn GridItem::GridItem(QContent *content,int row,int col,int totalRows,int totalCols,Animator *animator,Animator *animatorBackground,bool loadMovie,QGraphicsScene *scene)
  Creates the images and the movie required to display both this object and the SelectedItem object.

  \a content: Contains information used to create the pixmaps and the movie. This may be 0, in which case
      an 'empty' GridItem object will be created - see documentation above.
  \a row: The row index for this item, i.e. where it will be positioned within its containing grid.
  \a col: The column index for this item, i.e. where it will be positioned within its containing grid.
  \a totalRows: The total number of rows in the grid in which this item is contained.
  \a totalCols: The total number of columns in the grid in which this item is contained.
  \a animator: An object which is able to animate the GridItem when it is selected (i.e. by
  SelectedItem), in the absence of a QMovie object.
  \a animatorBackground: An object which (optionally) provides a background for manual animation.
  \a loadMovie: Determines whether or not the object should search for a movie file for its animation.
  \a scene: The (optional) scene on which this item should be drawn.
*/
GridItem::GridItem(QContent *_content,int _row,int _col,
                   int _totalRows,int _totalCols,
                   Animator *animator,Animator *animatorBackground,
                   bool _loadMovie,
                   QGraphicsScene *scene)
    : QGraphicsRectItem(0,scene)
    , content(_content)
    , row(_row)
    , col(_col)
    , totalRows(_totalRows)
    , totalCols(_totalCols)
    , selectedImageSize(-1)
    , basicPixmap()
    , selectedPixmap()
    , selectedMovie(0)
    , selectedAnimator(animator)
    , selectedBackgroundAnimator(animatorBackground)
    , loadMovie(_loadMovie)
{
    // Set up a pen to use for drawing operations.
    QPen pen(DEFAULT_PEN_COLOR);
    pen.setStyle(DEFAULT_PEN_STYLE);
    setPen(pen);

    updateImages();
}

/*!
  \internal
  \fn GridItem::~GridItem()
*/
GridItem::~GridItem()
{
    if ( selectedAnimator ) {
        delete selectedAnimator;
    }
    if ( selectedMovie ) {
        delete selectedMovie;
    }
}

/*!
  \internal
  \fn QContent *GridItem::getContent() const
*/

/*!
  \internal
  \fn void GridItem::setContent(QContent *content)
  Used to modify the contents (e.g. the icon) of this GridItem.
*/
void GridItem::setContent(QContent *_content)
{
    content = _content;
    updateImages();
    selectedPixmap = QPixmap();
}

/*!
  \internal
  \fn int GridItem::getRow() const
  Returns the grid row that this GridItem resides in. Rows are counted from 0.
*/

/*!
  \internal
  \fn int GridItem::getColumn() const
  Returns the grid column that this GridItem resides in. Columns are counted from 0.
*/

/*!
  \internal
  \fn void GridItem::updateImages()
  Called by the ctor. Creates the QPixmaps and the QMovie used to display this object and the
  SelectedItem object, based on information in content.
*/
void GridItem::updateImages()
{
    if ( selectedMovie ) {
        delete selectedMovie;
        selectedMovie = 0;
    }

    if ( !content ) {
        // We won't be able to make any images. It is valid to have an empty content - see doco above.
        basicPixmap = QPixmap();
        if ( renderer.isValid() ) {
            // Make sure it's not anymore! Load an empty byte array.
            renderer.load(QByteArray());
        }
        return;
    }

    // Retrieve the static images, one for displaying the GridItem (basicPixmap)
    // the other for displaying the SelectedItem when it is positioned over this
    // GridItem.
    int basicImageSize = getBasicImageSize();

    // First, attempt to create the SVG renderer.
    renderer.load(":image/" + content->iconName() + ".svg");
    if ( renderer.isValid() ) {
        // We use the renderer to create two Pixmaps -- one to display the GridItem itself, and
        // one to display the SelectedItem when it is magnifying the GridItem object.
        // Obviously, the SelectedItem's Pixmap is exactly the same as the GridItem's Pixmap,
        // except that it has a different size. We hang on to the renderer, because we may need
        // that to do manual animations.
        QImage image = QImage(basicImageSize,basicImageSize,QImage::Format_ARGB32_Premultiplied);
        image.fill(0);
        QPainter painter(&image);
        renderer.render(&painter,QRectF(0,0,basicImageSize,basicImageSize));
        painter.end();
        basicPixmap = QPixmap::fromImage(image);
    } else {
        // Probably not an SVG file - use the content's icon, directly, to get pixmaps.
        basicPixmap = content->icon().pixmap(basicImageSize);
    }

    // Create the movie.
    if ( loadMovie && !(content->iconName().isNull()) ) {
        selectedMovie = new QMovie(MOVIE_PREFIX  + content->iconName() + MOVIE_SUFFIX);
        if ( selectedMovie->isValid() ) {
            // TODO !!!!!!!!!!!! THIS HAS GOT TO GO. Waiting for Oslo...
            selectedMovie->setCacheMode(QMovie::CacheAll);
        } else {
            // This movie was never meant to be...
            delete selectedMovie;
            selectedMovie = 0;
        }
    }
}


/*!
  \internal
  Returns the SelectedItem object, based on information in content.
*/
const QPixmap &GridItem::selectedPic() const
{
    if (selectedPixmap.isNull()) {
        int selectedImageSize = getSelectedImageSize();
        if ( renderer.isValid() ) {
            QImage image = QImage(selectedImageSize,selectedImageSize,QImage::Format_ARGB32_Premultiplied);
            image.fill(0);
            QPainter painter(&image);
            const_cast<GridItem *>(this)->renderer.render(&painter,QRectF(0,0,selectedImageSize,selectedImageSize));
            painter.end();
            const_cast<GridItem *>(this)->selectedPixmap = QPixmap::fromImage(image);
        } else if ( content ) {
            const_cast<GridItem *>(this)->selectedPixmap = content->icon().pixmap(selectedImageSize);
        }
    }

    return selectedPixmap;
}

/*!
  \internal
  \fn const QPixmap &GridItem::basicPic() const
  This function should be used for drawing the GridItem if there is no valid SVG renderer.
  \sa getSvgRenderer()
*/

/*!
  \internal
  \fn QMovie *GridItem::movie() const
  This function should be used for animating a SelectedItem object, if the animated fil
  is available. If there is no animated file, the SelectedItem will use its own contrived
  animation objects.
*/

/*!
  \internal
  \fn QSvgRenderer *GridItem::getSvgRenderer()
  Returns a valid SVG renderer, or 0.
  The renderer can be used during painting. If there is no valid SVG renderer for this GridItem
  object, the pixmap methods can be used to retrieve pixmaps to draw. NOTE that if possible, the
  pixmap methods should be used, since rendering from SVG is very expensive.
  \sa selectedPic()
*/
QSvgRenderer *GridItem::getSvgRenderer()
{
    if ( renderer.isValid() ) {
        return &renderer;
    }
    return 0;
}

/*!
  \internal
  \fn Animator *GridItem::getSelectedAnimator() const
  Returns the Animator object for manual animation by the SelectedItem, or 0 if none is available.
*/

/*!
  \internal
  \fn Animator *GridItem::getSelectedBackgroundAnimator() const
  Returns the background animator for manual animation by the SelectedItem, or 0 if none is
  available.
*/

/*!
  \internal
  \fn void GridItem::paint(QPainter *painter,const QStyleOptionGraphicsItem *option,QWidget *widget)
  Draws this object, using \a basicPixmap.
*/
void GridItem::paint(QPainter *painter,const QStyleOptionGraphicsItem *,QWidget *)
{
    painter->setPen(pen());

    // Draw the broken lines around the edge of the image.
    int x1 = static_cast<int>(rect().x());
    int y1 = static_cast<int>(rect().y());
    int x2 = x1 + static_cast<int>(rect().width()) - 1;
    int y2 = y1 + static_cast<int>(rect().height()) - 1;
    if ( row > 0 ) {
        // Draw top line.
        painter->drawLine(x1+IMAGE_LINE_OFFSET, y1, x2-IMAGE_LINE_OFFSET, y1);
    }
    if ( row < totalRows-1 ) {
        // Draw bottom line.
        painter->drawLine(x1+IMAGE_LINE_OFFSET, y2, x2-IMAGE_LINE_OFFSET, y2);
    }
    if ( col > 0 ) {
        // Draw left line.
        painter->drawLine(x1, y1+IMAGE_LINE_OFFSET, x1, y2-IMAGE_LINE_OFFSET);
    }
    if ( col < totalCols-1 ) {
        // Draw right line.
        painter->drawLine(x2, y1+IMAGE_LINE_OFFSET, x2, y2-IMAGE_LINE_OFFSET);
    }

    // Position the image in the middle of the drawing area.
    if ( !(basicPixmap.isNull()) ) {
        // Position the image in the middle of the drawing area.
        int imgX = x1 + (static_cast<int>(rect().width()) - basicPixmap.width())/2;
        int imgY = y1 + (static_cast<int>(rect().height()) - basicPixmap.height())/2;
        painter->drawPixmap(imgX,imgY,basicPixmap); //x1,y1,image);
    }
    // If there's no valid pixmap, we don't have much chance of drawing anything, my friend...
}

/*!
  \internal
  \fn QRectF GridItem::getRenderingBounds()
  Creates on demand the bounding infomration for the SVG renderer.
*/
QRectF GridItem::getRenderingBounds()
{
    int requiredSize = getBasicImageSize();

    // We update the rendering bounds if a) we have a valid renderer (otherwise there's no point)
    // and b) either the bounds have not yet been created, or something has caused the basic
    // image size to change - which generally it will not.
    if ( renderer.isValid() &&
         (renderingBounds.isNull() || (requiredSize != renderingBounds.width())) ) {
        // Position the image in the middle of the drawing area.
        int x1 = static_cast<int>(rect().x());
        int y1 = static_cast<int>(rect().y());
        x1 += (static_cast<int>(rect().width()) - requiredSize)/2;
        y1 += (static_cast<int>(rect().height()) - requiredSize)/2;

        renderingBounds.setX(x1);
        renderingBounds.setY(y1);
        renderingBounds.setWidth(requiredSize);
        renderingBounds.setHeight(requiredSize);
    }

    return renderingBounds;
}

/*!
  \internal
  \fn int GridItem::getBasicImageSize() const;
  Returns the image size (in pixels) for GridItem's \a basicImage, for the current resolution.
*/
int GridItem::getBasicImageSize() const
{
    int imageSize = qApp->style()->pixelMetric(QStyle::PM_IconViewIconSize);
    return imageSize;
}

/*!
  \internal
  \fn int GridItem::getSelectedImageSize() const;
  Returns the size of this GridItem when it is selected (i.e. by SelectedItem).
*/
int GridItem::getSelectedImageSize() const
{
    int basicImageSize = getBasicImageSize();

    return basicImageSize + qRound(basicImageSize * SELECTED_IMAGE_SIZE_FACTOR);
}
