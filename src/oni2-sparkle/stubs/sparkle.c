#include "config.h"

#include <caml/alloc.h>
#include <caml/callback.h>
#include <caml/memory.h>
#include <caml/mlvalues.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef USE_SPARKLE
#import <Sparkle/Sparkle.h>

#include "utils.h"

CAMLprim value oni2_SparkleInit() {
  CAMLparam0();

  CAMLreturn(Val_unit);
}

CAMLprim value oni2_SparkleVersion() {
  CAMLparam0();
  CAMLlocal1(vVersion);

  NSDictionary *infoDictionary = [[NSBundle bundleForClass: [SUUpdater class]] infoDictionary];
  NSString *version = [infoDictionary valueForKey:(__bridge NSString*)kCFBundleVersionKey];

  vVersion = caml_copy_string([version UTF8String]);

  CAMLreturn(vVersion);
}

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

#endif
