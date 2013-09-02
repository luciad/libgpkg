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

describe 'ST_IsEmpty' do
  it 'should return NULL when passed NULL' do
    execute 'SELECT ST_IsEmpty(NULL)', :expect => nil
  end

  it 'should return the 0 for non-empty geometry' do
    execute "SELECT ST_IsEmpty(GeomFromText('Point(1 0)'))", :expect => 0
    execute "SELECT ST_IsEmpty(GeomFromText('LineString(1 1, 2 2)'))", :expect => 0
    execute "SELECT ST_IsEmpty(GeomFromText('Polygon((1 1, 2 2, 3 3))'))", :expect => 0
    execute "SELECT ST_IsEmpty(GeomFromText('Polygon((1 1, 2 2, 3 3), EMPTY)'))", :expect => 0
    execute "SELECT ST_IsEmpty(GeomFromText('GeometryCollection(Point(1 0), Point EMPTY)'))", :expect => 0
  end

  it 'should return the 1 for empty geometry' do
    execute "SELECT ST_IsEmpty(GeomFromText('Point empty'))", :expect => 1
    execute "SELECT ST_IsEmpty(GeomFromText('LineString empty'))", :expect => 1
    execute "SELECT ST_IsEmpty(GeomFromText('Polygon empty'))", :expect => 1
    execute "SELECT ST_IsEmpty(GeomFromText('MultiPoint empty'))", :expect => 1
    execute "SELECT ST_IsEmpty(GeomFromText('MultiLineString empty'))", :expect => 1
    execute "SELECT ST_IsEmpty(GeomFromText('MultiPolygon empty'))", :expect => 1
    execute "SELECT ST_IsEmpty(GeomFromText('GeometryCollection empty'))", :expect => 1
  end

  it 'should raise an error on invalid input' do
    execute "SELECT ST_IsEmpty(x'FFFFFFFFFF'))", :expect => :error
  end
end