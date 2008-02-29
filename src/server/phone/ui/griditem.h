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

#ifndef GRIDITEM_H
#define GRIDITEM_H

#include <QGraphicsRectItem>
#include <QPixmap>
#include <QSvgRenderer>

class QGraphicsScene;
class QMovie;
class QPainter;
class QStyleOptionGraphicsItem;
class QContent;
class Animator;


class GridItem : public QGraphicsRectItem
{
public:

    // Percentage of basic image size that needs to be added to retrieve size of
    // the selected (magnified) image.
    static const qreal SELECTED_IMAGE_SIZE_FACTOR;

    GridItem(QContent *content,int r,int c,
             int totalRows,int totalCols,
             Animator *animator,Animator *animatorBackground,
             bool loadMovie,
             QGraphicsScene *scene = 0);

    ~GridItem();

    void paint(QPainter *painter,const QStyleOptionGraphicsItem *option,QWidget *widget);

    QContent *getContent() const { return content; }

    void setContent(QContent *);

    int getRow() const { return row; }

    int getColumn() const { return col; }

    const QPixmap &basicPic() const { return basicPixmap; }

    const QPixmap &selectedPic() const;

    QSvgRenderer *getSvgRenderer();

    QRectF getSelectedRenderingBounds();

    QMovie *movie() const { return selectedMovie; }

    Animator *getSelectedAnimator() const { return selectedAnimator; }

    Animator *getSelectedBackgroundAnimator() const { return selectedBackgroundAnimator; }

    void updateImages();

    int getSelectedImageSize() const;

    int getBasicImageSize() const;

private:

    // Number of pixels which are 'chopped off' the lines which are drawn to frame the basic image.
    static const int IMAGE_LINE_OFFSET = 5;

    // Information for the movie file.
    static const QString MOVIE_PREFIX;
    static const QString MOVIE_SUFFIX;

    // Line colour used when drawing GridItem objects.
    static const QColor DEFAULT_PEN_COLOR;
    // Line style used when drawing GridItem objects.
    static const Qt::PenStyle DEFAULT_PEN_STYLE;

    // Unimplemented methods to prevent copying and assignment.
    GridItem(const GridItem &);
    GridItem & operator=(const GridItem &);

    // Creates on demand the bounding infomration for the SVG renderer.
    QRectF getRenderingBounds();

    // Contains information used to create the images/movie for this item.
    QContent *content;

    // This item's row index.
    int row;
    // This item's column index;
    int col;
    // The total number of rows in the grid in which this object belongs.
    int totalRows;
    // The total number of columns in the grid in which this object belongs.
    int totalCols;

    // Calculated in getSelectedImageSize()
    int selectedImageSize;

    QSvgRenderer renderer;
    QRectF renderingBounds;

    // Pixmap used to draw this item.
    QPixmap basicPixmap;
    // Pixmap used to draw SelectedItem when it is positioned over this GridItem object.
    QPixmap selectedPixmap;

    // Movie object used to animate the SelectedItem when it is positoned over this GridItem object.
    QMovie *selectedMovie;
    // Animator object used to manually animate the SelectedItem when it is positioned over this GridItem
    // object, if selectedMovie is not available.
    Animator *selectedAnimator;
    // When selectedAnimator is running, it may or may not have an animated background to go
    // with it.
    Animator *selectedBackgroundAnimator;
    // Determines whether or not this object should search for a movie file for animation.
    bool loadMovie;
};

#endif
