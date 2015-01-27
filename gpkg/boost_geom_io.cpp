#include <algorithm>
#include <vector>
#include "boost_geom_io.hpp"


using namespace gpkg;
namespace bg = boost::geometry;

struct create_sub_geometry : boost::static_visitor<int> {
  geom_type_t type;
  boostgeom_writer_t *writer;

  create_sub_geometry(geom_type_t type, boostgeom_writer_t *writer) : type(type), writer(writer) {
  }
  
  int operator()(boost::blank &blank) const {
    return SQLITE_IOERR;
  }

  int operator()(void *geom) const {
    return SQLITE_IOERR;
  }

  int operator()(Polygon *geom) const {
    if (type == GEOM_LINEARRING) {
      if (writer->child_count[writer->offset - 1] == 0) {
        writer->geometry_stack[writer->offset] = &geom->outer();
      } else {
        geom->inners().emplace_back();
        LinearRing *last_ring = &geom->inners().back();
        writer->geometry_stack[writer->offset] = last_ring;
      }
      return SQLITE_OK;
    } else {
      return SQLITE_IOERR;
    }
  }

  int operator()(MultiPoint *geom) const {
    if (type == GEOM_POINT) {
      geom->emplace_back();
      Point *last_point = &geom->back();
      writer->geometry_stack[writer->offset] = last_point;
      return SQLITE_OK;
    } else {
      return SQLITE_IOERR;
    }
  }

  int operator()(MultiLineString *geom) const {
    if (type == GEOM_LINESTRING) {
      geom->emplace_back();
      LineString *last_point = &geom->back();
      writer->geometry_stack[writer->offset] = last_point;
      return SQLITE_OK;
    } else {
      return SQLITE_IOERR;
    }
  }

  int operator()(MultiPolygon *geom) const {
    if (type == GEOM_POLYGON) {
      geom->emplace_back();
      Polygon *last_point = &geom->back();
      writer->geometry_stack[writer->offset] = last_point;
      return SQLITE_OK;
    } else {
      return SQLITE_IOERR;
    }
  }

  template<typename Geometry>
  static Geometry* add(GeometryCollection *geom) {
    geom->emplace_back();
    geom->back() = Geometry();
    return &boost::get<Geometry>(geom->back());
  }

  int operator()(GeometryCollection *geom) const {
    switch(type) {
      case GEOM_POINT:
        writer->geometry_stack[writer->offset] = add<Point>(geom);
        break;
      case GEOM_LINESTRING:
        writer->geometry_stack[writer->offset] = add<LineString>(geom);
        break;
      case GEOM_POLYGON:
        writer->geometry_stack[writer->offset] = add<Polygon>(geom);
        break;
      case GEOM_MULTIPOINT:
        writer->geometry_stack[writer->offset] = add<MultiPoint>(geom);
        break;
      case GEOM_MULTILINESTRING:
        writer->geometry_stack[writer->offset] = add<MultiLineString>(geom);
        break;
      case GEOM_MULTIPOLYGON:
        writer->geometry_stack[writer->offset] = add<MultiPolygon>(geom);
        break;
      case GEOM_GEOMETRYCOLLECTION:
        writer->geometry_stack[writer->offset] = add<GeometryCollection>(geom);
        break;
      default:
        return SQLITE_IOERR;
    }
    return SQLITE_OK;
  }
};

static int boostgeom_begin_geometry(const struct geom_consumer_t *consumer, const geom_header_t *header, errorstream_t *error) {
  boostgeom_writer_t *writer = (boostgeom_writer_t *) consumer;
  
  writer->offset++;
  writer->child_count[writer->offset] = 0;
  
  if (writer->offset == 0) {
    switch(header->geom_type) {
      case GEOM_POINT:
        writer->geometry_stack[writer->offset] = new Point();
        break;
      case GEOM_LINESTRING:
        writer->geometry_stack[writer->offset] = new LineString();
        break;
      case GEOM_POLYGON:
        writer->geometry_stack[writer->offset] = new Polygon();
        break;
      case GEOM_MULTIPOINT:
        writer->geometry_stack[writer->offset] = new MultiPoint();
        break;
      case GEOM_MULTILINESTRING:
        writer->geometry_stack[writer->offset] = new MultiLineString();
        break;
      case GEOM_MULTIPOLYGON:
        writer->geometry_stack[writer->offset] = new MultiPolygon();
        break;
      case GEOM_GEOMETRYCOLLECTION:
        writer->geometry_stack[writer->offset] = new GeometryCollection();
        break;
      case GEOM_LINEARRING:
        writer->geometry_stack[writer->offset] = new LinearRing();
        break;
      default:
        return SQLITE_IOERR;
    }
    return SQLITE_OK;
  } else {
    int result = boost::apply_visitor(create_sub_geometry(header->geom_type, writer), writer->geometry_stack[writer->offset - 1]);
    writer->child_count[writer->offset - 1] += 1;
    return result;
  }
}

