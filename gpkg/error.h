#ifndef ERROR_H
#define ERROR_H

#include "strbuf.h"

/**
 * @addtogroup error Error handling
 * @{
 */

/**
 * Container for error messages
 */
typedef struct {
  /** @private */
  strbuf_t message;
  /** @private */
  size_t error_count;
} errorstream_t;

/**
 * Initializes an error buffer.
 * @param error the buffer to initialize
 * @return SQLITE_OK on success, an error code otherwise
 */
int error_init(errorstream_t *error);

/**
 * Initializes a fixed size error buffer.
 * @param error the buffer to initialize
 * @param buffer the data buffer to use
 * @param length the length of buffer in bytes
 * @return SQLITE_OK on success, an error code otherwise
 */
int error_init_fixed(errorstream_t *error, char *buffer, size_t length);

/**
 * Resets an error to the state it had after calling error_init or error_init_fixed.
 * This function erases the content of the message buffer. It does not create a new buffer.
 * @param error the error buffer to reset
 * @return SQLITE_OK on success, an error code otherwise
 */
int error_reset(errorstream_t *error);

/**
 * Destroys an error buffer, freeing any internal data structures that have been allocated. If the error
 * buffer was initialized using error_init_fixed() the char buffer passed to error_init_fixed() will not be freed.
 *
 * @param error the error buffer to destroy
 */
void error_destroy(errorstream_t *error);

/**
 * Appends a formatted error message to this error buffer and increases the error count by one.
 *
 * @param error an error buffer
 * @param msg the error message
 *
 * @return SQLITE_OK on success, an error code otherwise
 */
int error_append(errorstream_t *error, const char *msg, ...);

/**
 * Returns the number of errors that have been accumulated in this error buffer.
 *
 * @param error an error buffer
 *
 * @return the number of errors
 */
size_t error_count(errorstream_t *error);

/**
 * Returns the accumulated error messages.
 *
 * @param error an error buffer
 *
 * @return the accumulated error messages
 */
char *error_message(errorstream_t *error);

/** @} */

#endif
