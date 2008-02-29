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

#ifndef __SDPLEXER_P_H__
#define __SDPLEXER_P_H__

#include <QString>

class QProcess;

#define SDP_LEXER_BUF_SIZE 1024

class sdpLexer {
    enum State { INITIAL,
                 EXPECT_ATTR,
                 EXPECT_BOOL,
                 EXPECT_HEX8,
                 EXPECT_HEX16,
                 EXPECT_HEX32,
                 EXPECT_HEX64,
                 EXPECT_HEX128,
                 EXPECT_UUID128,
                 EXPECT_DATA,
                 EXPECT_STRING
    };

public:
    explicit sdpLexer(QProcess *process);
    ~sdpLexer();

    const QString & text() const;
    int token();

private:
    QProcess *m_process;
    State m_state;
    int m_curpos;
    char m_buf[SDP_LEXER_BUF_SIZE];
    int m_buf_size;
    QString m_curText;
    int m_start_at;
};

#endif
