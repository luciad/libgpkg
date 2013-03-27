#include <sqlite3ext.h>
#include "gpkg.h"

int sqlite3_extension_init( sqlite3 *db, const char **pzErrMsg, const struct sqlite3_api_routines *pThunk ) {
	return gpkg_extension_init( db, pzErrMsg, pThunk );
}
