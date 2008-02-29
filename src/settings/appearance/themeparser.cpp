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
#include "themeparser.h"
#include <qtopiaapplication.h>
#include <QSettings>
#include <QFile>
#include <QFileInfo>
#include <QPixmap>

WidgetThemeParser::WidgetThemeParser()
{
    QSettings config("Trolltech","qpe");
    if (config.status()==QSettings::NoError){
        config.beginGroup("Appearance");
        debugMode = config.value("ThemeDebug", "off").toString().toLower() != "off";
    }else{
        debugMode = false;
    }
    readingDefaultClass = false;
    tokenNames << "theme" << "engine" <<  "class" << "font" <<  "pixmap";

    // enumerate color groups
    colorGroupDict.insert("normal", int((int)QPalette::Normal));
    colorGroupDict.insert("disabled", int((int)QPalette::Disabled));
    colorGroupDict.insert("active", int((int)QPalette::Active));
    colorGroupDict.insert("inactive", int((int)QPalette::Inactive));

    // enumerate color roles
    colorRoleDict.insert("foreground", int((int)QPalette::Foreground));
    colorRoleDict.insert("button", int((int)QPalette::Button));
    colorRoleDict.insert("light", int((int)QPalette::Light));
    colorRoleDict.insert("midlight", int((int)QPalette::Midlight));
    colorRoleDict.insert("dark", int((int)QPalette::Dark));
    colorRoleDict.insert("mid", int((int)QPalette::Mid));
    colorRoleDict.insert("text", int((int)QPalette::Text));
    colorRoleDict.insert("brighttext", int((int)QPalette::BrightText));
    colorRoleDict.insert("buttontext", int((int)QPalette::ButtonText));
    colorRoleDict.insert("base", int((int)QPalette::Base));
    colorRoleDict.insert("background", int((int)QPalette::Background));
    colorRoleDict.insert("shadow", int((int)QPalette::Shadow));
    colorRoleDict.insert("highlight", int((int)QPalette::Highlight));
    colorRoleDict.insert("highlightedtext", int((int)QPalette::HighlightedText));
}

bool WidgetThemeParser::parse(const QString &file )
{
    bool result = false;
    parserStatus.clear();
    parserStatus.push(ThemeParserInit);
    QFileInfo info(file);
    if (info.isFile()){
        if (debugMode)
            qWarning("!------- Widget theme parser starting to parse parse Theme file %s --------",
            info.filePath().toLatin1().data());
        QFile xmlFile( info.filePath() ); // no tr
        QXmlInputSource source;
        source.setData(xmlFile.readAll());
        QXmlSimpleReader reader;
        reader.setContentHandler( this );
        reader.setErrorHandler( this );
        result = reader.parse( source );
        if (!result){
            if (debugMode)
                qWarning("!------- Error unable to parse Theme file %s --------\n%s",
                info.filePath().toLatin1().data(), errorMessage.toLatin1().data());
            else
                qWarning("Unable to parse Theme file %s\n%s",
                info.filePath().toLatin1().data(), errorMessage.toLatin1().data());
        }
    } else {
        if (debugMode)
            qWarning("!-------  Error unable to open file %s ----------- !\n\n",
            info.filePath().toLatin1().data());
        else
            qWarning("Unable to open %s", info.filePath().toLatin1().data());
    }
    return result;
}

QPalette & WidgetThemeParser::palette()
{
    return pal;
}

bool WidgetThemeParser::startElement( const QString & /* namespaceURI */, const QString & /* localName */, const QString & qName, const QXmlAttributes & atts )
{
    bool result = true;
    int tokenID = tokenNames.indexOf(qName.toLower());
    int colorRole;
    switch (parserStatus.top()){
        case ThemeParserInit:
            if (tokenID == TokenIdTheme){
                parserStatus.push(ThemeParserHeaderRead);
                result = headerRead(atts);
            } else {
                ignoreNode("theme", qName);
            }
        break;

        case ThemeParserHeaderRead:
            if (tokenID == TokenIdEngine){
                parserStatus.push(ThemeParserEngineRead);
            } else if (tokenID == TokenIdClass){
                parserStatus.push(ThemeParserClassRead);
                if (!readingDefaultClass)
                    result = classRead(atts);
            } else{
                ignoreNode("engine or class", qName);
            }
        break;

        case ThemeParserClassRead:
            switch (tokenID){
                case TokenIdFont:
                    parserStatus.push(ThemeParserFontRead);
                break;

                case TokenIdPixmap:
                    parserStatus.push(ThemeParserPixmapRead);
                break;

                default:
                    colorRole = colorRoleDict.value(qName);
                    if (colorRole){
                        parserStatus.push(ThemeParserPaletteRead);
                    if (readingDefaultClass)
                        result = paletteRead(qName, atts);
                    } else {
                        ignoreNode("font or palette color ", qName);
                    }
            }
        break;

        default:
            ignoreNode("current node", qName);
    }
    return result;
}


