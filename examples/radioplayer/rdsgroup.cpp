/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
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

#include "rdsgroup.h"

/*#
    \class RdsGroup
    \brief The RdsGroup class provides a buffer for Radio Data System (RDS) blocks.

    The RdsGroup class provides a buffer for Radio Data System (RDS) blocks.

    Three bytes of RDS data at a time are injected into an RdsGroup object
    using addBlock().  The object will set its status to \c{Ok} once the
    entire group has been received, to \c{WaitingForMore} if more data is
    required to complete the group, or to \c{Invalid} if an error was
    encountered.

    \sa RdsProgramInfo
*/

/*#
    Create an empty group.
*/
RdsGroup::RdsGroup()
{
    _status = WaitingForMore;
    _prevBlock = -1;
}

/*#
    Create a copy of \a other.
*/
RdsGroup::RdsGroup( const RdsGroup& other )
{
    _data = other._data;
    _status = other._status;
    _prevBlock = other._prevBlock;
}

/*#
    Destroy this group.
*/
RdsGroup::~RdsGroup()
{
}

/*#
    Assign a copy of \a other to this object.
*/
RdsGroup& RdsGroup::operator=( const RdsGroup& other )
{
    _data = other._data;
    _status = other._status;
    _prevBlock = other._prevBlock;
    return *this;
}

/*#
    Add three bytes of RDS information from \a data to this group.
*/
void RdsGroup::addBlock( const char *data )
{
    // Abort if the error bit is set in the status byte.
    if ( ( data[2] & 0x80 ) != 0 ) {
        _status = Invalid;
        return;
    }

    // Get the block number and validate it.
    int block = ( data[2] & 0x07 );
    if ( block == 4 )
        block = 2;  // Convert C' blocks into C blocks.
    if ( block == 5 || block == 6 )
        return;     // Ignore E blocks.
    if ( block == 7 ) {
        // Invalid block number.
        _status = Invalid;
        return;
    }
    if ( block == _prevBlock )
        return;     // Ignore repeated blocks.
    if ( _data.isEmpty() && block != 0 )
        return;     // Ignore blocks that are out of phase with the group.
    if ( block != ( _data.size() / 2 ) ) {
        // Not the block that we expected to get.
        _status = Invalid;
        return;
    }

    // Add the two data bytes to the block.
    _data += data[0];
    _data += data[1];
    _prevBlock = block;

    // Determine if the group is complete (four blocks received).
    if ( _data.size() >= 8 )
        _status = Ok;
}

/*#
    Determine the type of group within this object.  Returns 0 to 15 for
    a valid group, or -1 for an invalid group.
*/
int RdsGroup::groupType() const
{
    if ( _data.size() >= 4 )
        return ( ( _data[3] >> 4 ) & 0x0F );
    else
        return -1;
}

/*#
    Determine if this group is type A.
*/
bool RdsGroup::isTypeAGroup() const
{
    if ( _data.size() >= 4 )
        return ( ( _data[3] & 0x08 ) == 0 );
    else
        return false;
}

/*#
    Determine if this group is type B.
*/
bool RdsGroup::isTypeBGroup() const
{
    if ( _data.size() >= 4 )
        return ( ( _data[3] & 0x08 ) != 0 );
    else
        return false;
}

/*#
    Get the least significant byte of \a block from this group.
*/
int RdsGroup::lsb( int block ) const
{
    int offset = block * 2;
    if ( offset >= 0 && offset < _data.size() )
        return ( _data[offset] & 0xFF );
    else
        return 0;
}

/*#
    Get the most significant byte of \a block from this group.
*/
int RdsGroup::msb( int block ) const
{
    int offset = block * 2 + 1;
    if ( offset >= 0 && offset < _data.size() )
        return ( _data[offset] & 0xFF );
    else
        return 0;
}

/*#
    Get the contents of \a block within this group as a 16-bit word.
*/
int RdsGroup::word( int block ) const
{
    int offset = block * 2;
    if ( offset >= 0 && ( offset + 1 ) < _data.size() )
        return ( _data[offset] & 0xFF ) |
               ( ( _data[offset + 1] & 0xFF ) << 8 );
    else
        return 0;
}

/*#
    Clear this group, ready to accept blocks for a new group.
*/
void RdsGroup::clear()
{
    _data = QByteArray();
    _status = WaitingForMore;
    _prevBlock = -1;
}
