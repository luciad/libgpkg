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

describe 'AsText' do
  AS_TEXT = 'SELECT AsText(GeomFromText(?))'

  it 'should format XY points correctly' do
    expect(query(AS_TEXT, 'Point(1 2)')).to have_result 'Point (1 2)'
  end

  it 'should format XYZ points correctly' do
    expect(query(AS_TEXT, 'Point Z(1 2 3)')).to have_result 'Point Z (1 2 3)'
  end

  it 'should format XYM points correctly' do
    expect(query(AS_TEXT, 'Point M(1 2 3)')).to have_result 'Point M (1 2 3)'
  end

  it 'should format XYZM points correctly' do
    expect(query(AS_TEXT, 'Point ZM(1 2 3 4)')).to have_result 'Point ZM (1 2 3 4)'
  end

  it 'should format XY line strings correctly' do
    expect(query(AS_TEXT, 'LineString(1 2, 3 4)')).to have_result 'LineString (1 2, 3 4)'
  end

  it 'should format XYZ line strings correctly' do
    expect(query(AS_TEXT, 'LineString Z(1 2 3, 4 5 6)')).to have_result 'LineString Z (1 2 3, 4 5 6)'
  end

  it 'should format XYM line strings correctly' do
    expect(query(AS_TEXT, 'LineString M(1 2 3, 4 5 6)')).to have_result 'LineString M (1 2 3, 4 5 6)'
  end

  it 'should format XYZM line strings correctly' do
    expect(query(AS_TEXT, 'LineString ZM(1 2 3 4, 5 6 7 8)')).to have_result 'LineString ZM (1 2 3 4, 5 6 7 8)'
  end

  it 'should format large geometries correctly' do
    expect(query(AS_TEXT, 'LineString (1 2, 3 4, 5 6, 7 8, 9 10, 11 12, 13 14, 15 16, 17 18, 19 20, 21 22, 23 24, 25 26, 27 28, 29 30)')).to have_result 'LineString (1 2, 3 4, 5 6, 7 8, 9 10, 11 12, 13 14, 15 16, 17 18, 19 20, 21 22, 23 24, 25 26, 27 28, 29 30)'
  end

  it 'should format polygons correctly' do
    expect(query(AS_TEXT, 'Polygon((0 0, 0 3, 3 3, 3 0),(1 1, 1 2, 2 2, 2 1))')).to have_result 'Polygon ((0 0, 0 3, 3 3, 3 0), (1 1, 1 2, 2 2, 2 1))'
  end

  it 'should format multilinestring correctly' do
    expect(query(AS_TEXT, 'MultiLineString((0 0, 0 3, 3 3, 3 0),(1 1, 1 2, 2 2, 2 1))')).to have_result 'MultiLineString ((0 0, 0 3, 3 3, 3 0), (1 1, 1 2, 2 2, 2 1))'
  end

  it 'should format multipolygons correctly' do
    expect(query(AS_TEXT, 'MultiPolygon(((0 0, 0 3, 3 3, 3 0),(1 1, 1 2, 2 2, 2 1)),empty,((0 0, 0 2, 2 1)))')).to have_result 'MultiPolygon (((0 0, 0 3, 3 3, 3 0), (1 1, 1 2, 2 2, 2 1)), EMPTY, ((0 0, 0 2, 2 1)))'
  end

  it 'should format multipolygons correctly' do
    expect(query(AS_TEXT, 'MultiPoint((0 0), eMpTy, (2 1))')).to have_result 'MultiPoint ((0 0), EMPTY, (2 1))'
  end

  it 'should format geometrycollections correctly' do
    expect(query(AS_TEXT, 'geometrycollection(point(0 0), linestring(2 1, 3 4))')).to have_result 'GeometryCollection (Point (0 0), LineString (2 1, 3 4))'
  end

  it 'should not allow geomcollection as geometry type' do
    expect(query(AS_TEXT, 'geomcollection(point(0 0), linestring(2 1, 3 4))')).to raise_sql_error
  end

  it 'should format empty circularstring correctly' do
    expect(query(AS_TEXT, 'CircularString empty')).to have_result 'CircularString EMPTY'
  end

  it 'should format XY CircularStrings correctly' do
    expect(query(AS_TEXT, 'CircularString(1 2, 3 4, 5 6)')).to have_result 'CircularString (1 2, 3 4, 5 6)'
  end

  it 'should raise an error on incorrect circularstring' do
    expect(query(AS_TEXT, 'CircularString(1 2, 3 4)')).to raise_sql_error
  end

  it 'should format XY CircularStrings correctly' do
    expect(query(AS_TEXT, 'circularstring (0 0 , 1 1 , 2 2 , 3 3 , 4 4 , 5 5 , 6 6 , 7 7, 8 8, 9 9, 10 10)')).to have_result 'CircularString (0 0, 1 1, 2 2, 3 3, 4 4, 5 5, 6 6, 7 7, 8 8, 9 9, 10 10)'
  end

  it 'should format XYZ CircularStrings correctly' do
    expect(query(AS_TEXT, 'CircularString Z (1 2 1, 3 4 2, 5 6 3)' )) .to have_result 'CircularString Z (1 2 1, 3 4 2, 5 6 3)'
  end

  it 'should format XYZ CircularStrings correctly' do
    expect(query(AS_TEXT, 'circularstring z(0 0 1, 1 1 2, 2 2 3, 3 3 4, 4 4 5, 5 5 6, 6 6 7, 7 7 8, 8 8 9, 9 9 10, 10 10 11)' )) .to have_result 'CircularString Z (0 0 1, 1 1 2, 2 2 3, 3 3 4, 4 4 5, 5 5 6, 6 6 7, 7 7 8, 8 8 9, 9 9 10, 10 10 11)'
  end

  it 'should format XYM CircularStrings correctlly' do
    expect(query(AS_TEXT, 'CircularString M (1 2 1, 3 4 2, 5 6 3)' )) .to have_result 'CircularString M (1 2 1, 3 4 2, 5 6 3)'
  end

  it 'should format XYM CircularStrings correctly' do
    expect(query(AS_TEXT, 'circularstring m(0 0 1, 1 1 2, 2 2 3, 3 3 4, 4 4 5, 5 5 6, 6 6 7, 7 7 8, 8 8 9, 9 9 10, 10 10 11)' )) .to have_result 'CircularString M (0 0 1, 1 1 2, 2 2 3, 3 3 4, 4 4 5, 5 5 6, 6 6 7, 7 7 8, 8 8 9, 9 9 10, 10 10 11)'
  end

  it 'should format XYZM CircularStrings correctlly' do
    expect(query(AS_TEXT, 'CircularString ZM (1 2 1 2, 3 4 2 3, 5 6 3 4)' )) .to have_result 'CircularString ZM (1 2 1 2, 3 4 2 3, 5 6 3 4)'
  end

  it 'should format empty compoundcurves correctly' do
    expect(query(AS_TEXT, 'CompoundCurve empty')).to have_result 'CompoundCurve EMPTY'
  end

  it 'should format XY compoundcurves correctly' do
    expect(query(AS_TEXT, 'CompoundCurve((0 0, 2 2, 4 3), circularstring(1 2, 3 4, 5 6))')).to have_result 'CompoundCurve ((0 0, 2 2, 4 3), CircularString (1 2, 3 4, 5 6))'
  end

  it 'should format XY compoundcurves correctly' do
    expect(query(AS_TEXT, 'CompoundCurve((0 0, 2 2, 4 3), circularstring(1 2, 3 4, 5 6), (0 0, 1 1, 3 3), (0 0, 2 2, 4 3),circularstring(1 2, 3 4, 5 6))')).to have_result 'CompoundCurve ((0 0, 2 2, 4 3), CircularString (1 2, 3 4, 5 6), (0 0, 1 1, 3 3), (0 0, 2 2, 4 3), CircularString (1 2, 3 4, 5 6))'
  end

  it 'should format XYZ compoundcurves correctly' do
    expect(query(AS_TEXT, 'CompoundCurve Z((0 0 1, 2 2 2, 4 3 3), circularstring z(1 2 1, 3 4 2, 5 6 3))')).to have_result 'CompoundCurve Z ((0 0 1, 2 2 2, 4 3 3), CircularString Z (1 2 1, 3 4 2, 5 6 3))'
  end

  it 'should format XYM compoundcurve correctly' do
    expect(query(AS_TEXT, 'CompoundCurve M((0 0 1, 2 2 2, 4 3 3), circularstring M(1 2 1, 3 4 2, 5 6 3))')).to have_result 'CompoundCurve M ((0 0 1, 2 2 2, 4 3 3), CircularString M (1 2 1, 3 4 2, 5 6 3))'
  end

  it 'should format XYZM compoundcurve correctly' do
    expect(query(AS_TEXT, 'CompoundCurve ZM((0 0 1 5, 2 2 2 6, 4 3 3 7), circularstring ZM(1 2 1 5, 3 4 2 6, 5 6 3 7))')).to have_result 'CompoundCurve ZM ((0 0 1 5, 2 2 2 6, 4 3 3 7), CircularString ZM (1 2 1 5, 3 4 2 6, 5 6 3 7))'
  end

  it 'should format an empty curvepolygon correctly' do
    expect(query(AS_TEXT, 'curvepolygon empty')).to have_result 'CurvePolygon EMPTY'
  end

  it 'should format XY curvepolygon correctly' do
    expect(query(AS_TEXT, 'curvepolygon(CompoundCurve ((0 0 , 2 2 , 4 3), circularstring (1 2 , 3 4 , 5 6 )), (2 3, 4 5, 0 0), circularstring (1 2 , 3 4 , 10 10 ))')).to have_result 'CurvePolygon (CompoundCurve ((0 0, 2 2, 4 3), CircularString (1 2, 3 4, 5 6)), (2 3, 4 5, 0 0), CircularString (1 2, 3 4, 10 10))'
  end

  it 'should format XYZ curvepolygon correctly' do
    expect(query(AS_TEXT, 'curvepolygon z(CompoundCurve Z((0 0 1, 2 2 2, 4 3 2), circularstring z(1 2 3, 3 4 3, 5 6 7)))')).to have_result 'CurvePolygon Z (CompoundCurve Z ((0 0 1, 2 2 2, 4 3 2), CircularString Z (1 2 3, 3 4 3, 5 6 7)))'
  end

  it 'should format XYM curvepolygon correctly' do
    expect(query(AS_TEXT, 'curvepolygon M(CompoundCurve M((0 0 1, 2 2 2, 4 3 2), circularstring M(1 2 3, 3 4 3, 5 6 7)))')).to have_result 'CurvePolygon M (CompoundCurve M ((0 0 1, 2 2 2, 4 3 2), CircularString M (1 2 3, 3 4 3, 5 6 7)))'
  end

  it  'should format XYZM curvepolygon correctly' do
    expect(query(AS_TEXT, 'curvepolygon ZM(CompoundCurve ZM((0 0 1 3, 2 2 2 5, 4 3 2 6), circularstring ZM(1 2 3 5, 3 4 3 8, 5 6 7 9)))')).to have_result 'CurvePolygon ZM (CompoundCurve ZM ((0 0 1 3, 2 2 2 5, 4 3 2 6), CircularString ZM (1 2 3 5, 3 4 3 8, 5 6 7 9)))'
  end
