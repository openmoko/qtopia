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

#ifndef __DIRDELETERDIALOG_H__
#define __DIRDELETERDIALOG_H__

#include <QDialog>
#include <QString>

class QObexFtpClient;
class DirDeleterDialogPrivate;
class QObexFolderListingEntryInfo;
class QPushButton;
class QLabel;
class QKeyEvent;
class DirDeleter;

class DirDeleterDialog : public QDialog
{
    Q_OBJECT

public:
    enum Result { Success, Canceled, Failed };

    static DirDeleterDialog::Result
            deleteDirectory(const QString &dir, QObexFtpClient *client, QWidget *parent = 0);

protected:
    void keyPressEvent(QKeyEvent *e);

private slots:
    void cancel();
    void deleteDone(bool error);

private:
    DirDeleterDialog(const QString &dir, QObexFtpClient *client, QWidget *parent = 0);
    ~DirDeleterDialog();

    QPushButton *m_cancelButton;
    QLabel *m_currentLabel;
    QLabel *m_removingLabel;
    DirDeleter *m_deleter;
};

#endif
