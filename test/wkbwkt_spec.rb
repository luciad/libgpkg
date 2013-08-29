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

describe 'WKBFromText' do
  AS_WKB = 'SELECT lower(hex(WKBFromText(?)))'
  
  it 'should parse XY points correctly' do
    execute AS_WKB, 'Point(1 2)', :expect => '0101000000000000000000f03f0000000000000040'
  end

  it 'should be case insensitive' do
    execute AS_WKB, 'pOiNt(1 2)', :expect => '0101000000000000000000f03f0000000000000040'
  end

  it 'should parse XYZ points correctly' do
    execute AS_WKB, 'Point Z(1 2 3)', :expect => '01e9030000000000000000f03f00000000000000400000000000000840'
  end

  it 'should parse XYM points correctly' do
    execute AS_WKB, 'Point M(1 2 3)', :expect => '01d1070000000000000000f03f00000000000000400000000000000840'
  end

  it 'should parse XYZM points correctly' do
    execute AS_WKB, 'Point ZM(1 2 3 4)', :expect => '01b90b0000000000000000f03f000000000000004000000000000008400000000000001040'
  end

  it 'should parse XY line strings correctly' do
    execute AS_WKB, 'LineString(1 2, 3 4)', :expect => '010200000002000000000000000000f03f000000000000004000000000000008400000000000001040'
  end

  it 'should parse XYZ line strings correctly' do
    execute AS_WKB, 'LineString Z(1 2 3, 4 5 6)', :expect => '01ea03000002000000000000000000f03f00000000000000400000000000000840000000000000104000000000000014400000000000001840'
  end

  it 'should parse XYM line strings correctly' do
    execute AS_WKB, 'LineString M(1 2 3, 4 5 6)', :expect => '01d207000002000000000000000000f03f00000000000000400000000000000840000000000000104000000000000014400000000000001840'
  end

  it 'should parse XYZM line strings correctly' do
    execute AS_WKB, 'LineString ZM(1 2 3 4, 5 6 7 8)', :expect => '01ba0b000002000000000000000000f03f000000000000004000000000000008400000000000001040000000000000144000000000000018400000000000001c400000000000002040'
  end

  it 'should raise error on unclosed WKT' do
    execute AS_WKB, 'LineString ZM(1 ', :expect => :error
  end

  it 'should raise error on unknown geometry type' do
    execute AS_WKB, 'MyGeometry EMPTY', :expect => :error
  end

  it 'should raise error on incorrect modifiers' do
    execute AS_WKB, 'Point X (1 2)', :expect => :error
  end

  it 'should raise error on incorrect empty set' do
    execute AS_WKB, 'Point empt', :expect => :error
  end
end

describe 'AsText' do
  AS_TEXT = 'SELECT AsText(GeomFromText(?))'

  it 'should format XY points correctly' do
    execute AS_TEXT, 'Point(1 2)', :expect => 'Point (1 2)'
  end

  it 'should format XYZ points correctly' do
    execute AS_TEXT, 'Point Z(1 2 3)', :expect => 'Point Z (1 2 3)'
  end

  it 'should format XYM points correctly' do
    execute AS_TEXT, 'Point M(1 2 3)', :expect => 'Point M (1 2 3)'
  end

  it 'should format XYZM points correctly' do
    execute AS_TEXT, 'Point ZM(1 2 3 4)', :expect => 'Point ZM (1 2 3 4)'
  end

  it 'should format XY line strings correctly' do
    execute AS_TEXT, 'LineString(1 2, 3 4)', :expect => 'LineString (1 2, 3 4)'
  end

  it 'should format XYZ line strings correctly' do
    execute AS_TEXT, 'LineString Z(1 2 3, 4 5 6)', :expect => 'LineString Z (1 2 3, 4 5 6)'
  end

  it 'should format XYM line strings correctly' do
    execute AS_TEXT, 'LineString M(1 2 3, 4 5 6)', :expect => 'LineString M (1 2 3, 4 5 6)'
  end

  it 'should format XYZM line strings correctly' do
    execute AS_TEXT, 'LineString ZM(1 2 3 4, 5 6 7 8)', :expect => 'LineString ZM (1 2 3 4, 5 6 7 8)'
  end

  it 'should format large geometries correctly' do
    execute AS_TEXT, 'LineString (1 2, 3 4, 5 6, 7 8, 9 10, 11 12, 13 14, 15 16, 17 18, 19 20, 21 22, 23 24, 25 26, 27 28, 29 30)', :expect => 'LineString (1 2, 3 4, 5 6, 7 8, 9 10, 11 12, 13 14, 15 16, 17 18, 19 20, 21 22, 23 24, 25 26, 27 28, 29 30)'
  end

  it 'should format polygons correctly' do
    execute AS_TEXT, 'Polygon((0 0, 0 3, 3 3, 3 0),(1 1, 1 2, 2 2, 2 1))', :expect => 'Polygon ((0 0, 0 3, 3 3, 3 0), (1 1, 1 2, 2 2, 2 1))'
  end

  it 'should format multilinestring correctly' do
    execute AS_TEXT, 'MultiLineString((0 0, 0 3, 3 3, 3 0),(1 1, 1 2, 2 2, 2 1))', :expect => 'MultiLineString ((0 0, 0 3, 3 3, 3 0), (1 1, 1 2, 2 2, 2 1))'
  end

  it 'should format multipolygons correctly' do
    execute AS_TEXT, 'MultiPolygon(((0 0, 0 3, 3 3, 3 0),(1 1, 1 2, 2 2, 2 1)),empty,((0 0, 0 2, 2 1)))', :expect => 'MultiPolygon (((0 0, 0 3, 3 3, 3 0), (1 1, 1 2, 2 2, 2 1)), EMPTY, ((0 0, 0 2, 2 1)))'
  end

  it 'should format multipolygons correctly' do
    execute AS_TEXT, 'MultiPoint((0 0), eMpTy, (2 1))', :expect => 'MultiPoint ((0 0), EMPTY, (2 1))'
  end

  it 'should format geometrycollections correctly' do
    execute AS_TEXT, 'geometrycollection(point(0 0), linestring(2 1, 3 4))', :expect => 'GeometryCollection (Point (0 0), LineString (2 1, 3 4))'
  end
end