#include <stdio.h>

#include <caml/alloc.h>
#include <caml/callback.h>
#include <caml/memory.h>
#include <caml/mlvalues.h>

#ifdef WIN32
#include <Windows.h>
#endif

CAMLprim value win32_free_console(value unit) {

#ifdef WIN32
  printf("yo from c\n");
  FreeConsole();
#endif

  return Val_unit;
};
