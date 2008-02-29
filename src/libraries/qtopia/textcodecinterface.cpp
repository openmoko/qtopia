/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

/*!
  \class TextCodecInterface textcodecinterface.h
  \brief The TextCodecInterface class defines the interface for text codec plugins.

  Internally, Qtopia works completely with Unicode text strings, however
  it is sometimes necessary to convert to or from other character sets.

  To add support for a character set format, you must create a
  Text Codec plugin. To do so, follow these steps:

  <ol>
   <li> Create a subclass of QTextCodec that implements the conversion.
	This procedure is a standard part of Qt, and is documented there.

   <li> Create a subclass of TextCodecInterface that creates your
	QTextCodec subclass. Ensure that your class:
    <ul>
	<li> Inherits TextCodecInterface
	<li> Inplements all pure-virtual methods of TextCodecInterface
	<li> Includes the declarations:
<pre>
public:
    QRESULT queryInterface( const QUuid&, QUnknownInterface** );
    Q_REFCOUNT
private:
    ulong ref;
</pre>
    </ul>

   <li> In the code of the TextCodecInterface subclass:
<pre>
QRESULT <i>YourTextCodecInterfaceSubClass</i>::queryInterface( const QUuid &uuid, QUnknownInterface **iface )
{
    *iface = 0;
    if ( uuid == IID_QUnknown )
        *iface = this;
    else if ( uuid == IID_QtopiaTextCodec )
        *iface = this;

    if ( *iface )
        (*iface)->addRef();
    return (*iface) ? QS_OK : QS_FALSE;
}

Q_EXPORT_INTERFACE()
{
    Q_CREATE_INSTANCE( <i>YourTextCodecInterfaceSubClass</i> )
}
</pre>

    <li> Build your code as a Shared Object (.so file)
	(such as by using the tmake lib template).
    <li> Ship your plugin in the Qtopia plugins/textcodecs/ directory.
  </ol>
*/
/*!
    \fn QStringList TextCodecInterface::names() const

    Returns the list of codec names. You should use IANA registered names.
*/
/*!
    \fn QTextCodec *TextCodecInterface::createForName( const QString &name )

    Returns a new QTextCodec (subclass) for the given codec \a name.
*/
/*!
    \fn QValueList<int> TextCodecInterface::mibEnums() const

    Returns the list of codec MIBs. You should use IANA registered MIB values.
*/
/*!
    \fn QTextCodec *TextCodecInterface::createForMib( int mib )

    Returns a new QTextCodec (subclass) for the given \a mib.
*/
