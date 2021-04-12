#pragma once

#define DB_EXPORT __attribute__((__visibility__("default")))

#include <gflags/gflags.h>

#define DB_GFLAGS_DEF_WRAPPER(type, real_type, name, default_value, help_str) \
  DEFINE_##type(name, default_value, help_str);                         \
  namespace db {                                                      \
  DB_EXPORT real_type& FLAGS_##name = ::FLAGS_##name;                 \
  }

#define DB_DEFINE_int(name, default_value, help_str)                  \
  DB_GFLAGS_DEF_WRAPPER(int32, gflags::int32, name, default_value, help_str)
#define DB_DEFINE_int64(name, default_value, help_str)                 \
  DB_GFLAGS_DEF_WRAPPER(int64, gflags::int64, name, default_value, help_str)
#define DB_DEFINE_double(name, default_value, help_str)                \
  DB_GFLAGS_DEF_WRAPPER(double, double, name, default_value, help_str)
#define DB_DEFINE_bool(name, default_value, help_str)              \
  DB_GFLAGS_DEF_WRAPPER(bool, bool, name, default_value, help_str)
#define DB_DEFINE_string(name, default_value, help_str)                \
  DB_GFLAGS_DEF_WRAPPER(string, std::string, name, default_value, help_str)

#define DB_GFLAGS_DECLARE_WRAPPER(type, real_type, name)  \
  DECLARE_##type(name);                                     \
  namespace db {                                          \
  extern real_type& FLAGS_##name;                           \
  } // namespace drtm

#define DB_DECLARE_int(name)                               \
  DB_GFLAGS_DECLARE_WRAPPER(int32, gflags::int32, name)
#define DB_DECLARE_int64(name)                             \
  DB_GFLAGS_DECLARE_WRAPPER(int64, gflags::int64, name)
#define DB_DECLARE_double(name)                    \
  DB_GFLAGS_DECLARE_WRAPPER(double, double, name)
#define DB_DECLARE_bool(name) DB_GFLAGS_DECLARE_WRAPPER(bool, bool, name)
#define DB_DECLARE_string(name)                            \
  DB_GFLAGS_DECLARE_WRAPPER(string, std::string, name)
 