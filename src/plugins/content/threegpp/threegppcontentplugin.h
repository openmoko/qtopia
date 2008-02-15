/****************************************************************************
**
** Copyright (C) 2007-2008 TROLLTECH ASA. All rights reserved.
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
#ifndef _3GPAUDIOCONTENTPLUGIN_H
#define _3GPAUDIOCONTENTPLUGIN_H

#include <qcontentplugin.h>
#include <qtopiaglobal.h>


class QTOPIA_PLUGIN_EXPORT ThreeGPPContentPlugin  : public QObject, public QContentPlugin 
{
    Q_OBJECT
    Q_INTERFACES(QContentPlugin)
public:
    ThreeGPPContentPlugin();
    ~ThreeGPPContentPlugin();

    virtual QStringList keys() const;
    virtual bool installContent( const QString &filePath, QContent *content ); 
    virtual bool updateContent( QContent *content);

private:
  
    void findUserData(QFile*, QContent*);  
    void readUserData(QFile*, QContent*);

    bool m_isAudioOnly;

};
    

#endif
