#ifndef TINYLOG_TINYLOG_H
#define TINYLOG_TINYLOG_H
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <stddef.h>
#include <stdio.h>

#ifdef _MSC_VER
#define TINYLOG_INTERNAL_RESTRICT __restrict
#elif defined(__cplusplus)
#define TINYLOG_INTERNAL_RESTRICT
#else
#define TINYLOG_INTERNAL_RESTRICT restrict
#endif // _MSC_VER || __cplusplus

#ifdef _WIN32
#ifdef TINYLOG_CONFIG_SHARED
#ifdef TINYLOG_CONFIG_BUILDING
#define TINYLOG_PUBLIC_API __declspec(dllexport)
#else
#define TINYLOG_PUBLIC_API __declspec(dllimport)
#endif // TINYLOG_CONFIG_BUILDING
#else
#define TINYLOG_PUBLIC_API // Not needed on Windows static builds (.lib)
#endif                     // TINYLOG_CONFIG_SHARED
#else
#define TINYLOG_PUBLIC_API // Not needed outside Windows
#endif                     // _WIN32

#ifndef TINYLOG_MESSAGE_SIZE
#define TINYLOG_MESSAGE_SIZE (128)
#endif // TINYLOG_MESSAGE_SIZE

typedef struct tinylog_message_s {
  char data[TINYLOG_MESSAGE_SIZE];
  int level;
} tinylog_message_t;

typedef struct tinylog_logger_s {
  tinylog_message_t *messages;
  FILE *log_file;
  char *internal_fmtbuf;
  size_t backlog;
  size_t write_idx;
  size_t count;
} tinylog_logger_t;

#define TINYLOG_LEVEL_DEBUG ((int)0)
#define TINYLOG_LEVEL_INFO ((int)1)
#define TINYLOG_LEVEL_WARNING ((int)2)
#define TINYLOG_LEVEL_ERROR ((int)3)

#define TINYLOG_ESUCCESS ((int)0)
#define TINYLOG_EINVAL ((int)1)
#define TINYLOG_EALLOC ((int)2)
#define TINYLOG_EWRITE ((int)3)
#define TINYLOG_EFULL ((int)4)
#define TINYLOG_EOPEN ((int)5)

int tinylog_open(tinylog_logger_t *TINYLOG_INTERNAL_RESTRICT const logger,
                 const char *TINYLOG_INTERNAL_RESTRICT filename,
                 size_t backlog);

int tinylog_close(tinylog_logger_t *TINYLOG_INTERNAL_RESTRICT const logger);

int tinylog_submit(
    tinylog_logger_t *TINYLOG_INTERNAL_RESTRICT const logger,
    const tinylog_message_t *TINYLOG_INTERNAL_RESTRICT const message);

int tinylog_write(tinylog_logger_t *TINYLOG_INTERNAL_RESTRICT const logger);

int tinylog_flush(tinylog_logger_t *TINYLOG_INTERNAL_RESTRICT const logger);

#ifdef TINYLOG_IMPLEMENTATION
#include <stdlib.h>
#include <string.h>

#define TINYLOG_INTERNAL_PREFIX_LEN (11)

static inline const char *tinylog_internal_level_stringify(int log_level) {
  switch (log_level) {
  default:
    return "[UNKNOWN] ";
  case TINYLOG_LEVEL_DEBUG:
    return "[DEBUG]   ";
  case TINYLOG_LEVEL_INFO:
    return "[INFO]    ";
  case TINYLOG_LEVEL_WARNING:
    return "[WARNING] ";
  case TINYLOG_LEVEL_ERROR:
    return "[ERROR]   ";
  }
}

static inline void tinylog_internal_format_message(
    char *TINYLOG_INTERNAL_RESTRICT out,
    const tinylog_message_t *TINYLOG_INTERNAL_RESTRICT const src,
    size_t *TINYLOG_INTERNAL_RESTRICT const lenout) {
  if (!out || !src || !lenout)
    return;
  memcpy(out, tinylog_internal_level_stringify(src->level),
         TINYLOG_INTERNAL_PREFIX_LEN - 1);
  size_t msg_len = strlen(src->data);
  memcpy(out + (TINYLOG_INTERNAL_PREFIX_LEN - 1), src->data, msg_len);
  out[(TINYLOG_INTERNAL_PREFIX_LEN - 1) + msg_len] = '\n';
  out[(TINYLOG_INTERNAL_PREFIX_LEN - 1) + msg_len + 1] = '\0';
  *lenout = (TINYLOG_INTERNAL_PREFIX_LEN - 1) + msg_len + 1;
}

