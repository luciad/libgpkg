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

  GEOM_TYPES = {
      'point' => 'Point',
      'polygon' => 'Polygon',
      'linestring' => 'LineString',
      'multipoint' => 'MultiPoint',
      'multipolygon' => 'MultiPolygon',
      'multilinestring' => 'MultiLineString',
      'geometrycollection' => 'GeomCollection',
      'geomcollection' => 'GeomCollection',
      'geometry' => 'Geometry',
      'circularstring' => 'CircularString',
  }

  GEOM_TYPES.each_pair do |param, type|
    it "should support '#{param}' as geometry type" do
      expect('SELECT InitSpatialMetadata()').to have_result nil
      expect('CREATE TABLE test (id int)').to have_result nil
      expect("SELECT AddGeometryColumn('test', 'geom', '#{param}', 0, 0, 0)").to have_result nil
      expect('test').to have_table_schema(
                             'id' => {:index => 0, :type => 'int', :not_null => false, :default => nil, :primary_key => false},
                             'geom' => {:index => 1, :type => type, :not_null => false, :default => nil, :primary_key => false}
                         )

      expect('CREATE TABLE test_st (id int)').to have_result nil
      expect("SELECT AddGeometryColumn('test_st', 'geom', 'st_#{param}', 0, 0, 0)").to have_result nil
      expect('test_st').to have_table_schema(
                            'id' => {:index => 0, :type => 'int', :not_null => false, :default => nil, :primary_key => false},
                            'geom' => {:index => 1, :type => type, :not_null => false, :default => nil, :primary_key => false}
                        )
    end
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