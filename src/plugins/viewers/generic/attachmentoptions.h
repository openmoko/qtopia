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

#ifndef AttachmentOptions_H
#define AttachmentOptions_H

#include <QByteArray>
#include <QContent>
#include <QDialog>
#include <QList>
#include <QSize>
#include <QString>

#include <qtopiaglobal.h>

class QByteArray;
class QContent;
class QLabel;
class QMailMessagePart;
class QPushButton;
class QString;

class AttachmentOptions : public QDialog
{
    Q_OBJECT

public:
    enum ContentClass 
    {
        Text,
        Image,
        Media,
        Multipart,
        Other
    };

    AttachmentOptions(QWidget* parent);
    ~AttachmentOptions();

    QSize sizeHint() const;

public slots:
    void setAttachment(QMailMessagePart& part);

    void viewAttachment();
    void saveAttachment();

private:
    QSize _parentSize;
    QLabel* _name;
    QLabel* _type;
    //QLabel* _comment;
    QLabel* _sizeLabel;
    QLabel* _size;
    QPushButton* _view;
    QPushButton* _save;
    QLabel* _document;
    QMailMessagePart* _part;
    ContentClass _class;
    QString _decodedText;
    QByteArray _decodedData;
    QList<QContent> _temporaries;
};

#endif

