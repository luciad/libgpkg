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

describe 'ST_IsMeasured' do
  it 'should return NULL when passed NULL' do
    expect('SELECT ST_IsMeasured(NULL)').to have_result nil
  end

  it 'should return the 0 for non-measured geometry' do
    expect("SELECT ST_IsMeasured(GeomFromText('Point(1 0)'))").to have_result 0
    expect("SELECT ST_IsMeasured(GeomFromText('LineString(1 1, 2 2)'))").to have_result 0
    expect("SELECT ST_IsMeasured(GeomFromText('Polygon((1 1, 2 2, 3 3))'))").to have_result 0
    expect("SELECT ST_IsMeasured(GeomFromText('Polygon((1 1, 2 2, 3 3), EMPTY)'))").to have_result 0
    expect("SELECT ST_IsMeasured(GeomFromText('GeometryCollection(Point(1 0), Point EMPTY)'))").to have_result 0

    expect("SELECT ST_IsMeasured(GeomFromText('Point Z(1 0 1)'))").to have_result 0
    expect("SELECT ST_IsMeasured(GeomFromText('LineString Z(1 1 1, 2 2 1)'))").to have_result 0
    expect("SELECT ST_IsMeasured(GeomFromText('Polygon Z((1 1 1, 2 2 2, 3 3 3))'))").to have_result 0
    expect("SELECT ST_IsMeasured(GeomFromText('Polygon Z((1 1 1, 2 2 3, 3 3 3), EMPTY)'))").to have_result 0
    expect("SELECT ST_IsMeasured(GeomFromText('GeometryCollection Z(Point Z(1 0 0), Point Z EMPTY)'))").to have_result 0
  end

  it 'should return the 1 for measured geometry' do
    expect("SELECT ST_IsMeasured(GeomFromText('Point M(1 0 1)'))").to have_result 1
    expect("SELECT ST_IsMeasured(GeomFromText('LineString M(1 1 1, 2 2 1)'))").to have_result 1
    expect("SELECT ST_IsMeasured(GeomFromText('Polygon M((1 1 1, 2 2 2, 3 3 3))'))").to have_result 1
    expect("SELECT ST_IsMeasured(GeomFromText('Polygon M((1 1 1, 2 2 3, 3 3 3), EMPTY)'))").to have_result 1
    expect("SELECT ST_IsMeasured(GeomFromText('GeometryCollection M(Point M(1 0 0), Point M EMPTY)'))").to have_result 1

    expect("SELECT ST_IsMeasured(GeomFromText('Point ZM(1 0 1 1)'))").to have_result 1
    expect("SELECT ST_IsMeasured(GeomFromText('LineString ZM(1 1 1 1, 2 2 1 1)'))").to have_result 1
    expect("SELECT ST_IsMeasured(GeomFromText('Polygon ZM((1 1 1 1, 2 2 2 2, 3 3 3 3))'))").to have_result 1
    expect("SELECT ST_IsMeasured(GeomFromText('Polygon ZM((1 1 1 1, 2 2 3 3, 3 3 3 3), EMPTY)'))").to have_result 1
    expect("SELECT ST_IsMeasured(GeomFromText('GeometryCollection ZM(Point ZM(1 0 0 0), Point ZM EMPTY)'))").to have_result 1
  end

  it 'should raise an error on invalid input' do
    expect("SELECT ST_IsMeasured(x'FFFFFFFFFF')").to raise_sql_error
  end
end