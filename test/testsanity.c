#include "unittest.h"
#include "sql.h"

void TestMinX() {
    sqlite3 *db;
    open_database(&db);
    double value = 0.0;
    int result = sql_exec_for_double(db, &value, "SELECT ST_MinX(GeomFromText('Point(1 0)'))");
    if (result != SQLITE_OK) {
        fail_m(sqlite3_errmsg(db));
    }
    close_database(db);

    assert_double_eql(1.0, value);
}

void TestMaxX() {
    sqlite3 *db;
    open_database(&db);
    double value = 0.0;
    int result = sql_exec_for_double(db, &value, "SELECT ST_MaxX(GeomFromText('Point(1 0)'))");
    if (result != SQLITE_OK) {
        fail_m(sqlite3_errmsg(db));
    }
    close_database(db);
    assert_double_eql(1.0, value);
}

void TestMinY() {
    sqlite3 *db;
    open_database(&db);
    double value = 0.0;
    int result = sql_exec_for_double(db, &value, "SELECT ST_MinX(GeomFromText('Point(1 1)'))");
    if (result != SQLITE_OK) {
        fail_m(sqlite3_errmsg(db));
    }
    close_database(db);
    assert_double_eql(1.0, value);
}

void TestMaxY() {
    sqlite3 *db;
    open_database(&db);
    double value = 0.0;
    int result = sql_exec_for_double(db, &value, "SELECT ST_MaxX(GeomFromText('Point(1 1)'))");
    if (result != SQLITE_OK) {
        fail_m(sqlite3_errmsg(db));
    }
    close_database(db);

    assert_double_eql(1.0, value);
}

void TestMinM() {
    sqlite3 *db;
    open_database(&db);
    double value = 0.0;
    int result = sql_exec_for_double(db, &value, "SELECT ST_MinM(GeomFromText('Point M(0 0 1)'))");
    if (result != SQLITE_OK) {
        fail_m(sqlite3_errmsg(db));
    }
    close_database(db);

    assert_double_eql(1.0, value);
}

void TestMaxM() {
    sqlite3 *db;
    open_database(&db);
    double value = 0.0;
    int result = sql_exec_for_double(db, &value, "SELECT ST_MaxM(GeomFromText('Point M(0 0 1)'))");
    if (result != SQLITE_OK) {
        fail_m(sqlite3_errmsg(db));
    }
    close_database(db);

    assert_double_eql(1.0, value);
}

void TestMinZ() {
    sqlite3 *db;
    open_database(&db);
    double value = 0.0;
    int result = sql_exec_for_double(db, &value, "SELECT ST_MinZ(GeomFromText('Point Z(0 0 1)'))");
    if (result != SQLITE_OK) {
        fail_m(sqlite3_errmsg(db));
    }
    close_database(db);

    assert_double_eql(1.0, value);
}

void TestMaxZ() {
    sqlite3 *db;
    open_database(&db);
    double value = 0.0;
    int result = sql_exec_for_double(db, &value, "SELECT ST_MaxZ(GeomFromText('Point Z(0 0 1)'))");
    if (result != SQLITE_OK) {
        fail_m(sqlite3_errmsg(db));
    }
    close_database(db);

    assert_double_eql(1.0, value);
}

void TestSRID() {
    sqlite3 *db;
    open_database(&db);
    int value = -2;
    int result = sql_exec_for_int(db, &value, "SELECT ST_SRID(GeomFromText('Point (0 0)'))");
    if (result != SQLITE_OK) {
        fail_m(sqlite3_errmsg(db));
    }
    close_database(db);

    assert_int_eql(0, value);
}

void TestIsValid() {
    sqlite3 *db;
    open_database(&db);
    int value = -2;
    int result = sql_exec_for_int(db, &value, "SELECT ST_IsValid(GeomFromText('Point (0 0)'))");
    if (result != SQLITE_OK) {
        fail_m(sqlite3_errmsg(db));
    }
    close_database(db);

    assert_int_eql(1, value);
}

void TestIsMeasured() {
    sqlite3 *db;
    open_database(&db);
    int value = -2;
    int result = sql_exec_for_int(db, &value, "SELECT ST_IsMeasured(GeomFromText('Point (0 0)'))");
    if (result != SQLITE_OK) {
        fail_m(sqlite3_errmsg(db));
    }
    close_database(db);

    assert_int_eql(0, value);
}

void TestIs3d() {
    sqlite3 *db;
    open_database(&db);
    int value = -2;
    int result = sql_exec_for_int(db, &value, "SELECT ST_Is3d(GeomFromText('Point (0 0)'))");
    if (result != SQLITE_OK) {
        fail_m(sqlite3_errmsg(db));
    }
    close_database(db);

    assert_int_eql(0, value);
}

void TestCoordDim() {
    sqlite3 *db;
    open_database(&db);
    int value = -2;
    int result = sql_exec_for_int(db, &value, "SELECT ST_CoordDim(GeomFromText('Point (0 0)'))");
    if (result != SQLITE_OK) {
        fail_m(sqlite3_errmsg(db));
    }
    close_database(db);

    assert_int_eql(2, value);
}

void TestGeometryType() {
    sqlite3 *db;
    open_database(&db);
    char* value = NULL;
    int result = sql_exec_for_string(db, &value, "SELECT ST_GeometryType(GeomFromText('Point (0 0)'))");
    if (result != SQLITE_OK) {
        fail_m(sqlite3_errmsg(db));
    }
    close_database(db);

    assert_str_eql("ST_Point", value);
}

void TestGeometryParsing() {
    sqlite3 *db;
    open_database(&db);
    char* value = NULL;
    int result = sql_exec_for_string(db, &value, "SELECT AsText(GeomFromWKB(AsBinary(GeomFromText('Point (-1 5)'))))");
    if (result != SQLITE_OK) {
        fail_m(sqlite3_errmsg(db));
    }
    close_database(db);

    assert_str_eql("Point (-1 5)", value);
}