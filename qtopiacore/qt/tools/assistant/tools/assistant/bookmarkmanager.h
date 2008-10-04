/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the Qt Assistant of the Qt Toolkit.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.  In addition, as a special
** exception, Nokia gives you certain additional rights. These rights
** are described in the Nokia Qt GPL Exception version 1.2, included in
** the file GPL_EXCEPTION.txt in this package.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/

#ifndef BOOKMARKMANAGER_H
#define BOOKMARKMANAGER_H

#include "ui_bookmarkdialog.h"

#include <QtCore/QUrl>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QByteArray>
#include <QtCore/QDataStream>

#include <QtGui/QIcon>
#include <QtGui/QDialog>
#include <QtGui/QWidget>
#include <QtGui/QTreeView>
#include <QtGui/QStandardItemModel>

QT_BEGIN_NAMESPACE

class QEvent;
class QLineEdit;
class QTreeView;
class QToolButton;
class QStandardItem;
class QHelpEngineCore;
class QAbstractItemModel;
class QSortFilterProxyModel;

class BookmarkManager;

class BookmarkDialog : public QDialog
{
    Q_OBJECT

public:
    BookmarkDialog(BookmarkManager *manager, const QString &title, 
        const QString &url, QWidget *parent = 0);
    ~BookmarkDialog();

private slots:
    void addAccepted();
    void addNewFolder();
    void toolButtonClicked();
    void itemChanged(QStandardItem *item);
    void textChanged(const QString& string);
    void selectBookmarkFolder(const QString &folderName);
    void customContextMenuRequested(const QPoint &point);
    void currentChanged(const QModelIndex& current, const QModelIndex& previous);
    
private:
    bool eventFilter(QObject *object, QEvent *e);

private:
    QString m_url;
    QString m_title;

    QString oldText;
    QStandardItem *renameItem;

    Ui::BookmarkDialog ui;
    BookmarkManager *bookmarkManager;
    QSortFilterProxyModel *proxyModel;
};

class TreeView : public QTreeView {
    Q_OBJECT
public:
    TreeView(QWidget* parent = 0) : QTreeView(parent) {}
    void subclassKeyPressEvent(QKeyEvent* event)
    {
        QTreeView::keyPressEvent(event);
    }
};

class BookmarkWidget : public QWidget
{
    Q_OBJECT

public:
    BookmarkWidget(BookmarkManager *manager, QWidget *parent = 0);
    ~BookmarkWidget();

signals:
    void addBookmark();
    void requestShowLink(const QUrl &url);

private slots:
    void removeClicked();
    void filterChanged();
    void expand(const QModelIndex& index);
    void activated(const QModelIndex &index);
    void customContextMenuRequested(const QPoint &point);

private:
    void setup();
    void expandItems();
    void focusInEvent(QFocusEvent *e);
    bool eventFilter(QObject *object, QEvent *event);

private:
    QRegExp regExp;
    TreeView *treeView;
    QLineEdit *searchField;
    QToolButton *addButton;
    QToolButton *removeButton;
    BookmarkManager *bookmarkManager;
    QSortFilterProxyModel* filterBookmarkModel;
};

class BookmarkModel : public QStandardItemModel
{
    Q_OBJECT

public:
    BookmarkModel(int rows, int columns, QObject *parent = 0);
    ~BookmarkModel();

    Qt::DropActions supportedDropActions() const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
};

class BookmarkManager : public QObject
{
    Q_OBJECT

public:
    BookmarkManager(QHelpEngineCore* helpEngine);
    ~BookmarkManager();

    BookmarkModel* treeBookmarkModel();
    BookmarkModel* listBookmarkModel();

    void saveBookmarks();
    QStringList bookmarkFolders() const;
    QModelIndex addNewFolder(const QModelIndex& index);
    void removeBookmarkItem(QTreeView *treeView, const QModelIndex& index);
    void showBookmarkDialog(QWidget* parent, const QString &name, const QString &url);
    void addNewBookmark(const QModelIndex& index, const QString &name, const QString &url);
    void setupBookmarkModels();

private slots:
    void itemChanged(QStandardItem *item);

private:    
    QString uniqueFolderName() const;
    void removeBookmarkFolderItems(QStandardItem *item);
    void readBookmarksRecursive(const QStandardItem *item, QDataStream &stream,
        const qint32 depth) const;

private:
    QString oldText;
    QIcon folderIcon;
    
    BookmarkModel *treeModel;
    BookmarkModel *listModel;
    QStandardItem *renameItem;
    QHelpEngineCore *helpEngine;
};

QT_END_NAMESPACE

#endif
