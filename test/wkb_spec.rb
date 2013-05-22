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

def to_wkb(db, wkt)
  db.get_first_value("SELECT lower(hex(WKBFromText(?)))", wkt)
end

describe "Well-Known Binary" do
  it "should parse XY points correctly" do
    expect(to_wkb(@db, 'Point(1 2)')).to eq('0101000000000000000000f03f0000000000000040')
  end

  it "should parse XYZ points correctly" do
    expect(to_wkb(@db, 'Point Z(1 2 3)')).to eq('01e9030000000000000000f03f00000000000000400000000000000840')
  end

  it "should parse XYM points correctly" do
    expect(to_wkb(@db, 'Point M(1 2 3)')).to eq('01d1070000000000000000f03f00000000000000400000000000000840')
  end

  it "should parse XYZM points correctly" do
    expect(to_wkb(@db, 'Point ZM(1 2 3 4)')).to eq('01b90b0000000000000000f03f000000000000004000000000000008400000000000001040')
  end

  it "should parse XY line strings correctly" do
    expect(to_wkb(@db, 'LineString(1 2, 3 4)')).to eq('010200000002000000000000000000f03f000000000000004000000000000008400000000000001040')
  end

  it "should parse XYZ line strings correctly" do
    expect(to_wkb(@db, 'LineString Z(1 2 3, 4 5 6)')).to eq('01ea03000002000000000000000000f03f00000000000000400000000000000840000000000000104000000000000014400000000000001840')
  end

  it "should parse XYM line strings correctly" do
    expect(to_wkb(@db, 'LineString M(1 2 3, 4 5 6)')).to eq('01d207000002000000000000000000f03f00000000000000400000000000000840000000000000104000000000000014400000000000001840')
  end

  it "should parse XYZM line strings correctly" do
    expect(to_wkb(@db, 'LineString ZM(1 2 3 4, 5 6 7 8)')).to eq('01ba0b000002000000000000000000f03f000000000000004000000000000008400000000000001040000000000000144000000000000018400000000000001c400000000000002040')
  end
end