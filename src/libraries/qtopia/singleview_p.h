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

#ifndef SINGLEVIEW_P_H
#define SINGLEVIEW_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qtopia API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <qcontent.h>
#include <qcontentset.h>
#include <qcategorymanager.h>

#include <qwidget.h>
#include <qpixmap.h>
#include <qpoint.h>

#include <QAbstractListModel>
#include <QItemSelectionModel>

class SingleView : public QWidget
{
    Q_OBJECT
public:
    explicit SingleView( QWidget* parent = 0, Qt::WFlags f = 0 );
    virtual ~SingleView(){}

    void setModel( QAbstractListModel* );

    QAbstractListModel* model() const;

    void setSelectionModel( QItemSelectionModel* );

    QItemSelectionModel* selectionModel() const;

signals:
    // Select key pressed
    void selected();

    // Only Qtopia PDA
    // Stylus held
    void held( const QPoint& );

protected slots:
    // Update current selection
    void currentChanged( const QModelIndex&, const QModelIndex& );

protected:
    // Draw scaled image onto widget
    void paintEvent( QPaintEvent* );

    // Update current selection
    void keyPressEvent( QKeyEvent* );

    // Emit held signal
    void mousePressEvent( QMouseEvent* );

    virtual QPixmap loadThumbnail( const QString &filename, const QSize &size );

private slots:

    void contentChanged(const QContentIdList&,const QContent::ChangeType);

private:
    // Move selection forward one image
    void moveForward();

    // Mode selection back one image
    void moveBack();

    bool right_pressed;

    QString current_file;
    QPixmap buffer;
    QAbstractListModel *model_;
    QItemSelectionModel *selection_;
};

#endif // SINGLEVIEW_P_H
