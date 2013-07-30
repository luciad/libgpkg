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
    expect(result_of('SELECT InitGpkg()')).to eq(nil)
    expect(result_of('CREATE TABLE test (id int)')).to eq(nil)
    expect(result_of("SELECT AddGeometryColumn('test', 'geom', 'point', 0)")).to eq(nil)
  end

  it 'should succeed even if InitGpkg was not called' do
    expect(result_of('CREATE TABLE test (id int)')).to eq(nil)
    expect(result_of("SELECT AddGeometryColumn('test', 'geom', 'point', 0)")).to eq(nil)
  end

  it 'should support all geometry types' do
    expect(result_of('SELECT InitGpkg()')).to eq(nil)
    expect(result_of('CREATE TABLE test (id int)')).to eq(nil)
    expect(result_of("SELECT AddGeometryColumn('test', 'geom1', 'point', 0)")).to eq(nil)
    expect(result_of("SELECT AddGeometryColumn('test', 'geom2', 'polygon', 0)")).to eq(nil)
    expect(result_of("SELECT AddGeometryColumn('test', 'geom3', 'linestring', 0)")).to eq(nil)
    expect(result_of("SELECT AddGeometryColumn('test', 'geom4', 'multipoint', 0)")).to eq(nil)
    expect(result_of("SELECT AddGeometryColumn('test', 'geom5', 'multipolygon', 0)")).to eq(nil)
    expect(result_of("SELECT AddGeometryColumn('test', 'geom6', 'multilinestring', 0)")).to eq(nil)
    expect(result_of("SELECT AddGeometryColumn('test', 'geom7', 'geometrycollection', 0)")).to eq(nil)
    expect(result_of("SELECT AddGeometryColumn('test', 'geom8', 'geometry', 0)")).to eq(nil)

    expect(table_structure_of('test')).to eq({
                                                 'id' => {:index => 0, :type => 'int', :not_null => false, :default => nil, :primary_key => false},
                                                 'geom1' => {:index => 1, :type => 'point', :not_null => false, :default => nil, :primary_key => false},
                                                 'geom2' => {:index => 2, :type => 'polygon', :not_null => false, :default => nil, :primary_key => false},
                                                 'geom3' => {:index => 3, :type => 'linestring', :not_null => false, :default => nil, :primary_key => false},
                                                 'geom4' => {:index => 4, :type => 'multipoint', :not_null => false, :default => nil, :primary_key => false},
                                                 'geom5' => {:index => 5, :type => 'multipolygon', :not_null => false, :default => nil, :primary_key => false},
                                                 'geom6' => {:index => 6, :type => 'multilinestring', :not_null => false, :default => nil, :primary_key => false},
                                                 'geom7' => {:index => 7, :type => 'geometrycollection', :not_null => false, :default => nil, :primary_key => false},
                                                 'geom8' => {:index => 8, :type => 'geometry', :not_null => false, :default => nil, :primary_key => false}
                                             })
  end

  it 'should raise error on unsupported geometry type' do
    expect(result_of('SELECT InitGpkg()')).to eq(nil)
    expect(result_of('CREATE TABLE test (id int)')).to eq(nil)
    expect { execute("SELECT AddGeometryColumn('test', 'geom', 'circularstring', 0)") }.to raise_error
  end

  it 'should raise error if table does not exist' do
    expect { execute("SELECT AddGeometryColumn('test', 'geom', 'point', 0)") }.to raise_error
  end

  it 'should raise error if column already exists' do
    expect(result_of('CREATE TABLE test (id int)')).to eq(nil)
    expect { execute("SELECT AddGeometryColumn('test', 'id', 'point', 0)") }.to raise_error
  end

  it 'should raise error if srid does not exists' do
    expect(result_of('CREATE TABLE test (id int)')).to eq(nil)
    expect { execute("SELECT AddGeometryColumn('test', 'geom', 'point', 20)") }.to raise_error
  end
end