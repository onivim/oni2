#include <caml/alloc.h>
#include <caml/bigarray.h>
#include <caml/callback.h>
#include <caml/custom.h>
#include <caml/memory.h>
#include <caml/mlvalues.h>
#include <caml/threads.h>

#include <oniguruma.h>

typedef struct _regexp {
  regex_t *regexp;
  OnigRegion *region;
} regexp_W;

void reonig_finalize_regexp(value v) {
  regexp_W *p;
  p = (regexp_W *)Data_custom_val(v);
  onig_region_free(p->region, 1);
  onig_free(p->regexp);
};

static struct custom_operations regexp_custom_ops = {
  identifier : "regexp handling",
  finalize : reonig_finalize_regexp,
  compare : custom_compare_default,
  hash : custom_hash_default,
  serialize : custom_serialize_default,
  deserialize : custom_deserialize_default
};

static value reonig_val_result_error(char *errorMsg) {
  CAMLparam0();
  CAMLlocal1(error);
  error = caml_alloc(1, 1);
  Store_field(error, 0, caml_copy_string(errorMsg));
  CAMLreturn(error);
}

static value reonig_val_result_ok(value val) {
  CAMLparam0();
  CAMLlocal1(result);
  result = caml_alloc(1, 0);
  Store_field(result, 0, val);
  CAMLreturn(result);
}

#define Val_none Val_int(0)

static value reonig_val_some(value v) {
  CAMLparam1(v);
  CAMLlocal1(some);
  some = caml_alloc(1, 0);
  Store_field(some, 0, v);
  CAMLreturn(some);
}

CAMLprim value reonig_create(value vPattern) {
  CAMLparam1(vPattern);
  CAMLlocal2(result, v);

  regex_t *reg;
  OnigErrorInfo einfo;

  char *pattern = String_val(vPattern);
  int status =
      onig_new(&reg, (UChar *)pattern, (UChar *)(pattern + strlen(pattern)),
               ONIG_OPTION_CAPTURE_GROUP, ONIG_ENCODING_UTF8,
               ONIG_SYNTAX_DEFAULT, &einfo);

  if (status != ONIG_NORMAL) {
    char s[ONIG_MAX_ERROR_MESSAGE_LEN];
    onig_error_code_to_str((UChar *)s, status, &einfo);
    result = reonig_val_result_error(s);
    onig_free(reg);
  } else {

    regexp_W regexpWrapper;
    OnigRegion *region = onig_region_new();
    regexpWrapper.regexp = reg;
    regexpWrapper.region = region;
    v = caml_alloc_custom(&regexp_custom_ops, sizeof(regexp_W), 0, 1);
    memcpy(Data_custom_val(v), &regexpWrapper, sizeof(regexp_W));
    result = reonig_val_result_ok(v);
  }

  CAMLreturn(result);
}

CAMLprim value reonig_end() { onig_end(); };

CAMLprim value reonig_search(value vStr, value vPos, value vRegExp) {
  CAMLparam3(vStr, vPos, vRegExp);
  CAMLlocal2(ret, v);

  UChar *searchData = String_val(vStr);
  size_t position = Int_val(vPos);
  size_t end = strlen(searchData);

  regexp_W *p = Data_custom_val(vRegExp);
  regex_t *regex = p->regexp;
  OnigRegion *region = p->region;
  int status =
      onig_search(regex, searchData, searchData + end, searchData + position,
                  searchData + end, region, ONIG_OPTION_NONE);

  if (status != ONIG_MISMATCH) {
    int num = region->num_regs;
    ret = caml_alloc(num, 0);
    for (int i = 0; i < num; i++) {
      v = caml_alloc(5, 0);
      int start = *(region->beg + i);
      if (start < 0) {
        start = 0;
      }

      int length = *(region->end + i) - *(region->beg + i);
      if (length < 0) {
        length = 0;
      }

      Store_field(v, 0, Val_int(i));
      Store_field(v, 1, Val_int(start));
      Store_field(v, 2, Val_int(length));
      Store_field(v, 3, Val_int(start + length));
      Store_field(v, 4, vStr);

      Store_field(ret, i, v);
    };
  } else {
    ret = Atom(0);
  }

  CAMLreturn(ret);
};
