#ifndef DFTRACER_COMMON_DFTRACER_LOGGING_H
#define DFTRACER_COMMON_DFTRACER_LOGGING_H

#ifdef DFTRACER_DEBUG
#include <dftracer/dftracer_config_dbg.hpp>
#else
#include <dftracer/dftracer_config.hpp>
#endif

/* External Headers */
#include <sys/types.h>
#include <unistd.h>

#include <chrono>
#include <string>

#define VA_ARGS(...) , ##__VA_ARGS__

inline std::string dftracer_macro_get_time() {
  auto dftracer_ts_millis =
      std::chrono::duration_cast<std::chrono::milliseconds>(
          std::chrono::system_clock::now().time_since_epoch())
          .count() %
      1000;
  auto dftracer_ts_t = std::time(0);
  auto now = std::localtime(&dftracer_ts_t);
  char dftracer_ts_time_str[256];
  sprintf(dftracer_ts_time_str, "%04d-%02d-%02d %02d:%02d:%02d.%ld",
          now->tm_year + 1900, now->tm_mon + 1, now->tm_mday, now->tm_hour,
          now->tm_min, now->tm_sec, dftracer_ts_millis);
  return dftracer_ts_time_str;
}

// #define DFTRACER_NOOP_MACRO do {} while (0)
//=============================================================================

#if defined(DFTRACER_LOGGER_CPP_LOGGER)  // CPP_LOGGER
// ---------------------------
#include <cpp-logger/clogger.h>

#define DFTRACER_LOG_STDOUT_REDIRECT(fpath) freopen((fpath), "a+", stdout);
#define DFTRACER_LOG_STDERR_REDIRECT(fpath) freopen((fpath), "a+", stderr);
#define DFTRACER_LOGGER_NAME "DFTRACER"

#define DFTRACER_INTERNAL_TRACE(file, line, function, name, logger_level) \
  cpp_logger_clog(logger_level, name, "[%s] %s [%s:%d]",                  \
                  dftracer_macro_get_time().c_str(), function, file, line);

#define DFTRACER_INTERNAL_TRACE_FORMAT(file, line, function, name,            \
                                       logger_level, format, ...)             \
  cpp_logger_clog(logger_level, name, "[%s] %s " format " [%s:%d]",           \
                  dftracer_macro_get_time().c_str(), function, ##__VA_ARGS__, \
                  file, line);

#define DFTRACER_LOG_PRINT(format, ...)                                  \
  DFTRACER_INTERNAL_TRACE_FORMAT(__FILE__, __LINE__, __FUNCTION__,       \
                                 DFTRACER_LOGGER_NAME, CPP_LOGGER_PRINT, \
                                 format, __VA_ARGS__);
#ifdef DFTRACER_LOGGER_LEVEL_TRACE
#define DFTRACER_LOGGER_INIT() \
  cpp_logger_clog_level(CPP_LOGGER_TRACE, DFTRACER_LOGGER_NAME);
#elif defined(DFTRACER_LOGGER_LEVEL_DEBUG)
#define DFTRACER_LOGGER_INIT() \
  cpp_logger_clog_level(CPP_LOGGER_DEBUG, DFTRACER_LOGGER_NAME);
#elif defined(DFTRACER_LOGGER_LEVEL_INFO)
#define DFTRACER_LOGGER_INIT() \
  cpp_logger_clog_level(CPP_LOGGER_INFO, DFTRACER_LOGGER_NAME);
#elif defined(DFTRACER_LOGGER_LEVEL_WARN)
#define DFTRACER_LOGGER_INIT() \
  cpp_logger_clog_level(CPP_LOGGER_WARN, DFTRACER_LOGGER_NAME);
#else
#define DFTRACER_LOGGER_INIT() \
  cpp_logger_clog_level(CPP_LOGGER_ERROR, DFTRACER_LOGGER_NAME);
#endif

#define DFTRACER_LOGGER_LEVEL(level) \
  cpp_logger_clog_level(level, DFTRACER_LOGGER_NAME);
