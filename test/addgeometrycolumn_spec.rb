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
    expect(@db.get_first_value('SELECT InitGpkg()')).to eq(nil)
    expect(@db.get_first_value('CREATE TABLE test (id int)')).to eq(nil)
    expect(@db.get_first_value("SELECT AddGeometryColumn('test', 'geom', 0, 'point', 2)")).to eq(nil)
  end

  it 'should succeed even if InitGpkg was not called' do
    expect(@db.get_first_value('CREATE TABLE test (id int)')).to eq(nil)
    expect(@db.get_first_value("SELECT AddGeometryColumn('test', 'geom', 0, 'point', 2)")).to eq(nil)
  end

  it 'should support all geometry types' do
    expect(@db.get_first_value('SELECT InitGpkg()')).to eq(nil)
    expect(@db.get_first_value('CREATE TABLE test (id int)')).to eq(nil)
    expect(@db.get_first_value("SELECT AddGeometryColumn('test', 'geom1', 0, 'point', 2)")).to eq(nil)
    expect(@db.get_first_value("SELECT AddGeometryColumn('test', 'geom2', 0, 'polygon', 2)")).to eq(nil)
    expect(@db.get_first_value("SELECT AddGeometryColumn('test', 'geom3', 0, 'linestring', 2)")).to eq(nil)
    expect(@db.get_first_value("SELECT AddGeometryColumn('test', 'geom4', 0, 'multipoint', 2)")).to eq(nil)
    expect(@db.get_first_value("SELECT AddGeometryColumn('test', 'geom5', 0, 'multipolygon', 2)")).to eq(nil)
    expect(@db.get_first_value("SELECT AddGeometryColumn('test', 'geom6', 0, 'multilinestring', 2)")).to eq(nil)
    expect(@db.get_first_value("SELECT AddGeometryColumn('test', 'geom7', 0, 'geometrycollection', 2)")).to eq(nil)
    expect(@db.get_first_value("SELECT AddGeometryColumn('test', 'geom8', 0, 'geometry', 2)")).to eq(nil)

    table_info = @db.execute("PRAGMA table_info('test')")
    expect(table_info[0]).to eq([0, 'id', 'int', 0, nil, 0])
    expect(table_info[1]).to eq([1, 'geom1', 'point', 0, nil, 0])
    expect(table_info[2]).to eq([2, 'geom2', 'polygon', 0, nil, 0])
    expect(table_info[3]).to eq([3, 'geom3', 'linestring', 0, nil, 0])
    expect(table_info[4]).to eq([4, 'geom4', 'multipoint', 0, nil, 0])
    expect(table_info[5]).to eq([5, 'geom5', 'multipolygon', 0, nil, 0])
    expect(table_info[6]).to eq([6, 'geom6', 'multilinestring', 0, nil, 0])
    expect(table_info[7]).to eq([7, 'geom7', 'geometrycollection', 0, nil, 0])
    expect(table_info[8]).to eq([8, 'geom8', 'geometry', 0, nil, 0])
  end

  it 'should raise error on unsupported geometry type' do
    expect(@db.get_first_value('SELECT InitGpkg()')).to eq(nil)
    expect(@db.get_first_value('CREATE TABLE test (id int)')).to eq(nil)
    expect{@db.get_first_value("SELECT AddGeometryColumn('test', 'geom', 0, 'circularstring', 2)")}.to raise_error
  end

  it 'should raise error if table does not exist' do
    expect { @db.get_first_value("SELECT AddGeometryColumn('test', 'geom', 0, 'point', 2)") } .to raise_error
  end

  it 'should raise error if column already exists' do
    expect(@db.get_first_value('CREATE TABLE test (id int)')).to eq(nil)
    expect { @db.get_first_value("SELECT AddGeometryColumn('test', 'id', 0, 'point', 2)") } .to raise_error
  end

  it 'should raise error if srid does not exists' do
    expect(@db.get_first_value('CREATE TABLE test (id int)')).to eq(nil)
    expect { @db.get_first_value("SELECT AddGeometryColumn('test', 'geom', 20, 'point', 2)") } .to raise_error
  end

  it 'should raise error if dimension is less than 2' do
    expect(@db.get_first_value('CREATE TABLE test (id int)')).to eq(nil)
    expect { @db.get_first_value("SELECT AddGeometryColumn('test', 'geom', 0, 'point', 1)") } .to raise_error
  end

  it 'should raise error if dimension is greater than 4' do
    expect(@db.get_first_value('CREATE TABLE test (id int)')).to eq(nil)
    expect { @db.get_first_value("SELECT AddGeometryColumn('test', 'geom', 0, 'point', 5)") } .to raise_error
  end
end