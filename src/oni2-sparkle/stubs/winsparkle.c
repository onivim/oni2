#include "config.h"

#include <caml/alloc.h>
#include <caml/callback.h>
#include <caml/memory.h>
#include <caml/mlvalues.h>
#include <caml/threads.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef USE_WIN_SPARKLE

#include <windows.h>
#include <shlwapi.h>

#include "winsparkle.h"

void oni2_WinSparkleCloseCallback() {
  static const value *closeCallback;
  if (closeCallback == NULL) {
    closeCallback = caml_named_value("oni2_close");
  }

  if (closeCallback != NULL) {
    caml_c_thread_register();
    caml_acquire_runtime_system();
    caml_callback(*closeCallback, Val_unit);
    caml_release_runtime_system();
  }
}

CAMLprim value oni2_SparkleInit() {
  CAMLparam0();

  char iniPath[MAX_PATH];

  GetModuleFileName(NULL, iniPath, MAX_PATH);
  PathRemoveFileSpec(iniPath);

  strcat(iniPath, "\\Oni2.ini");
  char version[16];
  wchar_t versionWide[16];
  

  GetPrivateProfileString("Application", "Version", "", version, 16, iniPath);

  mbstowcs(versionWide, version, 16);

  win_sparkle_set_app_details(L"Outrun Labs LLC", L"Onivim 2", versionWide);
  win_sparkle_set_shutdown_request_callback(oni2_WinSparkleCloseCallback);
  win_sparkle_init();

  CAMLreturn(Val_unit);
}

CAMLprim value oni2_SparkleVersion() {
  CAMLparam0();
  CAMLlocal1(vVersion);

  vVersion = caml_copy_string(WIN_SPARKLE_VERSION_STRING);

  CAMLreturn(vVersion);
}

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

  char *urlStr = caml_stat_strdup(String_val(vUrlStr));

  win_sparkle_set_appcast_url(urlStr);
  
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