#ifdef DFTRACER_LOGGER_LEVEL_TRACE
#define DFTRACER_LOG_TRACE()                                \
  DFTRACER_INTERNAL_TRACE(__FILE__, __LINE__, __FUNCTION__, \
                          DFTRACER_LOGGER_NAME, CPP_LOGGER_TRACE);
#define DFTRACER_LOG_TRACE_FORMAT(format, ...)                           \
  DFTRACER_INTERNAL_TRACE_FORMAT(__FILE__, __LINE__, __FUNCTION__,       \
                                 DFTRACER_LOGGER_NAME, CPP_LOGGER_TRACE, \
                                 format, __VA_ARGS__);
#else
#define DFTRACER_LOG_TRACE(...) DFTRACER_NOOP_MACRO
#define DFTRACER_LOG_TRACE_FORMAT(...) DFTRACER_NOOP_MACRO
#endif

#ifdef DFTRACER_LOGGER_LEVEL_DEBUG
#define DFTRACER_LOG_DEBUG(format, ...)                                  \
  DFTRACER_INTERNAL_TRACE_FORMAT(__FILE__, __LINE__, __FUNCTION__,       \
                                 DFTRACER_LOGGER_NAME, CPP_LOGGER_DEBUG, \
                                 format, __VA_ARGS__);
#else
#define DFTRACER_LOG_DEBUG(format, ...) DFTRACER_NOOP_MACRO
#endif

#ifdef DFTRACER_LOGGER_LEVEL_INFO
#define DFTRACER_LOG_INFO(format, ...)                                  \
  DFTRACER_INTERNAL_TRACE_FORMAT(__FILE__, __LINE__, __FUNCTION__,      \
                                 DFTRACER_LOGGER_NAME, CPP_LOGGER_INFO, \
                                 format, ##__VA_ARGS__);
#else
#define DFTRACER_LOG_INFO(format, ...) DFTRACER_NOOP_MACRO
#endif

#ifdef DFTRACER_LOGGER_LEVEL_WARN
#define DFTRACER_LOG_WARN(format, ...)                                  \
  DFTRACER_INTERNAL_TRACE_FORMAT(__FILE__, __LINE__, __FUNCTION__,      \
                                 DFTRACER_LOGGER_NAME, CPP_LOGGER_WARN, \
                                 format, __VA_ARGS__);
#else
#define DFTRACER_LOG_WARN(format, ...) DFTRACER_NOOP_MACRO
#endif

#ifdef DFTRACER_LOGGER_LEVEL_ERROR
#define DFTRACER_LOG_ERROR(format, ...)                                  \
  DFTRACER_INTERNAL_TRACE_FORMAT(__FILE__, __LINE__, __FUNCTION__,       \
                                 DFTRACER_LOGGER_NAME, CPP_LOGGER_ERROR, \
                                 format, __VA_ARGS__);
#else
#define DFTRACER_LOG_ERROR(format, ...) DFTRACER_NOOP_MACRO
#endif
#else
#define DFTRACER_LOGGER_INIT() DFTRACER_NOOP_MACRO
#define DFTRACER_LOGGER_LEVEL(level) DFTRACER_NOOP_MACRO
#define DFTRACER_LOG_PRINT(format, ...) fprintf(stdout, format, __VA_ARGS__);
#define DFTRACER_LOG_ERROR(format, ...) fprintf(stderr, format, __VA_ARGS__);
#define DFTRACER_LOG_WARN(format, ...) DFTRACER_NOOP_MACRO
#define DFTRACER_LOG_INFO(format, ...) DFTRACER_NOOP_MACRO
#define DFTRACER_LOG_DEBUG(format, ...) DFTRACER_NOOP_MACRO
#define DFTRACER_LOG_TRACE() DFTRACER_NOOP_MACRO
#define DFTRACER_LOG_TRACE_FORMAT(...) DFTRACER_NOOP_MACRO
#define DFTRACER_LOG_STDOUT_REDIRECT(fpath) DFTRACER_NOOP_MACRO
#define DFTRACER_LOG_STDERR_REDIRECT(fpath) DFTRACER_NOOP_MACRO
#endif  // DFTRACER_LOGGER_CPP_LOGGER
        // -----------------------------------------------
//=============================================================================

#endif /* DFTRACER_COMMON_DFTRACER_LOGGING_H */
