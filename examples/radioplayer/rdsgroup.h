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

#ifndef RDSGROUP_H
#define RDSGROUP_H

#include <QByteArray>

class RdsGroup
{
public:
    RdsGroup();
    RdsGroup( const RdsGroup& other );
    ~RdsGroup();

    enum Status
    {
        Ok,
        Invalid,
        WaitingForMore
    };

    RdsGroup& operator=( const RdsGroup& other );

    QByteArray data() const { return _data; }
    Status status() const { return _status; }

    void addBlock( const char *data );
    void clear();

    int groupType() const;
    bool isTypeAGroup() const;
    bool isTypeBGroup() const;

    int lsb( int block ) const;
    int msb( int block ) const;
    int word( int block ) const;

private:
    QByteArray _data;
    Status _status;
    int _prevBlock;
};

#endif
