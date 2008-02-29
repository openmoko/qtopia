/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
** 
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
** 
** A copy of the GNU GPL license version 2 is included in this package as 
** LICENSE.GPL.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
** See the GNU General Public License for more details.
**
** In addition, as a special exception Trolltech gives permission to link
** the code of this program with Qtopia applications copyrighted, developed
** and distributed by Trolltech under the terms of the Qtopia Personal Use
** License Agreement. You must comply with the GNU General Public License
** in all respects for all of the code used other than the applications
** licensed under the Qtopia Personal Use License Agreement. If you modify
** this file, you may extend this exception to your version of the file,
** but you are not obligated to do so. If you do not wish to do so, delete
** this exception statement from your version.
** 
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#include "themeparser.h"
#include <qtopia/qpeapplication.h>
#include <qtopia/config.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qpixmap.h>

WidgetThemeParser::WidgetThemeParser()
{
    Config config("qpe");
    if (config.isValid()){
	config.setGroup("Appearance");
	debugMode = config.readEntry("ThemeDebug", "off").lower() != "off";
    }else{
	debugMode = FALSE;
    }
    readingDefaultClass = FALSE;
    tokenNames << "theme" << "engine" <<  "class" << "font" <<  "pixmap";

    // enumerate color groups
    colorGroupDict.setAutoDelete(TRUE);
    colorGroupDict.insert("normal", new int((int)QPalette::Normal));
    colorGroupDict.insert("disabled", new int((int)QPalette::Disabled));
    colorGroupDict.insert("active", new int((int)QPalette::Active));
    colorGroupDict.insert("inactive", new int((int)QPalette::Inactive));

    // enumerate color roles
    colorRoleDict.setAutoDelete(TRUE);
    colorRoleDict.insert("foreground", new int((int)QColorGroup::Foreground));
    colorRoleDict.insert("button", new int((int)QColorGroup::Button));
    colorRoleDict.insert("light", new int((int)QColorGroup::Light));
    colorRoleDict.insert("midlight", new int((int)QColorGroup::Midlight));
    colorRoleDict.insert("dark", new int((int)QColorGroup::Dark));
    colorRoleDict.insert("mid", new int((int)QColorGroup::Mid));
    colorRoleDict.insert("text", new int((int)QColorGroup::Text));
    colorRoleDict.insert("brighttext", new int((int)QColorGroup::BrightText));
    colorRoleDict.insert("buttontext", new int((int)QColorGroup::ButtonText));
    colorRoleDict.insert("base", new int((int)QColorGroup::Base));
    colorRoleDict.insert("background", new int((int)QColorGroup::Background));
    colorRoleDict.insert("shadow", new int((int)QColorGroup::Shadow));
    colorRoleDict.insert("highlight", new int((int)QColorGroup::Highlight));
    colorRoleDict.insert("highlightedtext", new int((int)QColorGroup::HighlightedText));
}

bool WidgetThemeParser::parse(const QString &file )
{
    bool result = FALSE;
    parserStatus.clear();
    parserStatus.push(ThemeParserInit);
    QFileInfo info(file);
    if (info.isFile()){
	if (debugMode)
	    qWarning("!------- Widget theme parser starting to parse parse Theme file %s --------", 
		    info.filePath().local8Bit().data());
	QFile xmlFile( info.filePath() ); // no tr
	QXmlInputSource source( xmlFile );
	QXmlSimpleReader reader;
	reader.setContentHandler( this );
	reader.setErrorHandler( this );
	result = reader.parse( source );
	if (!result){
	    if (debugMode)
		qWarning("!------- Error unable to parse Theme file %s --------\n%s", 
			info.filePath().local8Bit().data(), errorMessage.local8Bit().data());
	    else
		qWarning("Unable to parse Theme file %s\n%s", 
			info.filePath().local8Bit().data(), errorMessage.local8Bit().data());
	}
    }else{
	if (debugMode)
	    qWarning("!-------  Error unable to open file %s ----------- !\n\n",
		    info.filePath().local8Bit().data());
	else
	    qWarning("Unable to open %s", info.filePath().local8Bit().data());
    }
    return result;
}

