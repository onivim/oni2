#include <caml/alloc.h>
#include <caml/callback.h>
#include <caml/memory.h>
#include <caml/mlvalues.h>
#include <caml/threads.h>

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
  Store_field(keymapEntry, 0, caml_copy_string(unmodified));
  Store_field(keymapEntry, 1, caml_copy_string(withShift));
  Store_field(keymapEntry, 2, caml_copy_string(withAltGraph));
  Store_field(keymapEntry, 3, caml_copy_string(withAltGraphShift));
  CAMLreturn(keymapEntry);
}
