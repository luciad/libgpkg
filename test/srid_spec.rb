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

require_relative 'gpkg'

describe 'ST_SRID' do
  it 'should return NULL when passed NULL' do
    execute 'SELECT ST_SRID(NULL)', :expect => nil
  end

  it 'should return the -1 if SRID was not specified' do
    execute "SELECT ST_SRID(GeomFromText('Point(1 0)'))", :expect => -1
  end

  it 'should return the actual SRID value if specified' do
    execute "SELECT ST_SRID(GeomFromText('Point(1 0)', 20))", :expect => 20
  end

  it 'should raise an error on invalid input' do
    execute "SELECT ST_SRID(x'FFFFFFFFFF'))", :expect => :error
  end

  it 'should return an update geometry when called with two parameters' do
      execute "SELECT hex(ST_SRID(GeomFromText('Point(1 0)', 20), 10))", :expect => '475000010A0000000101000000000000000000F03F0000000000000000'
      execute "SELECT ST_SRID(ST_SRID(GeomFromText('Point(1 0)', 20), 10))", :expect => 10
  end
end