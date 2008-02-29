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
#include "xmlreader.h"

/*! \class Node xmlreader.h
  \brief The Node class is a single element in an XML structure defined by XmlHandler.

  \ingroup qtopiaemb
  \sa XmlHandler
 */

/*! \fn Node *Node::nextNode() const;
  Returns the Node's next sibling.
*/

/*! \fn Node *Node::prevNode() const;
  Returns the Node's previous sibling.
*/

/*! \fn Node *Node::parentNode() const;
  Returns the Node's parent.
*/

/*! \fn Node *Node::lastChild() const;
  Returns the Node's last child.
*/

/*! \fn Node *Node::firstChild() const;
  Returns the Node's first child.
*/

/*! \fn void Node::setTagName( const QString &s );
  Sets the current tag's name to \a s.
*/

/*! \fn QString Node::tagName() const;
  Returns the current tag's name.
*/

/*! \fn void Node::setData (const QString &s );
  Sets the current tag's data to \a s.
*/

/*! \fn QString Node::data() const;
  Returns the current tag's data.
*/

/*! \fn void Node::appendData ( const QString s );
  Appends \a s to the current tag's data.
*/

/*!
  Creates an invalid Node
*/
Node::Node()
    : parent( 0 ), prev( 0 ),
      next( 0 ), first( 0 ), last( 0 )
{
}

/*!
  Destructs a Node, and deletes its children.
*/

Node::~Node()
{
    Node *n = first, *m;

    while ( n ) {
	m = n->next;
	delete n;
	n = m;
    }
}

/*!
  Safely sets Node \a child to be a child of this Node.
*/

void Node::addChild( Node *child )
{
    child->parent = this;

    if ( last )
	last->next = child;
    child->prev = last;

    if ( !first )
	first = child;
    last = child;
}

/*!
  Returns the value of the attribute tagged as \a name.
*/

QString Node::attribute( const QString& name )
{
    return attributes[name];
}

/*!
  Adds the attributes in \a a to the Node's current attributes,
  overwriting any conflicts with existing attributes.
*/

void Node::setAttributes( const QXmlAttributes &a )
{
    for ( int i = 0; i < a.length(); i++ )
	attributes[ a.qName( i ) ] = a.value( i );
}

/*!
  Returns all of the Node's attributes.
*/

QMap<QString, QString> Node::attributeMap()
{
    return attributes;
}

/*!
  Returns the value of the first attribute named \a tag from any
  of the Node's children.
*/

QString Node::subData(const QString& tag) const
{
    Node* c = firstChild();
    while ( c ) {
	if ( c->tagName() == tag )
	    return c->data();
	c = c->nextNode();
    }
    return QString::null;
}

/*! \class XmlHandler xmlreader.h
  \brief The XmlHandler class is a XML document structure holder.

  This class provides a tree of Nodes that developers can use to organise their
  XML data.

  \ingroup qtopiaemb
  \sa Node
*/

/*!
  Creates an empty XmlHandler.
*/

XmlHandler::XmlHandler()
    : node( 0 ), tree( 0 )
{
}

/*!
  Destructs an XmlHandler.
*/

XmlHandler::~XmlHandler()
{
}

/*!
  Initialises an XmlHandler with a root Node named DOCUMENT.
  Returns TRUE if successful.
*/

bool XmlHandler::startDocument()
{
    tree = node = new Node;
    node->setTagName( "DOCUMENT" );

    return TRUE;
}

/*!
  Returns TRUE if the XmlHandler contains a valid tree of Nodes.
*/

bool XmlHandler::endDocument()
{
    if ( node != tree )
	return FALSE;

    return TRUE;
}

/*!
  Creates a new sub-element, with name \a qName and attributes \a attr.
*/

bool XmlHandler::startElement( const QString &, const QString &,
			       const QString &qName, const QXmlAttributes &attr )
{
    Node *nnode = new Node;
    nnode->setAttributes( attr );
    nnode->setTagName( qName );

    node->addChild( nnode );
    node = nnode;

    return TRUE;
}

/*!
   Ends a sub-element safely.
*/

bool XmlHandler::endElement( const QString &, const QString &, const QString & )
{
    if ( node == tree )
	return FALSE;

    node = node->parentNode();
    return TRUE;
}

/*!
   Appends the data \a ch to the current Node.
*/

bool XmlHandler::characters( const QString &ch )
{
    node->appendData( ch );

    return TRUE;
}

/*! \fn Node *XmlHandler::firstNode() const;
  Returns the root Node of the tree.
*/
