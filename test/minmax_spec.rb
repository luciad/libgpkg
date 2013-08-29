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
    execute 'SELECT ST_MinX(NULL)'
  end

  it 'should return the X coordinate of a point' do
    execute "SELECT ST_MinX(GeomFromText('Point(1 0)'))", :expect => 1.0
  end

  it 'should return the minimum X coordinate of a line string' do
    execute "SELECT ST_MinX(GeomFromText('LineString(-1 0, 5 0)'))", :expect => -1.0
  end

  it 'should return the value from the GPB header' do
    execute "SELECT ST_MinX(x'475000030000000095950D08000014C097950D0800001C4059AFB70700002AC05BAFB707000024400103000000010000000300000000000000000014C000000000000024400000000000001C400000000000002AC000000000000014C00000000000002440')", :expect => -5.00000012
  end
end

describe 'ST_MaxX' do
  it 'should return NULL when passed NULL' do
    execute 'SELECT ST_MaxX(NULL)'
  end

  it 'should return the X coordinate of a point' do
    execute "SELECT ST_MaxX(GeomFromText('Point(1 0)'))", :expect => 1.0
  end

  it 'should return the maximum X coordinate of a line string' do
    execute "SELECT ST_MaxX(GeomFromText('LineString(-1 0, 5 0)'))", :expect => 5.0
  end

  it 'should return the value from the GPB header' do
    execute "SELECT ST_MaxX(x'475000030000000095950D08000014C097950D0800001C4059AFB70700002AC05BAFB707000024400103000000010000000300000000000000000014C000000000000024400000000000001C400000000000002AC000000000000014C00000000000002440')", :expect => 7.000000120000002
  end
end

describe 'ST_MinY' do
  it 'should return NULL when passed NULL' do
    execute 'SELECT ST_MinY(NULL)'
  end

  it 'should return the X coordinate of a point' do
    execute "SELECT ST_MinY(GeomFromText('Point(1 5)'))", :expect => 5.0
  end

  it 'should return the minimum Y coordinate of a line string' do
    execute "SELECT ST_MinY(GeomFromText('LineString(-1 7, 5 -3)'))", :expect => -3.0
  end

  it 'should return the value from the GPB header' do
    execute "SELECT ST_MinY(x'475000030000000095950D08000014C097950D0800001C4059AFB70700002AC05BAFB707000024400103000000010000000300000000000000000014C000000000000024400000000000001C400000000000002AC000000000000014C00000000000002440')", :expect => -13.00000023
  end
end

describe 'ST_MaxY' do
  it 'should return NULL when passed NULL' do
    execute 'SELECT ST_MaxY(NULL)'
  end

  it 'should return the X coordinate of a point' do
    execute "SELECT ST_MaxY(GeomFromText('Point(1 5)'))", :expect => 5.0
  end

  it 'should return the maximum Y coordinate of a line string' do
    execute "SELECT ST_MaxY(GeomFromText('LineString(-1 7, 5 -3)'))", :expect => 7.0
  end

  it 'should return the value from the GPB header' do
    execute "SELECT ST_MaxY(x'475000030000000095950D08000014C097950D0800001C4059AFB70700002AC05BAFB707000024400103000000010000000300000000000000000014C000000000000024400000000000001C400000000000002AC000000000000014C00000000000002440')", :expect => 10.000000230000003
  end
end

describe 'ST_MinM' do
  it 'should return NULL when passed NULL' do
    execute 'SELECT ST_MinM(NULL)'
  end

  it 'should return the M coordinate of a point' do
    execute "SELECT ST_MinM(GeomFromText('Point M (1 5 4)'))", :expect => 4.0
  end

  it 'should return NULL if M is undefined' do
    execute "SELECT ST_MinM(GeomFromText('Point Z (1 5 4)'))"
  end
end

describe 'ST_MaxM' do
  it 'should return NULL when passed NULL' do
    execute 'SELECT ST_MaxM(NULL)'
  end

  it 'should return the M coordinate of a point' do
    execute "SELECT ST_MaxM(GeomFromText('Point M(1 5 4)'))", :expect => 4.0
  end

  it 'should return NULL if M is undefined' do
    execute "SELECT ST_MaxM(GeomFromText('Point Z (1 5 4)'))"
  end
end

describe 'ST_MinZ' do
  it 'should return NULL when passed NULL' do
    execute 'SELECT ST_MinZ(NULL)'
  end

  it 'should return the Z coordinate of a point' do
    execute "SELECT ST_MinZ(GeomFromText('Point Z (1 5 4)'))", :expect => 4.0
  end

  it 'should return NULL if Z is undefined' do
    execute "SELECT ST_MinZ(GeomFromText('Point M (1 5 4)'))"
  end
end

describe 'ST_MaxZ' do
  it 'should return NULL when passed NULL' do
    execute 'SELECT ST_MaxZ(NULL)'
  end

  it 'should return the Z coordinate of a point' do
    execute "SELECT ST_MaxZ(GeomFromText('Point Z (1 5 4)'))", :expect => 4.0
  end

  it 'should return NULL if Z is undefined' do
    execute "SELECT ST_MaxZ(GeomFromText('Point M (1 5 4)'))"
  end
end