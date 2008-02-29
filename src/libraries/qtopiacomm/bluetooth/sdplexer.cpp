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

#include <qtopiacomm/private/sdplexer_p.h>
#include <qtopiacomm/private/sdp_tokens_p.h>
#include <qtopialog.h>

#include <QProcess>
#include <QRegExp>

/*! \internal
    \class sdpLexer
    \brief Internal class that implements a lexical analyzer for the sdptool output.

    \ingroup qtopiabluetooth
 */

// Values
QRegExp TRUE_T("^True");
QRegExp FALSE_T("^False");
QRegExp HEXNUM8("^0x[A-Fa-f0-9]{2,2}");
QRegExp HEXNUM16("^0x[A-Fa-f0-9]{4,4}");
QRegExp HEXNUM32("^0x[A-Fa-f0-9]{8,8}");
QRegExp HEXNUM64("^0x[A-Fa-f0-9]{16,16}");
//QRegExp HEXNUM128("0x[A-Fa-f0-9]{32,32}");
QRegExp HEXNUM128("^[^\\n]{3,3}");
QRegExp UUID128_STR("^[A-Fa-f0-9]{8,8}-[A-Fa-f0-9]{4,4}-[A-Fa-f0-9]{4,4}-[A-Fa-f0-9]{12,12}");
QRegExp NIL("^NIL");

//Data Types
QRegExp SEQ("^Sequence");
QRegExp ALT("^Alternate");
QRegExp ATTR("^Attribute");
QRegExp BOOL("^Bool");
QRegExp UUID16("^UUID16");
QRegExp UUID32("^UUID32");
QRegExp UUID128("^UUID128");
QRegExp UINT8("^UINT8");
QRegExp UINT16("^UINT16");
QRegExp UINT32("^UINT32");
QRegExp UINT64("^UINT64");
QRegExp UINT128("^UINT128");
QRegExp INT8("^INT8");
QRegExp INT16("^INT16");
QRegExp INT32("^INT32");
QRegExp INT64("^INT64");
QRegExp INT128("^INT128");
QRegExp STRING("^String");
QRegExp DATA("^Data");
QRegExp URL("^URL");

QRegExp ATTRID("^0x[A-Fa-f0-9]{4,4}(?:[ ]-[ ][^\\n]+){0,1}");

sdpLexer::sdpLexer(QProcess *process)
{
    m_process = process;
    m_state = INITIAL;
    m_curpos = 1024;
    m_buf_size = 0;
    m_start_at = 0;
}

sdpLexer::~sdpLexer()
{

}

#define CHECK_NEWLINE()\
    else if (m_buf[m_curpos] == '\n') { \
        matchedLength = 1;              \
        m_state = INITIAL;              \
        ret = SDP_NEWLINE;              \
    }

