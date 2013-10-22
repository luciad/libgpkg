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

describe 'CreateSpatialIndex' do
  index_prefix = mode == :gpkg ? 'rtree' : 'idx'
  index_id = mode == :gpkg ? 'id' : 'pkid'
  
  it 'should return NULL on success' do
    expect('SELECT InitSpatialMetadata()').to have_result nil
    expect('CREATE TABLE test (id int)').to have_result nil
    expect("SELECT AddGeometryColumn('test', 'geom', 'point', 0, 0, 0)").to have_result nil
    expect("SELECT CreateSpatialIndex('test', 'geom', 'rowid')").to have_result nil
    expect("select count(*) from sqlite_master where name = \"#{index_prefix}_test_geom\";").to have_result 1
  end

  it 'should create working spatial index' do
    expect('SELECT InitSpatialMetadata()').to have_result nil
    expect('CREATE TABLE test (id int)').to have_result nil
    expect("SELECT AddGeometryColumn('test', 'geom', 'point', 0, 0, 0)").to have_result nil
    expect("SELECT CreateSpatialIndex('test', 'geom', 'id')").to have_result nil

    # Insertion
    expect("INSERT INTO test VALUES (1, GeomFromText('POINT(1 1)'))").to have_result nil
    expect("SELECT #{index_id} FROM #{index_prefix}_test_geom").to have_result 1
    expect("SELECT count(*) FROM #{index_prefix}_test_geom").to have_result 1

    # Update where geometry changes
    expect("UPDATE test SET geom = GeomFromText('POINT(2 2)') WHERE id = 1").to have_result nil
    expect("SELECT #{index_id} FROM #{index_prefix}_test_geom").to have_result 1
    expect("SELECT count(*) FROM #{index_prefix}_test_geom").to have_result 1

    # Update where geometry changes from not NULL to NULL
    expect("UPDATE test SET geom = NULL WHERE id = 1").to have_result nil
    expect("SELECT #{index_id} FROM #{index_prefix}_test_geom").to have_result nil
    expect("SELECT count(*) FROM #{index_prefix}_test_geom").to have_result 0

    # Update where geometry changes from NULL to not NULL
    expect("UPDATE test SET geom = GeomFromText('POINT(2 2)') WHERE id = 1").to have_result nil
    expect("SELECT #{index_id} FROM #{index_prefix}_test_geom").to have_result 1
    expect("SELECT count(*) FROM #{index_prefix}_test_geom").to have_result 1

    # Update where id changes and geometry changes to NULL
    expect("UPDATE test SET id = 2, geom = NULL WHERE id = 1").to have_result nil
    expect("SELECT #{index_id} FROM #{index_prefix}_test_geom").to have_result nil
    expect("SELECT count(*) FROM #{index_prefix}_test_geom").to have_result 0

    # Update where id changes and geometry changes to not NULL
    expect("UPDATE test SET id = 3, geom = GeomFromText('POINT(2 2)') WHERE id = 2").to have_result nil
    expect("SELECT #{index_id} FROM #{index_prefix}_test_geom").to have_result 3
    expect("SELECT count(*) FROM #{index_prefix}_test_geom").to have_result 1

    # Update where id changes and geometry changes to not NULL
    expect("UPDATE test SET id = 4 WHERE id = 3").to have_result nil
    expect("SELECT #{index_id} FROM #{index_prefix}_test_geom").to have_result 4
    expect("SELECT count(*) FROM #{index_prefix}_test_geom").to have_result 1

    # Deletion
    expect('DELETE FROM test').to have_result nil
    expect("SELECT #{index_id} FROM #{index_prefix}_test_geom").to have_result nil
    expect("SELECT count(*) FROM #{index_prefix}_test_geom").to have_result 0
  end

  it 'should create working spatial index for existing data' do
    expect('SELECT InitSpatialMetadata()').to have_result nil
    expect('CREATE TABLE test (id int)').to have_result nil
    expect("SELECT AddGeometryColumn('test', 'geom', 'point', 0, 0, 0)").to have_result nil

    expect("INSERT INTO test VALUES (1, GeomFromText('POINT(11 12)'))").to have_result nil
    expect("INSERT INTO test VALUES (2, GeomFromText('POINT(21 22)'))").to have_result nil
    expect("INSERT INTO test VALUES (3, GeomFromText('POINT(31 32)'))").to have_result nil

    # Create spatial index based on initial data
    expect("SELECT CreateSpatialIndex('test', 'geom', 'id')").to have_result nil
    expect("SELECT count(*) FROM #{index_prefix}_test_geom").to have_result 3
  end

end

