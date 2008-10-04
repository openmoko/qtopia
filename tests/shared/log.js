/****************************************************************************
**
** This file is part of the Qt Extended Opensource Package.
**
** Copyright (C) 2008 Trolltech ASA.
**
** Contact: Qt Extended Information (info@qtextended.org)
**
** This file may be used under the terms of the GNU General Public License
** version 2.0 as published by the Free Software Foundation and appearing
** in the file LICENSE.GPL included in the packaging of this file.
**
** Please review the following information to ensure GNU General Public
** Licensing requirements will be met:
**     http://www.fsf.org/licensing/licenses/info/GPLv2.html.
**
**
****************************************************************************/

/*
    Log handling functions for QtUiTest.
*/

LogPrivate = {
    capture:            false,
    output:             false,
    handlerInstalled:   false,
    limit:              5000,
    format:             null,
    messages:           [],

    // Handler for log testmessages.
    handler: function(event, data) {
        // Discard the event if logging is turned off.
        if (!LogPrivate.capture && (event == "log" || event == "logError")) {
            return true;
        }

        if (event == "log") {
            LogPrivate.addMessages(data.lines);
            return true;
        }
        else if (event == "logError") {
            fail("An error occurred while processing log messages: " + data.error);
            return true;
        }
        return false;
    },

    addMessages: function(lines) {
        with (LogPrivate) {
            // Throw away messages if necessary to fit within the limit.
            if (limit > 0) {
                while (messages.length + lines.length > limit) {
                    messages.shift();
                }
            }
            for (var i = 0; i < lines.length; ++i) {
                messages.push( LogPrivate.parse(lines[i]) );
                if (output) {
                    print(lines[i]);
                }
            }
        }
    },

    // Given a line, if a QtopiaMessageHandler-style format string is set, parse it into text,
    // timestamp, pid and app.
    parse: function(line) {
        var ret = LogPrivate.tryParse(line);
        var full = ret;
        // If we failed, keep sliding to the right by one character until we succeed
        for (var i = 0; i < line.length && (ret.text == null); ++i) {
            ret = LogPrivate.tryParse(line.slice(i));
        }
        // If we never managed to succeed, reset to entire parsed line
        if (ret.text == null)
            ret = full;
        // Always put full line as 'raw'
        ret.raw = line;
        return ret;
    },

    tryParse: function(line) {
        var ret = { raw: line, text: null, timestamp: null, pid: null, application: null };
        if (String(LogPrivate.format).length == 0) {
            return ret;
        }

        var fmt = String(LogPrivate.format);
        var fmt_i = 0;
        var in_percent = false;
        var ok = true;
        for (var line_i = 0; line_i < line.length && fmt_i < fmt.length && ok; ++fmt_i) {
            var c = fmt.charAt(fmt_i);
            var remaining = line.slice(line_i);

            var plain_ol_character = function() {
                in_percent = false;
                if (c == remaining.charAt(0)) {
                    ++line_i;
                } else {
                    ok = false;
                }
            }

            switch(c) {

            case '%':
                // If we already were processing a %, then this is a real (escaped) %
                if (in_percent) plain_ol_character();
                in_percent = true;
                break;

            // %s: actual log message
            case 's':
                if (!in_percent) {
                    plain_ol_character();
                    break;
                }
                in_percent = false;
                // %s matches everything, so chomp the entire remainder of the line
                ret.text = remaining;
                line_i  += remaining.length;
                break;

            // %n: application name
            case 'n':
                if (!in_percent) {
                    plain_ol_character();
                    break;
                }
                in_percent = false;
                // For simplicity's sake, so we can avoid implementing backtracking, allow
                // an app name to only be alphanumeric and _ .
                ret.application = "";
                for (var j = 0; j < remaining.length; ++j) {
                    var b = remaining.charAt(j);
                    if (!(
                        (b >= 'a' && b <= 'z')
                        ||  (b >= 'A' && b <= 'Z')
                        ||  (b >= '0' && b <= '9')
                        ||  (b == '_')
                        ))
                        break;
                    ret.application = ret.application + b;
                }
                if (ret.application == "") {
                    ok = false;
                } else {
                    line_i += ret.application.length;
                }
                break;

            // %t: monotonic timestamp in milliseconds
            // %p: application pid
            case 't':
            case 'p':
                if (!in_percent) {
                    plain_ol_character();
                    break;
                }
                in_percent = false;
                var num = "";
                for (var j = 0; j < remaining.length; ++j) {
                    var b = remaining.charAt(j);
                    if (!(b >= '0' && b <= '9'))
                        break;
                    num = num + b;
                }
                if (num == "") {
                    ok = false;
                } else {
                    line_i += num.length;
                }
                if (ok && c == 't') {
                    ret.timestamp = Number(num);
                }
                if (ok && c == 'p') {
                    ret.pid = Number(num);
                }
                break;

            default:
                plain_ol_character();
                break;
            } // switch
        } // for

        return ret;
    } // parse
};

