require_relative 'gpkg'

describe "ST_MinX" do
  it "should return NULL when passed NULL" do
    expect(@db.get_first_value("SELECT ST_MinX(NULL)")).to eq(nil)
  end

  it "should return the X coordinate of a point" do
    expect(@db.get_first_value("SELECT ST_MinX(GeomFromText('Point(1 0)'))")).to eq(1.0)
  end
end

describe "ST_MaxX" do
  it "should return NULL when passed NULL" do
    expect(@db.get_first_value("SELECT ST_MaxX(NULL)")).to eq(nil)
  end

  it "should return the X coordinate of a point" do
    expect(@db.get_first_value("SELECT ST_MaxX(GeomFromText('Point(1 0)'))")).to eq(1.0)
  end
end

describe "ST_MinY" do
  it "should return NULL when passed NULL" do
    expect(@db.get_first_value("SELECT ST_MinY(NULL)")).to eq(nil)
  end

  it "should return the X coordinate of a point" do
    expect(@db.get_first_value("SELECT ST_MinY(GeomFromText('Point(1 5)'))")).to eq(5.0)
  end
end

describe "ST_MaxY" do
  it "should return NULL when passed NULL" do
    expect(@db.get_first_value("SELECT ST_MaxY(NULL)")).to eq(nil)
  end

  it "should return the X coordinate of a point" do
    expect(@db.get_first_value("SELECT ST_MaxY(GeomFromText('Point(1 5)'))")).to eq(5.0)
  end
end

describe "ST_MinM" do
  it "should return NULL when passed NULL" do
    expect(@db.get_first_value("SELECT ST_MinM(NULL)")).to eq(nil)
  end

  it "should return the M coordinate of a point" do
    expect(@db.get_first_value("SELECT ST_MinM(GeomFromText('Point M (1 5 4)'))")).to eq(4.0)
  end

  it "should return NULL if M is undefined" do
    expect(@db.get_first_value("SELECT ST_MinM(GeomFromText('Point Z (1 5 4)'))")).to eq(nil)
  end
end

describe "ST_MaxM" do
  it "should return NULL when passed NULL" do
    expect(@db.get_first_value("SELECT ST_MaxM(NULL)")).to eq(nil)
  end

  it "should return the M coordinate of a point" do
    expect(@db.get_first_value("SELECT ST_MaxM(GeomFromText('Point M(1 5 4)'))")).to eq(4.0)
  end

  it "should return NULL if M is undefined" do
    expect(@db.get_first_value("SELECT ST_MaxM(GeomFromText('Point Z (1 5 4)'))")).to eq(nil)
  end
end

describe "ST_MinZ" do
  it "should return NULL when passed NULL" do
    expect(@db.get_first_value("SELECT ST_MinZ(NULL)")).to eq(nil)
  end

  it "should return the Z coordinate of a point" do
    expect(@db.get_first_value("SELECT ST_MinZ(GeomFromText('Point Z (1 5 4)'))")).to eq(4.0)
  end

  it "should return NULL if Z is undefined" do
    expect(@db.get_first_value("SELECT ST_MinZ(GeomFromText('Point M (1 5 4)'))")).to eq(nil)
  end
end

describe "ST_MaxZ" do
  it "should return NULL when passed NULL" do
    expect(@db.get_first_value("SELECT ST_MaxZ(NULL)")).to eq(nil)
  end

  it "should return the Z coordinate of a point" do
    expect(@db.get_first_value("SELECT ST_MaxZ(GeomFromText('Point Z (1 5 4)'))")).to eq(4.0)
  end

  it "should return NULL if Z is undefined" do
    expect(@db.get_first_value("SELECT ST_MaxZ(GeomFromText('Point M (1 5 4)'))")).to eq(nil)
  end
end