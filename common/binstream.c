/*
 * Copyright 2013 Luciad (http://www.luciad.com)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <string.h>
#include "binstream.h"
#include "sqlite.h"

int binstream_init(binstream_t *stream, uint8_t *data, size_t length) {
    stream->data = data;
    stream->capacity = length;
    stream->limit = length;
    stream->position = 0;
    stream->end = LITTLE;
    stream->fixed_size = 1;
    return SQLITE_OK;
}

int binstream_init_growable(binstream_t *stream, size_t initial_cap) {
    void *data = sqlite3_malloc(initial_cap);
    if (data == NULL) {
        return SQLITE_NOMEM;
    }

    stream->data = data;
    stream->limit = 0;
    stream->capacity = initial_cap;
    stream->position = 0;
    stream->end = LITTLE;
    stream->fixed_size = 0;
    return SQLITE_OK;
}

void binstream_destroy(binstream_t *stream) {
    if (stream == NULL) {
      return;
    }

    if (stream->fixed_size == 0) {
        sqlite3_free(stream->data);
    }
}

static int binstream_ensureavailable(binstream_t *stream, size_t needed) {
    if (needed <= stream->limit) {
        return SQLITE_OK;
    } else {
        return SQLITE_IOERR;
    }
}

static int binstream_ensurecapacity(binstream_t *stream, size_t needed) {
    if (needed <= stream->capacity) {
        return SQLITE_OK;
    } else if (stream->fixed_size) {
        return SQLITE_IOERR;
    } else {
        size_t newcapacity = stream->capacity * 3 / 2;
        if (needed > newcapacity) {
            newcapacity = needed;
        }
        void *newdata = sqlite3_realloc(stream->data, newcapacity);
        if (newdata == NULL) {
            return SQLITE_NOMEM;
        }
        stream->data = newdata;
        stream->capacity = newcapacity;
        return SQLITE_OK;
    }
}

size_t binstream_position(binstream_t *stream) {
    return stream->position;
}

int binstream_seek(binstream_t *stream, size_t position) {
    int result = binstream_ensurecapacity(stream, position);
    if (result != SQLITE_OK) {
        return result;
    }
    stream->position = position;
    if (stream->position > stream->limit) {
        stream->limit = stream->position;
    }
    return SQLITE_OK;
}

int binstream_relseek(binstream_t *stream, int32_t amount) {
    size_t position = stream->position;
    if (amount < 0 && -amount > position) {
        return SQLITE_IOERR;
    }
    return binstream_seek(stream, position + amount);
}

uint8_t *binstream_data(binstream_t *stream) {
    return stream->data + stream->position;
}

size_t binstream_available(binstream_t *stream) {
    return stream->limit - stream->position;
}

void binstream_set_endianness(binstream_t *stream, binstream_endianness e) {
    stream->end = e;
}

binstream_endianness binstream_get_endianness(binstream_t *stream) {
    return stream->end;
}

int binstream_read_u8(binstream_t *stream, uint8_t *out) {
    int result = binstream_ensureavailable(stream, stream->position + 1);
    if (result != SQLITE_OK) {
        return result;
    }

    *out = stream->data[stream->position++];
    return SQLITE_OK;
}

int binstream_write_u8(binstream_t *stream, uint8_t val) {
    int result = binstream_ensurecapacity(stream, stream->position + 1);
    if (result != SQLITE_OK) {
        return result;
    }

    stream->data[stream->position++] = val;
    if (stream->position > stream->limit) {
        stream->limit = stream->position;
    }
    return SQLITE_OK;
}

int binstream_nread_u8(binstream_t *stream, uint8_t *out, size_t count) {
    int result = binstream_ensureavailable(stream, stream->position + count);
    if (result != SQLITE_OK) {
        return result;
    }

    memmove(out, stream->data + stream->position, count);
    stream->position += count;
    return SQLITE_OK;
}

int binstream_write_nu8(binstream_t *stream, uint8_t *val, size_t count) {
    int result = binstream_ensurecapacity(stream, stream->position + count);
    if (result != SQLITE_OK) {
        return result;
    }

    memmove(stream->data + stream->position, val, count);
    stream->position += count;
    if (stream->position > stream->limit) {
        stream->limit = stream->position;
    }
    return SQLITE_OK;
}

int binstream_read_u32(binstream_t *stream, uint32_t *out) {
    int result = binstream_ensureavailable(stream, stream->position + 4);
    if (result != SQLITE_OK) {
        return result;
    }

    uint32_t v1 = stream->data[stream->position++];
    uint32_t v2 = stream->data[stream->position++];
    uint32_t v3 = stream->data[stream->position++];
    uint32_t v4 = stream->data[stream->position++];
    if (stream->end == LITTLE) {
        *out = (v1 << 0) | (v2 << 8) | (v3 << 16) | (v4 << 24);
    } else {
        *out = (v4 << 0) | (v3 << 8) | (v2 << 16) | (v1 << 24);
    }
    return SQLITE_OK;
}

int binstream_write_u32(binstream_t *stream, uint32_t val) {
    int result = binstream_ensurecapacity(stream, stream->position + 4);
    if (result != SQLITE_OK) {
        return result;
    }

    uint8_t v1 = (uint8_t) ((val >> 0) & 0xFF);
    uint8_t v2 = (uint8_t) ((val >> 8) & 0xFF);
    uint8_t v3 = (uint8_t) ((val >> 16) & 0xFF);
    uint8_t v4 = (uint8_t) ((val >> 24) & 0xFF);

    if (stream->end == LITTLE) {
        stream->data[stream->position++] = v1;
        stream->data[stream->position++] = v2;
        stream->data[stream->position++] = v3;
        stream->data[stream->position++] = v4;
    } else {
        stream->data[stream->position++] = v4;
        stream->data[stream->position++] = v3;
        stream->data[stream->position++] = v2;
        stream->data[stream->position++] = v1;
    }
    if (stream->position > stream->limit) {
        stream->limit = stream->position;
    }
    return SQLITE_OK;
}

int binstream_read_u64(binstream_t *stream, uint64_t *out) {
    int result = binstream_ensureavailable(stream, stream->position + 8);
    if (result != SQLITE_OK) {
        return result;
    }

    uint64_t v1 = stream->data[stream->position++];
    uint64_t v2 = stream->data[stream->position++];
    uint64_t v3 = stream->data[stream->position++];
    uint64_t v4 = stream->data[stream->position++];
    uint64_t v5 = stream->data[stream->position++];
    uint64_t v6 = stream->data[stream->position++];
    uint64_t v7 = stream->data[stream->position++];
    uint64_t v8 = stream->data[stream->position++];
    if (stream->end == LITTLE) {
        *out = (v1 << 0) | (v2 << 8) | (v3 << 16) | (v4 << 24) | (v5 << 32) | (v6 << 40) | (v7 << 48) | (v8 << 56);
    } else {
        *out = (v8 << 0) | (v7 << 8) | (v6 << 16) | (v5 << 24) | (v4 << 32) | (v3 << 40) | (v2 << 48) | (v1 << 56);
    }
    return SQLITE_OK;
}

static void binstream_write_u64_unchecked(binstream_t *stream, uint64_t val) {
    uint8_t v1 = (uint8_t) ((val >> 0) & 0xFF);
    uint8_t v2 = (uint8_t) ((val >> 8) & 0xFF);
    uint8_t v3 = (uint8_t) ((val >> 16) & 0xFF);
    uint8_t v4 = (uint8_t) ((val >> 24) & 0xFF);
    uint8_t v5 = (uint8_t) ((val >> 32) & 0xFF);
    uint8_t v6 = (uint8_t) ((val >> 40) & 0xFF);
    uint8_t v7 = (uint8_t) ((val >> 48) & 0xFF);
    uint8_t v8 = (uint8_t) ((val >> 56) & 0xFF);
    if (stream->end == LITTLE) {
        stream->data[stream->position++] = v1;
        stream->data[stream->position++] = v2;
        stream->data[stream->position++] = v3;
        stream->data[stream->position++] = v4;
        stream->data[stream->position++] = v5;
        stream->data[stream->position++] = v6;
        stream->data[stream->position++] = v7;
        stream->data[stream->position++] = v8;
    } else {
        stream->data[stream->position++] = v8;
        stream->data[stream->position++] = v7;
        stream->data[stream->position++] = v6;
        stream->data[stream->position++] = v5;
        stream->data[stream->position++] = v4;
        stream->data[stream->position++] = v3;
        stream->data[stream->position++] = v2;
        stream->data[stream->position++] = v1;
    }
}

int binstream_write_u64(binstream_t *stream, uint64_t val) {
    int result = binstream_ensurecapacity(stream, stream->position + 8);
    if (result != SQLITE_OK) {
        return result;
    }

    binstream_write_u64_unchecked(stream, val);
    if (stream->position > stream->limit) {
        stream->limit = stream->position;
    }
    return SQLITE_OK;
}

int binstream_read_double(binstream_t *stream, double *out) {
    union {
        uint64_t L;
        double D;
    } T;
    int result = binstream_read_u64(stream, &T.L);
    if (result != SQLITE_OK) {
        return result;
    }

    *out = T.D;
    return SQLITE_OK;
}

int binstream_write_double(binstream_t *stream, double val) {
    return binstream_write_u64(stream, *((uint64_t *) &val));
}

int binstream_write_ndouble(binstream_t *stream, double *val, size_t count) {
    int result = binstream_ensurecapacity(stream, stream->position + sizeof(double) * count);
    if (result != SQLITE_OK) {
        return result;
    }
    for (int i = 0; i < count; i++) {
        binstream_write_u64_unchecked(stream, *((uint64_t *) &val[i]));
    }
    if (stream->position > stream->limit) {
        stream->limit = stream->position;
    }
    return SQLITE_OK;
}
