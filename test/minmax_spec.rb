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

describe 'ST_MinX' do
  it 'should return NULL when passed NULL' do
    expect(@db.get_first_value('SELECT ST_MinX(NULL)')).to eq(nil)
  end

  it 'should return the X coordinate of a point' do
    expect(@db.get_first_value("SELECT ST_MinX(GeomFromText('Point(1 0)'))")).to eq(1.0)
  end
end

describe 'ST_MaxX' do
  it 'should return NULL when passed NULL' do
    expect(@db.get_first_value('SELECT ST_MaxX(NULL)')).to eq(nil)
  end

  it 'should return the X coordinate of a point' do
    expect(@db.get_first_value("SELECT ST_MaxX(GeomFromText('Point(1 0)'))")).to eq(1.0)
  end
end

describe 'ST_MinY' do
  it 'should return NULL when passed NULL' do
    expect(@db.get_first_value('SELECT ST_MinY(NULL)')).to eq(nil)
  end

  it 'should return the X coordinate of a point' do
    expect(@db.get_first_value("SELECT ST_MinY(GeomFromText('Point(1 5)'))")).to eq(5.0)
  end
end

describe 'ST_MaxY' do
  it 'should return NULL when passed NULL' do
    expect(@db.get_first_value('SELECT ST_MaxY(NULL)')).to eq(nil)
  end

  it 'should return the X coordinate of a point' do
    expect(@db.get_first_value("SELECT ST_MaxY(GeomFromText('Point(1 5)'))")).to eq(5.0)
  end
end

describe 'ST_MinM' do
  it 'should return NULL when passed NULL' do
    expect(@db.get_first_value('SELECT ST_MinM(NULL)')).to eq(nil)
  end

  it 'should return the M coordinate of a point' do
    expect(@db.get_first_value("SELECT ST_MinM(GeomFromText('Point M (1 5 4)'))")).to eq(4.0)
  end

  it 'should return NULL if M is undefined' do
    expect(@db.get_first_value("SELECT ST_MinM(GeomFromText('Point Z (1 5 4)'))")).to eq(nil)
  end
end

describe 'ST_MaxM' do
  it 'should return NULL when passed NULL' do
    expect(@db.get_first_value('SELECT ST_MaxM(NULL)')).to eq(nil)
  end

  it 'should return the M coordinate of a point' do
    expect(@db.get_first_value("SELECT ST_MaxM(GeomFromText('Point M(1 5 4)'))")).to eq(4.0)
  end

  it 'should return NULL if M is undefined' do
    expect(@db.get_first_value("SELECT ST_MaxM(GeomFromText('Point Z (1 5 4)'))")).to eq(nil)
  end
end

describe 'ST_MinZ' do
  it 'should return NULL when passed NULL' do
    expect(@db.get_first_value('SELECT ST_MinZ(NULL)')).to eq(nil)
  end

  it 'should return the Z coordinate of a point' do
    expect(@db.get_first_value("SELECT ST_MinZ(GeomFromText('Point Z (1 5 4)'))")).to eq(4.0)
  end

  it 'should return NULL if Z is undefined' do
    expect(@db.get_first_value("SELECT ST_MinZ(GeomFromText('Point M (1 5 4)'))")).to eq(nil)
  end
end

describe 'ST_MaxZ' do
  it 'should return NULL when passed NULL' do
    expect(@db.get_first_value('SELECT ST_MaxZ(NULL)')).to eq(nil)
  end

  it 'should return the Z coordinate of a point' do
    expect(@db.get_first_value("SELECT ST_MaxZ(GeomFromText('Point Z (1 5 4)'))")).to eq(4.0)
  end

  it 'should return NULL if Z is undefined' do
    expect(@db.get_first_value("SELECT ST_MaxZ(GeomFromText('Point M (1 5 4)'))")).to eq(nil)
  end
end