QPalette & WidgetThemeParser::palette()
{
    return pal;
}

bool WidgetThemeParser::startElement( const QString & /* namespaceURI */, const QString & /* localName */, const QString & qName, const QXmlAttributes & atts )
{
    bool result = TRUE;
    int tokenID = tokenNames.findIndex(qName.lower());
    int * colorRole;
    switch (parserStatus.top()){
	case ThemeParserInit:
	    if (tokenID == TokenIdTheme){
		parserStatus.push(ThemeParserHeaderRead);
		result = headerRead(atts);
	    }else{
		ignoreNode("theme", qName);
	    }
	    break;

	case ThemeParserHeaderRead:
	    if (tokenID == TokenIdEngine){
		parserStatus.push(ThemeParserEngineRead);
	    }else if (tokenID == TokenIdClass){
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
	    colorRole = colorRoleDict[qName.lower()];
	    if (colorRole){
		parserStatus.push(ThemeParserPaletteRead);
		if (readingDefaultClass)
		    result = paletteRead(qName, atts);
	    }else{
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
    bool result = TRUE;
    QString text = qName.lower();
    int tokenID = tokenNames.findIndex(text);
    if (tokenID != -1 || colorRoleDict[text] || colorGroupDict[text] || pixmapTypeDict[text]){
	if (text == "class")
	    readingDefaultClass = FALSE;
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
	    qWarning("Not lowering stack for %s", text.local8Bit().data());
    }
    return result;
}

void WidgetThemeParser::ignoreNode(QString expected, QString received)
{
    qWarning("Widget theme parser is ignoring node %s when expecting %s", received.local8Bit().data(), expected.local8Bit().data());
}

bool  WidgetThemeParser::paletteRead(const QString &colorRoleName, const QXmlAttributes & attr)
{
    bool result = TRUE;
    int index;
    int *colorGroup = 0;
    QColor color;
    QBrush brush;
    QString colorGroupName, colorValue, imageName;
    QString debugMessage;
    int colorRole = *colorRoleDict[colorRoleName.lower()];

    index = attr.index("rgb");
    if (index != -1){
	colorValue = attr.value(index);
	color.setNamedColor(colorValue);
	brush.setColor(color);
    }else{
	errorMessage += "Palette definition is missing rgb attribute\n";
	return FALSE;
    }
    if (debugMode)
	debugMessage = QString("For theme style Default setting palette color %1 to %2").arg(colorRoleName).arg(colorValue); 

    index = attr.index("group");
    if (index != -1){
	colorGroupName = attr.value(index).lower();
	colorGroup = colorGroupDict[colorGroupName];
	if (debugMode)
	    debugMessage += ", for color group " + colorGroupName;
	if (!colorGroup){
	    errorMessage += "Palette definition has unknown colorgroup " + attr.value(index);
	    return FALSE;
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
	    brush.setPixmap(pixmap);	
    }


    if (debugMode)
	qWarning(debugMessage);
    if (colorGroup)
	pal.setBrush(QPalette::ColorGroup(*colorGroup), QColorGroup::ColorRole(colorRole), brush);
    else
	pal.setBrush(QColorGroup::ColorRole(colorRole), brush);

    return result; 
}


bool  WidgetThemeParser::classRead(const QXmlAttributes & attr)
{
    bool result = TRUE;
    int index = attr.index("name");
    if (index == -1){
	errorMessage += "Class definition is missing name attribute\n";
	return FALSE;
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
    bool result = TRUE;
    int index = attr.index("imageprefix");
    if (index != -1){
	QString pathItem = QPEApplication::qpeDir() + "pics/themes/" + attr.value(index) + "/";
	if (QFile::exists(pathItem)){
	    if (debugMode)
		qWarning("Adding a pixmap path of %s", pathItem.local8Bit().data());
	    pixmapPath.append(pathItem );
	}else{
	    if (debugMode)
		qWarning("Ignoring the non-existant pixmap path of %s", pathItem.local8Bit().data());
	}
    }else{
	if (debugMode)
	    qWarning("Theme node has no imageprefix attribute ");
    }
    return result;
}
