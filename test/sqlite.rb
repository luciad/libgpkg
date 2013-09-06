# Copyright 2013 Luciad (http://www.luciad.com)
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

require 'ffi'

##
# Minimal SQLite3 binding based on FFI.
# This class has been written to enable development of geopackage unit tests in Ruby.
#
module SQLite3
  LIBRARY = ENV['RUBY_SQLITE'] || 'sqlite3'

  OPEN_READONLY = 0x00000001
  OPEN_READWRITE = 0x00000002
  OPEN_CREATE = 0x00000004
  OPEN_DELETEONCLOSE = 0x00000008
  OPEN_EXCLUSIVE = 0x00000010
  OPEN_AUTOPROXY = 0x00000020
  OPEN_URI = 0x00000040
  OPEN_MEMORY = 0x00000080
  OPEN_MAIN_DB = 0x00000100
  OPEN_TEMP_DB = 0x00000200
  OPEN_TRANSIENT_DB = 0x00000400
  OPEN_MAIN_JOURNAL = 0x00000800
  OPEN_TEMP_JOURNAL = 0x00001000
  OPEN_SUBJOURNAL = 0x00002000
  OPEN_MASTER_JOURNAL = 0x00004000
  OPEN_NOMUTEX = 0x00008000
  OPEN_FULLMUTEX = 0x00010000
  OPEN_SHAREDCACHE = 0x00020000
  OPEN_PRIVATECACHE = 0x00040000
  OPEN_WAL = 0x00080000

  OK = 0
  ERROR = 1
  INTERNAL = 2
  PERM = 3
  ABORT = 4
  BUSY = 5
  LOCKED = 6
  NOMEM = 7
  READONLY = 8
  INTERRUPT = 9
  IOERR = 10
  CORRUPT = 11
  NOTFOUND = 12
  FULL = 13
  CANTOPEN = 14
  PROTOCOL = 15
  EMPTY = 16
  SCHEMA = 17
  TOOBIG = 18
  CONSTRAINT = 19
  MISMATCH = 20
  MISUSE = 21
  NOLFS = 22
  AUTH = 23
  FORMAT = 24
  RANGE = 25
  NOTADB = 26
  ROW = 100
  DONE = 101

  INTEGER = 1
  FLOAT = 2
  TEXT = 3
  BLOB = 4
  NULL = 5

  class Database
    extend FFI::Library
    ffi_lib SQLite3::LIBRARY

    private
    UTF8 = Encoding.find("UTF-8")
    BINARY = Encoding.find("ASCII-8BIT")

    TRANSIENT = FFI::Pointer.new(-1)

    public
    def initialize(name, flags)
      db_ptr = FFI::MemoryPointer.new :pointer
      res = sqlite3_open_v2(name, db_ptr, flags, nil)
      db = db_ptr.get_pointer(0)
      db_ptr.free

      if res != 0
        raise SQLite3Error.new(sqlite3_errstr(res))
      else
        @db = db
      end
    end

    def close
      if @db
        sqlite3_close_v2(@db)
        @db = nil
      end
    end

    def execute(*query)
      query.flatten!
      sql = query.first
      vars = query.slice(1, query.length - 1)

      stmt_ptr = FFI::MemoryPointer.new :pointer
      res = sqlite3_prepare_v2(@db, sql, -1, stmt_ptr, nil)
      stmt = stmt_ptr.get_pointer(0)
      stmt_ptr.free

      if res != SQLite3::OK
        raise SQLite3Error.new(sqlite3_errmsg(@db).strip)
      end

      vars.flatten.each_with_index do |var, i|
        case var
          when Integer
            res = sqlite3_bind_int64(stmt, i + 1, var)
          when Float
            res = sqlite3_bind_double(stmt, i + 1, var)
          when String
            if var.encoding == BINARY
              blob_ptr = FFI::MemoryPointer.new(:char, var.length)
              blob_ptr.put_bytes(0, var)
              res = sqlite3_bind_blob(stmt, i + 1, blob_ptr, var.length, TRANSIENT)
              blob_ptr.free
            else
              res = sqlite3_bind_text(stmt, i + 1, var.encode(UTF8), -1, TRANSIENT)
            end
          else
            raise ArgumentError.new("Unsupported parameter #{var}")
        end

        if res != SQLite3::OK
          raise SQLite3Error.new(sqlite3_errmsg(@db).strip)
        end
      end

      all_rows = []
      while (res = sqlite3_step(stmt)) == SQLite3::ROW
        cols = sqlite3_column_count(stmt)
        row = Array.new(cols) do |i|
          type = sqlite3_column_type(stmt, i)
          case type
            when SQLite3::INTEGER
              sqlite3_column_int64(stmt, i)
            when SQLite3::FLOAT
              sqlite3_column_double(stmt, i)
            when SQLite3::TEXT
              sqlite3_column_text(stmt, i).force_encoding(UTF8)
            when SQLite3::BLOB
              ptr = sqlite3_column_blob(stmt, i)
              length = sqlite3_column_bytes(stmt, i)
              ptr.get_bytes(0, length)
            else
              nil
          end
        end
        if block_given?
          yield row
        else
          all_rows << row
        end
      end

      sqlite3_finalize(stmt)

      if res != SQLite3::DONE
        raise SQLite3Error.new(sqlite3_errmsg(@db).strip)
      end

      if block_given?
        nil
      else
        all_rows
      end
    end

    def load_extension(file, entry_point)
      res = sqlite3_enable_load_extension(@db, 1)
      if res != SQLite3::OK
        raise SQLite3Error.new(sqlite3_errmsg(@db).strip)
      end

      err_ptr = FFI::MemoryPointer.new :pointer
      res = sqlite3_load_extension(@db, file, entry_point, err_ptr)
      err_msg_ptr = err_ptr.get_pointer(0)
      err_ptr.free

      if res != SQLite3::OK
        err_msg = err_msg_ptr.get_string(0)
        sqlite3_free(err_msg_ptr)
        raise SQLite3Error.new(err_msg) if err_msg
      end
    end

    def get_first_row( *query )
      execute( *query ) { |row| return row }
      nil
    end

    def get_first_value( *query )
      execute( *query ) { |row| return row[0] }
      nil
    end

    private
    attach_function :sqlite3_errstr, [:int], :string
    attach_function :sqlite3_errmsg, [:pointer], :string
    attach_function :sqlite3_free, [:pointer], :void

    attach_function :sqlite3_open_v2, [:string, :pointer, :int, :string], :int
    attach_function :sqlite3_close_v2, [:pointer], :int

    attach_function :sqlite3_enable_load_extension, [:pointer, :int], :int
    attach_function :sqlite3_load_extension, [:pointer, :string, :string, :pointer], :int

    attach_function :sqlite3_prepare_v2, [:pointer, :string, :int, :pointer, :pointer], :int
    attach_function :sqlite3_step, [:pointer], :int
    attach_function :sqlite3_reset, [:pointer], :int
    attach_function :sqlite3_finalize, [:pointer], :int

    attach_function :sqlite3_column_count, [:pointer], :int
    attach_function :sqlite3_column_type, [:pointer, :int], :int
    attach_function :sqlite3_column_text, [:pointer, :int], :string
    attach_function :sqlite3_column_int, [:pointer, :int], :int
    attach_function :sqlite3_column_int64, [:pointer, :int], :long_long
    attach_function :sqlite3_column_double, [:pointer, :int], :double
    attach_function :sqlite3_column_bytes, [:pointer, :int], :int
    attach_function :sqlite3_column_blob, [:pointer, :int], :pointer

    attach_function :sqlite3_bind_blob, [:pointer, :int, :pointer, :int, :pointer], :int
    attach_function :sqlite3_bind_double, [:pointer, :int, :double], :int
    attach_function :sqlite3_bind_int, [:pointer, :int, :int], :int
    attach_function :sqlite3_bind_int64, [:pointer, :int, :long_long], :int
    attach_function :sqlite3_bind_null, [:pointer, :int], :int
    attach_function :sqlite3_bind_text, [:pointer, :int, :string, :int, :pointer], :int
  end

  class SQLite3Error < RuntimeError; end
end