#ifndef GPKG_I18N_H
#define GPKG_I18N_H

typedef struct i18n_locale i18n_locale_t;

double i18n_strtod(
  const char *nptr,
  char **endptr,
  i18n_locale_t *locale
);

i18n_locale_t *i18n_locale_init(const char *locale_name);

void i18n_locale_destroy(i18n_locale_t *locale);

#endif

