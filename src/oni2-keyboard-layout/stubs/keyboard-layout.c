#include <caml/alloc.h>
#include <caml/callback.h>
#include <caml/memory.h>
#include <caml/mlvalues.h>
#include <caml/threads.h>

#define Val_none Val_int(0)

static value Val_some(value v) {
  CAMLparam1(v);
  CAMLlocal1(some);
  some = caml_alloc(1, 0);
  Store_field(some, 0, v);
  CAMLreturn(some);
}

static value copyMaybeString(const char *str) {
  CAMLparam0();
  CAMLlocal1(vMaybeStr);
  if (str[0] == '\0') {
    vMaybeStr = Val_none;
  } else {
    vMaybeStr = Val_some(caml_copy_string(str));
  }
  CAMLreturn(vMaybeStr);
}

/* Create an OCaml Keymap Entry
  Simple helper method to construct a keymap
  entry from strings
 */
value createKeymapEntry(
  const char *unmodified,
  const char *withShift,
  const char *withAltGraph,
  const char *withAltGraphShift
) {
  CAMLparam0();
  CAMLlocal1(keymapEntry);
  keymapEntry = caml_alloc(4, 0);
  Store_field(keymapEntry, 0, copyMaybeString(unmodified));
  Store_field(keymapEntry, 1, copyMaybeString(withShift));
  Store_field(keymapEntry, 2, copyMaybeString(withAltGraph));
  Store_field(keymapEntry, 3, copyMaybeString(withAltGraphShift));
  CAMLreturn(keymapEntry);
}
