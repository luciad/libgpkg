require_relative 'sqlite'

RSpec.configure do |c|
  c.before(:each) do
    @db = SQLite3::Database.new(':memory:', SQLite3::OPEN_READWRITE | SQLite3::OPEN_CREATE)
    @db.load_extension ENV['GPKG_EXTENSION']
  end

  c.after(:each) do
    @db.close
  end
end