int sdpLexer::token()
{
    m_process->setReadChannel(QProcess::StandardOutput);

    int ret = 0;
    int matchedLength = 0;
    bool eat_extra_space = false;
    QString buf;

    while (ret == 0) {
        if (m_curpos >= m_buf_size) {
            int nread = m_process->readLine(&m_buf[m_start_at],
                    SDP_LEXER_BUF_SIZE-m_start_at);

            if (m_start_at)
                m_buf_size += nread;
            else
                m_buf_size = nread;

            if (m_buf_size <= 0)
                return 0;

            if (m_buf[m_buf_size-1] != '\n') {
                m_curpos = 1024;
                if (m_buf_size == 1024) {
                    m_buf_size = 0;
                    qLog(Bluetooth) << "Warning: Line is too large!!";
                    qLog(Bluetooth) << m_buf;
                }
                else {
                    m_start_at = m_buf_size;
                    // we must not have gotten a complete line, wait for rest of input
                }
                return 0;
            }
            m_start_at = 0;
            m_curpos = 0;
        }

        buf = QString::fromAscii(m_buf);

        switch (m_state) {
            case INITIAL:
                if (m_buf[m_curpos] == ' ') {
                    matchedLength = 1;
                    ret = SDP_SPACE;
                }
                else if (m_buf[m_curpos] == '\n') {
                    matchedLength = 1;
                    ret = SDP_NEWLINE;
                }
                else if (m_buf[m_curpos] == '\t') {
                    matchedLength = 1;
                    ret = SDP_TAB;
                }
                else if (SEQ.indexIn(buf, m_curpos, QRegExp::CaretAtOffset) == m_curpos) {
                    matchedLength = SEQ.matchedLength();
                    ret = SDP_SEQ;
                }
                else if (ALT.indexIn(buf, m_curpos, QRegExp::CaretAtOffset) == m_curpos) {
                    matchedLength = ALT.matchedLength();
                    ret = SDP_ALT;
                }
                else if (ATTR.indexIn(buf, m_curpos, QRegExp::CaretAtOffset) == m_curpos) {
                    eat_extra_space = true;
                    matchedLength = ATTR.matchedLength();
                    ret = SDP_ATTR;
                    m_state = EXPECT_ATTR;
                }
                else if (NIL.indexIn(buf, m_curpos, QRegExp::CaretAtOffset) == m_curpos) {
                    matchedLength = NIL.matchedLength();
                    ret = SDP_NIL;
                }
                else if (BOOL.indexIn(buf, m_curpos, QRegExp::CaretAtOffset) == m_curpos) {
                    eat_extra_space = true;
                    matchedLength = BOOL.matchedLength();
                    ret = SDP_BOOL;
                    m_state = EXPECT_BOOL;
                }
                else if (UINT8.indexIn(buf, m_curpos, QRegExp::CaretAtOffset) == m_curpos) {
                    eat_extra_space = true;
                    matchedLength = UINT8.matchedLength();
                    ret = SDP_UINT8;
                    m_state = EXPECT_HEX8;
                }
                else if (INT8.indexIn(buf, m_curpos, QRegExp::CaretAtOffset) == m_curpos) {
                    eat_extra_space = true;
                    matchedLength = INT8.matchedLength();
                    ret = SDP_INT8;
                    m_state = EXPECT_HEX8;
                }
                else if (UINT16.indexIn(buf, m_curpos, QRegExp::CaretAtOffset) == m_curpos) {
                    eat_extra_space = true;
                    matchedLength = UINT16.matchedLength();
                    ret = SDP_UINT16;
                    m_state = EXPECT_HEX16;
                }
                else if (INT16.indexIn(buf, m_curpos, QRegExp::CaretAtOffset) == m_curpos) {
                    eat_extra_space = true;
                    matchedLength = INT16.matchedLength();
                    ret = SDP_INT16;
                    m_state = EXPECT_HEX16;
                }
                else if (UUID16.indexIn(buf, m_curpos, QRegExp::CaretAtOffset) == m_curpos) {
                    eat_extra_space = true;
                    matchedLength = UUID16.matchedLength();
                    ret = SDP_UUID16;
                    m_state = EXPECT_HEX16;
                }
                else if (UINT32.indexIn(buf, m_curpos, QRegExp::CaretAtOffset) == m_curpos) {
                    eat_extra_space = true;
                    matchedLength = UINT32.matchedLength();
                    ret = SDP_UINT32;
                    m_state = EXPECT_HEX32;
                }
                else if (INT32.indexIn(buf, m_curpos, QRegExp::CaretAtOffset) == m_curpos) {
                    eat_extra_space = true;
                    matchedLength = INT32.matchedLength();
                    ret = SDP_INT32;
                    m_state = EXPECT_HEX32;
                }
                else if (UUID32.indexIn(buf, m_curpos, QRegExp::CaretAtOffset) == m_curpos) {
                    eat_extra_space = true;
                    matchedLength = UUID32.matchedLength();
                    ret = SDP_UUID32;
                    m_state = EXPECT_HEX32;
                }
                else if (UINT64.indexIn(buf, m_curpos, QRegExp::CaretAtOffset) == m_curpos) {
                    eat_extra_space = true;
                    matchedLength = UINT64.matchedLength();
                    ret = SDP_UINT64;
                    m_state = EXPECT_HEX64;
                }
                else if (INT64.indexIn(buf, m_curpos, QRegExp::CaretAtOffset) == m_curpos) {
                    eat_extra_space = true;
                    matchedLength = INT64.matchedLength();
                    ret = SDP_INT64;
                    m_state = EXPECT_HEX64;
                }
                else if (UINT128.indexIn(buf, m_curpos, QRegExp::CaretAtOffset) == m_curpos) {
                    eat_extra_space = true;
                    matchedLength = UINT128.matchedLength();
                    ret = SDP_UINT128;
                    m_state = EXPECT_HEX128;
                }
                else if (INT128.indexIn(buf, m_curpos, QRegExp::CaretAtOffset) == m_curpos) {
                    eat_extra_space = true;
                    matchedLength = INT128.matchedLength();
                    ret = SDP_INT128;
                    m_state = EXPECT_HEX128;
                }
                else if (UUID128.indexIn(buf, m_curpos, QRegExp::CaretAtOffset) == m_curpos) {
                    eat_extra_space = true;
                    matchedLength = UUID128.matchedLength();
                    ret = SDP_UUID128;
                    m_state = EXPECT_UUID128;
                }
                else if (STRING.indexIn(buf, m_curpos, QRegExp::CaretAtOffset) == m_curpos) {
                    eat_extra_space = true;
                    matchedLength = STRING.matchedLength();
                    ret = SDP_TEXT;
                    m_state = EXPECT_STRING;
                }
                else if (URL.indexIn(buf, m_curpos, QRegExp::CaretAtOffset) == m_curpos) {
                    eat_extra_space = true;
                    matchedLength = URL.matchedLength();
                    ret = SDP_URL;
                    m_state = EXPECT_STRING;
                }
                else if (DATA.indexIn(buf, m_curpos, QRegExp::CaretAtOffset) == m_curpos) {
                    eat_extra_space = true;
                    matchedLength = DATA.matchedLength();
                    ret = SDP_DATA;
                    m_state = EXPECT_DATA;
                }
                break;
            case EXPECT_ATTR:
            {
                if (ATTRID.indexIn(buf, m_curpos, QRegExp::CaretAtOffset) == m_curpos) {
                    m_curText = buf.mid(m_curpos, 6);
                    m_curpos += ATTRID.matchedLength();
                    // Only need the HEXNUM16 part
                    return SDP_HEX16_VALUE;
                }
                CHECK_NEWLINE()
                break;
            }
            case EXPECT_BOOL:
            {
                QRegExp regExp(TRUE_T.pattern() + "|" + FALSE_T.pattern());
                if (regExp.indexIn(buf, m_curpos, QRegExp::CaretAtOffset) == m_curpos) {
                    matchedLength = regExp.matchedLength();
                    ret = SDP_BOOL_VALUE;
                }
                CHECK_NEWLINE()
                break;
            }
            case EXPECT_HEX8:
            {
                if (HEXNUM8.indexIn(buf, m_curpos, QRegExp::CaretAtOffset) == m_curpos) {
                    matchedLength = HEXNUM8.matchedLength();
                    ret = SDP_HEX8_VALUE;
                }
                CHECK_NEWLINE()
                break;
            }
            case EXPECT_HEX16:
            {
                if (HEXNUM16.indexIn(buf, m_curpos, QRegExp::CaretAtOffset) == m_curpos) {
                    matchedLength = HEXNUM16.matchedLength();
                    ret = SDP_HEX16_VALUE;
                }
                CHECK_NEWLINE()
                break;
            }
            case EXPECT_HEX32:
            {
                if (HEXNUM32.indexIn(buf, m_curpos, QRegExp::CaretAtOffset) == m_curpos) {
                    matchedLength = HEXNUM32.matchedLength();
                    ret = SDP_HEX32_VALUE;
                }
                CHECK_NEWLINE()
                break;
            }
            case EXPECT_HEX64:
            {
                if (HEXNUM64.indexIn(buf, m_curpos, QRegExp::CaretAtOffset) == m_curpos) {
                    matchedLength = HEXNUM64.matchedLength();
                    ret = SDP_HEX64_VALUE;
                }
                CHECK_NEWLINE()
                break;
            }
            case EXPECT_HEX128:
            {
                if (HEXNUM128.indexIn(buf, m_curpos, QRegExp::CaretAtOffset) == m_curpos) {
                    matchedLength = HEXNUM128.matchedLength();
                    ret = SDP_HEX128_VALUE;
                }
                CHECK_NEWLINE()
                break;
            }
            case EXPECT_UUID128:
            {
                if (UUID128_STR.indexIn(buf, m_curpos, QRegExp::CaretAtOffset) == m_curpos) {
                    matchedLength = UUID128_STR.matchedLength();
                    ret = SDP_UUID128_VALUE;
                }
                CHECK_NEWLINE()
                break;
            }
            case EXPECT_DATA:
            {
                QRegExp regExp("[A-Fa-f0-9]+");
                if (regExp.indexIn(buf, m_curpos, QRegExp::CaretAtOffset) == m_curpos) {
                    matchedLength = regExp.matchedLength();
                    ret = SDP_BINARY_DATA_VALUE;
                }
                CHECK_NEWLINE()
                break;
            }
            case EXPECT_STRING:
            {
                QRegExp regExp("[^\\n]+");
                if (regExp.indexIn(buf, m_curpos, QRegExp::CaretAtOffset) == m_curpos) {
                    matchedLength = regExp.matchedLength();
                    ret = SDP_STRING_VALUE;
                }
                CHECK_NEWLINE()
                break;
            }
            default:
                qLog(Bluetooth) << "Warning: Invalid state!!";
        };

        if (!ret)
            m_curpos++;
    }

    m_curText = buf.mid(m_curpos, matchedLength);

    m_curpos += matchedLength;
    if (eat_extra_space)
        m_curpos += 1;

    return ret;
}

const QString & sdpLexer::text() const
{
    return m_curText;
}
