#ifdef _WIN32
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0601

#undef WINVER
#define WINVER 0x0601

#define SPACE_SCAN_CODE 0x0039

// Parameter to pass to ToUnicodeEx: https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-tounicodeex#parameters
#define DONT_CHANGE_KEYBOARD_STATE 4

#include <windows.h>
#include <stdint.h>

#include <caml/alloc.h>
#include <caml/callback.h>
#include <caml/memory.h>
#include <caml/mlvalues.h>
#include <caml/threads.h>

#include <SDL2/SDL.h>

#include "keyboard-layout.h"

typedef struct KeycodeMapEntry {
  unsigned int scancode;
  const char *dom3code;
  SDL_Scancode sdlScancode;
  SDL_Keycode sdlKeycode;
} KeycodeMapEntry;

#define USB_KEYMAP_DECLARATION static const KeycodeMapEntry keyCodeMap[] =
#define USB_KEYMAP(usb, evdev, xkb, win, mac, code, id, sdlScancode, sdlKeycode) {win, code, sdlScancode, sdlKeycode}

#include "keycode_converter_data.h"

HKL getForegroundWindowHKL() {
  DWORD dwThreadID = 0;
  HWND hWnd = GetForegroundWindow();
  if (hWnd != NULL) {
    dwThreadID = GetWindowThreadProcessId(hWnd, NULL);
  }
  return GetKeyboardLayout(dwThreadID);
}

CAMLprim value oni2_KeyboardLayoutGetCurrentLayout() {
  CAMLparam0();
  CAMLlocal1(vLayout);

  ActivateKeyboardLayout(getForegroundWindowHKL(), 0);
  char layoutName[KL_NAMELENGTH];
  if (GetKeyboardLayoutName(layoutName))
    vLayout = caml_copy_string(layoutName);
  else
    vLayout = caml_copy_string("");

  CAMLreturn(vLayout);
}

CAMLprim value oni2_KeyboardLayoutGetCurrentLanguage() {
  CAMLparam0();
  CAMLlocal1(vLanguage);

  HKL layout = getForegroundWindowHKL();

  wchar_t wBuf[LOCALE_NAME_MAX_LENGTH];
  int wLen = LCIDToLocaleName(MAKELCID((size_t)layout & 0xFFFF, SORT_DEFAULT), wBuf, LOCALE_NAME_MAX_LENGTH, 0);

  int bufLen = (wLen + 1) * sizeof(char) * 4;
  char buf[bufLen];

  int realLen = WideCharToMultiByte(CP_UTF8, 0, wBuf, wLen, buf, bufLen, NULL, NULL);
  buf[realLen] = '\0';

  vLanguage = caml_copy_string(buf);

  CAMLreturn(vLanguage);
}

CAMLprim value oni2_KeyboardLayoutInit() {
  CAMLparam0();

  CAMLreturn(Val_unit);
}

#define UTF16_BUFFER_SIZE 40
#define UTF8_BUFFER_SIZE 10

int keyboardCharacterToUTF8(UINT keycode, UINT scancode, BYTE *keyboardState, char* dest, int destSize, HKL keyboardLayout) {
  wchar_t destUtf16[UTF16_BUFFER_SIZE];

  // CAREFUL: ToUnicodeEx has the side-effect of resetting dead key state, as well as others - see:
  // https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-tounicodeex#remarks
  // Fix #3157, for Windows 10 >1607 by passing bit 2 to `wFlags`
  int utf16Count = ToUnicodeEx(keycode, scancode, keyboardState, destUtf16, UTF16_BUFFER_SIZE, DONT_CHANGE_KEYBOARD_STATE, keyboardLayout);

  if (utf16Count == -1) {
    return -1;
  } else {
    // We have a UTF-16 string for the keycode - now convert it to 

    int utf8Count = WideCharToMultiByte(
      CP_UTF8,
      0, 
      destUtf16,
      utf16Count,
      dest,
      destSize,
      NULL,
      NULL
    );
    return utf8Count;
  }
}

void characterForNativeCode(
  HKL keyboardLayout,
  UINT keycode, 
  UINT scancode, 
  BYTE *keyboardState, 
  int shift, 
  int altGraph,
  char *dest,
  int destSize
) {
  memset(keyboardState, 0, 256);

  if (shift) {
    keyboardState[VK_SHIFT] = 0x80;
  }

  if (altGraph) {
    keyboardState[VK_MENU] = 0x80;
    keyboardState[VK_CONTROL] = 0x80;
  }

  int count = keyboardCharacterToUTF8(keycode, scancode, keyboardState, dest, destSize, keyboardLayout);

  if (count == -1) {
    keyboardState[VK_SHIFT] = 0x0;
    keyboardState[VK_MENU] = 0x0;
    keyboardState[VK_CONTROL] = 0x0;

    dest[0] = '\0';
  } else if (count > 0 && !iswcntrl(dest[0])) {
    dest[count] = '\0';
  } else {
    dest[0] = '\0';
  }
}

CAMLprim value oni2_KeyboardLayoutPopulateCurrentKeymap(value keymap, value Hashtbl_replace) {
  CAMLparam2(keymap, Hashtbl_replace);
  CAMLlocal2(keymapEntry, vSDLScancode);

  BYTE keyboardState[256];
  HKL keyboardLayout = getForegroundWindowHKL();

  // Allocate UTF-8 Buffers to be populated
  char unmodified[UTF8_BUFFER_SIZE]; 
  char withShift[UTF8_BUFFER_SIZE]; 
  char withAltGraph[UTF8_BUFFER_SIZE]; 
  char withAltGraphShift[UTF8_BUFFER_SIZE]; 

  size_t keyCodeMapSize = sizeof(keyCodeMap) / sizeof(keyCodeMap[0]);
  for (size_t i = 0; i < keyCodeMapSize; i++) {
    SDL_Scancode sdlScancode = keyCodeMap[i].sdlScancode;
    unsigned int scancode = keyCodeMap[i].scancode;

    if (sdlScancode && scancode > 0x0000) {
      unsigned int keycode = MapVirtualKeyEx(scancode, MAPVK_VSC_TO_VK, keyboardLayout);

      if ((MapVirtualKeyEx(keycode, MAPVK_VK_TO_CHAR, keyboardLayout) >> (sizeof(UINT) * 8 - 1)))
        continue;

      characterForNativeCode(keyboardLayout, keycode, scancode, keyboardState, 0, 0, unmodified, UTF8_BUFFER_SIZE);
      characterForNativeCode(keyboardLayout, keycode, scancode, keyboardState, 1, 0, withShift, UTF8_BUFFER_SIZE);
      characterForNativeCode(keyboardLayout, keycode, scancode, keyboardState, 0, 1, withAltGraph, UTF8_BUFFER_SIZE);
      characterForNativeCode(keyboardLayout, keycode, scancode, keyboardState, 1, 1, withAltGraphShift, UTF8_BUFFER_SIZE);

      keymapEntry = createKeymapEntry(
        (char *)unmodified,
        (char *)withShift,
        (char *)withAltGraph,
        (char *)withAltGraphShift
      );

      vSDLScancode = Val_int(sdlScancode);

      value args[] = {keymap, vSDLScancode, keymapEntry};
      caml_callbackN(Hashtbl_replace, 3, args);
    }
  }

  CAMLreturn(Val_unit);
}

#endif