struct assign_coordinates : boost::static_visitor<int> {
  const geom_header_t *header;
  size_t point_count;
  const double *coords;


  assign_coordinates(geom_header_t const *header, size_t point_count, double const *coords)
      : header(header), point_count(point_count), coords(coords) {
  }

  int operator()(boost::blank& geom) const {
    return SQLITE_IOERR;
  }

  int operator()(void* geom) const {
    return SQLITE_IOERR;
  }

  int operator()(Point* geom) const {
    bg::set<0>(*geom, coords[0]);
    bg::set<1>(*geom, coords[1]);
    return SQLITE_OK;
  }

  int operator()(LinearRing* geom) const {
    size_t offset = 0;
    for (size_t i = 0; i < point_count; i++) {
      geom->emplace_back(coords[offset], coords[offset + 1]);
      offset += header->coord_size;
    }
    return SQLITE_OK;
  }

  int operator()(LineString* geom) const {
    size_t offset = 0;
    for (size_t i = 0; i < point_count; i++) {
      geom->emplace_back(coords[offset], coords[offset + 1]);
      offset += header->coord_size;
    }
    return SQLITE_OK;
  }
};

static int boostgeom_coordinates(const struct geom_consumer_t *consumer, const geom_header_t *header, size_t point_count, const double *coords, int skip_coords, errorstream_t *error) {
  boostgeom_writer_t *writer = (boostgeom_writer_t *) consumer;
  return boost::apply_visitor(assign_coordinates(header, point_count, coords), writer->geometry_stack[writer->offset]);
}

static int boostgeom_end_geometry(const struct geom_consumer_t *consumer, const geom_header_t *header, errorstream_t *error) {
  int result = SQLITE_OK;

  boostgeom_writer_t *writer = (boostgeom_writer_t *) consumer;
  writer->offset--;

  if (writer->offset < 0) {
    writer->geometry = writer->geometry_stack[0];
  }

  return result;
}

int boostgeom_writer_init_srid(boostgeom_writer_t *writer, int srid) {
  geom_consumer_init(&writer->geom_consumer, NULL, NULL, boostgeom_begin_geometry, boostgeom_end_geometry, boostgeom_coordinates);
  writer->geometry = ::boost::blank();
  memset(writer->geometry_stack, 0, GEOM_MAX_DEPTH * sizeof(Geometry *));

  writer->offset = -1;

  return SQLITE_OK;
}

struct delete_geometry : boost::static_visitor<> {
  void operator()(::boost::blank& geom) const {
  }
  void operator()(Point* geom) const {
    delete geom;
  }
  void operator()(LineString* geom) const {
    delete geom;
  }
  void operator()(Polygon* geom) const {
    delete geom;
  }
  void operator()(LinearRing* geom) const {
    delete geom;
  }
  void operator()(MultiPoint* geom) const {
    delete geom;
  }
  void operator()(MultiLineString* geom) const {
    delete geom;
  }
  void operator()(MultiPolygon* geom) const {
    delete geom;
  }
  void operator()(GeometryCollection* geom) const {
    delete geom;
  }
};

void gpkg::delete_geometry(GeometryPtr &ptr) {
  boost::apply_visitor(::delete_geometry(), ptr);
}

void boostgeom_writer_destroy(boostgeom_writer_t *writer, int free_data) {
  if (writer == NULL) {
    return;
  }

  if (free_data) {
    gpkg::delete_geometry(writer->geometry);
    writer->geometry = boost::blank();
  }
}

geom_consumer_t *boostgeom_writer_geom_consumer(boostgeom_writer_t *writer) {
  return &writer->geom_consumer;
}

GeometryPtr boostgeom_writer_getgeometry(boostgeom_writer_t *writer) {
  return writer->geometry;
}

#define COORD_BATCH_SIZE 10

template <typename Range>
struct boostgeom_range
{
  static inline int apply(Range const& range, geom_consumer_t const *consumer, const geom_header_t *header, errorstream_t *error)
  {
    typedef typename boost::range_iterator<Range const>::type iterator_type;
    typedef typename boost::range_value<Range>::type point_type;

    int result = SQLITE_OK;
    double coord[2 * COORD_BATCH_SIZE];

    iterator_type begin = boost::begin(range);
    size_t remaining = static_cast<size_t>(boost::distance(range));
    iterator_type it = boost::begin(range);

    while (remaining > 0) {
      size_t points_to_read = (remaining > COORD_BATCH_SIZE ? COORD_BATCH_SIZE : remaining);
      for (size_t i = 0, ix = 0; i < points_to_read; i++, ix += 2) {
        const point_type &pt = *it;
        coord[ix] = bg::get<0>(pt);
        coord[ix + 1] = bg::get<1>(pt);
        ++it;
      }

      result = consumer->coordinates(consumer, header, points_to_read, coord, 0, error);
      if (result != SQLITE_OK) {
        return result;
      }

      remaining -= points_to_read;
    }

    return result;
  }
};

