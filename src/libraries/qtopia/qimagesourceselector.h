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

#ifndef QIMAGESOURCESELECTOR_H
#define QIMAGESOURCESELECTOR_H

#include <qdialog.h>
#include <qcontent.h>

class QDSAction;

class QImageSourceSelector;
class QImageSourceSelectorDialogPrivate;

class QTOPIA_EXPORT QImageSourceSelectorDialog : public QDialog
{
    Q_OBJECT

public:
    QImageSourceSelectorDialog( QWidget *parent );
    virtual ~QImageSourceSelectorDialog();

    void setMaximumImageSize( const QSize &s );
    QSize maximumImageSize() const;

    virtual void setContent( const QContent &image );
    virtual QContent content() const;

private:
    void init();

    QImageSourceSelector* selector;
    QImageSourceSelectorDialogPrivate* d;
};

#endif // QIMAGESOURCESELECTOR_H

