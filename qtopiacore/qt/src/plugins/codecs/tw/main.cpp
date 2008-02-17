/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the plugins of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://trolltech.com/products/qt/licenses/licensing/opensource/
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** In addition, as a special exception, Trolltech gives you certain
** additional rights. These rights are described in the Trolltech GPL
** Exception version 1.0, which can be found at
** http://www.trolltech.com/products/qt/gplexception/ and in the file
** GPL_EXCEPTION.txt in this package.
**
** In addition, as a special exception, Trolltech, as the sole copyright
** holder for Qt Designer, grants users of the Qt/Eclipse Integration
** plug-in the right for the Qt/Eclipse Integration to link to
** functionality provided by Qt Designer and its related libraries.
**
** Trolltech reserves all rights not expressly granted herein.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <qtextcodecplugin.h>
#include <qtextcodec.h>
#include <qstringlist.h>

#ifndef QT_NO_TEXTCODECPLUGIN

#include "qbig5codec.h"

class TWTextCodecs : public QTextCodecPlugin
{
public:
    TWTextCodecs() {}

    QList<QByteArray> names() const;
    QList<QByteArray> aliases() const;
    QList<int> mibEnums() const;

    QTextCodec *createForMib(int);
    QTextCodec *createForName(const QByteArray &);
};

QList<QByteArray> TWTextCodecs::names() const
{
    QList<QByteArray> list;
    list += QBig5Codec::_name();
    list += QBig5hkscsCodec::_name();
#ifdef Q_WS_X11
    list += QFontBig5Codec::_name();
    list += QFontBig5hkscsCodec::_name();
#endif
    return list;
}

QList<QByteArray> TWTextCodecs::aliases() const
{
    QList<QByteArray> list;
    list += QBig5Codec::_aliases();
    list += QBig5hkscsCodec::_aliases();
#ifdef Q_WS_X11
    list += QFontBig5Codec::_aliases();
    list += QFontBig5hkscsCodec::_aliases();
#endif
    return list;
}

QList<int> TWTextCodecs::mibEnums() const
{
    QList<int> list;
    list += QBig5Codec::_mibEnum();
    list += QBig5hkscsCodec::_mibEnum();
#ifdef Q_WS_X11
    list += QFontBig5Codec::_mibEnum();
    list += QFontBig5hkscsCodec::_mibEnum();
#endif
    return list;
}

QTextCodec *TWTextCodecs::createForMib(int mib)
{
    if (mib == QBig5Codec::_mibEnum())
        return new QBig5Codec;
    if (mib == QBig5hkscsCodec::_mibEnum())
        return new QBig5hkscsCodec;
#ifdef Q_WS_X11
    if (mib == QFontBig5hkscsCodec::_mibEnum())
        return new QFontBig5hkscsCodec;
    if (mib == QFontBig5Codec::_mibEnum())
        return new QFontBig5Codec;
#endif
    return 0;
}


QTextCodec *TWTextCodecs::createForName(const QByteArray &name)
{
    if (name == QBig5Codec::_name() || QBig5Codec::_aliases().contains(name))
        return new QBig5Codec;
    if (name == QBig5hkscsCodec::_name() || QBig5hkscsCodec::_aliases().contains(name))
        return new QBig5hkscsCodec;
#ifdef Q_WS_X11
    if (name == QFontBig5hkscsCodec::_name() || QFontBig5hkscsCodec::_aliases().contains(name))
        return new QFontBig5hkscsCodec;
    if (name == QFontBig5Codec::_name() || QFontBig5Codec::_aliases().contains(name))
        return new QFontBig5Codec;
#endif
    return 0;
}


Q_EXPORT_STATIC_PLUGIN(TWTextCodecs);
Q_EXPORT_PLUGIN2(qtwcodecs, TWTextCodecs);

#endif // QT_NO_TEXTCODECPLUGIN