static int read_boostgeom_point(const Point &geom, geom_consumer_t const *consumer, errorstream_t *error) {
  int result = SQLITE_OK;
  geom_header_t header = {
    GEOM_POINT,
    GEOM_XY,
    2
  };

  if (result == SQLITE_OK) {
    result = consumer->begin_geometry(consumer, &header, error);
  }
  if (result == SQLITE_OK) {
    double coord[] = {
        bg::get<0>(geom),
        bg::get<1>(geom)
    };
    result = consumer->coordinates(consumer, &header, 1, coord, 0, error);
  }
  if (result == SQLITE_OK) {
    result = consumer->end_geometry(consumer, &header, error);
  }
  return result;
}

static int read_boostgeom_linestring(const LineString &geom, geom_consumer_t const *consumer, errorstream_t *error) {
  int result = SQLITE_OK;
  geom_header_t header = {
    GEOM_LINESTRING,
    GEOM_XY,
    2
  };

  if (result == SQLITE_OK) {
    result = consumer->begin_geometry(consumer, &header, error);
  }
  if (result == SQLITE_OK) {
    result = boostgeom_range<LineString>::apply(geom, consumer, &header, error);
  }
  if (result == SQLITE_OK) {
    result = consumer->end_geometry(consumer, &header, error);
  }
  return result;
}

static int read_boostgeom_linearring(const LinearRing &geom, geom_consumer_t const *consumer, errorstream_t *error) {
  int result = SQLITE_OK;
  geom_header_t header = {
    GEOM_LINEARRING,
    GEOM_XY,
    2
  };

  if (result == SQLITE_OK) {
    result = consumer->begin_geometry(consumer, &header, error);
  }
  if (result == SQLITE_OK) {
    result = boostgeom_range<LinearRing>::apply(geom, consumer, &header, error);
  }
  if (result == SQLITE_OK) {
    result = consumer->end_geometry(consumer, &header, error);
  }
  return result;
}

static int read_boostgeom_polygon(const Polygon &geom, geom_consumer_t const *consumer, errorstream_t *error) {
  int result = SQLITE_OK;
  geom_header_t header = {
    GEOM_POLYGON,
    GEOM_XY,
    2
  };

  if (result == SQLITE_OK) {
    result = consumer->begin_geometry(consumer, &header, error);
  }

  const LinearRing& exterior = geom.outer();
  const std::vector<LinearRing>& interior = geom.inners();
  if (result == SQLITE_OK && (exterior.size() > 0 || std::any_of(interior.cbegin(), interior.cend(), [](const LinearRing& lr) { return lr.size() > 0; }) )) {
    result = read_boostgeom_linearring(exterior, consumer, error);

    std::vector<LinearRing> const & interior_rings = static_cast<std::vector<LinearRing> const &>(bg::interior_rings(geom));
    for (std::vector<LinearRing>::const_iterator it = interior_rings.begin() ; it != interior_rings.end(); ++it) {
      if (result != SQLITE_OK) {
        break;
      }
      result = read_boostgeom_linearring(*it, consumer, error);
    }
  }

  if (result == SQLITE_OK) {
    result = consumer->end_geometry(consumer, &header, error);
  }

  return result;
}

static int read_boostgeom_multipoint(const MultiPoint &geom, geom_consumer_t const *consumer, errorstream_t *error) {
  int result = SQLITE_OK;
  geom_header_t header = {
    GEOM_MULTIPOINT,
    GEOM_XY,
    2
  };

  if (result == SQLITE_OK) {
    result = consumer->begin_geometry(consumer, &header, error);
  }

  for (std::vector<Point>::const_iterator it = geom.begin() ; it != geom.end(); ++it) {
    if (result != SQLITE_OK) {
      break;
    }
    result = read_boostgeom_point(*it, consumer, error);
  }

  if (result == SQLITE_OK) {
    result = consumer->end_geometry(consumer, &header, error);
  }

  return result;
}

static int read_boostgeom_multilinestring(const MultiLineString &geom, geom_consumer_t const *consumer, errorstream_t *error) {
  int result = SQLITE_OK;
  geom_header_t header = {
    GEOM_MULTILINESTRING,
    GEOM_XY,
    2
  };

  if (result == SQLITE_OK) {
    result = consumer->begin_geometry(consumer, &header, error);
  }

  for (std::vector<LineString>::const_iterator it = geom.begin() ; it != geom.end(); ++it) {
    if (result != SQLITE_OK) {
      break;
    }
    result = read_boostgeom_linestring(*it, consumer, error);
  }

  if (result == SQLITE_OK) {
    result = consumer->end_geometry(consumer, &header, error);
  }

  return result;
}

