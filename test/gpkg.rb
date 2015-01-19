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

require_relative 'sqlite'

module GeoPackage
  module Pretty
    def format_query(query)
      case query
        when String
          sql = query
        when Array
          query = query.flatten
          sql = query.shift.dup
          query.each do |var|
            case var
              when String
                sql.sub! '?', "'#{var}'"
              else
                sql.sub! '?', var.to_s
            end

          end
        else
          raise ArgumentError.new
      end
      sql
    end
  end

  module Helpers
    def self.mode
      ENV['GPKG_ENTRY_POINT'].match(/^(?:gpkg_)?(.*)/)[1].to_sym
    end

    def self.geos_version
      db = SQLite3::Database.new(':memory:', SQLite3::OPEN_READWRITE | SQLite3::OPEN_CREATE)
      db.load_extension ENV['GPKG_EXTENSION'], "sqlite3_#{ENV['GPKG_ENTRY_POINT']}_init"
      db.get_first_value('SELECT GPKG_GeosVersion()').scan(/\d+/)[0..2].map { |s| s.to_i }
    end

    def mode
      Helpers.mode
    end

    def geos_version
      Helpers.geos_version
    end

    def query(*query)
      query.flatten
    end

    class HaveResult
      include RSpec::Matchers::Pretty
      include GeoPackage::Pretty

      def initialize(db, expected)
        @db = db
        @expected = expected
      end

      def matches?(query)
        @query = query.dup
        begin
          @actual = @db.get_first_value(query)
          @expected == @actual
        rescue SQLite3::SQLite3Error => e
          @error = e.message
          false
        end
      end

      def failure_message_for_should
        if @error
          result = " but raised error '#{@error}'"
        else
          result = " but was #{to_sentence(@actual)}"
        end
        "expected #{format_query(@query)} to have result#{to_sentence(@expected)}#{result}"
      end

      def failure_message_for_should_not
        if @error
          result = " but raised error '#{@actual}'"
        else
          result = ''
        end
        "expected #{format_query(@query)} to not have result#{to_sentence(@expected)}#{result}"
      end

      alias_method :failure_message, :failure_message_for_should
      alias_method :failure_message_when_negated, :failure_message_for_should_not
    end

    def have_result(result)
      HaveResult.new(@db, result)
    end

    class RaiseSQLError
      include RSpec::Matchers::Pretty
      include GeoPackage::Pretty

      def initialize(db)
        @db = db
      end

      def matches?(query)
        @query = query.dup
        begin
          @actual = @db.get_first_value(query)
          false
        rescue SQLite3::SQLite3Error => e
          @error = e.message
          true
        end
      end

      def failure_message_for_should
        "expected #{format_query(@query)} to raise SQL error, but returned #{@actual}"
      end

      def failure_message_for_should_not
        "expected #{format_query(@query)} to not raise SQL error '#{@error}'"
      end

      alias_method :failure_message, :failure_message_for_should
      alias_method :failure_message_when_negated, :failure_message_for_should_not
    end

    def raise_sql_error()
      RaiseSQLError.new(@db)
    end

    class HaveTableSchema
      def initialize(db, schema)
        @db = db
        @expected = schema
      end

      def matches?(target)
        @target = target
        @actual = @db.execute("PRAGMA table_info('#{target}')").inject({}) do |hash, row|
          hash[row[1]] = {
              :index => row[0],
              :type => row[2],
              :not_null => (row[3] != 0),
              :default => row[4],
              :primary_key => (row[5] != 0)
          }
          hash
        end

        @expected == @actual
      end

      def failure_message_for_should
        "expected #{@target} to have table schema\n#{format_schema(@expected)}\nbut was\n#{format_schema(@actual)}"
      end

      def failure_message_for_should_not
        "expected #{@target} to not have table schema\n#{format_schema(@expected)}"
      end

      def format_schema(schema)
        formatted_rows = schema.map do |row|
          "  #{row[0]}: #{row[1]}"
        end
        formatted_rows.join "\n"
      end
    end

    def have_table_schema(schema)
      HaveTableSchema.new(@db, schema)
    end
  end
end

##
# Configures RSpec so that each example runs with a new, empty SQLite3 memory database with the geopackage extension
# loaded.
# Some extra helper functions are also added to the RSpec DSL to simplify the examples.
#
RSpec.configure do |c|
  c.include(GeoPackage::Helpers)
  c.extend(GeoPackage::Helpers)

  c.before(:each) do
    @db = SQLite3::Database.new(':memory:', SQLite3::OPEN_READWRITE | SQLite3::OPEN_CREATE)
    @db.load_extension ENV['GPKG_EXTENSION'], "sqlite3_#{ENV['GPKG_ENTRY_POINT']}_init"
  end

  c.after(:each) do
    @db.close
  end
end

RSpec::Matchers.define :have_schema do |schema|
  match do |table|
    player.in_zone?(zone)
  end
end