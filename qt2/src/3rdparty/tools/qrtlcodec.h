/****************************************************************************
** $Id: qt/src/3rdparty/tools/qrtlcodec.h   2.3.12   edited 2005-10-27 $
**
** Definition of QHebrewCodec and QArabicCodec classes
**
** Copyright (C)1999-2003 Trolltech AS.  All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

/*
 * Based on work Copyright (c) 1999 Lars Knoll <knoll@mpi-hd.mpg.de>
 */

#ifndef QRTLCODEC_H
#define QRTLCODEC_H

#ifndef QT_H
#include "qtextcodec.h"
#endif // QT_H

#ifndef QT_NO_RTLCODEC

class Q_EXPORT QHebrewCodec : public QTextCodec {
public:
    virtual int mibEnum() const;
    const char* name() const;

    QCString fromUnicode(const QString& uc, int& len_in_out) const;
    QString toUnicode(const char* chars, int len) const;

    int heuristicContentMatch(const char* chars, int len) const;

protected:
    virtual bool to8bit(const QChar ch, QCString *str) const; 
    QString toUnicode(const char* chars, int len, const ushort *table) const;

};

class Q_EXPORT QArabicCodec : public QHebrewCodec {
public:
    virtual int mibEnum() const;
    const char* name() const;

    QString toUnicode(const char* chars, int len) const;

    int heuristicContentMatch(const char* chars, int len) const;

protected:
    virtual bool to8bit(const QChar ch, QCString *str) const; 
};

#endif

#endif
