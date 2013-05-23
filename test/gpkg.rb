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

module GeoPackageHelpers
  def execute(sql, *vars)
    @db.execute(sql, *vars)
  end

  def result_of(sql, *vars)
    @db.get_first_value(sql, *vars)
  end

  def table_structure_of(table)
    @db.execute("PRAGMA table_info('#{table}')").inject({}) do |hash, row|
      hash[row[1]] = {
          :index => row[0],
          :type => row[2],
          :not_null => (row[3] != 0),
          :default => row[4],
          :primary_key => (row[5] != 0)
      }
      hash
    end
  end
end

##
# Configures RSpec so that each example runs with a new, empty SQLite3 memory database with the geopackage extension
# loaded.
# Some extra helper functions are also added to the RSpec DSL to simplify the examples.
#
RSpec.configure do |c|
  c.include(GeoPackageHelpers)

  c.before(:each) do
    @db = SQLite3::Database.new(':memory:', SQLite3::OPEN_READWRITE | SQLite3::OPEN_CREATE)
    @db.load_extension ENV['GPKG_EXTENSION']
  end

  c.after(:each) do
    @db.close
  end
end