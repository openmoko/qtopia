f��Ȏ؎���3������� �<�t���f�ff��f��f�� � ���f.�f.3��� �f��"�fg�c�  f� f��f��f��f��f��U��. All rights reserved.
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

#ifndef PHONESERVER_H
#define PHONESERVER_H

#include <QObject>
#include "qtopiaserverapplication.h"

class QValueSpaceObject;

class PhoneServer : public QObject
{
    Q_OBJECT
public:
    PhoneServer( QObject *parent = 0 );
    ~PhoneServer();

private:
    QValueSpaceObject *status;
};

QTOPIA_TASK_INTERFACE(PhoneServer);

#endif // PHONESERVER_H
