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

#include <qtopia/smil/element.h>
#include <qtopia/smil/module.h>
#include <qpixmap.h>

class SmilSystem;
class AudioPlayer;

class SmilMediaParam : public SmilElement
{
public:
    SmilMediaParam(SmilSystem *sys, SmilElement *p, const QString &n, const QXmlAttributes &atts);

    enum ValueType { Data, Ref, Object };

    QString name;
    QString value;
    ValueType valueType;
    QString type;
};

class SmilMedia : public SmilElement
{
public:
    SmilMedia(SmilSystem *sys, SmilElement *p, const QString &n, const QXmlAttributes &atts);

    void setState(State s);
    void reset();

    SmilMediaParam *findParameter(const QString &name);

protected:
    QString source;
};

class SmilText : public SmilMedia
{
public:
    SmilText(SmilSystem *sys, SmilElement *p, const QString &n, const QXmlAttributes &atts);

    void addCharacters(const QString &ch);
    virtual void setData(const QByteArray &, const QString &);
    virtual void process();
    Duration implicitDuration();
    void paint(QPainter *p);

protected:
    QString text;
    QColor textColor;
    bool waiting;
};

class ImgPrivate;

class SmilImg : public SmilMedia
{
public:
    SmilImg(SmilSystem *sys, SmilElement *p, const QString &n, const QXmlAttributes &atts);
    ~SmilImg();

    virtual void setData(const QByteArray &, const QString &);
    virtual void process();
    virtual void setState(State s);
    Duration implicitDuration();
    void paint(QPainter *p);

protected:
    ImgPrivate *d;
    QPixmap pix;
    bool waiting;
};

class SmilAudio : public SmilMedia
{
public:
    SmilAudio(SmilSystem *sys, SmilElement *p, const QString &n, const QXmlAttributes &atts);
    ~SmilAudio();

    virtual void setData(const QByteArray &, const QString &);
    virtual void process();
    virtual void setState(State s);
    Duration implicitDuration();
    void paint(QPainter *p);

protected:
    static AudioPlayer *player;
    QByteArray audioData;
    QString audioType;
    bool waiting;
};

//===========================================================================

class SmilMediaModule : public SmilModule
{
public:
    SmilMediaModule();
    virtual ~SmilMediaModule();

    virtual SmilElement *beginParseElement(SmilSystem *, SmilElement *, const QString &qName, const QXmlAttributes &atts);
    virtual bool parseAttributes(SmilSystem *sys, SmilElement *e, const QXmlAttributes &atts);
    virtual void endParseElement(SmilElement *, const QString &qName);
    virtual QStringList elements() const;
    virtual QStringList attributes() const;
};