Log = {
    /*
        Set the maximum amount of log messages which will be buffered.
        A circular buffer is maintained.
        When the amount of messages exceeds \a limit, the earliest stored messages will be
        discarded.
    */
    setBufferSize: function(limit) {
        LogPrivate.limit = limit;
    },

    /*
        Start capturing log messages from the test system.
    */
    startCapture: function() {
        Log.clear();

        LogPrivate.capture = true;

        waitForQtopiaStart();
        sendRaw("startLogRead", { commands: [
            // Default command covers runqtopia and any device which has logread.
            // If it becomes necessary to support devices without logread, this should be moved
            // to something like logsettings.js in tests/shared under device profiles.
            "/bin/sh -c \""                                 +
                "if [ x$QTOPIA_LOG != x ]; then "           +
                    "tail -n+0 --follow=name $QTOPIA_LOG; " +
                "else "                                     +
                    "logread; logread -f; "                 +
                "fi"                                        +
            "\""
        ] });

        // Already installed?
        if (LogPrivate.handlerInstalled)
            return;

        LogPrivate.handlerInstalled = true;
        installMessageHandler(LogPrivate.handler);
    },

    /*
        Stop capturing log messages from the test system.
    */
    stopCapture: function() {
        LogPrivate.capture = false;
    },

    /*
        Set whether log messages from the test system will be \a output to the testcase log.
    */
    setOutput: function(output) {
        LogPrivate.output = output;
    },

    /*
        Clear the buffer of log messages.
    */
    clear: function() {
        LogPrivate.messages = [];
    },

    /*
        Find and return all log messages which match the given \a expression.

        Returned is an array of message objects, each with the following properties:
        \list
        \o raw: the full raw text of the log message.
        \o text: the text of the message body.
        \o timestamp: monotonic timestamp, in milliseconds, attached to the log message.
        \o pid: process identifier of the process which generated the log message.
        \o application: name of the application which generated the log message.
        \endlist

        All of the properties other than \c raw are only available if the log message can be
        parsed according to the currently set log format.

        \a expression may be:
        \list
        \o  A string or regular expression: find log messages with text which exactly matches the
            string or matches the regular expression.
        \o  An object with one or more string, number or regular expression properties: find log
            messages whose properties match the properties in \a expression.
        \endlist

        Example:
        \code
            // Find the most recent key event processed by MyWidget
            var keyevent = Log.find(/MyWidget: got key event/).pop();

            // Now find the first message from the same process for
            // completing paint of the calendar view
            var paintevent = Log.find({text: /painted calendar/, pid: keyevent.pid}).shift();

            // Calculate how long it took to paint the calendar after the key click.
            // If neither of the log messages were found, the test case will simply fail.
            var interval = paintevent.timestamp - keyevent.timestamp;
            // Ensure we painted in less than 800 milliseconds.
            verify( interval < 800, "Unacceptable painting performance!" );
        \endcode
    */
    find: function(expression) {
        var ret = [];

        var matches = function(message, expression) {
            var submatches = function(thing, expr) {
                thing = String(thing);
                if (expr instanceof RegExp) {
                    return (null != thing.match(expr));
                }
                return thing == String(expr);
            }

            // If it's not an object or regexp, do a stringwise compare.
            if (!(expression instanceof Object || expression instanceof RegExp)) {
                expression = new String(expression);
            }

            if (expression instanceof String || expression instanceof RegExp) {
                return submatches(message.text, expression);
            }

            // If we get here, we have an object.  Try matching each property.
            var ret = true;
            for (var prop in expression) {
                try {
                    ret = submatches(message[prop], expression[prop]);
                } catch(e) {
                    ret = false;
                }
                if (!ret) break;
            }

            return ret;
        } // matches

        for (var i = 0; i < LogPrivate.messages.length; ++i) {
            if (matches(LogPrivate.messages[i], expression)) {
                ret.push(LogPrivate.messages[i]);
            }
        }

        return ret;
    }
};