end

describe 'AsBinary' do
  AS_BINARY = 'SELECT hex(AsBinary(GeomFromText(?)))'

  it 'should format XY points correctly' do
    expect(query(AS_BINARY, 'Point(1 2)')).to have_result '0101000000000000000000F03F0000000000000040'
  end

  it 'should format XYZ points correctly' do
    expect(query(AS_BINARY, 'Point Z(1 2 3)')).to have_result '01E9030000000000000000F03F00000000000000400000000000000840'
  end

  it 'should format XYM points correctly' do
    expect(query(AS_BINARY, 'Point M(1 2 3)')).to have_result '01D1070000000000000000F03F00000000000000400000000000000840'
  end

  it 'should format XYZM points correctly' do
    expect(query(AS_BINARY, 'Point ZM(1 2 3 4)')).to have_result '01B90B0000000000000000F03F000000000000004000000000000008400000000000001040'
  end

  it 'should format XY line strings correctly' do
    expect(query(AS_BINARY, 'LineString(1 2, 3 4)')).to have_result '010200000002000000000000000000F03F000000000000004000000000000008400000000000001040'
  end

  it 'should format XYZ line strings correctly' do
    expect(query(AS_BINARY, 'LineString Z(1 2 3, 4 5 6)')).to have_result '01EA03000002000000000000000000F03F00000000000000400000000000000840000000000000104000000000000014400000000000001840'
  end

  it 'should format XYM line strings correctly' do
    expect(query(AS_BINARY, 'LineString M(1 2 3, 4 5 6)')).to have_result '01D207000002000000000000000000F03F00000000000000400000000000000840000000000000104000000000000014400000000000001840'
  end

  it 'should format XYZM line strings correctly' do
    expect(query(AS_BINARY, 'LineString ZM(1 2 3 4, 5 6 7 8)')).to have_result '01BA0B000002000000000000000000F03F000000000000004000000000000008400000000000001040000000000000144000000000000018400000000000001C400000000000002040'
  end

  it 'should format large geometries correctly' do
    expect(query(AS_BINARY, 'LineString (1 2, 3 4, 5 6, 7 8, 9 10, 11 12, 13 14, 15 16, 17 18, 19 20, 21 22, 23 24, 25 26, 27 28, 29 30)')).to have_result '01020000000F000000000000000000F03F000000000000004000000000000008400000000000001040000000000000144000000000000018400000000000001C40000000000000204000000000000022400000000000002440000000000000264000000000000028400000000000002A400000000000002C400000000000002E4000000000000030400000000000003140000000000000324000000000000033400000000000003440000000000000354000000000000036400000000000003740000000000000384000000000000039400000000000003A400000000000003B400000000000003C400000000000003D400000000000003E40'
  end

  it 'should format polygons correctly' do
    expect(query(AS_BINARY, 'Polygon((0 0, 0 3, 3 3, 3 0),(1 1, 1 2, 2 2, 2 1))')).to have_result '010300000002000000040000000000000000000000000000000000000000000000000000000000000000000840000000000000084000000000000008400000000000000840000000000000000004000000000000000000F03F000000000000F03F000000000000F03F0000000000000040000000000000004000000000000000400000000000000040000000000000F03F'
  end

  it 'should format multilinestring correctly' do
    expect(query(AS_BINARY, 'MultiLineString((0 0, 0 3, 3 3, 3 0),(1 1, 1 2, 2 2, 2 1))')).to have_result '01050000000200000001020000000400000000000000000000000000000000000000000000000000000000000000000008400000000000000840000000000000084000000000000008400000000000000000010200000004000000000000000000F03F000000000000F03F000000000000F03F0000000000000040000000000000004000000000000000400000000000000040000000000000F03F'
  end

  it 'should format multipolygons correctly' do
    expect(query(AS_BINARY, 'MultiPolygon(((0 0, 0 3, 3 3, 3 0),(1 1, 1 2, 2 2, 2 1)),empty,((0 0, 0 2, 2 1)))')).to have_result '010600000003000000010300000002000000040000000000000000000000000000000000000000000000000000000000000000000840000000000000084000000000000008400000000000000840000000000000000004000000000000000000F03F000000000000F03F000000000000F03F0000000000000040000000000000004000000000000000400000000000000040000000000000F03F0103000000000000000103000000010000000300000000000000000000000000000000000000000000000000000000000000000000400000000000000040000000000000F03F'
  end

  it 'should format multipolygons correctly' do
    expect(query(AS_BINARY, 'MultiPoint((0 0), eMpTy, (2 1))')).to have_result '0104000000030000000101000000000000000000000000000000000000000101000000000000000000F87F000000000000F87F01010000000000000000000040000000000000F03F'
  end

  it 'should format geometrycollections correctly' do
    expect(query(AS_BINARY, 'geometrycollection(point(0 0), linestring(2 1, 3 4))')).to have_result '0107000000020000000101000000000000000000000000000000000000000102000000020000000000000000000040000000000000F03F00000000000008400000000000001040'
  end

  it 'should format empty circularstring correctly' do
    expect(query(AS_BINARY, 'CircularString empty')).to have_result '010800000000000000'
  end

  it 'should format XY CircularStrings correctly' do
    expect(query(AS_BINARY, 'CircularString(1 2, 3 4, 5 6)')).to have_result '010800000003000000000000000000F03F00000000000000400000000000000840000000000000104000000000000014400000000000001840'
  end

  it 'should format XY CircularStrings correctly' do
    expect(query(AS_BINARY, 'circularstring (0 0 , 1 1 , 2 2 , 3 3 , 4 4 , 5 5 , 6 6 , 7 7, 8 8, 9 9, 10 10)')).to have_result '01080000000B00000000000000000000000000000000000000000000000000F03F000000000000F03F00000000000000400000000000000040000000000000084000000000000008400000000000001040000000000000104000000000000014400000000000001440000000000000184000000000000018400000000000001C400000000000001C40000000000000204000000000000020400000000000002240000000000000224000000000000024400000000000002440'
  end

  it 'should format XYZ CircularStrings correctly' do
    expect(query(AS_BINARY, 'CircularString Z (1 2 1, 3 4 2, 5 6 3)' )) .to have_result '01F003000003000000000000000000F03F0000000000000040000000000000F03F000000000000084000000000000010400000000000000040000000000000144000000000000018400000000000000840'
  end

  it 'should format XYM CircularStrings correctlly' do
    expect(query(AS_BINARY, 'CircularString M (1 2 1, 3 4 2, 5 6 3)' )) .to have_result '01D807000003000000000000000000F03F0000000000000040000000000000F03F000000000000084000000000000010400000000000000040000000000000144000000000000018400000000000000840'
  end

  it 'should format XYZM CircularStrings correctlly' do
    expect(query(AS_BINARY, 'CircularString ZM (1 2 1 2, 3 4 2 3, 5 6 3 4)' )) .to have_result '01C00B000003000000000000000000F03F0000000000000040000000000000F03F000000000000004000000000000008400000000000001040000000000000004000000000000008400000000000001440000000000000184000000000000008400000000000001040'
  end

  it 'should format empty compoundcurves correctly' do
    expect(query(AS_BINARY, 'CompoundCurve empty')).to have_result '010900000000000000'
  end

  it 'should format XY compoundcurves correctly' do
    expect(query(AS_BINARY, 'CompoundCurve((0 0, 2 2, 4 3), circularstring(1 2, 3 4, 5 6))')).to have_result '010900000002000000010200000003000000000000000000000000000000000000000000000000000040000000000000004000000000000010400000000000000840010800000003000000000000000000F03F00000000000000400000000000000840000000000000104000000000000014400000000000001840'
  end

  it 'should format XY compoundcurves correctly' do
    expect(query(AS_BINARY, 'CompoundCurve((0 0, 2 2, 4 3), circularstring(1 2, 3 4, 5 6), (0 0, 1 1, 3 3), (0 0, 2 2, 4 3),circularstring(1 2, 3 4, 5 6))')).to have_result '010900000005000000010200000003000000000000000000000000000000000000000000000000000040000000000000004000000000000010400000000000000840010800000003000000000000000000F03F0000000000000040000000000000084000000000000010400000000000001440000000000000184001020000000300000000000000000000000000000000000000000000000000F03F000000000000F03F00000000000008400000000000000840010200000003000000000000000000000000000000000000000000000000000040000000000000004000000000000010400000000000000840010800000003000000000000000000F03F00000000000000400000000000000840000000000000104000000000000014400000000000001840'
  end

  it 'should format XYZ compoundcurves correctly' do
    expect(query(AS_BINARY, 'CompoundCurve Z((0 0 1, 2 2 2, 4 3 3), circularstring z(1 2 1, 3 4 2, 5 6 3))')).to have_result '01F10300000200000001EA0300000300000000000000000000000000000000000000000000000000F03F00000000000000400000000000000040000000000000004000000000000010400000000000000840000000000000084001F003000003000000000000000000F03F0000000000000040000000000000F03F000000000000084000000000000010400000000000000040000000000000144000000000000018400000000000000840'
  end

  it 'should format XYM compoundcurve correctly' do
    expect(query(AS_BINARY, 'CompoundCurve M((0 0 1, 2 2 2, 4 3 3), circularstring M(1 2 1, 3 4 2, 5 6 3))')).to have_result '01D90700000200000001D20700000300000000000000000000000000000000000000000000000000F03F00000000000000400000000000000040000000000000004000000000000010400000000000000840000000000000084001D807000003000000000000000000F03F0000000000000040000000000000F03F000000000000084000000000000010400000000000000040000000000000144000000000000018400000000000000840'
  end

  it 'should format XYZM compoundcurve correctly' do
    expect(query(AS_BINARY, 'CompoundCurve ZM((0 0 1 5, 2 2 2 6, 4 3 3 7), circularstring ZM(1 2 1 5, 3 4 2 6, 5 6 3 7))')).to have_result '01C10B00000200000001BA0B00000300000000000000000000000000000000000000000000000000F03F000000000000144000000000000000400000000000000040000000000000004000000000000018400000000000001040000000000000084000000000000008400000000000001C4001C00B000003000000000000000000F03F0000000000000040000000000000F03F000000000000144000000000000008400000000000001040000000000000004000000000000018400000000000001440000000000000184000000000000008400000000000001C40'
  end

  it 'should format an empty curvepolygon correctly' do
    expect(query(AS_BINARY, 'curvepolygon empty')).to have_result '010A00000000000000'
  end

  it 'should format XY curvepolygon correctly' do
    expect(query(AS_BINARY, 'curvepolygon(CompoundCurve ((0 0 , 2 2 , 4 3), circularstring (1 2 , 3 4 , 5 6 )), (2 3, 4 5, 0 0), circularstring (1 2 , 3 4 , 10 10 ))')).to have_result '010A00000003000000010900000002000000010200000003000000000000000000000000000000000000000000000000000040000000000000004000000000000010400000000000000840010800000003000000000000000000F03F00000000000000400000000000000840000000000000104000000000000014400000000000001840010200000003000000000000000000004000000000000008400000000000001040000000000000144000000000000000000000000000000000010800000003000000000000000000F03F00000000000000400000000000000840000000000000104000000000000024400000000000002440'
  end

  it 'should parse XYZ curvepolygon correctly' do
    expect(query(AS_BINARY, 'curvepolygon z(CompoundCurve Z((0 0 1, 2 2 2, 4 3 2), circularstring z(1 2 3, 3 4 3, 5 6 7)))')).to have_result '01F20300000100000001F10300000200000001EA0300000300000000000000000000000000000000000000000000000000F03F00000000000000400000000000000040000000000000004000000000000010400000000000000840000000000000004001F003000003000000000000000000F03F00000000000000400000000000000840000000000000084000000000000010400000000000000840000000000000144000000000000018400000000000001C40'
  end

  it 'should parse XYM curvepolygon correctly' do
    expect(query(AS_BINARY, 'curvepolygon M(CompoundCurve M((0 0 1, 2 2 2, 4 3 2), circularstring M(1 2 3, 3 4 3, 5 6 7)))')).to have_result '01DA0700000100000001D90700000200000001D20700000300000000000000000000000000000000000000000000000000F03F00000000000000400000000000000040000000000000004000000000000010400000000000000840000000000000004001D807000003000000000000000000F03F00000000000000400000000000000840000000000000084000000000000010400000000000000840000000000000144000000000000018400000000000001C40'
  end

  it  'should parse XYZM curvepolygon correctly' do
    expect(query(AS_BINARY, 'curvepolygon ZM(CompoundCurve ZM((0 0 1 3, 2 2 2 5, 4 3 2 6), circularstring ZM(1 2 3 5, 3 4 3 8, 5 6 7 9)))')).to have_result '01C20B00000100000001C10B00000200000001BA0B00000300000000000000000000000000000000000000000000000000F03F00000000000008400000000000000040000000000000004000000000000000400000000000001440000000000000104000000000000008400000000000000040000000000000184001C00B000003000000000000000000F03F0000000000000040000000000000084000000000000014400000000000000840000000000000104000000000000008400000000000002040000000000000144000000000000018400000000000001C400000000000002240'
  end
