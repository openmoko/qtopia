/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#ifndef SELECTEDITEMCONNECTOR_H
#define SELECTEDITEMCONNECTOR_H

#include <QObject>
#include <QTimeLine>
#include <QImageReader>

class SelectedItem;
class GridItem;


class SelectedItemConnector : public QObject
{
    Q_OBJECT

public:

    explicit SelectedItemConnector(SelectedItem *selectedItem);

    void triggerItemSelected(GridItem *);

signals:

    void itemSelected(GridItem *);

    void selectionChanged(GridItem *);

public slots:

    void moving(int);

    void movingStateChanged(QTimeLine::State);

    void startAnimation();

    void animationStateChanged(QTimeLine::State);

    void animationChanged();

    void animationFinished();

    void animationError(QImageReader::ImageReaderError);

    void triggerSelectionChanged(GridItem *);

    void playing(int);

private:

    // Unimplemented methods to prevent copying and assignment.
    SelectedItemConnector(const SelectedItemConnector &);
    SelectedItemConnector & operator=(const SelectedItemConnector &);

    SelectedItem *selectedItem;
};

#endif
