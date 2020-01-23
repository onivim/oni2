#include <stdio.h>

#include <caml/alloc.h>
#include <caml/callback.h>
#include <caml/memory.h>
#include <caml/mlvalues.h>

CAMLprim value oni2c_native_exit(value vCode) {
  CAMLparam1(vCode);

  fflush(stdout);
  fflush(stderr);
  exit(Int_val(vCode));

  CAMLreturn(vCode);
};
