pkg_check_modules(DBUS REQUIRED IMPORTED_TARGET dbus-1)

add_library(jinx_dbus STATIC dbus.cpp)
add_library(jinx::dbus ALIAS jinx_dbus)
target_include_directories(jinx_dbus PUBLIC include)
target_link_libraries(jinx_dbus PUBLIC jinx PkgConfig::DBUS)
