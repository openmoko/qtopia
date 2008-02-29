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

#ifndef SELECTEDITEM_H
#define SELECTEDITEM_H

#include "griditemtable.h"

#include <QGraphicsRectItem>
#include <QColor>

class QGraphicsScene;
class GridItem;
class QPixmap;
class GridItem;
class QPainter;
class QStyleOptionGraphicsItem;
class QKeyEvent;
class SelectedItemConnector;
class QTimeLine;
class QMovie;
class QImage;


class SelectedItem : public QGraphicsRectItem
{
public:

    SelectedItem(const QString &backgroundFileName,
                 int margin,int moveTimeInterval = 500,QGraphicsScene *scene = 0);

    ~SelectedItem();

    bool addItem(GridItem *);

    GridItem *getCurrent() const;

    GridItem *getItem(int row,int column) const;

    void setCurrent(int row,int column,bool animate = true);

    void setCurrent(GridItem *gridItem,bool animate = true);

    QObject *getConnector() const;

    void setActive(bool isActive);

    void updateImages();

    void resize();

    void paint(QPainter *painter,const QStyleOptionGraphicsItem *option,QWidget *widget);

    void triggerItemSelected(GridItem *);

    void moveStep(int percent);

    void moveFinished();

    void playStep(int);

    void startAnimating();

    void startAnimationDelayed();

    // There is no animationChanged() function, since the SelectedItemConnector just calls update
    // on this object, and the paint(...) method takes care of the rest.

    void animationFinished();

    void resetAnimation();

protected:

    void keyPressEvent(QKeyEvent *event);

private:

    // Direction for keys to move in.
    typedef enum{Up,Down,Left,Right} Direction;

    // Get manual animations to run for 3 seconds.
    // TODO: make this configurable.
    static const int ANIMATION_TIME = 3000;

    static const int ANIMATION_DELAY_INTERVAL = 100;

    // Colour used to blend images during painting when the SelectedItem is 'active'.
    const QColor highlightColor;
    // Colour used to blend the background image during painting.
    const QColor backgroundHighlightColor;

    // Unimplemented methods to prevent copying and assignment.
    SelectedItem(const SelectedItem &);
    SelectedItem & operator=(const SelectedItem &);

    void createMoveTimeLine();

    void createPlayTimeLine();

    void moveTo(GridItem *);

    void moveRequested(Direction);

    bool isAnimating();

    void detachAnimation();

    void stopAnimation();

    QSize getSelectedSize(GridItem *item) const;

    QPoint getPos(GridItem *) const;

    QRect getGeometry(GridItem *) const;

    void drawBackground(QPainter *);

    void draw(QPainter *,const QPixmap &,int x,int y) const;

    void drawAnimated(QPainter *);

    static void blendColor(QImage &img,QColor color);

    // Provides the signals/slots mechanism for this object.
    SelectedItemConnector *connector;

    // The background image - created on demand.
    QPixmap *background;
    // Filename for the background image.
    QString backgroundFileName;

    // The SelectedItem is larger than the GridItem that it magnifies. The 'margin' attribute
    // is the number of extra pixels added to each border of the current GridItem.
    int margin;

    // The dimensions of this item.
    mutable QSize selectedSize;

    // Table of all items. Note that the scene is responsible for deleting the items.
    GridItemTable table;

    // The SelectedItem is positioned over whichever GridItem object is deemed to be current.
    GridItem *currentItem;
    // When SelectedItem is moving from the currentItem to a neighbouring GridItem object,
    // the destination object is stored in this attribute.
    GridItem *destItem;

    // Used to animate the current item. The movie object itself is obtained from currentItem,
    // and SelectedItem has no ownership of it. The movie is connected to connector's
    // animationChanged(), animationFinished() and animationError(...) slots.
    QMovie *movie;

    // When true, the SelectedItem is drawn differently. False by default.
    bool active;

    // Used to move the SelectedItem from the current GridItem object to a destination object.
    // The amount of time it takes to move an item is determine by the ctor's 'slideTimeInterval'
    // parameter. The timeline is connected to connector's moving(...) and movingStateChanged(...)
    // slots.
    QTimeLine *moveTimeLine;

    // Used to manually animate SVG items. (If a movie is available, it will be used to do the
    // animation instead.)
    QTimeLine *playTimeLine;

    // Number of milliseconds that it should take for the SelectedItem to move
    // across from the current GridItem object to a neighbour in the grid.
    int slideTimeInterval;

    // Used during manual animations, i.e. in response to a playTimeLine signal.
    // This will be a value in the range [0,100].
    int animationStage;

    bool animationPending;

    // The following values are used during moving - i.e. during moveStep(...) we have the means
    // to figure out what the magnified space between the two pixmaps shoudl be during this step,
    // and we use this to find the SelectedItem positions for the two GridItems, currentItem and
    // destItem. We can then use these values during paint(...).
    int currentX;
    int currentY;
    int destX;
    int destY;
};

#endif
