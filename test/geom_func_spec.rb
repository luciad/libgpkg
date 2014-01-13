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

if ENV['GPKG_HAVE_GEOM_FUNC']
  describe 'ST_Area' do
    it 'should return NULL when passed NULL' do
      expect('SELECT ST_Area(NULL)').to have_result nil
    end

    it 'should raise an error on invalid input' do
      expect("SELECT ST_Area(x'FFFFFFFFFF')").to raise_sql_error
    end

    it 'should return zero for 0 and 1 dimensional geometry' do
      expect("SELECT ST_Area(GeomFromText('Point(1 0)'))").to have_result 0
      expect("SELECT ST_Area(GeomFromText('LineString(1 1, 2 2)'))").to have_result 0
    end

    it 'should return a valid value for 2 dimensional geometry' do
      expect("SELECT ST_Area(GeomFromText('Polygon((0 0, 2 0,  1 2, 0 0))'))").to have_result 2.0
    end
  end

  describe 'ST_Length' do
    it 'should return NULL when passed NULL' do
      expect('SELECT ST_Length(NULL)').to have_result nil
    end

    it 'should raise an error on invalid input' do
      expect("SELECT ST_Length(x'FFFFFFFFFF')").to raise_sql_error
    end

    it 'should return zero for 0 dimensional geometry' do
      expect("SELECT ST_Area(GeomFromText('Point(1 0)'))").to have_result 0
    end

    it 'should return a valid value for 1 and 2 dimensional geometry' do
      expect("SELECT ST_Length(GeomFromText('LineString(1 1, 2 2)'))").to have_result 1.4142135623730951
      expect("SELECT ST_Length(GeomFromText('Polygon((0 0, 2 0,  1 2, 0 0))'))").to have_result 6.47213595499958
    end
  end

  describe 'ST_IsClosed' do
    it 'should return NULL when passed NULL' do
      expect('SELECT ST_IsClosed(NULL)').to have_result nil
    end

    it 'should raise an error on invalid input' do
      expect("SELECT ST_IsClosed(x'FFFFFFFFFF')").to raise_sql_error
      expect("SELECT ST_IsClosed(GeomFromText('Point(1 0)'))").to raise_sql_error
      expect("SELECT ST_IsClosed(GeomFromText('Polygon((0 0, 2 0,  1 2, 0 0))'))").to raise_sql_error

    end

    it 'should return a valid value' do
      expect("SELECT ST_IsClosed(GeomFromText('LineString(1 1, 2 2)'))").to have_result 0
    end
  end

  describe 'ST_IsSimple' do
    it 'should return NULL when passed NULL' do
      expect('SELECT ST_IsSimple(NULL)').to have_result nil
    end

    it 'should raise an error on invalid input' do
      expect("SELECT ST_IsSimple(x'FFFFFFFFFF')").to raise_sql_error
    end

    it 'should return a valid value' do
      expect("SELECT ST_IsSimple(GeomFromText('Point(1 0)'))").to have_result 1
      expect("SELECT ST_IsSimple(GeomFromText('LineString(1 1, 2 2)'))").to have_result 1
      expect("SELECT ST_IsSimple(GeomFromText('Polygon((0 0, 2 0,  1 2, 0 0))'))").to have_result 1
    end
  end

  describe 'ST_IsRing' do
    it 'should return NULL when passed NULL' do
      expect('SELECT ST_IsRing(NULL)').to have_result nil
    end

    it 'should raise an error on invalid input' do
      expect("SELECT ST_IsRing(x'FFFFFFFFFF')").to raise_sql_error
    end

    it 'should return a valid value' do
      expect("SELECT ST_IsRing(GeomFromText('Point(1 0)'))").to have_result 0
      expect("SELECT ST_IsRing(GeomFromText('LineString(1 1, 2 2)'))").to have_result 0
      expect("SELECT ST_IsRing(GeomFromText('Polygon((0 0, 2 0,  1 2, 0 0))'))").to have_result 0
    end
  end

  describe 'ST_IsValid' do
    it 'should return NULL when passed NULL' do
      expect('SELECT ST_IsValid(NULL)').to have_result nil
    end

    it 'should raise an error on invalid input' do
      expect("SELECT ST_IsValid(x'FFFFFFFFFF')").to raise_sql_error
    end

    it 'should return a valid value' do
      expect("SELECT ST_IsValid(GeomFromText('Point(1 0)'))").to have_result 1
      expect("SELECT ST_IsValid(GeomFromText('LineString(1 1, 2 2)'))").to have_result 1
      expect("SELECT ST_IsValid(GeomFromText('Polygon((0 0, 2 0,  1 2, 0 0))'))").to have_result 1
    end
  end

  # TODO
  # ST_Disjoint
  # ST_Intersects
  # ST_Touches
  # ST_Crosses
  # ST_Within
  # ST_Contains
  # ST_Overlaps
  # ST_Equals
  # ST_Relate
  # ST_Covers
  # ST_CoveredBy
  # ST_Distance
  # ST_HausdorffDistance
  # ST_Boundary
  # ST_ConvexHull
  # ST_Envelope
  # ST_Difference
  # ST_SymDifference
  # ST_Intersection
  # ST_Union
end