end

describe 'GeomFromText' do
  AS_GEOM = 'SELECT lower(hex(GeomFromText(?, -1)))'

  it 'should parse XY points correctly' do
    expect(query(AS_GEOM, 'Point(1 2)')).
        to have_result(
               mode == :gpkg ?
                   '47500001ffffffff0101000000000000000000f03f0000000000000040' :
                   '0001ffffffff000000000000f03f0000000000000040000000000000f03f00000000000000407c01000000000000000000f03f0000000000000040fe'
           )
  end

  it 'should be case insensitive' do
    expect(query(AS_GEOM, 'pOiNt(1 2)')).
        to have_result(
               mode == :gpkg ?
                   '47500001ffffffff0101000000000000000000f03f0000000000000040' :
                   '0001ffffffff000000000000f03f0000000000000040000000000000f03f00000000000000407c01000000000000000000f03f0000000000000040fe'
           )
  end

  it 'should parse XYZ points correctly' do
    expect(query(AS_GEOM, 'Point Z(1 2 3)')).
        to have_result(
               mode == :gpkg ?
                   '47500001ffffffff01e9030000000000000000f03f00000000000000400000000000000840' :
                   '0001ffffffff000000000000f03f0000000000000040000000000000f03f00000000000000407ce9030000000000000000f03f00000000000000400000000000000840fe'
           )
  end

  it 'should parse XYM points correctly' do
    expect(query(AS_GEOM, 'Point M(1 2 3)')).
        to have_result(
               mode == :gpkg ?
                   '47500001ffffffff01d1070000000000000000f03f00000000000000400000000000000840' :
                   '0001ffffffff000000000000f03f0000000000000040000000000000f03f00000000000000407cd1070000000000000000f03f00000000000000400000000000000840fe'
           )
  end

  it 'should parse XYZM points correctly' do
    expect(query(AS_GEOM, 'Point ZM(1 2 3 4)')).
        to have_result(
               mode == :gpkg ?
                   '47500001ffffffff01b90b0000000000000000f03f000000000000004000000000000008400000000000001040' :
                   '0001ffffffff000000000000f03f0000000000000040000000000000f03f00000000000000407cb90b0000000000000000f03f000000000000004000000000000008400000000000001040fe'
           )
  end

  it 'should parse XY line strings correctly' do
    expect(query(AS_GEOM, 'LineString(1 2, 3 4)')).
        to have_result(
               mode == :gpkg ?
                   '47500003ffffffff000000000000f03f000000000000084000000000000000400000000000001040010200000002000000000000000000f03f000000000000004000000000000008400000000000001040' :
                   '0001ffffffff000000000000f03f0000000000000040000000000000084000000000000010407c0200000002000000000000000000f03f000000000000004000000000000008400000000000001040fe'
           )
  end

  it 'should parse XYZ line strings correctly' do
    expect(query(AS_GEOM, 'LineString Z(1 2 3, 4 5 6)')).
        to have_result(
               mode == :gpkg ?
                   '47500005ffffffff000000000000f03f0000000000001040000000000000004000000000000014400000000000000840000000000000184001ea03000002000000000000000000f03f00000000000000400000000000000840000000000000104000000000000014400000000000001840' :
                   '0001ffffffff000000000000f03f0000000000000040000000000000104000000000000014407cea03000002000000000000000000f03f00000000000000400000000000000840000000000000104000000000000014400000000000001840fe'
           )
  end

  it 'should parse XYM line strings correctly' do
    expect(query(AS_GEOM, 'LineString M(1 2 3, 4 5 6)')).
        to have_result(
               mode == :gpkg ?
                   '47500007ffffffff000000000000f03f0000000000001040000000000000004000000000000014400000000000000840000000000000184001d207000002000000000000000000f03f00000000000000400000000000000840000000000000104000000000000014400000000000001840' :
                   '0001ffffffff000000000000f03f0000000000000040000000000000104000000000000014407cd207000002000000000000000000f03f00000000000000400000000000000840000000000000104000000000000014400000000000001840fe'
           )
  end

  it 'should parse XYZM line strings correctly' do
    expect(query(AS_GEOM, 'LineString ZM(1 2 3 4, 5 6 7 8)')).
        to have_result(
               mode == :gpkg ?
                   '47500009ffffffff000000000000f03f00000000000014400000000000000040000000000000184000000000000008400000000000001c400000000000001040000000000000204001ba0b000002000000000000000000f03f000000000000004000000000000008400000000000001040000000000000144000000000000018400000000000001c400000000000002040' :
                   '0001ffffffff000000000000f03f0000000000000040000000000000144000000000000018407cba0b000002000000000000000000f03f000000000000004000000000000008400000000000001040000000000000144000000000000018400000000000001c400000000000002040fe'
           )
  end

  it 'should raise error on unclosed WKT' do
    expect(query(AS_GEOM, 'LineString ZM(1 ')).to raise_sql_error
  end

  it 'should raise error on unknown geometry type' do
    expect(query(AS_GEOM, 'MyGeometry EMPTY')).to raise_sql_error
  end

  it 'should raise error on incorrect modifiers' do
    expect(query(AS_GEOM, 'Point X (1 2)')).to raise_sql_error
  end

  it 'should raise error on incorrect empty set' do
    expect(query(AS_GEOM, 'Point empt')).to raise_sql_error
  end

  it 'should parse empty point correctly' do
    expect(query(AS_GEOM, 'Point empty')).
        to have_result(
               mode == :gpkg ?
                   '47500011ffffffff0101000000000000000000f87f000000000000f87f' :
                   '0001ffffffff000000000000f87f000000000000f87f000000000000f87f000000000000f87f7c01000000000000000000f87f000000000000f87ffe'
           )
  end

  it 'should parse empty linestring correctly' do
    expect(query(AS_GEOM, 'linestring empty')).
        to have_result(
               mode == :gpkg ?
                   '47500013ffffffff000000000000f87f000000000000f87f000000000000f87f000000000000f87f010200000000000000' :
                   '0001ffffffff000000000000f87f000000000000f87f000000000000f87f000000000000f87f7c0200000000000000fe'
           )
  end

  it 'should parse empty polygon correctly' do
    expect(query(AS_GEOM, 'polygon empty')).
        to have_result(
               mode == :gpkg ?
                   '47500013ffffffff000000000000f87f000000000000f87f000000000000f87f000000000000f87f010300000000000000' :
                   '0001ffffffff000000000000f87f000000000000f87f000000000000f87f000000000000f87f7c0300000000000000fe'
           )
  end

  it 'should parse empty multipoint correctly' do
    expect(query(AS_GEOM, 'MultiPoint empty')).
        to have_result(
               mode == :gpkg ?
                   '47500013ffffffff000000000000f87f000000000000f87f000000000000f87f000000000000f87f010400000000000000' :
                   '0001ffffffff000000000000f87f000000000000f87f000000000000f87f000000000000f87f7c0400000000000000fe'
           )
  end

  it 'should parse empty multilinestring correctly' do
    expect(query(AS_GEOM, 'multilinestring empty')).
        to have_result(
               mode == :gpkg ?
                   '47500013ffffffff000000000000f87f000000000000f87f000000000000f87f000000000000f87f010500000000000000' :
                   '0001ffffffff000000000000f87f000000000000f87f000000000000f87f000000000000f87f7c0500000000000000fe'
           )
  end

  it 'should parse empty multipolygon correctly' do
    expect(query(AS_GEOM, 'multipolygon empty')).
        to have_result(
               mode == :gpkg ?
                   '47500013ffffffff000000000000f87f000000000000f87f000000000000f87f000000000000f87f010600000000000000' :
                   '0001ffffffff000000000000f87f000000000000f87f000000000000f87f000000000000f87f7c0600000000000000fe'
           )
  end
