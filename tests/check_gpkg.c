#include <sqlite3.h>
#include <stdlib.h>
#include <check.h>
#include "gpkg.h"
#include "sql.h"

sqlite3 *db;

void setup() {
    sqlite3_auto_extension(gpkg_extension_init);
    sqlite3_open(":memory:", &db);
}

void teardown() {
    sqlite3_close(db);
}

START_TEST (test_gpkg) {
     char* err;

     int result = sqlite3_exec(
       db,
       "CREATE TABLE test (geom GEOMETRY);"
       "CREATE VIRTUAL TABLE test_index USING rtree (id, minx, maxx, miny, maxy);"
       "CREATE TRIGGER test_insert INSERT ON test BEGIN INSERT INTO test_index VALUES (NEW.rowid, ST_MinX(NEW.geom), ST_MaxX(NEW.geom), ST_MinY(NEW.geom), ST_MaxY(NEW.geom)); END",
       NULL, NULL, &err
     );
     fail_unless(result == SQLITE_OK, "Could not setup tables: %s", err);
     sqlite3_free(err);

     result = sqlite3_exec(
       db,
       "INSERT INTO test VALUES (ST_GeomFromText('Point(1 2)'));",
       NULL, NULL, &err
    );
    fail_unless(result == SQLITE_OK, "Could insert point: %s", err);
    sqlite3_free(err);
}
END_TEST

char* GEOM_FROM_TEXT[] = {
	"Point(1 2)", 
	"47504211000000000101000000000000000000F03F0000000000000040",
	"LineString(1 2, -3 -4)", 
	"475042130000000000000000000008C0000000000000F03F00000000000010C00000000000000040010200000002000000000000000000F03F000000000000004000000000000008C000000000000010C0",
	"Polygon((1 2, -3 -4), (8 9, -13 -14))",
	"47504213000000000000000000002AC000000000000020400000000000002CC0000000000000224001030000000200000002000000000000000000F03F000000000000004000000000000008C000000000000010C002000000000000000000204000000000000022400000000000002AC00000000000002CC0",
	"MultiPoint((1 2), (5 6))",
	"4750421300000000000000000000F03F0000000000001440000000000000004000000000000018400104000000020000000101000000000000000000F03F0000000000000040010100000000000000000014400000000000001840",
	"MultiLineString((1 2, -3 -4), EMPTY)",
	"47504211000000000101000000000000000000F03F0000000000000040",
	"MultiPolygon(EMPTY, ((1 2, -3 -4), (8 9, -13 -14)))",
	"47504211000000000101000000000000000000F03F0000000000000040",
	"GeometryCollection (Point(1 2), LineString(6 9.54, 3.14 2.56)",
	"47504211000000000101000000000000000000F03F0000000000000040"
};
#define GEOM_FROM_TEXT_COUNT 7

START_TEST (test_geomfromtext) {
    char *err = NULL;
	char *text = NULL;
	int result;

	char* input = GEOM_FROM_TEXT[_i * 2];
	char* expected = GEOM_FROM_TEXT[(_i * 2) + 1];
	
	result = sql_exec_for_string(db, &text, &err, "SELECT hex(ST_GeomFromText('%s'))", input);
    fail_unless(result == SQLITE_OK, "Could not execute query: %s", err);
	if (text) {
		fail_unless(strcmp(text, expected) == 0, "Assertion ST_GeomFromText('%s') == '%s' failed. Actual was '%s'", input, expected, text);
	}
	
    sqlite3_free(err);
    sqlite3_free(text);
}
END_TEST

Suite* gpkg_suite(void) {
  Suite *s = suite_create("gpkg");

  /* Core test case */
  TCase *tc_core = tcase_create("core");
  tcase_add_checked_fixture(tc_core, setup, teardown);
  tcase_add_test(tc_core, test_gpkg);
  tcase_add_loop_test(tc_core, test_geomfromtext, 0, GEOM_FROM_TEXT_COUNT);
  suite_add_tcase(s, tc_core);

  return s;
}

int main() {
  int number_failed;
  Suite *s = gpkg_suite ();
  SRunner *sr = srunner_create (s);
  srunner_run_all (sr, CK_NORMAL);
  number_failed = srunner_ntests_failed (sr);
  srunner_free (sr);
  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
