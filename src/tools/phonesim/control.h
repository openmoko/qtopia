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

#ifndef CONTROL_H
#define CONTROL_H

#include <HardwareManipulator>

class ControlWidget;

class Control: public HardwareManipulator
{
Q_OBJECT

public:
    Control(const QString& ruleFile, QObject *parent=0);
    virtual ~Control();

public slots:
    void handleFromData( const QString& );
    void handleToData( const QString& );
    void setPhoneNumber( const QString& );

protected:
    virtual void warning( const QString&, const QString& );

private:
    ControlWidget *widget;
    friend class ControlWidget;
};

class ControlFactory : public HardwareManipulatorFactory
{
public:
    inline virtual HardwareManipulator *create(QObject *parent)
        { return new Control(ruleFile(), parent); }
};


#endif
