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

#ifndef SERVEREDIT_H
#define SERVEREDIT_H

#include <QDialog>
#include <QHash>
#include <QLineEdit>
#include <QTextEdit>

#include "ui_serveredit.h"

class ServerItem;

class ServerEdit : public QDialog, private Ui_ServerEditBase
{
    Q_OBJECT
public:
    ServerEdit( QWidget *parent = 0, Qt::WFlags f = 0 );
    ~ServerEdit();
    bool wasModified() const;
    QHash<QString,QString> serverList() const;

public slots:
    void accept();

private slots:
    void init();
    void addNewServer();
    void removeServer();
    void editServer();
    void contextMenuShow();

private:
    bool m_modified;
    QAction *removeServerAction;
    QAction *editServerAction;
    QStringList serversToRemove;
    QStringList permanentServers;
};

class QLabel;

class ServerEditor : public QDialog
{

Q_OBJECT

public:    
    enum Mode { New, ViewOnly, ViewEdit };
    ServerEditor( Mode mode, ServerEdit *parent = 0, const QString &name = "", 
                  const QString &url = "" );
    QString name() const;
    QString url() const;
    bool wasModified() const;

public slots:
    virtual void accept();

private:
    Mode m_mode;
    ServerEdit *m_parent;
    bool m_modified;
    QLabel *m_nameLabel;
    QLabel *m_urlLabel;
    QLineEdit *m_nameLineEdit;
    QTextEdit *m_urlTextEdit; 
    QString m_initialName;
    QString m_initialUrl; 
};

////////////////////////////////////////////////////////////////////////
/////
///// inline ServerEdit implementations
/////
inline bool ServerEdit::wasModified() const
{
    return m_modified;
}

////////////////////////////////////////////////////////////////////////
/////
///// inline ServerEditor implementations
/////
inline QString ServerEditor::name() const
{
    return m_nameLineEdit->text();
}

inline QString ServerEditor::url() const
{
    return m_urlTextEdit->toPlainText();
}

inline bool ServerEditor::wasModified() const
{
    return m_modified;
}

#endif
