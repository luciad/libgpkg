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
    expect('SELECT ST_MinX(NULL)').to have_result nil
  end

  it 'should return the X coordinate of a point' do
    expect("SELECT ST_MinX(GeomFromText('Point(1 0)'))").to have_result 1.0
  end

  it 'should return the minimum X coordinate of a line string' do
    expect("SELECT ST_MinX(GeomFromText('LineString(-1 0, 5 0)'))").to have_result -1.0
  end

  it 'should return the minimum X coordinate of a circular string' do
    expect("SELECT ST_MinX(GeomFromText('CircularString(10 0, 0 10, -10 0)'))").to have_result -10.0
  end

  it 'should return the minimum X coordinate of a circular string' do
    expect("SELECT ST_MinX(GeomFromText('CircularString(10 0, 0 10, -10 0, 0 -10, 10 0)'))").to have_result -10.0
  end

  it 'should return the minimum X coordinate of a circular string' do
    expect("SELECT ST_MinX(GeomFromText('circularstring( 0 0, 10 10, 20 0, 30 10, 40 0, 50 -10, 60 0, 70 10, 80 0, 100 20, 120 0) '))").to have_result 0.0
  end

  it 'should return the minimum X coordinate of a circular string' do
    expect("SELECT ST_MinX(GeomFromText('circularstring z( 0 0 10, 10 10 10, 20 0 10, 30 10 20, 40 0 20, 50 -10 30, 60 0 30, 70 10 30, 80 0 10, 100 20 20, 120 0 -10) '))").to have_result 0.0
  end


  if mode == :gpkg
    it 'should return the value from the GPB header' do
      expect("SELECT ST_MinX(x'475000030000000095950D08000014C097950D0800001C4059AFB70700002AC05BAFB707000024400103000000010000000300000000000000000014C000000000000024400000000000001C400000000000002AC000000000000014C00000000000002440')").
          to have_result -5.00000012
    end
  end
end

describe 'ST_MaxX' do
  it 'should return NULL when passed NULL' do
    expect('SELECT ST_MaxX(NULL)').to have_result nil
  end

  it 'should return the X coordinate of a point' do
    expect("SELECT ST_MaxX(GeomFromText('Point(1 0)'))").to have_result 1.0
  end

  it 'should return the maximum X coordinate of a line string' do
    expect("SELECT ST_MaxX(GeomFromText('LineString(-1 0, 5 0)'))").to have_result 5.0
  end

  it 'should return the maximum X coordinate of a circular string' do
    expect("SELECT ST_MaxX(GeomFromText('CircularString(10 0, 0 10, -10 0)'))").to have_result 10.0
  end

  it 'should return the maximum X coordinate of a circular string' do
    expect("SELECT ST_MaxX(GeomFromText('CircularString(10 0, 0 10, -10 0, 0 -10, 10 0)'))").to have_result 10.0
  end

  it 'should return the maximum X coordinate of a circular string' do
    expect("SELECT ST_MaxX(GeomFromText('CircularString(10 0, 0 10, -10 0, 0 -10, 10 0, 20 10, 30 0)'))").to have_result 30.0
  end

  it 'should return the maximum X coordinate of a circular string' do
    expect("SELECT ST_MaxX(GeomFromText('circularstring( 0 0, 10 10, 20 0, 30 10, 40 0, 50 -10, 60 0, 70 10, 80 0, 100 20, 120 0) '))").to have_result 120.0
  end

  it 'should return the maximum X coordinate of a circular string' do
    expect("SELECT ST_MaxX(GeomFromText('circularstring z( 0 0 10, 10 10 10, 20 0 10, 30 10 20, 40 0 20, 50 -10 30, 60 0 30, 70 10 30, 80 0 10, 100 20 20, 120 0 -10) '))").to have_result 120.0
  end

  if mode == :gpkg
    it 'should return the value from the GPB header' do
      expect("SELECT ST_MaxX(x'475000030000000095950D08000014C097950D0800001C4059AFB70700002AC05BAFB707000024400103000000010000000300000000000000000014C000000000000024400000000000001C400000000000002AC000000000000014C00000000000002440')").
          to have_result 7.000000120000002
    end
  end
end

describe 'ST_MinY' do
  it 'should return NULL when passed NULL' do
    expect('SELECT ST_MinY(NULL)').to have_result nil
  end

  it 'should return the X coordinate of a point' do
    expect("SELECT ST_MinY(GeomFromText('Point(1 5)'))").to have_result 5.0
  end

  it 'should return the minimum Y coordinate of a line string' do
    expect("SELECT ST_MinY(GeomFromText('LineString(-1 7, 5 -3)'))").to have_result -3.0
  end


  it 'should return the minimum Y coordinate of a circular string' do
    expect("SELECT ST_MinY(GeomFromText('CircularString(10 0, 0 10, -10 0)'))").to have_result 0.0
  end

  it 'should return the minimum Y coordinate of a circular string' do
    expect("SELECT ST_MinY(GeomFromText('CircularString(10 0, 0 10, -10 0, 0 -10, 10 0)'))").to have_result -10.0
  end

  it 'should return the minimum Y coordinate of a circular string' do
    expect("SELECT ST_MinY(GeomFromText('circularstring( 0 0, 10 10, 20 0, 30 10, 40 0, 50 -10, 60 0, 70 10, 80 0, 100 20, 120 0) '))").to have_result -10.0
  end

  it 'should return the minimum Y coordinate of a circular string' do
    expect("SELECT ST_MinY(GeomFromText('circularstring z( 0 0 10, 10 10 10, 20 0 10, 30 10 20, 40 0 20, 50 -10 30, 60 0 30, 70 10 30, 80 0 10, 100 20 20, 120 0 -10) '))").to have_result -10.0
  end

  if mode == :gpkg
    it 'should return the value from the GPB header' do
      expect("SELECT ST_MinY(x'475000030000000095950D08000014C097950D0800001C4059AFB70700002AC05BAFB707000024400103000000010000000300000000000000000014C000000000000024400000000000001C400000000000002AC000000000000014C00000000000002440')").
          to have_result -13.00000023
    end
  end
