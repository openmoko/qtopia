/*  This file is part of the KDE project.

    Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).

    This library is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 2.1 or 3 of the License.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef Phonon_QT7_DISPLAYLINKCALLBACK_H
#define Phonon_QT7_DISPLAYLINKCALLBACK_H

#include <QuickTime/QuickTime.h>
#undef check // avoid name clash;

#include <QObject>

QT_BEGIN_NAMESPACE

namespace Phonon
{
namespace QT7
{
    // Hide the fact that CoreVideo is beeing
    // used to support panther:
    class LinkTimeProxy
    {
        public:
            CVTimeStamp getCVTimeStamp() const;
            void setCVTimeStamp(const CVTimeStamp &timeStamp);
            
        private:
            CVTimeStamp m_timeStamp;
    };

    class DisplayLinkCallback
    {
        public:
            static void retain();
            static void release();
            static LinkTimeProxy currentTime();

        private:
            static CVDisplayLinkRef m_displayLink;
            static int m_refCount;
            static CVTimeStamp m_timeStamp;
            static bool m_initialized;
    };

}} // namespace Phonon::QT7

QT_END_NAMESPACE

#endif // Phonon_QT7_DISPLAYLINKCALLBACK_H
