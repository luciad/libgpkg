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

describe 'AddGeometryColumn' do
  it 'should return NULL on success' do
    expect('SELECT InitSpatialMetadata()').to have_result nil
    expect('CREATE TABLE test (id int)').to have_result nil
    expect("SELECT AddGeometryColumn('test', 'geom', 'point', 0, 0, 0)").to have_result nil
  end

  it 'should succeed even if InitSpatialMetadata was not called' do
    expect('CREATE TABLE test (id int)').to have_result nil
    expect("SELECT AddGeometryColumn('test', 'geom', 'point', 0, 0, 0)").to have_result nil
  end

  it 'should support all geometry types' do
    expect('SELECT InitSpatialMetadata()').to have_result nil
    expect('CREATE TABLE test1 (id int)').to have_result nil
    expect("SELECT AddGeometryColumn('test1', 'geom1', 'point', 0, 0, 0)").to have_result nil
    expect('CREATE TABLE test2 (id int)').to have_result nil
    expect("SELECT AddGeometryColumn('test2', 'geom2', 'polygon', 0, 0, 0)").to have_result nil
    expect('CREATE TABLE test3 (id int)').to have_result nil
    expect("SELECT AddGeometryColumn('test3', 'geom3', 'linestring', 0, 0, 0)").to have_result nil
    expect('CREATE TABLE test4 (id int)').to have_result nil
    expect("SELECT AddGeometryColumn('test4', 'geom4', 'multipoint', 0, 0, 0)").to have_result nil
    expect('CREATE TABLE test5 (id int)').to have_result nil
    expect("SELECT AddGeometryColumn('test5', 'geom5', 'multipolygon', 0, 0, 0)").to have_result nil
    expect('CREATE TABLE test6 (id int)').to have_result nil
    expect("SELECT AddGeometryColumn('test6', 'geom6', 'multilinestring', 0, 0, 0)").to have_result nil
    expect('CREATE TABLE test7 (id int)').to have_result nil
    expect("SELECT AddGeometryColumn('test7', 'geom7', 'geometrycollection', 0, 0, 0)").to have_result nil
    expect('CREATE TABLE test8 (id int)').to have_result nil
    expect("SELECT AddGeometryColumn('test8', 'geom8', 'geometry', 0, 0, 0)").to have_result nil
    expect('CREATE TABLE test9 (id int)').to have_result nil
    expect("SELECT AddGeometryColumn('test9', 'geom9', 'circularstring', 0, 0, 0)").to have_result nil

    expect('test').to have_table_schema(
                          'id' => {:index => 0, :type => 'int', :not_null => false, :default => nil, :primary_key => false},
                          'geom1' => {:index => 1, :type => 'Point', :not_null => false, :default => nil, :primary_key => false},
                          'geom2' => {:index => 2, :type => 'Polygon', :not_null => false, :default => nil, :primary_key => false},
                          'geom3' => {:index => 3, :type => 'LineString', :not_null => false, :default => nil, :primary_key => false},
                          'geom4' => {:index => 4, :type => 'MultiPoint', :not_null => false, :default => nil, :primary_key => false},
                          'geom5' => {:index => 5, :type => 'MultiPolygon', :not_null => false, :default => nil, :primary_key => false},
                          'geom6' => {:index => 6, :type => 'MultiLineString', :not_null => false, :default => nil, :primary_key => false},
                          'geom7' => {:index => 7, :type => 'GeometryCollection', :not_null => false, :default => nil, :primary_key => false},
                          'geom8' => {:index => 8, :type => 'Geometry', :not_null => false, :default => nil, :primary_key => false}
                      )
  end


  it 'should raise error if table does not exist' do
    expect("SELECT AddGeometryColumn('test', 'geom', 'point', 0, 0, 0)").to raise_sql_error
  end

  it 'should raise error if column already exists' do
    expect('CREATE TABLE test (id int)').to have_result nil
    expect("SELECT AddGeometryColumn('test', 'id', 'point', 0, 0, 0)").to raise_sql_error
  end

  it 'should raise error if srid does not exists' do
    expect('CREATE TABLE test (id int)').to have_result nil
    expect("SELECT AddGeometryColumn('test', 'geom', 'point', 20)").to raise_sql_error
  end
end