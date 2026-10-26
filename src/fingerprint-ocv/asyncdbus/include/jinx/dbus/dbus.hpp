/*
Copyright (C) 2022  pom@vro.life

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as published
by the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#ifndef __jinx_dbus_hpp__
#define __jinx_dbus_hpp__

#include <chrono>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <system_error>
#include <unordered_map>
#include <functional>

#include <dbus-1.0/dbus/dbus.h>

#include "jinx/async.hpp"
#include "jinx/logging.hpp"
#include "jinx/queue2.hpp"
#include <jinx/macros.hpp>
#include <jinx/hash.hpp>
#include <jinx/raii.hpp>
#include <jinx/posix.hpp>

namespace jinx {
namespace dbus {

JINX_RAII_SHARED_OBJECT(
    AsyncDBusConnection, 
    ::DBusConnection, 
    dbus_connection_ref,
    dbus_connection_unref);

JINX_RAII_SHARED_OBJECT(
    AsyncDBusMessage, 
    ::DBusMessage, 
    dbus_message_ref,
    dbus_message_unref);

JINX_RAII_SHARED_OBJECT(
    AsyncDBusPendingCall, 
    ::DBusPendingCall, 
    dbus_pending_call_ref,
    dbus_pending_call_unref);

JINX_RAII_SIMPLE_OBJECT(
    AsyncDBusError, 
    ::DBusError, 
    dbus_error_free)

enum class ErrorAsyncDBus : size_t {
    no_error = 0,
    failed =                     hash::hash_string("org.freedesktop.DBus.Error.Failed"),
    no_memory =                  hash::hash_string("org.freedesktop.DBus.Error.NoMemory"),
    service_unknown =            hash::hash_string("org.freedesktop.DBus.Error.ServiceUnknown"),
    name_has_no_owner =          hash::hash_string("org.freedesktop.DBus.Error.NameHasNoOwner"),
    no_reply =                   hash::hash_string("org.freedesktop.DBus.Error.NoReply"),
    io_error =                   hash::hash_string("org.freedesktop.DBus.Error.IOError"),
    bad_address =                hash::hash_string("org.freedesktop.DBus.Error.BadAddress"),
    not_supported =              hash::hash_string("org.freedesktop.DBus.Error.NotSupported"),
    limits_exceeded =            hash::hash_string("org.freedesktop.DBus.Error.LimitsExceeded"),
    access_denied =              hash::hash_string("org.freedesktop.DBus.Error.AccessDenied"),
    auth_failed =                hash::hash_string("org.freedesktop.DBus.Error.AuthFailed"),
    no_server =                  hash::hash_string("org.freedesktop.DBus.Error.NoServer"),
    timeout =                    hash::hash_string("org.freedesktop.DBus.Error.Timeout"),
    no_network =                 hash::hash_string("org.freedesktop.DBus.Error.NoNetwork"),
    address_in_use =             hash::hash_string("org.freedesktop.DBus.Error.AddressInUse"),
    disconnected =               hash::hash_string("org.freedesktop.DBus.Error.Disconnected"),
    invalid_args =               hash::hash_string("org.freedesktop.DBus.Error.InvalidArgs"),
    file_not_found =             hash::hash_string("org.freedesktop.DBus.Error.FileNotFound"),
    file_exists =                hash::hash_string("org.freedesktop.DBus.Error.FileExists"),
    unknown_method =             hash::hash_string("org.freedesktop.DBus.Error.UnknownMethod"),
    unknown_object =             hash::hash_string("org.freedesktop.DBus.Error.UnknownObject"),
    unknown_interface =          hash::hash_string("org.freedesktop.DBus.Error.UnknownInterface"),
    unknown_property =           hash::hash_string("org.freedesktop.DBus.Error.UnknownProperty"),
    property_read_only =         hash::hash_string("org.freedesktop.DBus.Error.PropertyReadOnly"),
    timed_out =                  hash::hash_string("org.freedesktop.DBus.Error.TimedOut"),
    match_rule_not_found =       hash::hash_string("org.freedesktop.DBus.Error.MatchRuleNotFound"),
    match_rule_invalid =         hash::hash_string("org.freedesktop.DBus.Error.MatchRuleInvalid"),
    spawn_exec_failed =          hash::hash_string("org.freedesktop.DBus.Error.Spawn.ExecFailed"),
    spawn_fork_failed =          hash::hash_string("org.freedesktop.DBus.Error.Spawn.ForkFailed"),
    spawn_child_exited =         hash::hash_string("org.freedesktop.DBus.Error.Spawn.ChildExited"),
    spawn_child_signaled =       hash::hash_string("org.freedesktop.DBus.Error.Spawn.ChildSignaled"),
    spawn_failed =               hash::hash_string("org.freedesktop.DBus.Error.Spawn.Failed"),
    spawn_setup_failed =         hash::hash_string("org.freedesktop.DBus.Error.Spawn.FailedToSetup"),
    spawn_config_invalid =       hash::hash_string("org.freedesktop.DBus.Error.Spawn.ConfigInvalid"),
    spawn_service_invalid =      hash::hash_string("org.freedesktop.DBus.Error.Spawn.ServiceNotValid"),
    spawn_service_not_found =    hash::hash_string("org.freedesktop.DBus.Error.Spawn.ServiceNotFound"),
    spawn_permissions_invalid =  hash::hash_string("org.freedesktop.DBus.Error.Spawn.PermissionsInvalid"),
    spawn_file_invalid =         hash::hash_string("org.freedesktop.DBus.Error.Spawn.FileInvalid"),
    spawn_no_memory =            hash::hash_string("org.freedesktop.DBus.Error.Spawn.NoMemory"),
    unix_process_id_unknown =    hash::hash_string("org.freedesktop.DBus.Error.UnixProcessIdUnknown"),
    invalid_signature =          hash::hash_string("org.freedesktop.DBus.Error.InvalidSignature"),
    invalid_file_content =       hash::hash_string("org.freedesktop.DBus.Error.InvalidFileContent"),
    selinux_security_context_unknown =    hash::hash_string("org.freedesktop.DBus.Error.SELinuxSecurityContextUnknown"),
    adt_audit_data_unknown =     hash::hash_string("org.freedesktop.DBus.Error.AdtAuditDataUnknown"),
    object_path_in_use =         hash::hash_string("org.freedesktop.DBus.Error.ObjectPathInUse"),
    inconsistent_message =       hash::hash_string("org.freedesktop.DBus.Error.InconsistentMessage"),
    interactive_authorization_required = hash::hash_string("org.freedesktop.us.Error.InteractiveAuthorizationRequired"),
    not_container = hash::hash_string("org.freedesktop.DBus.Error.NotContainer"),
};

JINX_ERROR_DEFINE(dbus, ErrorAsyncDBus);

template<typename AsyncImpl, typename EventEngine=typename AsyncImpl::EventEngineType>
class AsyncDBusAgent
{
    struct WatchData {
        AsyncDBusAgent* _self;
        DBusWatch* _watch;
        typename EventEngine::EventHandleIO _event_handle;
    };

    struct TimeoutData {
        AsyncDBusAgent* _self;
        DBusTimeout *_timeout;
        typename EventEngine::EventHandleTimer _timer_handle;
    };

    EventEngine& _eve;
    AsyncDBusConnection _conn{};

    std::unordered_map<DBusWatch*, WatchData*> _watchs{};
    std::unordered_map<DBusTimeout*, TimeoutData*> _timeouts{};

    typename EventEngine::EventHandleTimer _dispatch_event{};

public:
    explicit AsyncDBusAgent(EventEngine& eve, AsyncDBusConnection& conn)
    : _eve(eve), _conn(conn)
    {
        dbus_connection_set_watch_functions(
            _conn, 
            add_watch, 
            remove_watch, 
            toggled_watch, 
            this, 
            nullptr);

        dbus_connection_set_timeout_functions(
            _conn, 
            add_timeout, 
            remove_timeout, 
            toggled_timeout, 
            this, 
            nullptr);

        dbus_connection_set_dispatch_status_function(
            _conn, 
            handle_dispatch_status, 
            this, 
            nullptr);

        dbus_connection_set_wakeup_main_function(
            _conn, 
            handle_wakeup_main, 
            this, 
            nullptr);
    }

    AsyncDBusConnection& get_connection() { return _conn; }

    void dispatch() {
        dbus_dispatch({}, this);
    }

private:
    static void handle_dispatch_status(DBusConnection* conn, DBusDispatchStatus status, void* data) {
        auto* self = reinterpret_cast<AsyncDBusAgent*>(data);

        if (status == DBUS_DISPATCH_DATA_REMAINS) {
            struct timeval timeval{ 0 , 0 };
            self->_eve.add_timer(
                self->_dispatch_event, 
                &timeval, 
                dbus_dispatch, 
                self) >> JINX_IGNORE_RESULT;
        }
    }

    static void dbus_dispatch(const error::Error& error, void* data) {
        auto* self = reinterpret_cast<AsyncDBusAgent*>(data);
        self->_dispatch_event.reset();
        DBusDispatchStatus status;
        while (true) { 
            status = dbus_connection_dispatch(self->_conn);
            if (status != DBUS_DISPATCH_DATA_REMAINS) {
                break;
            }
        }

        if (status == DBUS_DISPATCH_NEED_MEMORY) {
            jinx_log_error() << "dbus_connection_dispatch out of memory" << std::endl;
        }
    }

    static void handle_wakeup_main(void* data) {
        auto* self = reinterpret_cast<AsyncDBusAgent*>(data);
        self->_eve.wakeup();
    }

    void enable_watch(WatchData* watch_data) {
        auto events = dbus_watch_get_flags(watch_data->_watch);
        unsigned int flags = EventEngine::IOFlagPersist;
        if ((events & DBUS_WATCH_READABLE) != 0) {
            flags |= EventEngine::IOFlagRead;
        }
        if ((events & DBUS_WATCH_WRITABLE) != 0) {
            flags |= EventEngine::IOFlagWrite;
        }
        int dbus_fd = dbus_watch_get_unix_fd(watch_data->_watch);
        _eve.add_io(
            flags, 
            watch_data->_event_handle, 
            dbus_fd, 
            handle_watch_event, 
            watch_data) >> JINX_IGNORE_RESULT;
    }

    void disable_watch(WatchData* watch_data) {
        _eve.remove_io(watch_data->_event_handle) >> JINX_IGNORE_RESULT;
    }

    static dbus_bool_t add_watch(::DBusWatch* watch, void* data)
    {
        auto* self = reinterpret_cast<AsyncDBusAgent*>(data);

        auto pair = self->_watchs.emplace(
            watch,
            new WatchData{
                self,
                watch,
                {}
            }
        );

        if (dbus_watch_get_enabled(watch) == FALSE) {
            return TRUE;
        }

        self->enable_watch(pair.first->second);
        return TRUE;
    }

    static void remove_watch(::DBusWatch* watch, void* data)
    {
        auto* self = reinterpret_cast<AsyncDBusAgent*>(data);

        auto iter = self->_watchs.find(watch);
        if (iter == self->_watchs.end()) {
            return;
        }

        if (dbus_watch_get_enabled(watch) != 0) {
            self->disable_watch((iter->second));
        }

        delete iter->second;
        self->_watchs.erase(iter);
    }

    static void toggled_watch(::DBusWatch* watch, void* data)
    {
        auto* self = reinterpret_cast<AsyncDBusAgent*>(data);

        auto iter = self->_watchs.find(watch);
        if (iter == self->_watchs.end()) {
            return;
        }

        if (dbus_watch_get_enabled(watch) == 0) {
            self->disable_watch((iter->second));
        } else {
            self->enable_watch((iter->second));
        }
    }

    static void handle_watch_event(unsigned int flags, const error::Error& error, void* data)
    {
        auto* watch_data = reinterpret_cast<WatchData*>(data);

        unsigned int dbus_flags = 0;
        if ((flags & EventEngine::IOFlagRead) != 0) {
            dbus_flags |= DBUS_WATCH_READABLE;
        }
        if ((flags & EventEngine::IOFlagWrite) != 0) {
            dbus_flags |= DBUS_WATCH_WRITABLE;
        }
        
        if (dbus_watch_handle(watch_data->_watch, dbus_flags) == FALSE) {
            jinx_log_error() << "dbus_watch_handle out of memory" << std::endl;
        }

        handle_dispatch_status(watch_data->_self->_conn, DBUS_DISPATCH_DATA_REMAINS, watch_data->_self);
    }

    void enable_timeout(TimeoutData* timeout_data)
    {
        auto timeout_ms = dbus_timeout_get_interval(timeout_data->_timeout);
        
        struct timeval timeval{
            timeout_ms / 1000,
            (timeout_ms % 1000 ) * 1000
        };
        _eve.add_timer(
            timeout_data->_timer_handle, 
            &timeval, 
            handle_timeout_event, 
            timeout_data) >> JINX_IGNORE_RESULT;
    }

    void disable_timeout(TimeoutData* timeout_data) {
        _eve.remove_timer(timeout_data->_timer_handle) >> JINX_IGNORE_RESULT;
    }

    static dbus_bool_t add_timeout(::DBusTimeout* timeout, void* data)
    {
        auto* self = reinterpret_cast<AsyncDBusAgent*>(data);

        auto pair = self->_timeouts.emplace(
            timeout,
            new TimeoutData{self, timeout, {}}
        );

        if (dbus_timeout_get_enabled(timeout) == 0) {
            return TRUE;
        }

        self->enable_timeout(pair.first->second);
        return TRUE;
    }

    static void toggled_timeout(::DBusTimeout* timeout, void* data)
    {
        auto* self = reinterpret_cast<AsyncDBusAgent*>(data);

        auto iter = self->_timeouts.find(timeout);
        if (iter == self->_timeouts.end()) {
            return;
        }

        if (dbus_timeout_get_enabled(timeout) == 0) {
            self->disable_timeout((iter->second));
        } else {
            self->enable_timeout((iter->second));
        }
    }

    static void remove_timeout(::DBusTimeout* timeout, void* data)
    {
        auto* self = reinterpret_cast<AsyncDBusAgent*>(data);

        auto iter = self->_timeouts.find(timeout);
        if (iter == self->_timeouts.end()) {
            return;
        }

        self->disable_timeout((iter->second));

        delete iter->second;
        self->_timeouts.erase(iter);
    }

    static void handle_timeout_event(const error::Error& error, void* data)
    {
        auto* timeout_data = reinterpret_cast<TimeoutData*>(data);

        if (dbus_timeout_handle(timeout_data->_timeout) == FALSE) {
            struct timeval timeval{ 0, 0 };
            timeout_data->_self->_eve.add_timer(
                timeout_data->_timer_handle, 
                &timeval, 
                handle_timeout_event, 
                data) >> JINX_IGNORE_RESULT;
        }
    }
};

class AsyncDBusSendWithReply : public Awaitable, public MixinResult<AsyncDBusMessage>
{
    AsyncDBusConnection _conn{};
    AsyncDBusPendingCall _call{};
    AsyncDBusMessage _message{};
    unsigned int _timeout{0};

public:
    template<typename Rep, typename Period>
    AsyncDBusSendWithReply& operator() (
        AsyncDBusConnection& conn, 
        AsyncDBusMessage& _msg,
        const std::chrono::duration<Rep, Period>& timeout) 
    {
        _conn = conn;
        _message = _msg;
        _timeout = std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count();
        _call.reset();
        return *this;
    }

protected:
    void async_finalize() noexcept override {
        if (_call != nullptr and dbus_pending_call_get_completed(_call) == 0) {
            dbus_pending_call_cancel(_call);
        }
        Awaitable::async_finalize();
    }

    Async async_poll() override {
        if (_call == nullptr) {
            auto success = dbus_connection_send_with_reply(
                _conn, 
                _message, 
                _call.address(), 
                _timeout
            );
            if (success == 0) {
                return async_throw(posix::ErrorPosix::NotEnoughMemory);
            }
            dbus_pending_call_set_notify(_call, notify, this, nullptr);
            return async_suspend();
        }
        if (dbus_pending_call_get_completed(_call) == FALSE) {
            // TODO raise error
            ::abort();
        }
        emplace_result(dbus_pending_call_steal_reply(_call));
        return async_return();
    }

    static void notify(DBusPendingCall* call, void* data) {
        auto* self = reinterpret_cast<AsyncDBusSendWithReply*>(data);
        self->async_resume() >> JINX_IGNORE_RESULT;
    }
};

class AsyncDBusSend : public Awaitable, public MixinResult<dbus_uint32_t>
{
    AsyncDBusConnection _conn{};
    AsyncDBusMessage _message{};

public:
    AsyncDBusSend& operator() (
        AsyncDBusConnection& conn, 
        AsyncDBusMessage& _msg) 
    {
        _conn = conn;
        _message = _msg;
        return *this;
    }

protected:
    void async_finalize() noexcept override {
        Awaitable::async_finalize();
    }

    Async async_poll() override {
        dbus_uint32_t _client_serial{0};
        auto success = dbus_connection_send(
            _conn, 
            _message, 
            &_client_serial
        );
        if (success == 0) {
            return async_throw(posix::ErrorPosix::NotEnoughMemory);
        }
        emplace_result(_client_serial);
        return async_return();
    }

    static void notify(DBusPendingCall* call, void* data) {
        auto* self = reinterpret_cast<AsyncDBusSend*>(data);
        self->async_resume() >> JINX_IGNORE_RESULT;
    }
};

class AsyncDBusObject;
class AsyncDBusInterface;

class AsyncDBusNode
{
public:
    typedef std::function<Async()> CallbackType; // TODO heapless callback

private:
    friend class AsyncDBusInterface;
    friend class AsyncDBusObject;

    AsyncDBusObject* _object;
    std::string _xml{};

public:
    explicit AsyncDBusNode(AsyncDBusObject* object, const std::string& path)
    : _object(object)
    {
        _xml.reserve(4096);
        _xml.append("<!DOCTYPE node PUBLIC \"-//freedesktop//DTD D-BUS Object Introspection 1.0//EN\"\n");
        _xml.append("\"http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd\">\n");
        _xml.append("<node name=\"");
        _xml.append(path);
        _xml.append("\">");
    }

    void complete() {
        _xml.append("</node>");
    }

    template<typename F>
    void add_interface(const std::string& name, F&& func);
};

class AsyncDBusInterface
{
    friend class AsyncDBusObject;
    AsyncDBusNode* _node;
    std::string _iface{};
    // std::string _xml{};

public:
    explicit AsyncDBusInterface(AsyncDBusNode* node, const std::string& iface)
    : _node(node), _iface(iface)
    {
        _node->_xml.append("<interface name=\"");
        _node->_xml.append(iface);
        _node->_xml.append("\">");
    }

    ~AsyncDBusInterface() = default;

    void complete() {
        _node->_xml.append("</interface>");
    }

    template<typename T>
    void add_method(
        const std::string& name, 
        const std::string& in_signature, 
        const std::string& out_signature,
        Async(T::*callback)());
    
    void add_signal(const std::string& name, const std::string& signature) {
        _node->_xml.append("<signal name=\"");
        _node->_xml.append(name);
        _node->_xml.append("\">");

        DBusSignatureIter iter{};

        if (not signature.empty()) {
            int narg = 0;
            dbus_signature_iter_init(&iter, signature.c_str());
            do {
                _node->_xml.append("<arg name=\"arg");
                _node->_xml.append(std::to_string(narg ++));
                _node->_xml.append("\" type=\"");
                _node->_xml.append(dbus_signature_iter_get_signature(&iter));
                _node->_xml.append("\"/>");
            } while(dbus_signature_iter_next(&iter) == TRUE);
        }
        
        _node->_xml.append("</signal>");
    }

    void add_property(const std::string& name, const std::string& type, const std::string& access)
    {
        _node->_xml.append("<property name=\"");
        _node->_xml.append(name);
        _node->_xml.append("\" type=\"");
        _node->_xml.append(type);
        _node->_xml.append("\" access=\"");
        _node->_xml.append(access);
        _node->_xml.append("\">");
        _node->_xml.append("</property>");
    }

    AsyncDBusInterface(const AsyncDBusInterface&) = delete;
    AsyncDBusInterface& operator=(const AsyncDBusInterface&) = delete;

    AsyncDBusInterface(AsyncDBusInterface&&) = default;
    AsyncDBusInterface& operator = (AsyncDBusInterface&&) noexcept = default;
};

template<typename F>
void AsyncDBusNode::add_interface(const std::string& name, F&& func) {
    AsyncDBusInterface iface{this, name};

    func(iface);

    iface.complete();
}

class AsyncDBusObject 
: public AsyncRoutine, private Queue2<std::queue<AsyncDBusMessage>>::CallbackPut
{
    typedef Queue2<std::queue<AsyncDBusMessage>> QueueType;

    DBusObjectPathVTable _vtable{ unregister, message };
    QueueType _queue{};
    QueueType::Get _get_message{};
    AsyncDBusMessage _pending_message{};
    
    std::unordered_map<size_t, AsyncDBusNode::CallbackType> _methods{};

    AsyncDBusNode::CallbackType _current_callback{};
    std::string _path{};
    std::string _xml{};

    bool _async_finalized{false};

    AsyncDBusSend _send_message{};

protected:
    AsyncDBusConnection _connection{};

    template<typename F>
    void add_node(const std::string& path, F&& func) {
        _path = path;
        AsyncDBusNode node(this, path);

        func(node);

        node.add_interface(
            "org.freedesktop.DBus.Introspectable", 
            [&](AsyncDBusInterface& iface){
                iface.add_method(
                    "Introspect", 
                    "", 
                    "s", 
                    &AsyncDBusObject::handle_introspect);
        });

        node.add_interface(
            "org.freedesktop.DBus.Properties", 
            [&](AsyncDBusInterface& iface){
                iface.add_method(
                    "Get", 
                    "ss", 
                    "v", 
                    &AsyncDBusObject::handle_get_property);
                iface.add_method(
                    "Set", 
                    "ssv", 
                    "", 
                    &AsyncDBusObject::handle_set_property);
                iface.add_method(
                    "GetAll", 
                    "s", 
                    "a{sv}", 
                    &AsyncDBusObject::handle_get_all_properties);
        });

        node.complete();
        _xml = std::move(node._xml);
    }
    
public:
    AsyncDBusObject()= default;

    AsyncDBusObject& operator ()(AsyncDBusConnection& conn, size_t max=0) 
    {
        _async_finalized = false;
        _queue.set_max_size(max);
        _connection = conn;

        DBusError error{};
        dbus_error_init(&error);

        dbus_connection_try_register_object_path(
            _connection, 
            _path.c_str(), 
            &_vtable, 
            this,
            &error);

        if (dbus_error_is_set(&error) == TRUE) {
            jinx_log_error() << error.message << std::endl;
            async_throw(make_error(static_cast<ErrorAsyncDBus>(hash::hash_string(error.name))));
        }

        async_start(&AsyncDBusObject::init);
        return *this;
    }

    template<typename T>
    void add_method(
        const std::string& iface,
        const std::string& method, 
        const std::string& in_signature, 
        Async(T::*callback)()
    ) {
        size_t
        key = hash::hash_string(iface.c_str());
        key = hash::hash_string(method.c_str(), key);
        key = hash::hash_string(in_signature.c_str(), key);

        _methods.emplace(key, std::bind(callback, static_cast<T*>(this)));
    }

    QueueType* get_message_queue() {
        return &_queue;
    }

protected:
    AsyncDBusMessage& get_message() {
        return _get_message.get_result();
    }

    virtual Async init() {
        return run();
    }

    Async run() {
        _current_callback = nullptr;
        return *this / _get_message(&_queue) / &AsyncDBusObject::dispatch_message;
    }

    Async dispatch_message() {
        auto& msg = get_message();
        
        const auto* iface = dbus_message_get_interface(msg);
        const auto* method = dbus_message_get_member(msg);
        const auto* signature = dbus_message_get_signature(msg);

        size_t 
        key = hash::hash_string(iface);
        key = hash::hash_string(method, key);
        key = hash::hash_string(signature, key);

        auto iter = _methods.find(key);
        if (iter != _methods.end()) {
            _current_callback = iter->second;
            return handle_message();
        }

        // drop message
        return run();
    }

    void async_finalize() noexcept override {
        _async_finalized = true; // for unregister
        dbus_connection_unregister_object_path(_connection, _path.c_str());
        _get_message.reset();
        _queue.reset();
        _connection.reset();
        AsyncRoutine::async_finalize();
    }
 
    virtual Async handle_message() {
        async_start(&AsyncDBusObject::handle_message);
        return _current_callback();
    }

    virtual Async handle_get_property() {
        auto& msg = get_message();
        AsyncDBusMessage reply(
            dbus_message_new_error_printf(msg, DBUS_ERROR_UNKNOWN_PROPERTY, "no such property")
        );
        return *this / _send_message(_connection, reply) / &AsyncDBusObject::run;
    }

    virtual Async handle_set_property() {
        auto& msg = get_message();
        AsyncDBusMessage reply(
            dbus_message_new_error_printf(msg, DBUS_ERROR_UNKNOWN_PROPERTY, "no such property")
        );
        return *this / _send_message(_connection, reply) / &AsyncDBusObject::run;
    }

    virtual Async handle_get_all_properties() {
        auto& msg = get_message();
        AsyncDBusMessage reply(
            dbus_message_new_method_return(msg)
        );
        DBusMessageIter iter{};
        dbus_message_iter_init_append(reply, &iter);
        DBusMessageIter sub{};
        dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY, "{sv}", &sub);
        dbus_message_iter_close_container(&iter, &sub);
        return *this / _send_message(_connection, reply) / &AsyncDBusObject::run;
    }

private:
    Async handle_introspect() {
        auto& msg = get_message();
        AsyncDBusMessage reply(dbus_message_new_method_return(msg));
        DBusMessageIter iter{};
        const char* str = _xml.c_str();
        dbus_message_iter_init_append(reply, &iter);
        dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &str);
        return *this / _send_message(_connection, reply) / &AsyncDBusObject::run;
    }

    AsyncDBusMessage queue2_put() override {
        return std::move(_pending_message);
    }

    void queue2_cancel_pending_put() override {
        async_resume(make_error(ErrorAwaitable::Cancelled)) >> JINX_IGNORE_RESULT;
    }

    static void unregister(DBusConnection* conn, void* data) {
        auto* self = reinterpret_cast<AsyncDBusObject*>(data);
        if (not self->_async_finalized) {
            self->async_resume(make_error(ErrorAwaitable::Cancelled)) >> JINX_IGNORE_RESULT;
        }
    }

    static DBusHandlerResult message(DBusConnection* conn, DBusMessage* msg, void* data) {
        auto* self = reinterpret_cast<AsyncDBusObject*>(data);
        self->async_resume() >> JINX_IGNORE_RESULT;
        
        self->_pending_message.reset(msg);
        self->_pending_message.ref();
        if (self->_queue.put(self).is_not(Queue2Status::Error)) {
            return DBUS_HANDLER_RESULT_HANDLED;
        }
        self->_pending_message.reset();
        return DBUS_HANDLER_RESULT_NEED_MEMORY;
    }
};

template<typename T>
void AsyncDBusInterface::add_method(
    const std::string& name, 
    const std::string& in_signature, 
    const std::string& out_signature,
    Async(T::*callback)())
{
    DBusError error{};
    dbus_error_init(&error);
    AsyncDBusError auto_free_error(&error);

    dbus_signature_validate(in_signature.c_str(), &error);
    if (dbus_error_is_set(&error)) {
        throw std::invalid_argument(error.message);
    }

    dbus_signature_validate(out_signature.c_str(), &error);
    if (dbus_error_is_set(&error)) {
        throw std::invalid_argument(error.message);
    }

    _node->_object->add_method(_iface, name, in_signature, callback);

    _node->_xml.append("<method name=\"");
    _node->_xml.append(name);
    _node->_xml.append("\">");

    DBusSignatureIter iter{};

    if (not in_signature.empty()) {
        int narg = 0;
        dbus_signature_iter_init(&iter, in_signature.c_str());
        do {
            _node->_xml.append("<arg name=\"arg");
            _node->_xml.append(std::to_string(narg ++));
            _node->_xml.append("\" type=\"");
            _node->_xml.append(dbus_signature_iter_get_signature(&iter));
            _node->_xml.append("\" direction=\"in\"/>");
        } while(dbus_signature_iter_next(&iter) == TRUE);
    }
    
    if (not out_signature.empty()) {
        int narg = 0;
        dbus_signature_iter_init(&iter, out_signature.c_str());
        do {
            _node->_xml.append("<arg name=\"arg");
            _node->_xml.append(std::to_string(narg ++));
            _node->_xml.append("\" type=\"");
            _node->_xml.append(dbus_signature_iter_get_signature(&iter));
            _node->_xml.append("\" direction=\"out\"/>");
        } while(dbus_signature_iter_next(&iter) == TRUE);
    }

    _node->_xml.append("</method>");
}

} // namespace dbus
} // namespace jinx

#endif
