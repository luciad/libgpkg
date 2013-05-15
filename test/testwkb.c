#include <stdio.h>
#include <sqlite3.h>

#include "wkb.h"
#include "wkt.h"
#include "unittest.h"
#include "gpkg.h"

allocator_t allocator = {
        (void * ( *)(int)) malloc,
        (void * ( *)(void *, int)) realloc,
        free
};

void check_wkt(char* wkt, char* expected_wkb) {
    char *actual_wkb_str = NULL;

    wkb_writer_t wkb_writer;
    wkb_writer_init(&wkb_writer, &allocator);

    int result = wkt_read_geometry(wkt, strlen(wkt), wkb_writer_geom_consumer(&wkb_writer));
    if (result != 0) {
        fail_m("Could not parse WKT");
    }

    uint8_t *actual_wkb = wkb_writer_getwkb(&wkb_writer);
    size_t actual_len = wkb_writer_length(&wkb_writer);

    actual_wkb_str = nBinToHex(actual_wkb, actual_len);
    assert_str_eql_m("WKB", expected_wkb, actual_wkb_str);

    wkb_writer_destroy(&wkb_writer);
    free(actual_wkb_str);
}

void TestParsePoint() {
    check_wkt("Point(1 2)", "0101000000000000000000f03f0000000000000040");
    check_wkt("Point Z(1 2 3)", "01e9030000000000000000f03f00000000000000400000000000000840");
    check_wkt("Point M(1 2 3)", "01d1070000000000000000f03f00000000000000400000000000000840");
    check_wkt("Point ZM(1 2 3 4)", "01b90b0000000000000000f03f000000000000004000000000000008400000000000001040");
}

void TestParseLineString() {
    check_wkt("LineString(1 2, 3 4)", "010200000002000000000000000000f03f000000000000004000000000000008400000000000001040");
    check_wkt("LineString Z(1 2 3, 4 5 6)", "01ea03000002000000000000000000f03f00000000000000400000000000000840000000000000104000000000000014400000000000001840");
    check_wkt("LineString M(1 2 3, 4 5 6)", "01d207000002000000000000000000f03f00000000000000400000000000000840000000000000104000000000000014400000000000001840");
    check_wkt("LineString ZM(1 2 3 4, 5 6 7 8)", "01ba0b000002000000000000000000f03f000000000000004000000000000008400000000000001040000000000000144000000000000018400000000000001c400000000000002040");
}