end

describe 'ST_MaxY' do
  it 'should return NULL when passed NULL' do
    expect('SELECT ST_MaxY(NULL)').to have_result nil
  end

  it 'should return the X coordinate of a point' do
    expect("SELECT ST_MaxY(GeomFromText('Point(1 5)'))").to have_result 5.0
  end

  it 'should return the maximum Y coordinate of a line string' do
    expect("SELECT ST_MaxY(GeomFromText('LineString(-1 7, 5 -3)'))").to have_result 7.0
  end

  it 'should return the maximum Y coordinate of a circular string' do
    expect("SELECT ST_MaxY(GeomFromText('CircularString(10 0, 0 10, -10 0)'))").to have_result 10.0
  end

  it 'should return the maximum Y coordinate of a circular string' do
    expect("SELECT ST_MaxY(GeomFromText('CircularString(10 0, 0 10, -10 0, 0 -10, 10 0)'))").to have_result 10.0
  end

  it 'should return the maximum Y coordinate of a circular string' do
    expect("SELECT ST_MaxY(GeomFromText('CircularString(10 0, 0 10, -10 0, 0 -10, 10 0, 30 20, 50 0)'))").to have_result 20.0
  end

  it 'should return the maximum Y coordinate of a circular string' do
    expect("SELECT ST_MaxY(GeomFromText('circularstring( 0 0, 10 10, 20 0, 30 10, 40 0, 50 -10, 60 0, 70 10, 80 0, 100 20, 120 0) '))").to have_result 20.0
  end

  it 'should return the maximum Y coordinate of a circular string' do
    expect("SELECT ST_MaxY(GeomFromText('circularstring z( 0 0 10, 10 10 10, 20 0 10, 30 10 20, 40 0 20, 50 -10 30, 60 0 30, 70 10 30, 80 0 10, 100 20 20, 120 0 -10) '))").to have_result 20.0
  end

  it 'should return the maximum Y coordinate of a circular string' do
    expect("SELECT ST_MaxY(GeomFromText('compoundcurve z(circularstring z( 0 0 10, 10 10 10, 20 0 10, 30 10 20, 40 0 20, 50 -10 30, 60 0 30, 70 10 30, 80 0 10, 100 20 20, 120 0 -10), (0 0 0, 10 10 0, -10 10 2, 50 50 4)) '))").to have_result 50.0
  end

  if mode == :gpkg
    it 'should return the value from the GPB header' do
      expect("SELECT ST_MaxY(x'475000030000000095950D08000014C097950D0800001C4059AFB70700002AC05BAFB707000024400103000000010000000300000000000000000014C000000000000024400000000000001C400000000000002AC000000000000014C00000000000002440')").
          to have_result 10.000000230000003
    end
  end
end

describe 'ST_MinM' do
  it 'should return NULL when passed NULL' do
    expect('SELECT ST_MinM(NULL)').to have_result nil
  end

  it 'should return the M coordinate of a point' do
    expect("SELECT ST_MinM(GeomFromText('Point M (1 5 4)'))").to have_result 4.0
  end

  it 'should return NULL if M is undefined' do
    expect("SELECT ST_MinM(GeomFromText('Point Z (1 5 4)'))").to have_result nil
  end
end

describe 'ST_MaxM' do
  it 'should return NULL when passed NULL' do
    expect('SELECT ST_MaxM(NULL)').to have_result nil
  end

  it 'should return the M coordinate of a point' do
    expect("SELECT ST_MaxM(GeomFromText('Point M(1 5 4)'))").to have_result 4.0
  end

  it 'should return NULL if M is undefined' do
    expect("SELECT ST_MaxM(GeomFromText('Point Z (1 5 4)'))").to have_result nil
  end
end

describe 'ST_MinZ' do
  it 'should return NULL when passed NULL' do
    expect('SELECT ST_MinZ(NULL)').to have_result nil
  end

  it 'should return the Z coordinate of a point' do
    expect("SELECT ST_MinZ(GeomFromText('Point Z (1 5 4)'))").to have_result 4.0
  end

  it 'should return NULL if Z is undefined' do
    expect("SELECT ST_MinZ(GeomFromText('Point M (1 5 4)'))").to have_result nil
  end

  it 'should return NULL if Z is undefined' do
    expect("SELECT ST_MinZ(GeomFromText('CircularString(10 0, 0 10, -10 0, 0 -10, 10 0, 30 20, 50 0)'))").to have_result nil
  end

  it 'should return the minimum Z coordinate of a circular string' do
    expect("SELECT ST_MinZ(GeomFromText('circularstring z( 0 0 10, 10 10 10, 20 0 10, 30 10 20, 40 0 20, 50 -10 30, 60 0 30, 70 10 30, 80 0 10, 100 20 20, 120 0 -10) '))").to have_result -10.0
  end
end

describe 'ST_MaxZ' do
  it 'should return NULL when passed NULL' do
    expect('SELECT ST_MaxZ(NULL)').to have_result nil
  end

  it 'should return the Z coordinate of a point' do
    expect("SELECT ST_MaxZ(GeomFromText('Point Z (1 5 4)'))").to have_result 4.0
  end

  it 'should return NULL if Z is undefined' do
    expect("SELECT ST_MaxZ(GeomFromText('Point M (1 5 4)'))").to have_result nil
  end
end