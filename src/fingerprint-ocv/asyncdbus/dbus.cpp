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
#include "jinx/dbus/dbus.hpp"

namespace jinx {
namespace dbus {

JINX_ERROR_IMPLEMENT(dbus, {
    switch(code.as<ErrorAsyncDBus>()) {
        case ErrorAsyncDBus::failed:                     return "org.freedesktop.DBus.Error.Failed";
        case ErrorAsyncDBus::no_memory:                  return "org.freedesktop.DBus.Error.NoMemory";
        case ErrorAsyncDBus::service_unknown:            return "org.freedesktop.DBus.Error.ServiceUnknown";
        case ErrorAsyncDBus::name_has_no_owner:          return "org.freedesktop.DBus.Error.NameHasNoOwner";
        case ErrorAsyncDBus::no_reply:                   return "org.freedesktop.DBus.Error.NoReply";
        case ErrorAsyncDBus::io_error:                   return "org.freedesktop.DBus.Error.IOError";
        case ErrorAsyncDBus::bad_address:                return "org.freedesktop.DBus.Error.BadAddress";
        case ErrorAsyncDBus::not_supported:              return "org.freedesktop.DBus.Error.NotSupported";
        case ErrorAsyncDBus::limits_exceeded:            return "org.freedesktop.DBus.Error.LimitsExceeded";
        case ErrorAsyncDBus::access_denied:              return "org.freedesktop.DBus.Error.AccessDenied";
        case ErrorAsyncDBus::auth_failed:                return "org.freedesktop.DBus.Error.AuthFailed";
        case ErrorAsyncDBus::no_server:                  return "org.freedesktop.DBus.Error.NoServer";
        case ErrorAsyncDBus::timeout:                    return "org.freedesktop.DBus.Error.Timeout";
        case ErrorAsyncDBus::no_network:                 return "org.freedesktop.DBus.Error.NoNetwork";
        case ErrorAsyncDBus::address_in_use:             return "org.freedesktop.DBus.Error.AddressInUse";
        case ErrorAsyncDBus::disconnected:               return "org.freedesktop.DBus.Error.Disconnected";
        case ErrorAsyncDBus::invalid_args:               return "org.freedesktop.DBus.Error.InvalidArgs";
        case ErrorAsyncDBus::file_not_found:             return "org.freedesktop.DBus.Error.FileNotFound";
        case ErrorAsyncDBus::file_exists:                return "org.freedesktop.DBus.Error.FileExists";
        case ErrorAsyncDBus::unknown_method:             return "org.freedesktop.DBus.Error.UnknownMethod";
        case ErrorAsyncDBus::unknown_object:             return "org.freedesktop.DBus.Error.UnknownObject";
        case ErrorAsyncDBus::unknown_interface:          return "org.freedesktop.DBus.Error.UnknownInterface";
        case ErrorAsyncDBus::unknown_property:           return "org.freedesktop.DBus.Error.UnknownProperty";
        case ErrorAsyncDBus::property_read_only:         return "org.freedesktop.DBus.Error.PropertyReadOnly";
        case ErrorAsyncDBus::timed_out:                  return "org.freedesktop.DBus.Error.TimedOut";
        case ErrorAsyncDBus::match_rule_not_found:       return "org.freedesktop.DBus.Error.MatchRuleNotFound";
        case ErrorAsyncDBus::match_rule_invalid:         return "org.freedesktop.DBus.Error.MatchRuleInvalid";
        case ErrorAsyncDBus::spawn_exec_failed:          return "org.freedesktop.DBus.Error.Spawn.ExecFailed";
        case ErrorAsyncDBus::spawn_fork_failed:          return "org.freedesktop.DBus.Error.Spawn.ForkFailed";
        case ErrorAsyncDBus::spawn_child_exited:         return "org.freedesktop.DBus.Error.Spawn.ChildExited";
        case ErrorAsyncDBus::spawn_child_signaled:       return "org.freedesktop.DBus.Error.Spawn.ChildSignaled";
        case ErrorAsyncDBus::spawn_failed:               return "org.freedesktop.DBus.Error.Spawn.Failed";
        case ErrorAsyncDBus::spawn_setup_failed:         return "org.freedesktop.DBus.Error.Spawn.FailedToSetup";
        case ErrorAsyncDBus::spawn_config_invalid:       return "org.freedesktop.DBus.Error.Spawn.ConfigInvalid";
        case ErrorAsyncDBus::spawn_service_invalid:      return "org.freedesktop.DBus.Error.Spawn.ServiceNotValid";
        case ErrorAsyncDBus::spawn_service_not_found:    return "org.freedesktop.DBus.Error.Spawn.ServiceNotFound";
        case ErrorAsyncDBus::spawn_permissions_invalid:  return "org.freedesktop.DBus.Error.Spawn.PermissionsInvalid";
        case ErrorAsyncDBus::spawn_file_invalid:         return "org.freedesktop.DBus.Error.Spawn.FileInvalid";
        case ErrorAsyncDBus::spawn_no_memory:            return "org.freedesktop.DBus.Error.Spawn.NoMemory";
        case ErrorAsyncDBus::unix_process_id_unknown:    return "org.freedesktop.DBus.Error.UnixProcessIdUnknown";
        case ErrorAsyncDBus::invalid_signature:          return "org.freedesktop.DBus.Error.InvalidSignature";
        case ErrorAsyncDBus::invalid_file_content:       return "org.freedesktop.DBus.Error.InvalidFileContent";
        case ErrorAsyncDBus::selinux_security_context_unknown:    return "org.freedesktop.DBus.Error.SELinuxSecurityContextUnknown";
        case ErrorAsyncDBus::adt_audit_data_unknown:     return "org.freedesktop.DBus.Error.AdtAuditDataUnknown";
        case ErrorAsyncDBus::object_path_in_use:         return "org.freedesktop.DBus.Error.ObjectPathInUse";
        case ErrorAsyncDBus::inconsistent_message:       return "org.freedesktop.DBus.Error.InconsistentMessage";
        case ErrorAsyncDBus::interactive_authorization_required: return "org.freedesktop.DBus.Error.InteractiveAuthorizationRequired";
        case ErrorAsyncDBus::not_container:              return "org.freedesktop.DBus.Error.NotContainer";
        default: break;
    }
    return "unknown error";
});

} // namespace dbus
} // namespace jinx
