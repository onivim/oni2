#include "config.h"

#ifdef USE_SPARKLE
#include <caml/alloc.h>
#include <caml/callback.h>
#include <caml/memory.h>
#include <caml/mlvalues.h>
#include <string.h>

#import <Sparkle/Sparkle.h>

#include "utils.h"

CAMLprim value oni2_SparkleGetSharedInstance() {
  CAMLparam0();
  CAMLlocal1(vUpdater);

  SUUpdater *updater = [SUUpdater sharedUpdater];

  vUpdater = oni2_wrapPointer(updater);

  NSLog(@"NSBundle: %@", [NSBundle mainBundle]);

  CAMLreturn(vUpdater);
}

CAMLprim value oni2_SparkleDebugToString(value vData) {
  CAMLparam1(vData);
  CAMLlocal1(vStr);

  NSObject *data = oni2_unwrapPointer(vData);
  NSString *nsStr = [NSString stringWithFormat:@"%@", data];

  vStr = caml_copy_string([nsStr UTF8String]);

  CAMLreturn(vStr);
}

CAMLprim value oni2_SparkleDebugLog(value vData) {
  CAMLparam1(vData);
  
  NSObject *data = oni2_unwrapPointer(vData);
  NSLog(@"%@", data);

  CAMLreturn(Val_unit);
}

#endif
