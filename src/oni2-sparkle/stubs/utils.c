#include <caml/callback.h>
#include <caml/memory.h>
#include <caml/mlvalues.h>
#include <caml/threads.h>
#include <caml/alloc.h>


value oni2_wrapPointer(void *p) {
  value v = caml_alloc(1, Abstract_tag);
  *((void **) Data_abstract_val(v)) = p;
  return v;
}

void *oni2_unwrapPointer(value v) {
  return *((void **) Data_abstract_val(v));
}