bool WidgetThemeParser::endElement( const QString & /* namespaceURI */, const QString & /* localName */, const QString & qName )
{
    bool result = true;
    QString text = qName.toLower();
    int tokenID = tokenNames.indexOf(text);
    if (tokenID != -1 || colorRoleDict.value(text) || colorGroupDict.value(text) || pixmapTypeDict.value(text)){
        if (text == "class")
            readingDefaultClass = false;
        if (!parserStatus.isEmpty()){
            parserStatus.pop();
            if (parserStatus.isEmpty())
                parserStatus.push(ThemeParserFinished);
        }else{
            qWarning("Unexpected end of file");
            parserStatus.push(ThemeParserFinished);
        }
    }else{
        if (debugMode)
            qWarning("Not lowering stack for %s", text.toLatin1().data());
    }
    return result;
}

void WidgetThemeParser::ignoreNode(QString expected, QString received)
{
    qWarning("Widget theme parser is ignoring node %s when expecting %s", received.toLatin1().data(), expected.toLatin1().data());
}

bool  WidgetThemeParser::paletteRead(const QString &colorRoleName, const QXmlAttributes & attr)
{
    bool result = true;
    int index;
    int *colorGroup = 0;
    QColor color;
    QBrush brush;
    QString colorGroupName, colorValue, imageName;
    QString debugMessage;
    int colorRole = colorRoleDict.value(colorRoleName.toLower());

    index = attr.index("rgb");
    if (index != -1){
        colorValue = attr.value(index);
        color.setNamedColor(colorValue);
        brush.setColor(color);
    }else{
        errorMessage += "Palette definition is missing rgb attribute\n";
        return false;
    }
    if (debugMode)
        debugMessage = QString("For theme style Default setting palette color %1 to %2").arg(colorRoleName).arg(colorValue);

    index = attr.index("group");
    if (index != -1){
        colorGroupName = attr.value(index).toLower();
        colorGroup = (int *)colorGroupDict.value(colorGroupName);
        if (debugMode)
            debugMessage += ", for color group " + colorGroupName;
        if (!colorGroup){
            errorMessage += "Palette definition has unknown colorgroup " + attr.value(index);
            return false;
        }
    }

    index = attr.index("image");
    if (index != -1) {
        QStringList::Iterator it;
        imageName = attr.value(index);
        if (!imageName.isEmpty()){
            for (it = pixmapPath.begin(); it != pixmapPath.end(); it++) {
                QFileInfo fileinfo((*it) + imageName);

                if (fileinfo.exists()) {
                    imageName = fileinfo.filePath();
                    break;
                }
            }
        }
        if (debugMode)
            debugMessage += ", image of " + imageName;
        // we can't store brush pixmaps in the QPixmapCache because of order of destruction of objects
        QPixmap pixmap(imageName);
        if (!pixmap.isNull())
            brush.setTexture(pixmap);
    }


    if (debugMode)
        qWarning(debugMessage.toAscii().data());
    if (colorGroup)
        pal.setBrush(QPalette::ColorGroup(*colorGroup), QPalette::ColorRole(colorRole), brush);
    else
        pal.setBrush(QPalette::ColorRole(colorRole), brush);

    return result;
}


bool  WidgetThemeParser::classRead(const QXmlAttributes & attr)
{
    bool result = true;
    int index = attr.index("name");
    if (index == -1){
        errorMessage += "Class definition is missing name attribute\n";
        return false;
    }
    readingDefaultClass = attr.value(index) == "Default";

    return result;
}


QString WidgetThemeParser::errorString()
{
    static const char *parserStatesText[] = { "initialization", "cleanup", "theme read", "font read",  "parser palette read",
        "class read", "pixmap read"};
        QString result;
        result = errorMessage.append(QString("!-------  Error whilst performing %1 near, line %2, column %3 ----------- !\n\n").arg(parserStatesText[parserStatus.top()]). arg(QString::number(locator->lineNumber())).arg(QString::number(locator->columnNumber())));
        return result;
}


void WidgetThemeParser::setDocumentLocator ( QXmlLocator * locator )
{
    this->locator = locator;
}

bool WidgetThemeParser::headerRead(const QXmlAttributes & attr)
{
    bool result = true;
    int index = attr.index("imageprefix");
    if (index != -1){
        QString pathItem = Qtopia::qtopiaDir() + "pics/themes/" + attr.value(index) + "/";
        if (QFile::exists(pathItem)){
            if (debugMode)
                qWarning("Adding a pixmap path of %s", pathItem.toLatin1().data());
            pixmapPath.append(pathItem );
        }else{
            if (debugMode)
                qWarning("Ignoring the non-existant pixmap path of %s", pathItem.toLatin1().data());
        }
    }else{
        if (debugMode)
            qWarning("Theme node has no imageprefix attribute ");
    }
    return result;
}
