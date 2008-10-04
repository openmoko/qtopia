/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtDBus module of the Qt Toolkit.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.  In addition, as a special
** exception, Nokia gives you certain additional rights. These rights
** are described in the Nokia Qt GPL Exception version 1.2, included in
** the file GPL_EXCEPTION.txt in this package.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/

#ifndef QDBUS_SYMBOLS_P_H
#define QDBUS_SYMBOLS_P_H

#include <QtCore/qglobal.h>
#include <dbus/dbus.h>

QT_BEGIN_NAMESPACE

#if !defined QT_LINKED_LIBDBUS

void *qdbus_resolve_conditionally(const char *name); // doesn't print a warning
void *qdbus_resolve_me(const char *name); // prints a warning
bool qdbus_loadLibDBus();

# define DEFINEFUNC(ret, func, args, argcall, funcret)          \
    typedef ret (* _q_PTR_##func) args;                         \
    static inline ret q_##func args                             \
    {                                                           \
        static _q_PTR_##func ptr;                               \
        if (!ptr)                                               \
            ptr = (_q_PTR_##func) qdbus_resolve_me(#func);      \
        funcret ptr argcall;                                    \
    }

#else // defined QT_LINKED_LIBDBUS

inline bool qdbus_loadLibDBus() { return true; }

# define DEFINEFUNC(ret, func, args, argcall, funcret) \
    static inline ret q_##func args { funcret func argcall; }

#endif // defined QT_LINKED_LIBDBUS

/* dbus-bus.h */
DEFINEFUNC(void, dbus_bus_add_match, (DBusConnection *connection,
                                      const char     *rule,
                                      DBusError      *error),
           (connection, rule, error), )
DEFINEFUNC(void, dbus_bus_remove_match, (DBusConnection *connection,
                                         const char     *rule,
                                         DBusError      *error),
           (connection, rule, error), )
DEFINEFUNC(dbus_bool_t, dbus_bus_register,(DBusConnection *connection,
                                           DBusError      *error),
           (connection, error), return)
DEFINEFUNC(DBusConnection *, dbus_bus_get_private, (DBusBusType     type,
                                                    DBusError      *error),
           (type, error), return)
DEFINEFUNC(const char*, dbus_bus_get_unique_name, (DBusConnection *connection),
           (connection), return)

/* dbus-connection.h */
DEFINEFUNC(dbus_bool_t        , dbus_connection_add_filter, (DBusConnection            *connection,
                                                             DBusHandleMessageFunction  function,
                                                             void                      *user_data,
                                                             DBusFreeFunction           free_data_function),
           (connection, function, user_data, free_data_function), return)
DEFINEFUNC(void               , dbus_connection_close,      (DBusConnection             *connection),
           (connection), return)
DEFINEFUNC(DBusDispatchStatus , dbus_connection_dispatch,   (DBusConnection             *connection),
           (connection), return)
DEFINEFUNC(DBusDispatchStatus , dbus_connection_get_dispatch_status, (DBusConnection             *connection),
           (connection), return)
DEFINEFUNC(dbus_bool_t        , dbus_connection_get_is_connected, (DBusConnection             *connection),
           (connection), return)
DEFINEFUNC(DBusConnection*    , dbus_connection_open_private, (const char                 *address,
                                                               DBusError                  *error),
           (address, error), return)
DEFINEFUNC(DBusConnection*    , dbus_connection_ref,          (DBusConnection             *connection),
           (connection), return)
DEFINEFUNC(dbus_bool_t        , dbus_connection_send,         (DBusConnection             *connection,
                                                               DBusMessage                *message,
                                                               dbus_uint32_t              *client_serial),
           (connection, message, client_serial), return)
DEFINEFUNC(dbus_bool_t        , dbus_connection_send_with_reply, (DBusConnection             *connection,
                                                                  DBusMessage                *message,
                                                                  DBusPendingCall           **pending_return,
                                                                  int                         timeout_milliseconds),
           (connection, message, pending_return, timeout_milliseconds), return)
DEFINEFUNC(DBusMessage *      , dbus_connection_send_with_reply_and_block, (DBusConnection             *connection,
                                                                            DBusMessage                *message,
                                                                            int                         timeout_milliseconds,
                                                                            DBusError                  *error),
           (connection, message, timeout_milliseconds, error), return)
DEFINEFUNC(void               , dbus_connection_set_exit_on_disconnect, (DBusConnection             *connection,
                                                                         dbus_bool_t                 exit_on_disconnect),
           (connection, exit_on_disconnect), )
DEFINEFUNC(dbus_bool_t        , dbus_connection_set_timeout_functions, (DBusConnection             *connection,
                                                                        DBusAddTimeoutFunction      add_function,
                                                                        DBusRemoveTimeoutFunction   remove_function,
                                                                        DBusTimeoutToggledFunction  toggled_function,
                                                                        void                       *data,
                                                                        DBusFreeFunction            free_data_function),
           (connection, add_function, remove_function, toggled_function, data, free_data_function), return)
DEFINEFUNC(dbus_bool_t        , dbus_connection_set_watch_functions, (DBusConnection             *connection,
                                                                      DBusAddWatchFunction        add_function,
                                                                      DBusRemoveWatchFunction     remove_function,
                                                                      DBusWatchToggledFunction    toggled_function,
                                                                      void                       *data,
                                                                      DBusFreeFunction            free_data_function),
           (connection, add_function, remove_function, toggled_function, data, free_data_function), return)

DEFINEFUNC(void               , dbus_connection_unref, (DBusConnection             *connection),
           (connection), )
DEFINEFUNC(dbus_bool_t , dbus_timeout_get_enabled, (DBusTimeout      *timeout),
           (timeout), return)
DEFINEFUNC(int         , dbus_timeout_get_interval, (DBusTimeout      *timeout),
           (timeout), return)
DEFINEFUNC(dbus_bool_t , dbus_timeout_handle, (DBusTimeout      *timeout),
           (timeout), return)

DEFINEFUNC(dbus_bool_t  , dbus_watch_get_enabled, (DBusWatch        *watch),
           (watch), return)
DEFINEFUNC(int , dbus_watch_get_fd, (DBusWatch        *watch),
           (watch), return)
DEFINEFUNC(unsigned int , dbus_watch_get_flags, (DBusWatch        *watch),
           (watch), return)
DEFINEFUNC(dbus_bool_t  , dbus_watch_handle, (DBusWatch        *watch,
                                              unsigned int      flags),
           (watch, flags), return)

/* dbus-errors.h */
DEFINEFUNC(void        , dbus_error_free, (DBusError       *error),
           (error), )
DEFINEFUNC(void        , dbus_error_init, (DBusError       *error),
           (error), )
DEFINEFUNC(dbus_bool_t , dbus_error_is_set, (const DBusError *error),
           (error), return)

/* dbus-memory.h */
DEFINEFUNC(void  , dbus_free, (void  *memory), (memory), )

/* dbus-message.h */
DEFINEFUNC(DBusMessage* , dbus_message_copy, (const DBusMessage *message),
           (message), return)
DEFINEFUNC(const char*   , dbus_message_get_error_name, (DBusMessage   *message),
           (message), return)
DEFINEFUNC(const char*   , dbus_message_get_interface, (DBusMessage   *message),
           (message), return)
DEFINEFUNC(const char*   , dbus_message_get_member, (DBusMessage   *message),
           (message), return)
DEFINEFUNC(dbus_bool_t   , dbus_message_get_no_reply, (DBusMessage   *message),
           (message), return)
DEFINEFUNC(const char*   , dbus_message_get_path, (DBusMessage   *message),
           (message), return)
DEFINEFUNC(const char*   , dbus_message_get_sender, (DBusMessage   *message),
           (message), return)
DEFINEFUNC(dbus_uint32_t , dbus_message_get_serial, (DBusMessage   *message),
           (message), return)
DEFINEFUNC(const char*   , dbus_message_get_signature, (DBusMessage   *message),
           (message), return)
DEFINEFUNC(int           , dbus_message_get_type, (DBusMessage   *message),
           (message), return)
DEFINEFUNC(dbus_bool_t , dbus_message_iter_append_basic, (DBusMessageIter *iter,
                                                          int              type,
                                                          const void      *value),
           (iter, type, value), return)
DEFINEFUNC(dbus_bool_t , dbus_message_iter_append_fixed_array, (DBusMessageIter *iter,
                                                                int              element_type,
                                                                const void      *value,
                                                                int              n_elements),
           (iter, element_type, value, n_elements), return)
DEFINEFUNC(dbus_bool_t , dbus_message_iter_close_container, (DBusMessageIter *iter,
                                                             DBusMessageIter *sub),
           (iter, sub), return)
DEFINEFUNC(int         , dbus_message_iter_get_arg_type, (DBusMessageIter *iter),
           (iter), return)
DEFINEFUNC(void        , dbus_message_iter_get_basic, (DBusMessageIter *iter,
                                                       void            *value),
           (iter, value), )
DEFINEFUNC(int         , dbus_message_iter_get_element_type, (DBusMessageIter *iter),
           (iter), return)
DEFINEFUNC(void        , dbus_message_iter_get_fixed_array, (DBusMessageIter *iter,
                                                             void            *value,
                                                             int             *n_elements),
           (iter, value, n_elements), return)
DEFINEFUNC(char*       , dbus_message_iter_get_signature, (DBusMessageIter *iter),
           (iter), return)
DEFINEFUNC(dbus_bool_t , dbus_message_iter_init, (DBusMessage     *message,
                                                  DBusMessageIter *iter),
           (message, iter), return)
DEFINEFUNC(void        , dbus_message_iter_init_append, (DBusMessage     *message,
                                                         DBusMessageIter *iter),
           (message, iter), return)
DEFINEFUNC(dbus_bool_t , dbus_message_iter_next, (DBusMessageIter *iter),
           (iter), return)
DEFINEFUNC(dbus_bool_t , dbus_message_iter_open_container, (DBusMessageIter *iter,
                                                            int              type,
                                                            const char      *contained_signature,
                                                            DBusMessageIter *sub),
           (iter, type, contained_signature, sub), return)
DEFINEFUNC(void        , dbus_message_iter_recurse, (DBusMessageIter *iter,
                                                     DBusMessageIter *sub),
           (iter, sub), )
DEFINEFUNC(DBusMessage* , dbus_message_new, (int          message_type),
           (message_type), return)
DEFINEFUNC(DBusMessage* , dbus_message_new_method_call, (const char  *bus_name,
                                                         const char  *path,
                                                         const char  *interface,
                                                         const char  *method),
           (bus_name, path, interface, method), return)
DEFINEFUNC(DBusMessage* , dbus_message_new_signal, (const char  *path,
                                                    const char  *interface,
                                                    const char  *name),
           (path, interface, name), return)
DEFINEFUNC(DBusMessage*  , dbus_message_ref, (DBusMessage   *message),
           (message), return)
DEFINEFUNC(dbus_bool_t   , dbus_message_set_destination, (DBusMessage   *message,
                                                          const char    *destination),
           (message, destination), return)
DEFINEFUNC(dbus_bool_t   , dbus_message_set_error_name, (DBusMessage   *message,
                                                         const char    *name),
           (message, name), return)
DEFINEFUNC(void          , dbus_message_set_no_reply, (DBusMessage   *message,
                                                       dbus_bool_t    no_reply),
           (message, no_reply), return)
DEFINEFUNC(dbus_bool_t   , dbus_message_set_path, (DBusMessage   *message,
                                                   const char    *object_path),
           (message, object_path), return)
DEFINEFUNC(dbus_bool_t   , dbus_message_set_reply_serial, (DBusMessage   *message,
                                                           dbus_uint32_t  reply_serial),
           (message, reply_serial), return)
DEFINEFUNC(dbus_bool_t   , dbus_message_set_sender, (DBusMessage   *message,
                                                     const char    *sender),
           (message, sender), return)
DEFINEFUNC(void          , dbus_message_unref, (DBusMessage   *message),
           (message), )

/* dbus-pending-call.h */
DEFINEFUNC(dbus_bool_t  , dbus_pending_call_set_notify, (DBusPendingCall               *pending,
                                                         DBusPendingCallNotifyFunction  function,
                                                         void                          *user_data,
                                                         DBusFreeFunction               free_user_data),
           (pending, function, user_data, free_user_data), return)
DEFINEFUNC(DBusMessage* , dbus_pending_call_steal_reply, (DBusPendingCall               *pending),
           (pending), return)
DEFINEFUNC(void         , dbus_pending_call_unref, (DBusPendingCall               *pending),
           (pending), return)

/* dbus-server.h */
DEFINEFUNC(dbus_bool_t , dbus_server_allocate_data_slot, (dbus_int32_t     *slot_p),
           (slot_p), return)
DEFINEFUNC(void        , dbus_server_disconnect, (DBusServer     *server),
           (server), )
DEFINEFUNC(char*       , dbus_server_get_address, (DBusServer     *server),
           (server), return)
DEFINEFUNC(dbus_bool_t , dbus_server_get_is_connected, (DBusServer     *server),
           (server), return)
DEFINEFUNC(DBusServer* , dbus_server_listen, (const char     *address,
                                              DBusError      *error),
           (address, error), return)
DEFINEFUNC(dbus_bool_t , dbus_server_set_data, (DBusServer       *server,
                                                int               slot,
                                                void             *data,
                                                DBusFreeFunction  free_data_func),
           (server, slot, data, free_data_func), return)
DEFINEFUNC(void        , dbus_server_set_new_connection_function, (DBusServer                *server,
                                                                   DBusNewConnectionFunction  function,
                                                                   void                      *data,
                                                                   DBusFreeFunction           free_data_function),
           (server, function, data, free_data_function), )
DEFINEFUNC(dbus_bool_t , dbus_server_set_timeout_functions, (DBusServer                *server,
                                                             DBusAddTimeoutFunction     add_function,
                                                             DBusRemoveTimeoutFunction  remove_function,
                                                             DBusTimeoutToggledFunction toggled_function,
                                                             void                      *data,
                                                             DBusFreeFunction           free_data_function),
           (server, add_function, remove_function, toggled_function, data, free_data_function), return)
DEFINEFUNC(dbus_bool_t , dbus_server_set_watch_functions, (DBusServer                *server,
                                                           DBusAddWatchFunction       add_function,
                                                           DBusRemoveWatchFunction    remove_function,
                                                           DBusWatchToggledFunction   toggled_function,
                                                           void                      *data,
                                                           DBusFreeFunction           free_data_function),
           (server, add_function, remove_function, toggled_function, data, free_data_function), return)
DEFINEFUNC(void        , dbus_server_unref, (DBusServer     *server),
           (server), )

/* dbus-signature.h */
DEFINEFUNC(dbus_bool_t     , dbus_signature_validate, (const char       *signature,
                                                       DBusError        *error),
           (signature, error), return)
DEFINEFUNC(dbus_bool_t     , dbus_signature_validate_single, (const char       *signature,
                                                              DBusError        *error),
           (signature, error), return)
DEFINEFUNC(dbus_bool_t     , dbus_type_is_basic, (int            typecode),
           (typecode), return);
DEFINEFUNC(dbus_bool_t     , dbus_type_is_fixed, (int            typecode),
           (typecode), return)

QT_END_NAMESPACE

#endif
