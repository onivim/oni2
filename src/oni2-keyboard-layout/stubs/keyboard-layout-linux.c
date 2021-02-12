#ifdef __linux__

#include <stdio.h>
#include <stdbool.h>
#include <wctype.h>

#include <X11/XKBlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XKBrules.h>

#include <caml/alloc.h>
#include <caml/callback.h>
#include <caml/memory.h>
#include <caml/mlvalues.h>
#include <caml/threads.h>
#include <caml/fail.h>

#include <SDL2/SDL.h>

#include "keyboard-layout.h"


// We want to fail if we get null data anywhere, especially in initialization
#define RAISE_NULL char message[64]; \
sprintf(message, "NULL data at %s:%d", __FILE__, __LINE__); \
caml_failwith(message)

#define RAISE_IF_UNINITIALIZED if (xDisplay == NULL) \
caml_failwith("Oni2_KeyboardLayout not initialized!")

typedef struct KeycodeMapEntry {
  uint xkbKeycode;
  const char *dom3Code;
  SDL_Scancode sdlScancode;
  SDL_Keycode sdlKeycode;
} KeycodeMapEntry;

#define USB_KEYMAP_DECLARATION static const KeycodeMapEntry keyCodeMap[] =
#define USB_KEYMAP(usb, evdev, xkb, win, mac, code, id, sdlScancode, sdlKeycode) {xkb, code, sdlScancode, sdlKeycode}

#include "keycode_converter_data.h"

// In the atom JS lib, these are all attributes of the Manager class
// Since we aren't using OOP, they can just be globals

Display *xDisplay;
XIC xInputContext;
XIM xInputMethod;

CAMLprim value oni2_KeyboardLayoutInit() {
  CAMLparam0();

  xDisplay = XOpenDisplay("");
  if (xDisplay == NULL) {
    RAISE_NULL;
  }

  xInputMethod = XOpenIM(xDisplay, 0, 0, 0);
  if (xInputMethod == NULL) {
    RAISE_NULL;
  }
  
  XIMStyles *styles = NULL;
  XGetIMValues(xInputMethod, XNQueryInputStyle, &styles, NULL);
  if (styles == NULL) {
    RAISE_NULL;
  }

  XIMStyle bestMatchStyle = 0;
  for (int i = 0; i < styles->count_styles; i++) {
    XIMStyle thisStyle = styles->supported_styles[i];
    if (thisStyle == (XIMPreeditNothing | XIMStatusNothing)) {
      bestMatchStyle = thisStyle;
      break;
    }
  }

  XFree(styles);
  if (bestMatchStyle == 0) {
    RAISE_NULL;
  } 
  
  Window window;
  int revertTo;
  XGetInputFocus(xDisplay, &window, &revertTo);

  if (window != BadRequest) {
    xInputContext = XCreateIC(
      xInputMethod,
      XNInputStyle,
      bestMatchStyle,
      XNClientWindow,
      window,
      XNFocusWindow,
      window,
      NULL
    );
  }

  CAMLreturn(Val_unit);
}


void oni2_priv_GetCurrentKeyboardLayout(char *layout) {
  XkbRF_VarDefsRec vdr;
  if (XkbRF_GetNamesProp(xDisplay, NULL, &vdr) && vdr.layout) {
    XkbStateRec xkbState;
    XkbGetState(xDisplay, XkbUseCoreKbd, &xkbState);

    if (vdr.variant) {
      sprintf(layout, "%s,%s[%d]", vdr.layout, vdr.variant, xkbState.group);
    } else {
      sprintf(layout, "%s[%d]", vdr.layout, xkbState.group);
    }

    // TODO: Is there an actual unitializer for XkbRF_VarDefsRec?
    // Looks like we have to manually free this fields...
    if (vdr.variant) {
      free(vdr.variant);
    }

    if (vdr.layout) {
      free(vdr.layout);
    }

    if (vdr.options) {
      free(vdr.options);
    }

    if (vdr.model) {
      free(vdr.model);
    }
  } else {
    layout[0] = '\0';
  }
}

