#define TINYLOG_IMPLEMENTATION
#define TINYLOG_MESSAGE_SIZE (32)
#include "tinylog.h"

int main(void) {
  tinylog_logger_t logger;
  int err = 0;

  err = tinylog_open(&logger, "./example.txt", 5);
  if (err != TINYLOG_ESUCCESS) {
    puts("Could not create a logger instance!");
    return 1;
  } else
    puts("Successfully created logger instance!");

  tinylog_message_t msg = {.data = "EXAMPLE INFO",
                           .level = TINYLOG_LEVEL_DEBUG};
  err = tinylog_submit(&logger, &msg);
  if (err != TINYLOG_ESUCCESS) {
    puts("Could not submit new message!");
    tinylog_close(&logger);
    return 1;
  }

  msg.level = TINYLOG_LEVEL_INFO;
  err = tinylog_submit(&logger, &msg);
  if (err != TINYLOG_ESUCCESS) {
    puts("Could not submit new message!");
    tinylog_close(&logger);
    return 1;
  }

  msg.level = TINYLOG_LEVEL_WARNING;
  err = tinylog_submit(&logger, &msg);
  if (err != TINYLOG_ESUCCESS) {
    puts("Could not submit new message!");
    tinylog_close(&logger);
    return 1;
  }

  msg.level = TINYLOG_LEVEL_ERROR;
  err = tinylog_submit(&logger, &msg);
  if (err != TINYLOG_ESUCCESS) {
    puts("Could not submit new message!");
    tinylog_close(&logger);
    return 1;
  }

  err = tinylog_flush(&logger);
  if (err != TINYLOG_ESUCCESS) {
    puts("Could not write logs to file.");
    tinylog_close(&logger);
    return 1;
  }

  tinylog_close(&logger);
  return 0;
}
