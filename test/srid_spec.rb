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

describe 'ST_SRID' do
  it 'should return NULL when passed NULL' do
    expect('SELECT ST_SRID(NULL)').to have_result nil
  end

  it 'should return the -1 if SRID was not specified' do
    expect("SELECT ST_SRID(GeomFromText('Point(1 0)'))").to have_result (mode == :spl4 ? 0 : -1)
  end

  it 'should return the actual SRID value if specified' do
    expect("SELECT ST_SRID(GeomFromText('Point(1 0)', 20))").to have_result 20
  end

  it 'should raise an error on invalid input' do
    expect("SELECT ST_SRID(x'FFFFFFFFFF')").to raise_sql_error
  end

  it 'should return an update geometry when called with two parameters' do
      expect("SELECT hex(ST_SRID(GeomFromText('Point(1 0)', 20), 10))").
          to have_result(
                 mode == :gpkg ?
                     '475000010A0000000101000000000000000000F03F0000000000000000' :
                     '00010A000000000000000000F03F0000000000000000000000000000F03F00000000000000007C01000000000000000000F03F0000000000000000FE'
             )

      expect("SELECT ST_SRID(ST_SRID(GeomFromText('Point(1 0)', 20), 10))").to have_result 10
  end
end