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

describe 'InitSpatialMetadata' do
  it 'should return NULL' do
    expect('SELECT InitSpatialMetadata()').to have_result nil
  end

  it 'should return NULL when called a second time' do
    expect('SELECT InitSpatialMetadata()').to have_result nil
    expect('SELECT InitSpatialMetadata()').to have_result nil
  end
end

describe 'CheckGpkg' do
  it 'should raise error when called before InitSpatialMetadata' do
    expect('SELECT CheckSpatialMetadata()').to raise_sql_error
  end

  it 'should return NULL when called after InitSpatialMetadata' do
    expect('SELECT InitSpatialMetadata()').to have_result nil
    expect('SELECT CheckSpatialMetadata()').to have_result nil
  end
end