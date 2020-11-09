#ifdef __APPLE__

#import <Cocoa/Cocoa.h>
#import <Carbon/Carbon.h>

#include <caml/alloc.h>
#include <caml/callback.h>
#include <caml/memory.h>
#include <caml/mlvalues.h>
#include <caml/threads.h>


#define UNWRAP_REF(ref) (Field(ref, 0))

NSString *oni2_priv_GetCurrentKeyboardLayout() {
  TISInputSourceRef source = TISCopyCurrentKeyboardInputSource();
  CFStringRef sourceID = (CFStringRef)TISGetInputSourceProperty(source, kTISPropertyInputSourceID);

  return (NSString *)sourceID;
}

NSString *oni2_priv_GetCurrentKeyboardLanguage() {
  TISInputSourceRef source = TISCopyCurrentKeyboardInputSource();
  NSArray *langs = (NSArray *)TISGetInputSourceProperty(source, kTISPropertyInputSourceLanguages);
  NSString *lang = (NSString *)[langs objectAtIndex:0];

  return lang;
}

static void notificationHandler(
  CFNotificationCenterRef center, 
  void *manager,
  CFStringRef name, 
  const void *object, 
  CFDictionaryRef userInfo
) {
  // Maintain a reference to the callbacks from the OCcaml side
  // We let OCaml manage the list since they have better abstractions than we do
  static const value *callbackListRef = NULL;
  // If we haven't gotten the reference yet, get it.
  if (callbackListRef == NULL) {
    callbackListRef = caml_named_value("oni2_KeyboardLayoutCallbackListRef");
    // If we didn't get anything back from the OCaml runtime, return early.
    if (callbackListRef == NULL) {
      NSLog(@"Unable to acquire callback list reference!");
      return;
    }
  }

  NSString *nsLayout = oni2_priv_GetCurrentKeyboardLayout();
  NSString *nsLanguage = oni2_priv_GetCurrentKeyboardLanguage();

  value vLayout = caml_copy_string([nsLayout UTF8String]);
  value vLanguage = caml_copy_string([nsLanguage UTF8String]);
  value args[] = {vLanguage, vLayout};

  value callbackList = UNWRAP_REF(*callbackListRef);

  caml_c_thread_register();
  caml_acquire_runtime_system();

  while (callbackList != Val_emptylist) {
    value callback = Field(callbackList, 0);
    caml_callbackN(callback, 2, args);
    callbackList = Field(callbackList, 1);
  }

  caml_release_runtime_system();
}

CAMLprim value oni2_KeyboardLayoutInit() {
  CAMLparam0();

  CFNotificationCenterAddObserver(
    CFNotificationCenterGetDistributedCenter(),
    NULL,
    notificationHandler,
    kTISNotifySelectedKeyboardInputSourceChanged,
    NULL,
    CFNotificationSuspensionBehaviorDeliverImmediately
  );

  CAMLreturn(Val_unit);
}



#endif
