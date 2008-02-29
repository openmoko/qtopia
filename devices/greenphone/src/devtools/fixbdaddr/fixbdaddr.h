/****************************************************************************
**
** Copyright (C) 2006-2007 TROLLTECH ASA. All rights reserved.
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
#ifndef STARTUPFLAGS_H
#define STARTUPFLAGS_H

#include <QDialog>

class FixBdaddr_Private;

class FixBdaddr : public QDialog
{
    Q_OBJECT

public:
    FixBdaddr(QWidget* parent = 0, Qt::WFlags fl = 0);
    ~FixBdaddr();

protected slots:
    void accept();
    void configValue(const QString &key, const QString &value);

private:
    FixBdaddr_Private *m_data;
};

#endif // STARTUPFLAGS_H

