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

#ifndef QPIMSOURCEDIALOG_H
#define QPIMSOURCEDIALOG_H

#include <QDialog>
#include <qtopiaglobal.h>

class QPimModel;
class QPimSourceModel;

class QPimSourceDialogData;
class QTOPIAPIM_EXPORT QPimSourceDialog : public QDialog
{
    Q_OBJECT
public:
    QPimSourceDialog(QWidget *parent = 0);
    ~QPimSourceDialog();

    virtual void setPimModel(QPimModel *);

    QPimSourceModel* pimSourceModel() const;

private:
    void accept();

    QPimSourceDialogData *d;
};
#endif //QPIMSOURCEDIALOG_H
