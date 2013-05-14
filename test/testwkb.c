#include <stdio.h>
#include <sqlite3.h>
#include "cutest/CuTest.h"

#include "wkb.h"
#include "wkt.h"
#import "util.h"
#include "gpkg.h"

void check_wkt(CuTest *tc, char* wkt, char* expected_wkb) {
    char *actual_wkb_str = NULL;

    wkb_writer_t wkb_writer;
    wkb_writer_init(&wkb_writer);

    int result = wkt_read_geometry(wkt, strlen(wkt), wkb_writer_geom_consumer(&wkb_writer));
    if (result != 0) {
        CuFail(tc, "Could not parse WKT");
        goto exit;
    }

    uint8_t *actual_wkb = wkb_writer_getwkb(&wkb_writer);
    size_t actual_len = wkb_writer_length(&wkb_writer);

    actual_wkb_str = nBinToHex(actual_wkb, actual_len);
    CuAssertStrEquals_Msg(tc, "WKB", expected_wkb, actual_wkb_str);
    exit:

    wkb_writer_destroy(&wkb_writer);
    free(actual_wkb_str);
}

const alloc_t allocator = {
        (void * ( *)(int)) malloc,
        (void * ( *)(void *, int)) realloc,
        free
};

void TestParsePoint(CuTest *tc) {
    gpkg_init(&allocator);
    check_wkt(tc, "Point(1 2)", "0101000000000000000000f03f0000000000000040");
    check_wkt(tc, "Point Z(1 2 3)", "01e9030000000000000000f03f00000000000000400000000000000840");
    check_wkt(tc, "Point M(1 2 3)", "01d1070000000000000000f03f00000000000000400000000000000840");
    check_wkt(tc, "Point ZM(1 2 3 4)", "01b90b0000000000000000f03f000000000000004000000000000008400000000000001040");
    check_wkt(tc, "LineString(1 2, 3 4)", "010200000002000000000000000000f03f000000000000004000000000000008400000000000001040");
    check_wkt(tc, "LineString Z(1 2 3, 4 5 6)", "");
    check_wkt(tc, "LineString M(1 2 3, 4 5 6)", "");
    check_wkt(tc, "LineString ZM(1 2 3 4, 5 6 7 8)", "");
}