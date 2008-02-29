/****************************************************************************
**
** Copyright (C) 1992-2008 Trolltech ASA. All rights reserved.
**
** This file is part of the tools applications of the Qt Toolkit.
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

#include "atom.h"
#include "location.h"

/*! \class Atom
    \brief The Atom class is the fundamental unit for representing
    documents internally.

  Atoms have a \i type and are completed by a \i string whose
  meaning depends on the \i type. For example, the string
  \quotation
      \i italic text looks nicer than \bold bold text
  \endquotation
  is represented by the following atoms:
  \quotation
      (FormattingLeft, ATOM_FORMATTING_ITALIC)
      (String, "italic")
      (FormattingRight, ATOM_FORMATTING_ITALIC)
      (String, " text is more attractive than ")
      (FormattingLeft, ATOM_FORMATTING_BOLD)
      (String, "bold")
      (FormattingRight, ATOM_FORMATTING_BOLD)
      (String, " text")
  \endquotation

  \also Text
*/

/*! \enum Atom::Type

  \value AbstractLeft
  \value AbstractRight
  \value AutoLink
  \value BaseName
  \value BriefLeft
  \value BriefRight
  \value C
  \value CaptionLeft
  \value CaptionRight
  \value Code
  \value CodeBad
  \value CodeNew
  \value CodeOld
  \value FormatElse
  \value FormatEndif
  \value FormatIf
  \value FootnoteLeft
  \value FootnoteRight
  \value FormattingLeft
  \value FormattingRight
  \value GeneratedList
  \value Image
  \value ImageText
  \value InlineImage
  \value Link
  \value LinkNode
  \value ListLeft
  \value ListItemNumber
  \value ListTagLeft
  \value ListTagRight
  \value ListItemLeft
  \value ListItemRight
  \value ListRight
  \value Nop
  \value ParaLeft
  \value ParaRight
  \value QuotationLeft
  \value QuotationRight
  \value RawString
  \value SectionLeft
  \value SectionRight
  \value SectionHeadingLeft
  \value SectionHeadingRight
  \value SidebarLeft
  \value SidebarRight
  \value String
  \value TableLeft
  \value TableRight
  \value TableHeaderLeft
  \value TableHeaderRight
  \value TableRowLeft
  \value TableRowRight
  \value TableItemLeft
  \value TableItemRight
  \value TableOfContents
  \value Target
  \value UnhandledFormat
  \value UnknownCommand
*/

static const struct {
    const char *english;
    int no;
} atms[] = {
    { "AbstractLeft", Atom::AbstractLeft },
    { "AbstractRight", Atom::AbstractRight },
    { "AutoLink", Atom::AutoLink },
    { "BaseName", Atom::BaseName },
    { "BriefLeft", Atom::BriefLeft },
    { "BriefRight", Atom::BriefRight },
    { "C", Atom::C },
    { "CaptionLeft", Atom::CaptionLeft },
    { "CaptionRight", Atom::CaptionRight },
    { "Code", Atom::Code },
    { "CodeBad", Atom::CodeBad },
    { "CodeNew", Atom::CodeNew },
    { "CodeOld", Atom::CodeOld },
    { "FootnoteLeft", Atom::FootnoteLeft },
    { "FootnoteRight", Atom::FootnoteRight },
    { "FormatElse", Atom::FormatElse },
    { "FormatEndif", Atom::FormatEndif },
    { "FormatIf", Atom::FormatIf },
    { "FormattingLeft", Atom::FormattingLeft },
    { "FormattingRight", Atom::FormattingRight },
    { "GeneratedList", Atom::GeneratedList },
    { "Image", Atom::Image },
    { "ImageText", Atom::ImageText },
    { "InlineImage", Atom::InlineImage },
    { "LegaleseLeft", Atom::LegaleseLeft },
    { "LegaleseRight", Atom::LegaleseRight },
    { "Link", Atom::Link },
    { "LinkNode", Atom::LinkNode },
    { "ListLeft", Atom::ListLeft },
    { "ListItemNumber", Atom::ListItemNumber },
    { "ListTagLeft", Atom::ListTagLeft },
    { "ListTagRight", Atom::ListTagRight },
    { "ListItemLeft", Atom::ListItemLeft },
    { "ListItemRight", Atom::ListItemRight },
    { "ListRight", Atom::ListRight },
    { "Nop", Atom::Nop },
    { "ParaLeft", Atom::ParaLeft },
    { "ParaRight", Atom::ParaRight },
    { "QuotationLeft", Atom::QuotationLeft },
    { "QuotationRight", Atom::QuotationRight },
    { "RawString", Atom::RawString },
    { "SectionLeft", Atom::SectionLeft },
    { "SectionRight", Atom::SectionRight },
    { "SectionHeadingLeft", Atom::SectionHeadingLeft },
    { "SectionHeadingRight", Atom::SectionHeadingRight },
    { "SidebarLeft", Atom::SidebarLeft },
    { "SidebarRight", Atom::SidebarRight },
    { "String", Atom::String },
    { "TableLeft", Atom::TableLeft },
    { "TableRight", Atom::TableRight },
    { "TableHeaderLeft", Atom::TableHeaderLeft },
    { "TableHeaderRight", Atom::TableHeaderRight },
    { "TableRowLeft", Atom::TableRowLeft },
    { "TableRowRight", Atom::TableRowRight },
    { "TableItemLeft", Atom::TableItemLeft },
    { "TableItemRight", Atom::TableItemRight },
    { "TableOfContents", Atom::TableOfContents },
    { "Target", Atom::Target },
    { "UnhandledFormat", Atom::UnhandledFormat },
    { "UnknownCommand", Atom::UnknownCommand },
    { 0, 0 }
};

/*! \fn Atom::Atom( Type type, const QString& string )

  Constructs an atom (\a type, \a string) outside of any atom list.
*/

/*! \fn Atom( Atom *prev, Type type, const QString& string )

  Constructs an atom (\a type, \a string) that follows \a prev in \a
  prev's atom list.
*/

/*! \fn void Atom::appendChar( QChar ch )

  Appends \a ch to the string parameter of this atom.

  \also string()
*/

/*! \fn void Atom::appendString( const QString& string )

  Appends \a string to the string parameter of this atom.

  \also string()
*/

/*! \fn void Atom::chopString()

  \also string()
*/

/*! \fn Atom *Atom::next()

  Returns the next atom in the atom list.

  \also type(), string()
*/

/*! \fn const Atom *Atom::next() const
    \overload
*/

/*! \fn Type Atom::type() const

  Returns the type of this atom.

  \also string(), next()
*/

/*!
  Returns the type of this atom as a string. Returns "Invalid" if
  type() returns an impossible value.

  This is only useful for debugging.

  \also type()
*/
QString Atom::typeString() const
{
    static bool deja = false;

    if ( !deja ) {
	int i = 0;
	while ( atms[i].english != 0 ) {
	    if ( atms[i].no != i )
		Location::internalError( tr("atom %1 missing").arg(i) );
	    i++;
	}
	deja = true;
    }

    int i = (int) type();
    if ( i < 0 || i > (int) Last ) {
	return "Invalid";
    } else {
	return atms[i].english;
    }
}

/*! \fn const QString& Atom::string() const

  Returns the string parameter that together with the type
  characterizes this atom.

  \also type(), next()
*/
