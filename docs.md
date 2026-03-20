# TinyLog Reference Documentation

## Macros

### Compile-Time Configuration Macros

> NOTE: If any of these macros are defined/redefined, `TINYLOG_IMPLEMENTATION` must also be defined ONCE before inclusion of the `tinylog.h` header in EXACTLY 1 source file.

#### TINYLOG_MESSAGE_SIZE

`TINYLOG_MESSAGE_SIZE` states the size of each message (in characters).
It can be defined as any valid integer value ONCE before inclusion of the first `tinylog.h` header.

#### NDEBUG

When `NDEBUG` is not defined before the first inclusion of the `tinylog.h` header, the internal buffers in each logger instance will be zero-initialized.

#### TINYLOG_CONFIG_BUILDING

On non-Windows systems, this macro has no effects.

On Windows systems, this macro must be included before the first inclusion of the `tinylog.h` header to take effect.
If it is defined, it generates `__declspec(dllexport)` declarations for exporting into a shared library (`.dll` file).

#### TINYLOG_IMPLEMENTATION

When `TINYLOG_IMPLEMENTATION` is defined before the first inclusion of the `tinylog.h` header, the default implementation is generated with the current configuration.
When it is not defined, all functions must be externally linked into the binary.

### Constant Value Macros

> NOTE: Do not depend on the specific values these macros provide. Rather, rely on the macro itself. Values are possible subjects to change.

#### Error Codes

| Error              | Meaning                   | Used By                                                                             | Notes                                 |
| ------------------ | ------------------------- | ----------------------------------------------------------------------------------- | ------------------------------------- |
| `TINYLOG_ESUCCESS` | No error; Successful call | `tinylog_open`, `tinylog_close`, `tinylog_submit`, `tinylog_write`, `tinylog_flush` | None                                  |
| `TINYLOG_EINVAL`   | Invalid parameter         | `tinylog_open`, `tinylog_close`, `tinylog_submit`, `tinylog_write`, `tinylog_flush` | If this is returned, this is a no-op. |
| `TINYLOG_EALLOC`   | Memory allocation failure | `tinylog_open`                                                                      | None                                  |
| `TINYLOG_EWRITE`   | File writing error        | `tinylog_write`, `tinylog_flush`                                                    | The file may be corrupted             |
| `TINYLOG_EFULL`    | Full waiting queue        | `tinylog_submit`                                                                    | None                                  |
| `TINYLOG_EOPEN`    | Could not open file       | `tinylog_open`                                                                      | None                                  |

#### Log Levels

| Name                    | Use Case               |
| ----------------------- | ---------------------- |
| `TINYLOG_LEVEL_DEBUG`   | Debug messages         |
| `TINYLOG_LEVEL_INFO`    | Informational messages |
| `TINYLOG_LEVEL_WARNING` | Warning messages       |
| `TINYLOG_LEVEL_ERROR`   | Error messages         |

## Types

### struct tinylog_message_s and tinylog_message_t

These 2 names refer to the same type.

Definition:

```c
typedef struct tinylog_message_s {
  char data[TINYLOG_MESSAGE_SIZE];
  int level;
} tinylog_message_t;
```

You may depend on this type's layout.

Set `level` to any of the following:

- `TINYLOG_LEVEL_DEBUG` for debug messages.
- `TINYLOG_LEVEL_INFO` for informational messages.
- `TINYLOG_LEVEL_WARNING` for warning messages.
- `TINYLOG_LEVEL_ERROR` for error messages.
  > Default behavior is provided for any other value.

Set `data` to any null-terminated message that fits within `TINYLOG_MESSAGE_SIZE` characters.

### struct tinylog_logger_s and tinylog_logger_t

These 2 names refer to the same type.

Do not depend on this type's layout; it is subject to change.
You may only use provided functions to configure these.

## Functions

### tinylog_open

Open a logger file.

Declaration:

```c
int tinylog_open(tinylog_logger_t *TINYLOG_INTERNAL_RESTRICT const logger,
                 const char *TINYLOG_INTERNAL_RESTRICT filename,
                 size_t backlog);
```

Parameters:

- `tinylog_logger_t *TINYLOG_INTERNAL_RESTRICT const logger`: The logger to open
- `const char *TINYLOG_INTERNAL_RESTRICT filename`: The file to open
- `size_t backlog`: The amount of space to reserve in the queue

Notes:

1. On failure, no leaks occur.
2. This function opens the file in an append-only mode. It does not clear previous contents.

### tinylog_close

Close a logger file.

Declaration:

```c
int tinylog_close(tinylog_logger_t *TINYLOG_INTERNAL_RESTRICT const logger);
```

Parameters:

- `tinylog_logger_t *TINYLOG_INTERNAL_RESTRICT const logger`: The logger to close

Notes:

1. This does NOT `NULL` out values. Use-After-Free (UAF) errors are possible.
2. This does NOT free the logger if it was on heap. It only frees the internal buffers and closes the internal files.

### tinylog_submit

Submit a new message to the logger.

Declaration:

```c
int tinylog_submit(
    tinylog_logger_t *TINYLOG_INTERNAL_RESTRICT const logger,
    const tinylog_message_t *TINYLOG_INTERNAL_RESTRICT const message);
```

Parameters:

- `tinylog_logger_t *TINYLOG_INTERNAL_RESTRICT const logger`: The logger to submit a new message to.
- `const tinylog_message_t *TINYLOG_INTERNAL_RESTRICT const message`: The message to submit. Contents must be valid.

### tinylog_write

Write pending messages in a logger to its file.

Declaration:

```c
int tinylog_write(tinylog_logger_t *TINYLOG_INTERNAL_RESTRICT const logger);
```

Parameters:

- `tinylog_logger_t *TINYLOG_INTERNAL_RESTRICT const logger`: The logger to write with.

### tinylog_flush

Ensure all messages have been written to the logger's file.

Declaration:

```c
int tinylog_flush(tinylog_logger_t *TINYLOG_INTERNAL_RESTRICT const logger)
```

Parameters:

- `tinylog_logger_t *TINYLOG_INTERNAL_RESTRICT const logger`: The logger to flush.

## Disclaimers

1. Any undocumented items in the `tinylog.h` header are not for public use. Do not depend on them.
2. The library is not thread-safe. Manual synchronisation is needed.

## Compatibility Notes

### Shared/Dynamic Library vs Static Library Usage

If you are building a _shared/dynamic_ library (using `premake5 <system> --shared`), `TINYLOG_CONFIG_SHARED` must be defined. Otherwise, there may be _linker errors_.
