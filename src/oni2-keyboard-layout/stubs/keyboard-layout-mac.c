#ifdef __APPLE__

#import <Cocoa/Cocoa.h>
#import <Carbon/Carbon.h>

#include <caml/alloc.h>
#include <caml/callback.h>
#include <caml/memory.h>
#include <caml/mlvalues.h>
#include <caml/threads.h>

#include "keyboard-layout.h"

typedef struct KeycodeMapEntry {
  uint16_t virtualKeyCode;
  const char *dom3Code;
} KeycodeMapEntry;

#define USB_KEYMAP_DECLARATION static const KeycodeMapEntry keyCodeMap[] =
#define USB_KEYMAP(usb, evdev, xkb, win, mac, code, id) {mac, code}

#define UNWRAP_REF(ref) (Field(ref, 0))

#include "keycode_converter_data.h"

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
  CAMLparam0();
  CAMLlocal2(vLayout, vLanguage);
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

  vLayout = caml_copy_string([nsLayout UTF8String]);
  vLanguage = caml_copy_string([nsLanguage UTF8String]);
  value args[] = {vLanguage, vLayout};

  value callbackList = UNWRAP_REF(*callbackListRef);

  caml_c_thread_register();
  caml_acquire_runtime_system();

  // Loop through the callback list and call all of them with the langusage and layout
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

CAMLprim value oni2_KeyboardLayoutGetCurrentLayout() {
  CAMLparam0();
  CAMLlocal1(vLayout);

  vLayout = caml_copy_string([oni2_priv_GetCurrentKeyboardLayout() UTF8String]);

  CAMLreturn(vLayout);
}

CAMLprim value oni2_KeyboardLayoutGetCurrentLanguage() {
  CAMLparam0();
  CAMLlocal1(vLanguage);

  vLanguage = caml_copy_string([oni2_priv_GetCurrentKeyboardLanguage() UTF8String]);

  CAMLreturn(vLanguage);
}

/* Given a keyboard layout, a key code, and modifiers,
   fill a buffer of UTF-16 characters with the resulting
   string
 */
size_t characterForNativeCode(
  const UCKeyboardLayout *keyboardLayout,
  uint16_t virtualKeyCode,
  EventModifiers modifiers, 
  unichar *characters
) {
  uint32_t modifierKeyState = (modifiers >> 8) & 0xFF;
  uint32_t deadKeyState = 0;
  size_t charCount = 0;

  OSStatus status = UCKeyTranslate(
    keyboardLayout,
    (uint16_t)virtualKeyCode,
    kUCKeyActionDown,
    modifierKeyState,
    LMGetKbdLast(),
    kUCKeyTranslateNoDeadKeysBit,
    &deadKeyState,
    4 * sizeof(characters[0]),
    &charCount,
    characters
  );

  if (status == noErr && deadKeyState != 0) {
    status = UCKeyTranslate(
      keyboardLayout,
      (uint16_t)virtualKeyCode,
      kUCKeyActionDown,
      modifierKeyState,
      LMGetKbdLast(),
      kUCKeyTranslateNoDeadKeysBit,
      &deadKeyState,
      4 * sizeof(characters[0]),
      &charCount,
      characters
    );
  }

  if (status == noErr && !iscntrl(characters[0])) {
    return charCount;
  } else {
    return 0;
  }
}

CAMLprim value oni2_KeyboardLayoutPopulateCurrentKeymap(value keymap, value Hashtbl_replace) {
  CAMLparam2(keymap, Hashtbl_replace);
  CAMLlocal2(keymapEntry, vDom3Code);

  // Allocate string pointers to be filled
  const char *unmodified;
  const char *withShift;
  const char *withAltGraph;
  const char *withAltGraphShift;

  // Allocate UTF-16 Buffers to be populated
  unichar uniUnmodified[4]; 
  unichar uniWithShift[4]; 
  unichar uniWithAltGraph[4]; 
  unichar uniWithAltGraphShift[4]; 

  TISInputSourceRef source = TISCopyCurrentKeyboardInputSource();
  CFDataRef layoutData = (CFDataRef)TISGetInputSourceProperty(source, kTISPropertyUnicodeKeyLayoutData);

  if (layoutData == NULL) {
    CAMLreturn(Val_unit);
  }

  const UCKeyboardLayout *keyboardLayout = (UCKeyboardLayout *)CFDataGetBytePtr(layoutData);

  size_t keyCodeMapSize = sizeof(keyCodeMap) / sizeof(keyCodeMap[0]);
  for (size_t i = 0; i < keyCodeMapSize; i++) {
    const char *dom3Code = keyCodeMap[i].dom3Code;
    int virtualKeyCode = keyCodeMap[i].virtualKeyCode;

    if (dom3Code && virtualKeyCode < 0xffff) {
      int ccUnmodified = characterForNativeCode(
        keyboardLayout,
        virtualKeyCode,
        0, 
        uniUnmodified
      );
      int ccWithShift = characterForNativeCode(
        keyboardLayout,
        virtualKeyCode, 
        (1 << shiftKeyBit), 
        uniWithShift
      );
      int ccWithAltGraph = characterForNativeCode(
        keyboardLayout,
        virtualKeyCode,
        (1 << optionKeyBit),
        uniWithAltGraph
      );
      int ccWithAltGraphShift = characterForNativeCode(
        keyboardLayout,
        virtualKeyCode,
        (1 << shiftKeyBit) | (1 << optionKeyBit),
        uniWithAltGraphShift
      );

      // The NSStrings we allocate are simply mechanisms to convert the UTF-16 code points to UTF-8
      // so they should be autoreleased.
      @autoreleasepool {
        unmodified = [[NSString stringWithCharacters:uniUnmodified length:ccUnmodified] UTF8String];
        withShift = [[NSString stringWithCharacters:uniWithShift length:ccWithShift] UTF8String];
        withAltGraph = [[NSString stringWithCharacters:uniWithAltGraph length:ccWithAltGraph] UTF8String];
        withAltGraphShift = [[NSString stringWithCharacters:uniWithAltGraphShift length:ccWithAltGraphShift] UTF8String];
      }
      
      keymapEntry = createKeymapEntry(unmodified, withShift, withAltGraph, withAltGraphShift);
      vDom3Code = caml_copy_string(dom3Code);

      value args[] = {keymap, vDom3Code, keymapEntry};
      caml_callbackN(Hashtbl_replace, 3, args);
    }
  }

  CAMLreturn(Val_unit);
}



#endif
