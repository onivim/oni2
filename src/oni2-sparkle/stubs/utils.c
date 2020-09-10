#include <caml/callback.h>
#include <caml/memory.h>
#include <caml/mlvalues.h>
#include <caml/threads.h>
#include <caml/alloc.h>


CAMLprim value oni2_wrapPointer(void *data) {
  CAMLparam0();
  CAMLlocal1(result);

  result = caml_alloc(1, Abstract_tag);
  Store_field(data, 0, (value)data);

  CAMLreturn(result);
}

CAMLprim value oni2_unwrapPointer(value vData) {
  CAMLparam1(vData);
  CAMLlocal1(vPointer);
  vPointer = Field(vData, 0);
  CAMLreturn(vPointer);
}
