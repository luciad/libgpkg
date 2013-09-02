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
    execute 'SELECT InitSpatialMetadata()'
    execute 'CREATE TABLE test (id int)'
    execute "SELECT AddGeometryColumn('test', 'geom', 'point', 0)"
  end

  it 'should succeed even if InitSpatialMetadata was not called' do
    execute 'CREATE TABLE test (id int)'
    execute "SELECT AddGeometryColumn('test', 'geom', 'point', 0)"
  end

  it 'should support all geometry types' do
    execute 'SELECT InitSpatialMetadata()'
    execute 'CREATE TABLE test (id int)'
    execute "SELECT AddGeometryColumn('test', 'geom1', 'point', 0)"
    execute "SELECT AddGeometryColumn('test', 'geom2', 'polygon', 0)"
    execute "SELECT AddGeometryColumn('test', 'geom3', 'linestring', 0)"
    execute "SELECT AddGeometryColumn('test', 'geom4', 'multipoint', 0)"
    execute "SELECT AddGeometryColumn('test', 'geom5', 'multipolygon', 0)"
    execute "SELECT AddGeometryColumn('test', 'geom6', 'multilinestring', 0)"
    execute "SELECT AddGeometryColumn('test', 'geom7', 'geometrycollection', 0)"
    execute "SELECT AddGeometryColumn('test', 'geom8', 'geometry', 0)"

    check_table 'test',
                'id' => {:index => 0, :type => 'int', :not_null => false, :default => nil, :primary_key => false},
                'geom1' => {:index => 1, :type => 'Point', :not_null => false, :default => nil, :primary_key => false},
                'geom2' => {:index => 2, :type => 'Polygon', :not_null => false, :default => nil, :primary_key => false},
                'geom3' => {:index => 3, :type => 'LineString', :not_null => false, :default => nil, :primary_key => false},
                'geom4' => {:index => 4, :type => 'MultiPoint', :not_null => false, :default => nil, :primary_key => false},
                'geom5' => {:index => 5, :type => 'MultiPolygon', :not_null => false, :default => nil, :primary_key => false},
                'geom6' => {:index => 6, :type => 'MultiLineString', :not_null => false, :default => nil, :primary_key => false},
                'geom7' => {:index => 7, :type => 'GeometryCollection', :not_null => false, :default => nil, :primary_key => false},
                'geom8' => {:index => 8, :type => 'Geometry', :not_null => false, :default => nil, :primary_key => false}
  end

  it 'should raise error on unsupported geometry type' do
    execute 'SELECT InitSpatialMetadata()'
    execute 'CREATE TABLE test (id int)'
    execute "SELECT AddGeometryColumn('test', 'geom', 'circularstring', 0)", :expect => :error
  end

  it 'should raise error if table does not exist' do
    execute "SELECT AddGeometryColumn('test', 'geom', 'point', 0)", :expect => :error
  end

  it 'should raise error if column already exists' do
    execute 'CREATE TABLE test (id int)'
    execute "SELECT AddGeometryColumn('test', 'id', 'point', 0)", :expect => :error
  end

  it 'should raise error if srid does not exists' do
    execute 'CREATE TABLE test (id int)'
    execute "SELECT AddGeometryColumn('test', 'geom', 'point', 20)", :expect => :error
  end
end