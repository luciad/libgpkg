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

RSpec.configure do |c|
  c.before(:each) do
    @db = SQLite3::Database.new(':memory:', SQLite3::OPEN_READWRITE | SQLite3::OPEN_CREATE)
    @db.load_extension ENV['GPKG_EXTENSION']
  end

  c.after(:each) do
    @db.close
  end
end