int tinylog_open(tinylog_logger_t *TINYLOG_INTERNAL_RESTRICT const logger,
                 const char *TINYLOG_INTERNAL_RESTRICT filename,
                 size_t backlog) {
  if (!logger || !filename || !backlog)
    return TINYLOG_EINVAL;

  logger->internal_fmtbuf =
      malloc((TINYLOG_INTERNAL_PREFIX_LEN - 1) + TINYLOG_MESSAGE_SIZE + 2);
  if (!(logger->internal_fmtbuf))
    return TINYLOG_EALLOC;

  logger->messages = malloc(sizeof(tinylog_message_t) * backlog);
  if (!(logger->messages)) {
    free(logger->internal_fmtbuf);
    return TINYLOG_EALLOC;
  }

  logger->log_file = fopen(filename, "ab");
  if (!(logger->log_file)) {
    free(logger->internal_fmtbuf);
    free(logger->messages);
    return TINYLOG_EOPEN;
  }

  logger->write_idx = 0;
  logger->count = 0;
  logger->backlog = backlog;

#ifndef NDEBUG
  memset(logger->messages, 0, sizeof(tinylog_message_t) * backlog);
  memset(logger->internal_fmtbuf, 0,
         (TINYLOG_INTERNAL_PREFIX_LEN - 1) + TINYLOG_MESSAGE_SIZE + 2);
#endif // NDEBUG

  return TINYLOG_ESUCCESS;
}

int tinylog_close(tinylog_logger_t *TINYLOG_INTERNAL_RESTRICT const logger) {
  if (!logger)
    return TINYLOG_EINVAL;
  free(logger->internal_fmtbuf);
  free(logger->messages);
  if (logger->log_file)
    fclose(logger->log_file);
  return TINYLOG_ESUCCESS;
}

int tinylog_submit(
    tinylog_logger_t *TINYLOG_INTERNAL_RESTRICT const logger,
    const tinylog_message_t *TINYLOG_INTERNAL_RESTRICT const message) {
  if (!logger || !message)
    return TINYLOG_EINVAL;

  if (logger->count == logger->backlog)
    return TINYLOG_EFULL;

  size_t idx = (logger->write_idx + logger->count) % logger->backlog;
  logger->messages[idx].level = message->level;
  memcpy(logger->messages[idx].data, message->data, TINYLOG_MESSAGE_SIZE - 1);
  logger->messages[idx].data[TINYLOG_MESSAGE_SIZE - 1] = '\0';

  logger->count++;

  return TINYLOG_ESUCCESS;
}

int tinylog_write(tinylog_logger_t *TINYLOG_INTERNAL_RESTRICT const logger) {
  if (!logger)
    return TINYLOG_EINVAL;

  for (size_t i = 0; i < logger->count; i++) {
    size_t len = 0;
    size_t idx = (logger->write_idx + i) % logger->backlog;
    tinylog_internal_format_message(logger->internal_fmtbuf,
                                    &(logger->messages[idx]), &len);
    size_t written = fwrite(logger->internal_fmtbuf, 1, len, logger->log_file);
    while (written < len) {
      if (ferror(logger->log_file))
        return TINYLOG_EWRITE;
      written += fwrite(logger->internal_fmtbuf + written, 1, len - written,
                        logger->log_file);
    }
  }

  logger->write_idx = (logger->write_idx + logger->count) % logger->backlog;
  logger->count = 0;

  return TINYLOG_ESUCCESS;
}

int tinylog_flush(tinylog_logger_t *TINYLOG_INTERNAL_RESTRICT const logger) {
  if (!logger)
    return TINYLOG_EINVAL;

  int write_pending = tinylog_write(logger);
  if (write_pending != TINYLOG_ESUCCESS)
    return write_pending;

  fflush(logger->log_file);

  return TINYLOG_ESUCCESS;
}
#endif // TINYLOG_IMPLEMENTATION

#ifdef __cplusplus
}
#endif // __cplusplus
#endif // TINYLOG_TINYLOG_H
