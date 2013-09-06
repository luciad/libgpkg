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

describe 'GPKG_IsAssignable' do
  ASSIGNABLE = 'SELECT GPKG_IsAssignable(?, ?)'

  TYPES = {
      :geometry => nil,
      :point => :geometry,
      :curve => :geometry,
      :linestring => :curve,
      :surface => :geometry,
      :curvepolygon => :surface,
      :polygon => :curvepolygon,
      :geometrycollection => :geometry,
      :multisurface => :geometrycollection,
      :multipolygon => :multisurface,
      :multicurve => :geometrycollection,
      :multilinestring => :multicurve,
      :multipoint => :geometrycollection,
  }

  RELATIONSHIPS = TYPES.inject({}) do |map, type|
    TYPES.each_key do |other_type|
      map[[other_type, type[0]]] = false
    end
    map
  end

  TYPES.each_key do |type|
    super_type = type
    until super_type.nil? do
      RELATIONSHIPS[[type, super_type]] = true
      super_type = TYPES[super_type]
    end
  end

  RELATIONSHIPS.each do |type_pair, assignable|
    actual_type = type_pair[0].to_s
    expected_type = type_pair[1].to_s
    it "should consider #{actual_type} #{assignable ? 'assignable' : 'not assignable'} to #{expected_type}" do
      expect(query('SELECT GPKG_IsAssignable(?, ?)', expected_type, actual_type)).to have_result( assignable ? 1 : 0 )
    end
  end
end