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

#ifndef QIMAGEDOCUMENTSELECTOR_H
#define QIMAGEDOCUMENTSELECTOR_H

#include <qcontent.h>
#include <qcontentset.h>
#include <qcategorymanager.h>
#include <qsoftmenubar.h>
#include <qdocumentselector.h>

#include <QString>
#include <QStringList>
#include <QPixmap>
#include <QSize>
#include <QPoint>
#include <QDialog>

class QImageDocumentSelectorPrivate;

class QTOPIA_EXPORT QImageDocumentSelector : public QWidget
{
    Q_OBJECT
public:
    explicit QImageDocumentSelector( QWidget* parent = 0 );

    ~QImageDocumentSelector();

    enum ViewMode { Single, Thumbnail };
    void setViewMode( ViewMode mode );
    ViewMode viewMode() const;

    void setThumbnailSize( const QSize& size );
    QSize thumbnailSize() const;

    QContentFilter filter() const;
    void setFilter( const QContentFilter &filter );

    QDocumentSelector::SortMode sortMode() const;
    void setSortMode( QDocumentSelector::SortMode sortMode );

    void setSortCriteria( const QContentSortCriteria &sort );
    QContentSortCriteria sortCriteria() const;

    QContent currentDocument() const;
    const QContentSet &documents() const;

    QSize sizeHint() const;

    void setDefaultCategories( const QStringList &categories );
    QStringList defaultCategories() const;

    void setSelectPermission( QDrmRights::Permission permission );
    QDrmRights::Permission selectPermission() const;

    void setMandatoryPermissions( QDrmRights::Permissions permissions );
    QDrmRights::Permissions mandatoryPermissions() const;

public slots:
    void setFocus();

signals:
    void documentSelected( const QContent& image );
    void documentHeld( const QContent& image, const QPoint& pos );

    void documentsChanged();

private:
    QImageDocumentSelectorPrivate *d;
};

class QImageDocumentSelectorDialogPrivate;

class QTOPIA_EXPORT QImageDocumentSelectorDialog : public QDialog
{
    Q_OBJECT
public:
    explicit QImageDocumentSelectorDialog( QWidget* parent = 0 );

    ~QImageDocumentSelectorDialog();

    void setThumbnailSize( const QSize& size );
    QSize thumbnailSize() const;

    QContentFilter filter() const;
    void setFilter( const QContentFilter &filter );

    QDocumentSelector::SortMode sortMode() const;
    void setSortMode( QDocumentSelector::SortMode sortMode );

    void setSortCriteria( const QContentSortCriteria &sort );
    QContentSortCriteria sortCriteria() const;

    QContent selectedDocument() const;
    const QContentSet &documents() const;

    void setDefaultCategories( const QStringList &categories );
    QStringList defaultCategories() const;

    void setSelectPermission( QDrmRights::Permission permission );
    QDrmRights::Permission selectPermission() const;

    void setMandatoryPermissions( QDrmRights::Permissions permissions );
    QDrmRights::Permissions mandatoryPermissions() const;

public slots:
    void accept();

    void reject();

private slots:
    void setContextBar();
    void viewImage();

private:
    void init();
    QImageDocumentSelectorDialogPrivate *d;
    QImageDocumentSelector *selector;
};

#endif // QIMAGEDOCUMENTSELECTOR_H

