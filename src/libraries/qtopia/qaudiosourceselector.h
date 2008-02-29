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

#ifndef QAUDIOSOURCESELECTOR_H
#define QAUDIOSOURCESELECTOR_H

#include <qdialog.h>
#include <qcontent.h>

class QDSAction;

class QAudioSourceSelector;
class QAudioSourceSelectorDialogPrivate;

class QTOPIA_EXPORT QAudioSourceSelectorDialog : public QDialog
{
    Q_OBJECT

public:
    QAudioSourceSelectorDialog( QWidget *parent );
    virtual ~QAudioSourceSelectorDialog();

    void setDefaultAudio(const QString &type, const QString &subFormat, int fr, int ch);

    void setContent( const QContent& audio );
    QContent content() const;

private:
    void init();

    QAudioSourceSelector* selector;
    QAudioSourceSelectorDialogPrivate* d;
};

#endif // QAUDIOSOURCESELECTOR_H

