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
      expect("SELECT ST_Area(GeomFromText('Polygon((0 0, 2 0, 1 2, 0 0))'))").to have_result 2.0
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
      expect("SELECT ST_Length(GeomFromText('Polygon((0 0, 2 0, 1 2, 0 0))'))").to have_result 6.47213595499958
    end
  end

  describe 'ST_IsClosed' do
    it 'should return NULL when passed NULL' do
      expect('SELECT ST_IsClosed(NULL)').to have_result nil
    end

    it 'should raise an error on invalid input' do
      expect("SELECT ST_IsClosed(x'FFFFFFFFFF')").to raise_sql_error
      expect("SELECT ST_IsClosed(GeomFromText('Point(1 0)'))").to raise_sql_error
      expect("SELECT ST_IsClosed(GeomFromText('Polygon((0 0, 2 0, 1 2, 0 0))'))").to raise_sql_error

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
      expect("SELECT ST_IsSimple(GeomFromText('Polygon((0 0, 2 0, 1 2, 0 0))'))").to have_result 1
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
      expect("SELECT ST_IsRing(GeomFromText('Polygon((0 0, 2 0, 1 2, 0 0))'))").to have_result 0
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
      expect("SELECT ST_IsValid(GeomFromText('Polygon((0 0, 2 0, 1 2, 0 0))'))").to have_result 1
    end
  end

  describe 'ST_Disjoint' do
    it 'should return NULL when either argument is NULL' do
      expect("SELECT ST_Disjoint(NULL, GeomFromText('Polygon((0 0, 2 0, 1 2, 0 0))'))").to have_result nil
      expect("SELECT ST_Disjoint(GeomFromText('Polygon((0 0, 2 0, 1 2, 0 0))'), NULL)").to have_result nil
      expect("SELECT ST_Disjoint(NULL, NULL)").to have_result nil
    end

    it 'should raise an error on invalid input' do
      expect("SELECT ST_Disjoint(x'FFFFFFFFFF', GeomFromText('Polygon((0 0, 2 0, 1 2, 0 0))'))").to raise_sql_error
      expect("SELECT ST_Disjoint(GeomFromText('Polygon((0 0, 2 0, 1 2, 0 0))'), x'FFFFFFFFFF')").to raise_sql_error
      expect("SELECT ST_Disjoint(x'FFFFFFFFFF', x'FFFFFFFFFF')").to raise_sql_error
    end

    it 'should raise a valid value' do
      expect("SELECT ST_Disjoint(GeomFromText('Polygon((0 0, 2 0, 1 2, 0 0))'), GeomFromText('Polygon((0 0, 2 0, 1 2, 0 0))'))").to have_result 0
      expect("SELECT ST_Disjoint(GeomFromText('Polygon((0 10, 2 10, 1 12, 0 10))'), GeomFromText('Polygon((0 0, 2 0, 1 2, 0 0))'))").to have_result 1
    end
  end

  describe 'ST_Intersects' do
    it 'should return NULL when either argument is NULL' do
      expect("SELECT ST_Intersects(NULL, GeomFromText('Polygon((0 0, 2 0, 1 2, 0 0))'))").to have_result nil
      expect("SELECT ST_Intersects(GeomFromText('Polygon((0 0, 2 0, 1 2, 0 0))'), NULL)").to have_result nil
      expect("SELECT ST_Intersects(NULL, NULL)").to have_result nil
    end

    it 'should raise an error on invalid input' do
      expect("SELECT ST_Intersects(x'FFFFFFFFFF', GeomFromText('Polygon((0 0, 2 0, 1 2, 0 0))'))").to raise_sql_error
      expect("SELECT ST_Intersects(GeomFromText('Polygon((0 0, 2 0, 1 2, 0 0))'), x'FFFFFFFFFF')").to raise_sql_error
      expect("SELECT ST_Intersects(x'FFFFFFFFFF', x'FFFFFFFFFF')").to raise_sql_error
    end

    it 'should raise a valid value' do
      expect("SELECT ST_Intersects(GeomFromText('Polygon((0 0, 2 0, 1 2, 0 0))'), GeomFromText('Polygon((0 0, 2 0, 1 2, 0 0))'))").to have_result 1
      expect("SELECT ST_Intersects(GeomFromText('Polygon((0 10, 2 10, 1 12, 0 10))'), GeomFromText('Polygon((0 0, 2 0, 1 2, 0 0))'))").to have_result 0
    end
  end

  describe 'ST_Touches' do
    it 'should return NULL when either argument is NULL' do
      expect("SELECT ST_Touches(NULL, GeomFromText('Polygon((0 0, 2 0, 1 2, 0 0))'))").to have_result nil
      expect("SELECT ST_Touches(GeomFromText('Polygon((0 0, 2 0, 1 2, 0 0))'), NULL)").to have_result nil
      expect("SELECT ST_Touches(NULL, NULL)").to have_result nil
    end

    it 'should raise an error on invalid input' do
      expect("SELECT ST_Touches(x'FFFFFFFFFF', GeomFromText('Polygon((0 0, 2 0, 1 2, 0 0))'))").to raise_sql_error
      expect("SELECT ST_Touches(GeomFromText('Polygon((0 0, 2 0, 1 2, 0 0))'), x'FFFFFFFFFF')").to raise_sql_error
      expect("SELECT ST_Touches(x'FFFFFFFFFF', x'FFFFFFFFFF')").to raise_sql_error
    end

    it 'should raise a valid value' do
      expect("SELECT ST_Touches(GeomFromText('Polygon((0 0, 2 0, 0 2, 0 0))'), GeomFromText('Polygon((0 2, 2 0, 2 2, 0 2))'))").to have_result 1
      expect("SELECT ST_Touches(GeomFromText('Polygon((0 10, 2 10, 1 12, 0 10))'), GeomFromText('Polygon((0 0, 2 0, 1 2, 0 0))'))").to have_result 0
    end
  end

  describe 'ST_Crosses' do
    it 'should return NULL when either argument is NULL' do
      expect("SELECT ST_Crosses(NULL, GeomFromText('Polygon((0 0, 2 0, 1 2, 0 0))'))").to have_result nil
      expect("SELECT ST_Crosses(GeomFromText('Polygon((0 0, 2 0, 1 2, 0 0))'), NULL)").to have_result nil
      expect("SELECT ST_Crosses(NULL, NULL)").to have_result nil
    end

    it 'should raise an error on invalid input' do
      expect("SELECT ST_Crosses(x'FFFFFFFFFF', GeomFromText('Polygon((0 0, 2 0, 1 2, 0 0))'))").to raise_sql_error
      expect("SELECT ST_Crosses(GeomFromText('Polygon((0 0, 2 0, 1 2, 0 0))'), x'FFFFFFFFFF')").to raise_sql_error
      expect("SELECT ST_Crosses(x'FFFFFFFFFF', x'FFFFFFFFFF')").to raise_sql_error
    end

    it 'should raise a valid value' do
      expect("SELECT ST_Crosses(GeomFromText('LineString(0 0, 2 2)'), GeomFromText('LineString(0 2, 2 0)'))").to have_result 1
      expect("SELECT ST_Crosses(GeomFromText('LineString(0 0, 2 2)'), GeomFromText('LineString(10 2, 12 0)'))").to have_result 0
    end
  end

  describe 'ST_Within' do
    it 'should return NULL when either argument is NULL' do
      expect("SELECT ST_Within(NULL, GeomFromText('Polygon((0 0, 2 0, 1 2, 0 0))'))").to have_result nil
      expect("SELECT ST_Within(GeomFromText('Polygon((0 0, 2 0, 1 2, 0 0))'), NULL)").to have_result nil
      expect("SELECT ST_Within(NULL, NULL)").to have_result nil
    end

    it 'should raise an error on invalid input' do
      expect("SELECT ST_Within(x'FFFFFFFFFF', GeomFromText('Polygon((0 0, 2 0, 1 2, 0 0))'))").to raise_sql_error
      expect("SELECT ST_Within(GeomFromText('Polygon((0 0, 2 0, 1 2, 0 0))'), x'FFFFFFFFFF')").to raise_sql_error
      expect("SELECT ST_Within(x'FFFFFFFFFF', x'FFFFFFFFFF')").to raise_sql_error
    end

    it 'should raise a valid value' do
      expect("SELECT ST_Within(GeomFromText('Polygon((1 1, 2 1, 2 2, 1 2, 1 1))'), GeomFromText('Polygon((0 0, 3 0, 3 3, 0 3, 0 0))'))").to have_result 1
      expect("SELECT ST_Within(GeomFromText('Polygon((0 0, 3 0, 3 3, 0 3, 0 0))'), GeomFromText('Polygon((1 1, 2 1, 2 2, 1 2, 1 1))'))").to have_result 0
    end
  end

  describe 'ST_Contains' do
    it 'should return NULL when either argument is NULL' do
      expect("SELECT ST_Contains(NULL, GeomFromText('Polygon((0 0, 2 0, 1 2, 0 0))'))").to have_result nil
      expect("SELECT ST_Contains(GeomFromText('Polygon((0 0, 2 0, 1 2, 0 0))'), NULL)").to have_result nil
      expect("SELECT ST_Contains(NULL, NULL)").to have_result nil
    end

    it 'should raise an error on invalid input' do
      expect("SELECT ST_Contains(x'FFFFFFFFFF', GeomFromText('Polygon((0 0, 2 0, 1 2, 0 0))'))").to raise_sql_error
      expect("SELECT ST_Contains(GeomFromText('Polygon((0 0, 2 0, 1 2, 0 0))'), x'FFFFFFFFFF')").to raise_sql_error
      expect("SELECT ST_Contains(x'FFFFFFFFFF', x'FFFFFFFFFF')").to raise_sql_error
    end

    it 'should raise a valid value' do
      expect("SELECT ST_Contains(GeomFromText('Polygon((1 1, 2 1, 2 2, 1 2, 1 1))'), GeomFromText('Polygon((0 0, 3 0, 3 3, 0 3, 0 0))'))").to have_result 0
      expect("SELECT ST_Contains(GeomFromText('Polygon((0 0, 3 0, 3 3, 0 3, 0 0))'), GeomFromText('Polygon((1 1, 2 1, 2 2, 1 2, 1 1))'))").to have_result 1
    end
  end

  describe 'ST_Overlaps' do
    it 'should return NULL when either argument is NULL' do
      expect("SELECT ST_Overlaps(NULL, GeomFromText('Polygon((0 0, 2 0, 1 2, 0 0))'))").to have_result nil
      expect("SELECT ST_Overlaps(GeomFromText('Polygon((0 0, 2 0, 1 2, 0 0))'), NULL)").to have_result nil
      expect("SELECT ST_Overlaps(NULL, NULL)").to have_result nil
    end

    it 'should raise an error on invalid input' do
      expect("SELECT ST_Overlaps(x'FFFFFFFFFF', GeomFromText('Polygon((0 0, 2 0, 1 2, 0 0))'))").to raise_sql_error
      expect("SELECT ST_Overlaps(GeomFromText('Polygon((0 0, 2 0, 1 2, 0 0))'), x'FFFFFFFFFF')").to raise_sql_error
      expect("SELECT ST_Overlaps(x'FFFFFFFFFF', x'FFFFFFFFFF')").to raise_sql_error
    end

    it 'should raise a valid value' do
      expect("SELECT ST_Overlaps(GeomFromText('Polygon((1 1, 2 1, 2 2, 1 2, 1 1))'), GeomFromText('Polygon((0 0, 3 0, 3 3, 0 3, 0 0))'))").to have_result 0
      expect("SELECT ST_Overlaps(GeomFromText('Polygon((0 0, 3 0, 3 3, 0 3, 0 0))'),GeomFromText('Polygon((1 0, 4 0, 4 3, 1 3, 1 0))'))").to have_result 1
    end
  end

  describe 'ST_Equals' do
    it 'should return NULL when either argument is NULL' do
      expect("SELECT ST_Equals(NULL, GeomFromText('Polygon((0 0, 2 0, 1 2, 0 0))'))").to have_result nil
      expect("SELECT ST_Equals(GeomFromText('Polygon((0 0, 2 0, 1 2, 0 0))'), NULL)").to have_result nil
      expect("SELECT ST_Equals(NULL, NULL)").to have_result nil
    end

    it 'should raise an error on invalid input' do
      expect("SELECT ST_Equals(x'FFFFFFFFFF', GeomFromText('Polygon((0 0, 2 0, 1 2, 0 0))'))").to raise_sql_error
      expect("SELECT ST_Equals(GeomFromText('Polygon((0 0, 2 0, 1 2, 0 0))'), x'FFFFFFFFFF')").to raise_sql_error
      expect("SELECT ST_Equals(x'FFFFFFFFFF', x'FFFFFFFFFF')").to raise_sql_error
    end

    it 'should raise a valid value' do
      expect("SELECT ST_Equals(GeomFromText('Polygon((0 0, 2 0, 0 2, 0 0))'), GeomFromText('Polygon((0 0, 2 0, 0 2, 0 0))'))").to have_result 1
      expect("SELECT ST_Equals(GeomFromText('Polygon((0 10, 2 10, 1 12, 0 10))'), GeomFromText('Polygon((0 0, 2 0, 1 2, 0 0))'))").to have_result 0
    end
  end

  describe 'ST_Covers' do
    it 'should return NULL when either argument is NULL' do
      expect("SELECT ST_Covers(NULL, GeomFromText('Polygon((0 0, 2 0, 1 2, 0 0))'))").to have_result nil
      expect("SELECT ST_Covers(GeomFromText('Polygon((0 0, 2 0, 1 2, 0 0))'), NULL)").to have_result nil
      expect("SELECT ST_Covers(NULL, NULL)").to have_result nil
    end

    it 'should raise an error on invalid input' do
      expect("SELECT ST_Covers(x'FFFFFFFFFF', GeomFromText('Polygon((0 0, 2 0, 1 2, 0 0))'))").to raise_sql_error
      expect("SELECT ST_Covers(GeomFromText('Polygon((0 0, 2 0, 1 2, 0 0))'), x'FFFFFFFFFF')").to raise_sql_error
      expect("SELECT ST_Covers(x'FFFFFFFFFF', x'FFFFFFFFFF')").to raise_sql_error
    end

    it 'should raise a valid value' do
      expect("SELECT ST_Covers(GeomFromText('Polygon((1 1, 2 1, 2 2, 1 2, 1 1))'), GeomFromText('Polygon((0 0, 3 0, 3 3, 0 3, 0 0))'))").to have_result 0
      expect("SELECT ST_Covers(GeomFromText('Polygon((0 0, 3 0, 3 3, 0 3, 0 0))'),GeomFromText('Polygon((1 1, 2 1, 2 2, 1 2, 1 1))'))").to have_result 1
    end
  end

  describe 'ST_CoveredBy' do
    it 'should return NULL when either argument is NULL' do
      expect("SELECT ST_CoveredBy(NULL, GeomFromText('Polygon((0 0, 2 0, 1 2, 0 0))'))").to have_result nil
      expect("SELECT ST_CoveredBy(GeomFromText('Polygon((0 0, 2 0, 1 2, 0 0))'), NULL)").to have_result nil
      expect("SELECT ST_CoveredBy(NULL, NULL)").to have_result nil
    end

    it 'should raise an error on invalid input' do
      expect("SELECT ST_CoveredBy(x'FFFFFFFFFF', GeomFromText('Polygon((0 0, 2 0, 1 2, 0 0))'))").to raise_sql_error
      expect("SELECT ST_CoveredBy(GeomFromText('Polygon((0 0, 2 0, 1 2, 0 0))'), x'FFFFFFFFFF')").to raise_sql_error
      expect("SELECT ST_CoveredBy(x'FFFFFFFFFF', x'FFFFFFFFFF')").to raise_sql_error
    end

    it 'should raise a valid value' do
      expect("SELECT ST_CoveredBy(GeomFromText('Polygon((1 1, 2 1, 2 2, 1 2, 1 1))'), GeomFromText('Polygon((0 0, 3 0, 3 3, 0 3, 0 0))'))").to have_result 1
      expect("SELECT ST_CoveredBy(GeomFromText('Polygon((0 0, 3 0, 3 3, 0 3, 0 0))'),GeomFromText('Polygon((1 1, 2 1, 2 2, 1 2, 1 1))'))").to have_result 0
    end
  end

  describe 'ST_Distance' do
    it 'should return NULL when either argument is NULL' do
      expect("SELECT ST_Distance(NULL, GeomFromText('Polygon((0 0, 2 0, 1 2, 0 0))'))").to have_result nil
      expect("SELECT ST_Distance(GeomFromText('Polygon((0 0, 2 0, 1 2, 0 0))'), NULL)").to have_result nil
      expect("SELECT ST_Distance(NULL, NULL)").to have_result nil
    end

    it 'should raise an error on invalid input' do
      expect("SELECT ST_Distance(x'FFFFFFFFFF', GeomFromText('Polygon((0 0, 2 0, 1 2, 0 0))'))").to raise_sql_error
      expect("SELECT ST_Distance(GeomFromText('Polygon((0 0, 2 0, 1 2, 0 0))'), x'FFFFFFFFFF')").to raise_sql_error
      expect("SELECT ST_Distance(x'FFFFFFFFFF', x'FFFFFFFFFF')").to raise_sql_error
    end

    it 'should raise a valid value' do
      expect("SELECT ST_Distance(GeomFromText('Point(0 0)'), GeomFromText('Point(0 2)'))").to have_result 2.0
    end
  end
  
  # TODO
  # ST_HausdorffDistance
  # ST_Relate
  # ST_Boundary
  # ST_ConvexHull
  # ST_Envelope
  # ST_Difference
  # ST_SymDifference
  # ST_Intersection
  # ST_Union
end