static int read_boostgeom_multipolygon(const MultiPolygon &geom, geom_consumer_t const *consumer, errorstream_t *error) {
  int result = SQLITE_OK;
  geom_header_t header = {
    GEOM_MULTIPOLYGON,
    GEOM_XY,
    2
  };

  if (result == SQLITE_OK) {
    result = consumer->begin_geometry(consumer, &header, error);
  }

  for (std::vector<Polygon>::const_iterator it = geom.begin() ; it != geom.end(); ++it) {
    if (result != SQLITE_OK) {
      break;
    }
    result = read_boostgeom_polygon(*it, consumer, error);
  }

  if (result == SQLITE_OK) {
    result = consumer->end_geometry(consumer, &header, error);
  }

  return result;
}

static int read_boostgeom_geometry(const Geometry &geom, geom_consumer_t const *consumer, errorstream_t *error);

static int read_boostgeom_geometrycollection(const GeometryCollection &geom, geom_consumer_t const *consumer, errorstream_t *error) {
  int result = SQLITE_OK;
  geom_header_t header = {
    GEOM_GEOMETRYCOLLECTION,
    GEOM_XY,
    2
  };

  if (result == SQLITE_OK) {
    result = consumer->begin_geometry(consumer, &header, error);
  }

  for (std::vector<Geometry>::const_iterator it = geom.begin() ; it != geom.end(); ++it) {
    if (result != SQLITE_OK) {
      break;
    }
    result = read_boostgeom_geometry(*it, consumer, error);
  }

  if (result == SQLITE_OK) {
    result = consumer->end_geometry(consumer, &header, error);
  }

  return result;
}

struct read_geometry : boost::static_visitor<int> {
  geom_consumer_t const *consumer;
  errorstream_t *error;

  read_geometry(geom_consumer_t const *consumer, errorstream_t *error) : consumer(consumer), error(error) {
  }

  int operator()(const Point& geom) const {
    return read_boostgeom_point(geom, consumer, error);
  }

  int operator()(const Point* geom) const {
    return read_boostgeom_point(*geom, consumer, error);
  }

  int operator()(const LineString& geom) const {
    return read_boostgeom_linestring(geom, consumer, error);
  }

  int operator()(const LineString* geom) const {
    return read_boostgeom_linestring(*geom, consumer, error);
  }

  int operator()(const LinearRing& geom) const {
    return read_boostgeom_linearring(geom, consumer, error);
  }

  int operator()(const LinearRing* geom) const {
    return read_boostgeom_linearring(*geom, consumer, error);
  }

  int operator()(const Polygon& geom) const {
    return read_boostgeom_polygon(geom, consumer, error);
  }

  int operator()(const Polygon* geom) const {
    return read_boostgeom_polygon(*geom, consumer, error);
  }

  int operator()(const MultiPoint& geom) const {
    return read_boostgeom_multipoint(geom, consumer, error);
  }

  int operator()(const MultiPoint* geom) const {
    return read_boostgeom_multipoint(*geom, consumer, error);
  }

  int operator()(const MultiLineString& geom) const {
    return read_boostgeom_multilinestring(geom, consumer, error);
  }

  int operator()(const MultiLineString* geom) const {
    return read_boostgeom_multilinestring(*geom, consumer, error);
  }

  int operator()(const MultiPolygon& geom) const {
    return read_boostgeom_multipolygon(geom, consumer, error);
  }

  int operator()(const MultiPolygon* geom) const {
    return read_boostgeom_multipolygon(*geom, consumer, error);
  }

  int operator()(const GeometryCollection& geom) const {
    return read_boostgeom_geometrycollection(geom, consumer, error);
  }

  int operator()(const GeometryCollection* geom) const {
    return read_boostgeom_geometrycollection(*geom, consumer, error);
  }

  int operator()(const boost::blank& geom) const {
    return SQLITE_IOERR;
  }
};

static int read_boostgeom_geometry(const Geometry &geom, geom_consumer_t const *consumer, errorstream_t *error) {
  return boost::apply_visitor(read_geometry(consumer, error), geom);
}

int boostgeom_read_geometry(const GeometryPtr geom, geom_consumer_t const *consumer, errorstream_t *error) {
  int result;

  result = consumer->begin(consumer, error);
  if (result != SQLITE_OK) {
    goto exit;
  }

  result = boost::apply_visitor(read_geometry(consumer, error), geom);
  if (result != SQLITE_OK) {
    goto exit;
  }

  result = consumer->end(consumer, error);
  if (result != SQLITE_OK) {
    goto exit;
  }

  exit:
  return result;
}
