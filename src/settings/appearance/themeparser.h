/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
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

#ifndef THEMEPARSER_H
#define THEMEPARSER_H
#include <QXmlDefaultHandler>
#include <QPalette>
#include <QList>
#include <QStack>
#include <QMultiHash>


class WidgetThemeParser : public QXmlDefaultHandler
{
public:
    WidgetThemeParser();
    bool parse(const QString &file);
    QPalette &palette();

private:
    bool startElement(const QString & namespaceURI, const QString & localName, const QString & qName, const QXmlAttributes & atts);
    bool endElement(const QString & namespaceURI, const QString & localName, const QString & qName);
    QString errorString();
    void ignoreNode(QString expected, QString received);
    void setDocumentLocator ( QXmlLocator * locator );
    bool headerRead(const QXmlAttributes & attr);
    bool paletteRead(const QString &colorRoleName, const QXmlAttributes & attr);
    bool classRead(const QXmlAttributes & attr);

    QPalette pal;
    bool debugMode, readingDefaultClass;
    QString errorMessage;
    QXmlLocator * locator;
    QStack<int> parserStatus;
    enum TokenID { TokenIdTheme, TokenIdEngine, TokenIdClass, TokenIdFont,  TokenIdPixmap };
    enum ParserStates { ThemeParserInit, ThemeParserFinished, ThemeParserHeaderRead, ThemeParserEngineRead, ThemeParserFontRead,
        ThemeParserPaletteRead, ThemeParserClassRead, ThemeParserPixmapRead};
    QStringList tokenNames;
    QStringList pixmapPath;
    QMultiHash<QString, int> colorRoleDict, colorGroupDict, pixmapTypeDict;

};
#endif //THEMEPARSER_H
