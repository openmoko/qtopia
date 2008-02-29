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

#ifndef QDOCUMENTSELECTOR_H
#define QDOCUMENTSELECTOR_H

#include <qcontentfilter.h>
#include <QDialog>

class QDrmContent;
class QDocumentSelectorPrivate;
class QContentSortCriteria;

class QTOPIA_EXPORT QDocumentSelector : public QWidget
{
    Q_OBJECT
public:

    enum Option
    {
        None         = 0,
        NewDocument  = 1,
        TypeSelector = 2,
        NestTypes  = 4,
        ContextMenu  = 8
    };

    enum SortMode
    {
        Alphabetical,
        ReverseAlphabetical,
        Chronological,
        ReverseChronological,
        SortCriteria
    };

    enum Selection
    {
        NewSelected,
        DocumentSelected,
        Cancelled
    };

    Q_DECLARE_FLAGS( Options, Option );

    explicit QDocumentSelector( QWidget *parent = 0 );

    ~QDocumentSelector();

    QContentFilter filter() const;
    void setFilter( const QContentFilter &filter );

    void setDefaultCategories( const QStringList &categories );
    QStringList defaultCategories() const;

    void setSelectPermission( QDrmRights::Permission permission );
    QDrmRights::Permission selectPermission() const;

    void setMandatoryPermissions( QDrmRights::Permissions permissions );
    QDrmRights::Permissions mandatoryPermissions() const;

    void setSortMode( SortMode mode );
    SortMode sortMode() const;

    void setSortCriteria( const QContentSortCriteria &sort );
    QContentSortCriteria sortCriteria() const;

    Options options() const;
    void setOptions( Options options );
    void enableOptions( Options option );
    void disableOptions( Options option );

    QContent currentDocument() const;

    bool newCurrent() const;

    const QContentSet &documents() const;

    static Selection select( QWidget *parent, QContent *content, QDrmRights::Permission permission, const QString &title, const QContentFilter &filter, Options options = ContextMenu, SortMode sortMode = Alphabetical );

    static Selection select( QWidget *parent, QContent *content, const QString &title, const QContentFilter &filter, Options options = ContextMenu, SortMode sortMode = Alphabetical );

signals:
    void documentSelected( const QContent &content );
    void currentChanged();
    void newSelected();
    void documentsChanged();

private:
    QDocumentSelectorPrivate *d;
};

class QDocumentSelectorDialogPrivate;

class QTOPIA_EXPORT QDocumentSelectorDialog : public QDialog
{
    Q_OBJECT
public:

    explicit QDocumentSelectorDialog( QWidget *parent = 0 );
    ~QDocumentSelectorDialog();

    QContentFilter filter() const;
    void setFilter( const QContentFilter &filter );

    void setDefaultCategories( const QStringList &categories );
    QStringList defaultCategories() const;

    void setSelectPermission( QDrmRights::Permission permission );
    QDrmRights::Permission selectPermission() const;

    void setMandatoryPermissions( QDrmRights::Permissions permissions );
    QDrmRights::Permissions mandatoryPermissions() const;

    void setSortMode( QDocumentSelector::SortMode mode );
    QDocumentSelector::SortMode sortMode() const;

    void setSortCriteria( const QContentSortCriteria &sort );
    QContentSortCriteria sortCriteria() const;

    QDocumentSelector::Options options() const;
    void setOptions( QDocumentSelector::Options options );
    void enableOptions( QDocumentSelector::Options option );
    void disableOptions( QDocumentSelector::Options option );

    QContent selectedDocument() const;
    bool newSelected() const;

    const QContentSet &documents() const;

private:
    QDocumentSelectorDialogPrivate *d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QDocumentSelector::Options );
#endif // QDOCUMENTSELECTOR_H
