/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#include "taskmanagerservice.h"

/*!
    \service TaskManagerService TaskManager
    \brief Provides the Qtopia TaskManager service.

    The \i TaskManager service enables applications to cause the home
    screen to switch between tasks or show a list of all running tasks.
*/

/*!
    \internal
    \fn TaskManagerService::TaskManagerService(QObject *parent)
*/

/*!
    \internal
 */
TaskManagerService::~TaskManagerService()
{
}

/*!
    \fn void TaskManagerService::multitask()
    Switch to the next task.

    This slot corresponds to the QCop service message
    \c{TaskManager::multitask()}.
*/

/*!
    \fn void TaskManagerService::showRunningTasks()
    Show a list of all running tasks.

    This slot corresponds to the QCop service message
    \c{TaskManager::showRunningTasks()}.
*/

