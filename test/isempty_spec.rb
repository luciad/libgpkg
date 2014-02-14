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
    expect('SELECT ST_IsEmpty(NULL)').to have_result nil
  end

  it 'should return the 0 for non-empty geometry' do
    expect("SELECT ST_IsEmpty(GeomFromText('Point(1 0)'))").to have_result 0
    expect("SELECT ST_IsEmpty(GeomFromText('LineString(1 1, 2 2)'))").to have_result 0
    expect("SELECT ST_IsEmpty(GeomFromText('Polygon((1 1, 2 2, 3 3))'))").to have_result 0
    expect("SELECT ST_IsEmpty(GeomFromText('Polygon((1 1, 2 2, 3 3), EMPTY)'))").to have_result 0
    expect("SELECT ST_IsEmpty(GeomFromText('GeometryCollection(Point(1 0), Point EMPTY)'))").to have_result 0
    expect("SELECT ST_IsEmpty(GeomFromText('CircularString(1 1, 2 2, 2 2)'))").to have_result 0
    expect("SELECT ST_IsEmpty(GeomFromText('Curvepolygon((1 1, 2 2))'))").to have_result 0
    expect("SELECT ST_IsEmpty(GeomFromText('compoundcurve(circularstring(1 1, 2 2, 2 2))'))").to have_result 0
  end

  it 'should return the 1 for empty geometry' do
    expect("SELECT ST_IsEmpty(GeomFromText('Point empty'))").to have_result 1
    expect("SELECT ST_IsEmpty(GeomFromText('LineString empty'))").to have_result 1
    expect("SELECT ST_IsEmpty(GeomFromText('Polygon empty'))").to have_result 1
    expect("SELECT ST_IsEmpty(GeomFromText('MultiPoint empty'))").to have_result 1
    expect("SELECT ST_IsEmpty(GeomFromText('MultiLineString empty'))").to have_result 1
    expect("SELECT ST_IsEmpty(GeomFromText('MultiPolygon empty'))").to have_result 1
    expect("SELECT ST_IsEmpty(GeomFromText('GeometryCollection empty'))").to have_result 1
    expect("SELECT ST_IsEmpty(GeomFromText('CircularString empty'))").to have_result 1
    expect("SELECT ST_IsEmpty(GeomFromText('Compoundcurve empty'))").to have_result 1
    expect("SELECT ST_IsEmpty(GeomFromText('Curvepolygon empty'))").to have_result 1
  end

  it 'should raise an error on invalid input' do
    expect("SELECT ST_IsEmpty(x'FFFFFFFFFF')").to raise_sql_error
  end
end