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

#ifndef _HIERARCHICALDOCUMENTVIEW_H_
#define _HIERARCHICALDOCUMENTVIEW_H_

#include "documentview.h"

class QLabel;
class QCategoryManager;
class QVBoxLayout;
class QSoftMenuBar;

// Stores a name, content filter & icon. To be used in a model.
class ContentFilterProperty 
{
	public:
    	ContentFilterProperty (const QString &name, const QContentFilter &value, const QIcon &icon)
        	: mName(name), mValue(value), mIcon(icon) {}
    	QString name() const {return mName;}
    	QContentFilter value() const {return mValue;}
    	QIcon icon() const {return mIcon;}
	private:
    	QString mName;
    	QContentFilter mValue;
    	QIcon mIcon;
};

// A model to store & display ContentFilterProperty-objects (incl. icon) in a list view.
class AbstractContentFilterPropertyListModel : public QAbstractListModel 
{
    Q_OBJECT
            
    public:   
        AbstractContentFilterPropertyListModel(QObject *parent=0): QAbstractListModel( parent ) {};
        virtual QVariant data ( const QModelIndex &index, int role) const;
        virtual int rowCount ( const QModelIndex &parent) const;
        virtual QContentFilter value(int index) const;
        QModelIndex createIndex(const QContentFilter &filter) const;
        virtual QString findPropertyName(const QContentFilter &filter) const;
        
    protected:
        virtual int findIndex(const QContentFilter &filter) const;
        
        QList<ContentFilterProperty> mList;
    
    public slots:
        virtual void reload() = 0;
};

// A model to store content filters on mime types.
class MimeTypeFilterModel : public AbstractContentFilterPropertyListModel 
{
    Q_OBJECT
            
    public:
        MimeTypeFilterModel(QObject *parent=0): AbstractContentFilterPropertyListModel( parent ){};
    
    public slots:
        virtual void reload();
    
    protected:
        void addProperty(const QString &label, const QMimeType &mimeType,const QIcon &icon);
};

// A model to store content filters on categories.
class CategoryFilterListModel : public  AbstractContentFilterPropertyListModel 
{
    Q_OBJECT
            
    public:
        CategoryFilterListModel(QObject *parent=0);
    
    public slots:
        virtual void reload();
    
    protected:
        QCategoryManager* categoryManager;
};

// Extends the DocumentLauncherView by making selection of mime type and category compulsory. 
// After selection of these the expected document list is shown filtered by mime type and category.
class HierarchicalDocumentLauncherView : public DocumentLauncherView 
{
    Q_OBJECT
            
    public:
        HierarchicalDocumentLauncherView(QWidget* parent = 0, Qt::WFlags fl = 0);
        virtual ~HierarchicalDocumentLauncherView();
        
        virtual void resetSelection();
        
    protected:
        enum FilterNavigation { NavigateForward, NavigateBackward };
        
        QList<QContentFilter> selectedFilters;
        
        MimeTypeFilterModel *modelTypes;
        CategoryFilterListModel *modelCategories;
        QCategoryManager *categoryManager;

        void filterNavigate(FilterNavigation navigation, QModelIndex selectedItem = QModelIndex());
        void setCurrentItem(const QContentFilter& pFilter);
        
        bool eventFilter(QObject *obj, QEvent *event);
        
        QLabel* selectedFilterLabel;
       
        virtual void enterNavigationMode();
        virtual void exitNavigationMode();
        
        virtual void handleReturnPressed(const QModelIndex &item);
        virtual void handleItemClicked(const QModelIndex &item, bool setCurrentIndex);
		
		QMenu* dummyMenu;
        
    private:
        void init();
};
        
#endif
