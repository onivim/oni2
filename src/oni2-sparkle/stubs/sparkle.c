#include "config.h"

#include <caml/alloc.h>
#include <caml/callback.h>
#include <caml/memory.h>
#include <caml/mlvalues.h>
#include <string.h>

#ifdef USE_SPARKLE
#import <Sparkle/Sparkle.h>

#include "utils.h"

CAMLprim value oni2_SparkleGetSharedInstance() {
  CAMLparam0();
  CAMLlocal1(vUpdater);

  SUUpdater *updater = [SUUpdater sharedUpdater];

  vUpdater = oni2_wrapPointer(updater);

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

CAMLprim value oni2_SparkleSetFeedURL(value vUpdater, value vUrlStr) {
  CAMLparam2(vUpdater, vUrlStr);
  
  SUUpdater *updater = oni2_unwrapPointer(vUpdater);
  NSString *nsUrlStr = [NSString stringWithUTF8String:String_val(vUrlStr)];

  NSLog(@"Setting Updater feed URL: %@", nsUrlStr);

  NSURL *nsUrl = [NSURL URLWithString:nsUrlStr];

  [updater setFeedURL:nsUrl];

  CAMLreturn(Val_unit);
}

CAMLprim value oni2_SparkleGetFeedURL(value vUpdater) {
  CAMLparam1(vUpdater);
  CAMLlocal1(vStr);
  
  SUUpdater *updater = oni2_unwrapPointer(vUpdater);
  NSURL *nsUrl = [updater feedURL];
  NSString *nsStr = [nsUrl absoluteString];
  vStr = caml_copy_string([nsStr UTF8String]);

  CAMLreturn(vStr);
}

CAMLprim value oni2_SparkleSetAutomaticallyChecksForUpdates(value vUpdater, value vChecks) {
  CAMLparam2(vUpdater, vChecks);

  SUUpdater *updater = oni2_unwrapPointer(vUpdater);

  NSLog(@"Setting Updater automatic checks: %d", Bool_val(vChecks));

  [updater setAutomaticallyChecksForUpdates:Bool_val(vChecks)];

  CAMLreturn(Val_unit);
}

CAMLprim value oni2_SparkleGetAutomaticallyChecksForUpdates(value vUpdater) {
  CAMLparam1(vUpdater);

  SUUpdater *updater = oni2_unwrapPointer(vUpdater);
  int checks = [updater automaticallyChecksForUpdates];

  CAMLreturn(Val_bool(checks));
}

CAMLprim value oni2_SparkleCheckForUpdates(value vUpdater) {
  CAMLparam1(vUpdater);

  SUUpdater *updater = oni2_unwrapPointer(vUpdater);
  [updater checkForUpdates:NULL];

  NSLog(@"Checking for updates");

  CAMLreturn(Val_unit);
}

#elif USE_WIN_SPARKLE

#include "winsparkle.h"

// On WinSparkle, there is no instance -- the library just uses global variables.
// So here we just return a basic `unit`
CAMLprim value oni2_SparkleGetSharedInstance() {
  CAMLparam0();

  CAMLreturn(Val_unit);
}

// This function doesn't make too much sense since we lose the OOP aspect on
// Windows, so it's basically a noop.
CAMLprim value oni2_SparkleDebugToString(value vData) {
  CAMLparam1(vData);
  CAMLlocal1(vStr);

  vStr = caml_copy_string("Unimplemented");

  CAMLreturn(vStr);
}

// Again, this function doesn't make much sense on Windows, so it's a noop.
CAMLprim value oni2_SparkleDebugLog(value vData) {
  CAMLparam1(vData);
  
  CAMLreturn(Val_unit);
}

CAMLprim value oni2_SparkleSetFeedURL(value vUpdater, value vUrlStr) {
  CAMLparam2(vUpdater, vUrlStr);

  win_sparkle_set_appcast_url(String_val(vUrlStr));

  CAMLreturn(Val_unit);
}

CAMLprim value oni2_SparkleGetFeedURL(value vUpdater) {
  CAMLparam1(vUpdater);
  CAMLlocal1(vStr);
  
  vStr = caml_copy_string("Unimplemented");

  CAMLreturn(vStr);
}

CAMLprim value oni2_SparkleSetAutomaticallyChecksForUpdates(value vUpdater, value vChecks) {
  CAMLparam2(vUpdater, vChecks);

  win_sparkle_set_automatic_check_for_updates(Bool_val(vChecks));

  CAMLreturn(Val_unit);
}

CAMLprim value oni2_SparkleGetAutomaticallyChecksForUpdates(value vUpdater) {
  CAMLparam1(vUpdater);

  int checks = win_sparkle_get_automatic_check_for_updates();

  CAMLreturn(Val_bool(checks));
}

CAMLprim value oni2_SparkleCheckForUpdates(value vUpdater) {
  CAMLparam1(vUpdater);

  win_sparkle_check_update_with_ui();

  CAMLreturn(Val_unit);
}

#endif