end

describe 'GeomFromWKB' do
  FROM_WKB = 'SELECT lower(hex(GeomFromWKB(AsBinary(GeomFromText(?)), -1)))'

  it 'should parse XY points correctly' do
    expect(query(FROM_WKB, 'Point(1 2)')).
        to have_result(
               mode == :gpkg ?
                   '47500001ffffffff0101000000000000000000f03f0000000000000040' :
                   '0001ffffffff000000000000f03f0000000000000040000000000000f03f00000000000000407c01000000000000000000f03f0000000000000040fe'
           )
  end

  it 'should parse XYZ points correctly' do
    expect(query(FROM_WKB, 'Point Z(1 2 3)')).
        to have_result(
               mode == :gpkg ?
                   '47500001ffffffff01e9030000000000000000f03f00000000000000400000000000000840' :
                   '0001ffffffff000000000000f03f0000000000000040000000000000f03f00000000000000407ce9030000000000000000f03f00000000000000400000000000000840fe'
           )
  end

  it 'should parse XYM points correctly' do
    expect(query(FROM_WKB, 'Point M(1 2 3)')).
        to have_result(
               mode == :gpkg ?
                   '47500001ffffffff01d1070000000000000000f03f00000000000000400000000000000840' :
                   '0001ffffffff000000000000f03f0000000000000040000000000000f03f00000000000000407cd1070000000000000000f03f00000000000000400000000000000840fe'
           )
  end

  it 'should parse XYZM points correctly' do
    expect(query(FROM_WKB, 'Point ZM(1 2 3 4)')).
        to have_result(
               mode == :gpkg ?
                   '47500001ffffffff01b90b0000000000000000f03f000000000000004000000000000008400000000000001040' :
                   '0001ffffffff000000000000f03f0000000000000040000000000000f03f00000000000000407cb90b0000000000000000f03f000000000000004000000000000008400000000000001040fe'
           )
  end

  it 'should parse XY line strings correctly' do
    expect(query(FROM_WKB, 'LineString(1 2, 3 4)')).
        to have_result(
               mode == :gpkg ?
                   '47500003ffffffff000000000000f03f000000000000084000000000000000400000000000001040010200000002000000000000000000f03f000000000000004000000000000008400000000000001040' :
                   '0001ffffffff000000000000f03f0000000000000040000000000000084000000000000010407c0200000002000000000000000000f03f000000000000004000000000000008400000000000001040fe'
           )
  end

  it 'should parse XYZ line strings correctly' do
    expect(query(FROM_WKB, 'LineString Z(1 2 3, 4 5 6)')).
        to have_result(
               mode == :gpkg ?
                   '47500005ffffffff000000000000f03f0000000000001040000000000000004000000000000014400000000000000840000000000000184001ea03000002000000000000000000f03f00000000000000400000000000000840000000000000104000000000000014400000000000001840' :
                   '0001ffffffff000000000000f03f0000000000000040000000000000104000000000000014407cea03000002000000000000000000f03f00000000000000400000000000000840000000000000104000000000000014400000000000001840fe'
           )
  end

  it 'should parse XYM line strings correctly' do
    expect(query(FROM_WKB, 'LineString M(1 2 3, 4 5 6)')).
        to have_result(
               mode == :gpkg ?
                   '47500007ffffffff000000000000f03f0000000000001040000000000000004000000000000014400000000000000840000000000000184001d207000002000000000000000000f03f00000000000000400000000000000840000000000000104000000000000014400000000000001840' :
                   '0001ffffffff000000000000f03f0000000000000040000000000000104000000000000014407cd207000002000000000000000000f03f00000000000000400000000000000840000000000000104000000000000014400000000000001840fe'
           )
  end

  it 'should parse XYZM line strings correctly' do
    expect(query(FROM_WKB, 'LineString ZM(1 2 3 4, 5 6 7 8)')).
        to have_result(
               mode == :gpkg ?
                   '47500009ffffffff000000000000f03f00000000000014400000000000000040000000000000184000000000000008400000000000001c400000000000001040000000000000204001ba0b000002000000000000000000f03f000000000000004000000000000008400000000000001040000000000000144000000000000018400000000000001c400000000000002040' :
                   '0001ffffffff000000000000f03f0000000000000040000000000000144000000000000018407cba0b000002000000000000000000f03f000000000000004000000000000008400000000000001040000000000000144000000000000018400000000000001c400000000000002040fe'
           )
  end

  it 'should parse empty point correctly' do
    expect(query(FROM_WKB, 'Point empty')).
        to have_result(
               mode == :gpkg ?
                   '47500011ffffffff0101000000000000000000f87f000000000000f87f' :
                   '0001ffffffff000000000000f87f000000000000f87f000000000000f87f000000000000f87f7c01000000000000000000f87f000000000000f87ffe'
           )
  end

  it 'should parse empty linestring correctly' do
    expect(query(FROM_WKB, 'linestring empty')).
        to have_result(
               mode == :gpkg ?
                   '47500013ffffffff000000000000f87f000000000000f87f000000000000f87f000000000000f87f010200000000000000' :
                   '0001ffffffff000000000000f87f000000000000f87f000000000000f87f000000000000f87f7c0200000000000000fe'
           )
  end

  it 'should parse empty polygon correctly' do
    expect(query(FROM_WKB, 'polygon empty')).
        to have_result(
               mode == :gpkg ?
                   '47500013ffffffff000000000000f87f000000000000f87f000000000000f87f000000000000f87f010300000000000000' :
                   '0001ffffffff000000000000f87f000000000000f87f000000000000f87f000000000000f87f7c0300000000000000fe'
           )
  end

  it 'should parse empty multipoint correctly' do
    expect(query(FROM_WKB, 'MultiPoint empty')).
        to have_result(
               mode == :gpkg ?
                   '47500013ffffffff000000000000f87f000000000000f87f000000000000f87f000000000000f87f010400000000000000' :
                   '0001ffffffff000000000000f87f000000000000f87f000000000000f87f000000000000f87f7c0400000000000000fe'
           )
  end

  it 'should parse empty multilinestring correctly' do
    expect(query(FROM_WKB, 'multilinestring empty')).
        to have_result(
               mode == :gpkg ?
                   '47500013ffffffff000000000000f87f000000000000f87f000000000000f87f000000000000f87f010500000000000000' :
                   '0001ffffffff000000000000f87f000000000000f87f000000000000f87f000000000000f87f7c0500000000000000fe'
           )
  end

  it 'should parse empty multipolygon correctly' do
    expect(query(FROM_WKB, 'multipolygon empty')).
        to have_result(
               mode == :gpkg ?
                   '47500013ffffffff000000000000f87f000000000000f87f000000000000f87f000000000000f87f010600000000000000' :
                   '0001ffffffff000000000000f87f000000000000f87f000000000000f87f000000000000f87f7c0600000000000000fe'
           )
  end
end