CAMLprim value oni2_KeyboardLayoutGetCurrentLayout() {
  CAMLparam0();
  CAMLlocal1(vLayout);

  RAISE_IF_UNINITIALIZED;

  char layout[256];
  oni2_priv_GetCurrentKeyboardLayout(layout);

  vLayout = caml_copy_string(layout);

  CAMLreturn(vLayout);
}

CAMLprim value oni2_KeyboardLayoutGetCurrentLanguage() {
  CAMLparam0();
  CAMLlocal1(vLanguage);

  RAISE_IF_UNINITIALIZED;

  char language[256];
  oni2_priv_GetCurrentKeyboardLayout(language);

  vLanguage = caml_copy_string(language);

  CAMLreturn(vLanguage);
}

void characterForNativeCode(
  XIC xInputContext,
  XKeyEvent *xKeyEvent, 
  uint xkbKeycode, 
  uint state,
  char *buffer
) {
  xKeyEvent->keycode = xkbKeycode;
  xKeyEvent->state = state;

  if (xInputContext) {
    int count = Xutf8LookupString(xInputContext, xKeyEvent, buffer, 2, NULL, NULL);
    if (count > 0 && !iswcntrl(buffer[0])) {
      buffer[count] = '\0';
    } else {
      buffer[0] = '\0';
    }
  } else {
    int count = XLookupString(xKeyEvent, buffer, 2, NULL, NULL);
    if (count > 0 && !iscntrl(buffer[0])) {
      buffer[count] = '\0';
    } else {
      buffer[0] = '\0';
    }
  }
}

CAMLprim value oni2_KeyboardLayoutPopulateCurrentKeymap(value keymap, value Hashtbl_replace) {
  CAMLparam2(keymap, Hashtbl_replace);
  CAMLlocal2(keymapEntry, vSDLScancode);

  RAISE_IF_UNINITIALIZED;

  // Allocate UTF-8 Buffers to be populated
  char unmodified[4] = {'\0'}; 
  char withShift[4] = {'\0'}; 
  char withAltGraph[4] = {'\0'};
  char withAltGraphShift[4] = {'\0'};

  XMappingEvent xEventMap = {MappingNotify, 0, false, xDisplay, 0, MappingKeyboard, 0, 0};
  XRefreshKeyboardMapping(&xEventMap);

  XkbStateRec xkbState;
  XkbGetState(xDisplay, XkbUseCoreKbd, &xkbState);
  uint keyboardBaseState = 0x0000;
  if (xkbState.group == 1) {
    keyboardBaseState = 0x2000;
  } else if (xkbState.group == 2) {
    keyboardBaseState = 0x4000;
  }

  XEvent xEvent;
  memset(&xEvent, 0, sizeof(XEvent));
  XKeyEvent *xKeyEvent = &xEvent.xkey;
  xKeyEvent->display = xDisplay;
  xKeyEvent->type = KeyPress;

  size_t keyCodeMapSize = sizeof(keyCodeMap) / sizeof(keyCodeMap[0]);
  for (size_t i = 0; i < keyCodeMapSize; i++) {
    SDL_Scancode sdlScancode = keyCodeMap[i].sdlScancode;
    uint xkbKeycode = keyCodeMap[i].xkbKeycode;

    if (sdlScancode && xkbKeycode > 0x0000) {
      characterForNativeCode(xInputContext, xKeyEvent, xkbKeycode, keyboardBaseState, unmodified);
      characterForNativeCode(xInputContext, xKeyEvent, xkbKeycode, keyboardBaseState | ShiftMask, withShift);
      characterForNativeCode(xInputContext, xKeyEvent, xkbKeycode, keyboardBaseState | Mod5Mask, withAltGraph);
      characterForNativeCode(xInputContext, xKeyEvent, xkbKeycode, keyboardBaseState | Mod5Mask | ShiftMask, withAltGraphShift);
      
      keymapEntry = createKeymapEntry(
        unmodified, 
        withShift, 
        withAltGraph, 
        withAltGraphShift
      );
      vSDLScancode = Val_int(sdlScancode);

      value args[] = {keymap, vSDLScancode, keymapEntry};
      caml_callbackN(Hashtbl_replace, 3, args);
    }
  }

  CAMLreturn(Val_unit);
}

#endif
