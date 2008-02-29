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

#ifndef PHONELAUNCHERVIEW_H
#define PHONELAUNCHERVIEW_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QContent>

class QPixmap;
class QResizeEvent;
class QFocusEvent;
class GridItem;
class QColor;
class SelectedItem;
class QKeyEvent;
class QMouseEvent;
class Animator;


class PhoneLauncherView : public QGraphicsView
{
    Q_OBJECT

public:

    PhoneLauncherView(int rows, int cols, const QString &mapping,
                      const QString &animator,const QString &animatorBackground,
                      QWidget *parent=0);

    virtual ~PhoneLauncherView();

    void addItem(QContent *app, int pos = -1);

    void setBusy(bool v);

    QContent *currentItem() const;

    void setCurrentItem(int);

    void updateImages();

signals:

    void clicked(QContent);

    void highlighted(QContent);

protected:

    void keyPressEvent(QKeyEvent *event);

    void mousePressEvent(QMouseEvent *event);

    void mouseReleaseEvent(QMouseEvent *event);

    void resizeEvent (QResizeEvent *event);

    void focusInEvent(QFocusEvent *event);

private slots:

    void itemSelectedHandler(GridItem *);

    void selectionChangedHandler(GridItem *);

private:

    void addSelectedItem(const QString &backgroundFileName,int moveTimeDuration);

    GridItem *createItem(QContent *content,int row,int column) const;

    void getItemDimensions(int &itemWidth,int &itemHeight) const;

    void getRowAndColumn(int idx,int &row,int &column) const;

    GridItem *getCurrentItem() const;

    GridItem *getItemAt(const QPoint &point) const;

    // Potential value of margin - used for selectedItem.
    static const int MARGIN_MOUSE_PREFERRED = 2;
    // Potential value of margin - used for selectedItem.
    static const int MARGIN_DEFAULT = 6;

    // Number of milliseconds that selectedItem takes to move across to a neighbouring
    // GridItem - used for selectedItem.
    static const int DEFAULT_MOVE_TIME_DURATION = 400;

    // Potential file used for the background image of selectedItem.
    static const QString SELECTED_BACKGROUND_FILE_MOUSE_PREFERRED;
    // Potential file used for the background image of selectedItem.
    static const QString SELECTED_BACKGROUND_FILE_DEFAULT;

    // Number of rows in the grid.
    int rows;
    // Number of columns in the grid.
    int columns;

    // The scene that is displayed by this view.
    QGraphicsScene *scene;

    // The selected item, which is positioned over and highlights the currently selected GridItem.
    SelectedItem *selectedItem;
    // Used by mousePressEvent(...) and mouseReleaseEvent(...).
    GridItem *pressedItem;

    // String to be passed to AnimatorFactory to construct manual Animator objects for GridItems.
    QString animatorDescription;
    // Background animator for manual animation - to be shared by all the GridItem objects.
    Animator *animatorBackground;

    // Provided by the ctor as a list of characters that map to GridItem objects according
    // to their position.
    QString iconMapping;

    // One of MARGIN_MOUSE_PREFERRED or MARGIN_DEFAULT. Passed to SelectedItem.
    int margin;
};

#endif
