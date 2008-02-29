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
#ifndef QCONTENTFILTERSELECTOR_H
#define QCONTENTFILTERSELECTOR_H

#include <qcontentfiltermodel.h>

class QContentFilterSelectorPrivate;

class QTOPIA_EXPORT QContentFilterSelector : public QWidget
{
    Q_OBJECT
public:
    explicit QContentFilterSelector( QWidget *parent = 0 );
    explicit QContentFilterSelector( const QContentFilterModel::Template &modelTemplate,
                                     QWidget *parent = 0 );
    explicit QContentFilterSelector( QContent::Property property,
                                     QContentFilterModel::TemplateOptions options = QContentFilterModel::SelectAll,
                                     const QStringList &checked = QStringList(),
                                     QWidget *parent = 0 );
    explicit QContentFilterSelector( QContentFilter::FilterType type,
                                     const QString &scope = QString(),
                                     QContentFilterModel::TemplateOptions options = QContentFilterModel::SelectAll,
                                     const QStringList &checked = QStringList(),
                                     QWidget *parent = 0 );
    virtual ~QContentFilterSelector();

    QContentFilter filter();
    void setFilter( const QContentFilter &filter );

    QContentFilterModel::Template modelTemplate() const;
    void setModelTemplate( const QContentFilterModel::Template &modelTemplate );
    void setModelTemplate( QContent::Property property,
                           QContentFilterModel::TemplateOptions options = QContentFilterModel::SelectAll,
                           const QStringList &checked = QStringList() );
    void setModelTemplate( QContentFilter::FilterType type,
                           const QString &scope = QString(),
                           QContentFilterModel::TemplateOptions options = QContentFilterModel::SelectAll,
                           const QStringList &checked = QStringList() );

    QContentFilter checkedFilter() const;

    QString checkedLabel() const;

signals:
    void filterSelected( const QContentFilter &filter );

private:
    void init();

    QContentFilterSelectorPrivate *d;
};

class QContentFilterDialogPrivate;

class QTOPIA_EXPORT QContentFilterDialog : public QDialog
{
    Q_OBJECT
public:
    explicit QContentFilterDialog( QWidget *parent = 0 );
    explicit QContentFilterDialog( const QContentFilterModel::Template &modelTemplate,
                                   QWidget *parent = 0 );
    explicit QContentFilterDialog( QContent::Property property,
                                   QContentFilterModel::TemplateOptions options = QContentFilterModel::SelectAll,
                                   const QStringList &checked = QStringList(),
                                   QWidget *parent = 0 );
    explicit QContentFilterDialog( QContentFilter::FilterType type,
                                   const QString &scope = QString(),
                                   QContentFilterModel::TemplateOptions options = QContentFilterModel::SelectAll,
                                   const QStringList &checked = QStringList(),
                                   QWidget *parent = 0 );
    virtual ~QContentFilterDialog();

    QContentFilter filter();
    void setFilter( const QContentFilter &filter );

    QContentFilterModel::Template modelTemplate() const;
    void setModelTemplate( const QContentFilterModel::Template &modelTemplate );
    void setModelTemplate( QContent::Property property,
                           QContentFilterModel::TemplateOptions options = QContentFilterModel::SelectAll,
                           const QStringList &checked = QStringList() );
    void setModelTemplate( QContentFilter::FilterType type,
                           const QString &scope = QString(),
                           QContentFilterModel::TemplateOptions options = QContentFilterModel::SelectAll,
                           const QStringList &checked = QStringList() );

    QContentFilter selectedFilter() const;
    QContentFilter checkedFilter() const;

    QString checkedLabel() const;

private slots:
    void filterSelected( const QContentFilter &filter );

private:
    void init();

    QContentFilterDialogPrivate *d;
};

#endif
