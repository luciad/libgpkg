#include "i18n.h"

#ifdef GPKG_HAVE_CONFIG_H
#include "config.h"
#endif

#include "sqlite.h"

#if defined(LOCALE_USE__CREATE_LOCALE)

#include <locale.h>
#include <stdlib.h>

struct i18n_locale {
  _locale_t locale;
};

double i18n_strtod(const char *nptr, char **endptr, i18n_locale_t *locale) {
  return _strtod_l(nptr, endptr, locale->locale);
}

i18n_locale_t *i18n_locale_init(const char *locale_name) {
  _locale_t locale;
  i18n_locale_t *locale_struct;

  locale_struct = (i18n_locale_t *)sqlite3_malloc(sizeof(i18n_locale_t));
  if (locale_struct == NULL) {
    return NULL;
  }

  locale = _create_locale(LC_ALL, "C");
  if (locale == NULL) {
    sqlite3_free(locale_struct);
    return NULL;
  }

  locale_struct->locale = locale;
  return locale_struct;
}

void i18n_locale_destroy(i18n_locale_t *locale) {
  if (locale != NULL) {
    _free_locale(locale->locale);
    locale->locale = NULL;

    sqlite3_free(locale);
  }
}

#elif defined(LOCALE_USE_NEWLOCALE)

#define _POSIX_C_SOURCE 200809L
#define _GNU_SOURCE

#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif

#ifdef HAVE_XLOCALE_H
#include <xlocale.h>
#endif

#include <stdlib.h>

struct i18n_locale {
  locale_t locale;
};

double i18n_strtod(const char *nptr, char **endptr, i18n_locale_t *locale) {
  return strtod_l(nptr, endptr, locale->locale);
}

i18n_locale_t *i18n_locale_init(const char *locale_name) {
  locale_t locale;
  i18n_locale_t *locale_struct;

  locale_struct = (i18n_locale_t *)sqlite3_malloc(sizeof(i18n_locale_t));
  if (locale_struct == NULL) {
    return NULL;
  }

  locale = newlocale(LC_ALL, "C", NULL);
  if (locale == NULL) {
    sqlite3_free(locale_struct);
    return NULL;
  }

  locale_struct->locale = locale;
  return locale_struct;
}

void i18n_locale_destroy(i18n_locale_t *locale) {
  if (locale != NULL) {
    freelocale(locale->locale);
    locale->locale = NULL;

    sqlite3_free(locale);
  }
}

#elif defined(LOCALE_USE_SET_LOCALE)

#include <locale.h>
#include <stdlib.h>

struct i18n_locale {
  int dummy;
};

static struct i18n_locale DUMMY_LOCALE;

double i18n_strtod(const char *nptr, char **endptr, i18n_locale_t *locale) {
  char *old_locale = setlocale(LC_NUMERIC, "C");
  double result = strtod(nptr, endptr);
  setlocale(LC_NUMERIC, old_locale);
  return result;
}

i18n_locale_t *i18n_locale_init(const char *locale_name) {
  return &DUMMY_LOCALE;
}

void i18n_locale_destroy(i18n_locale_t *locale) {
}

#else

#include <stdlib.h>

struct i18n_locale {
  int dummy;
};

static struct i18n_locale DUMMY_LOCALE;

double i18n_strtod(const char *nptr, char **endptr, i18n_locale_t *locale) {
  return strtod(nptr, endptr);
}

i18n_locale_t *i18n_locale_init(const char *locale_name) {
  return &DUMMY_LOCALE;
}

void i18n_locale_destroy(i18n_locale_t *locale) {
}

#endif
