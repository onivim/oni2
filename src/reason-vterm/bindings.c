#include <stdio.h>
#include <string.h>

#include <caml/alloc.h>
#include <caml/bigarray.h>
#include <caml/callback.h>
#include <caml/fail.h>
#include <caml/memory.h>
#include <caml/mlvalues.h>
#include <caml/threads.h>

#include <vterm.h>

static value reason_libvterm_Val_color(const VTermColor *pColor) {

  // Colors are packed as follows:
  // [ 8-bit red] [8-bit green] [8-bit blue / index] [2 control bits] (least
  // significant bits) Bits 0 & 1 - determine type of color:
  // - 0: Background
  // - 1: Foreground
  // - 2: RGB Color (24-bit)
  // - 3: Index color (256 colors)
  // Bits 2-9 - either the blue color, or the index
  // Bits 10-17 - green color, if RGB
  // Bits 18-25 - blue color, if RGB
  int colorVal = 0;
  if (VTERM_COLOR_IS_DEFAULT_BG(pColor)) {
    colorVal = 0;
  } else if (VTERM_COLOR_IS_DEFAULT_FG(pColor)) {
    colorVal = 1;
  } else if (VTERM_COLOR_IS_RGB(pColor)) {
    colorVal = 2 + (pColor->rgb.red << 18) + (pColor->rgb.green << 10) +
               (pColor->rgb.blue << 2);
  } else {
    colorVal = 3 + (pColor->indexed.idx << 2);
  }

  return Val_int(colorVal);
}

static value reason_libvterm_Val_screencell(const VTermScreenCell *pScreenCell) {
  CAMLparam0();
  CAMLlocal1(ret);

  uint32_t c = pScreenCell->chars[0];

  ret = caml_alloc(4, 0);
  Store_field(ret, 0, Val_int(c));

  int isReverse = pScreenCell->attrs.reverse;

  value originalForeground = reason_libvterm_Val_color(&pScreenCell->fg);
  value originalBackground = reason_libvterm_Val_color(&pScreenCell->bg);

  value fg = isReverse ? originalBackground : originalForeground;
  value bg = isReverse ? originalForeground : originalBackground;

  Store_field(ret, 1, fg);
  Store_field(ret, 2, bg);
  int style = 0 + (pScreenCell->attrs.bold ? 1 : 0) +
              (pScreenCell->attrs.italic ? 2 : 0) +
              (pScreenCell->attrs.underline ? 4 : 0);

  Store_field(ret, 3, Val_int(style));
  CAMLreturn(ret);
}

void reason_libvterm_onOutputF(const char *s, size_t len, void *user) {
  CAMLparam0();
  CAMLlocal1(ret);

  static value *reason_libvterm_onOutput = NULL;

  if (reason_libvterm_onOutput == NULL) {
    reason_libvterm_onOutput =
        (value *)caml_named_value("reason_libvterm_onOutput");
  }

  ret = caml_alloc_string(len);
  memcpy((char*)String_val(ret), s, len);

  caml_callback2(*reason_libvterm_onOutput, Val_int(user), ret);

  CAMLreturn0;
}

int reason_libvterm_onScreenSetTermPropF(VTermProp prop, VTermValue *val,
                                         void *user) {
  CAMLparam0();
  CAMLlocal2(ret, str);

  switch (prop) {
  case VTERM_PROP_CURSORVISIBLE:
    ret = caml_alloc(1, 0);
    Store_field(ret, 0, Val_bool(val->boolean));
    break;
  case VTERM_PROP_CURSORBLINK:
    ret = caml_alloc(1, 1);
    Store_field(ret, 0, Val_bool(val->boolean));
    break;
  case VTERM_PROP_ALTSCREEN:
    ret = caml_alloc(1, 2);
    Store_field(ret, 0, Val_bool(val->boolean));
    break;
  case VTERM_PROP_TITLE:
    ret = caml_alloc(1, 3);
    str = caml_copy_string(val->string.str);
    Store_field(ret, 0, str);
    break;
  case VTERM_PROP_ICONNAME:
    ret = caml_alloc(1, 4);
    str = caml_copy_string(val->string.str);
    Store_field(ret, 0, str);
    break;
  case VTERM_PROP_REVERSE:
    ret = caml_alloc(1, 5);
    Store_field(ret, 0, Val_bool(val->boolean));
    break;
  case VTERM_PROP_CURSORSHAPE:
    ret = caml_alloc(1, 6);
    Store_field(ret, 0, Val_int(val->number));
    break;
  case VTERM_PROP_MOUSE:
    ret = caml_alloc(1, 7);
    Store_field(ret, 0, Val_int(val->number));
    break;
  default:
    ret = Val_int(0);
    break;
  }

  static value *reason_libvterm_onScreenSetTermProp = NULL;

  if (reason_libvterm_onScreenSetTermProp == NULL) {
    reason_libvterm_onScreenSetTermProp =
        (value *)caml_named_value("reason_libvterm_onScreenSetTermProp");
  }

  caml_callback2(*reason_libvterm_onScreenSetTermProp, Val_int(user), ret);
  CAMLreturn(0);
}

int reason_libvterm_onScreenBellF(void *user) {
  CAMLparam0();

  static value *reason_libvterm_onScreenBell = NULL;

  if (reason_libvterm_onScreenBell == NULL) {
    reason_libvterm_onScreenBell =
        (value *)caml_named_value("reason_libvterm_onScreenBell");
  }

  caml_callback(*reason_libvterm_onScreenBell, Val_int(user));

  CAMLreturn(0);
}

int reason_libvterm_onScreenMoveRectF(VTermRect dest, VTermRect src,
                                      void *user) {
  CAMLparam0();

  static value *reason_libvterm_onScreenMoveRect = NULL;

  if (reason_libvterm_onScreenMoveRect == NULL) {
    reason_libvterm_onScreenMoveRect =
        (value *)caml_named_value("reason_libvterm_onScreenMoveRect");
  }

  value *pArgs = (value *)malloc(sizeof(value) * 9);
  pArgs[0] = Val_int(user);
  pArgs[1] = Val_int(dest.start_row);
  pArgs[2] = Val_int(dest.start_col);
  pArgs[3] = Val_int(dest.end_row);
  pArgs[4] = Val_int(dest.end_col);
  pArgs[5] = Val_int(src.start_row);
  pArgs[6] = Val_int(src.start_col);
  pArgs[7] = Val_int(src.end_row);
  pArgs[8] = Val_int(src.end_col);

  caml_callbackN(*reason_libvterm_onScreenMoveRect, 9, pArgs);
  free(pArgs);

  CAMLreturn(0);
}

int reason_libvterm_onScreenMoveCursorF(VTermPos pos, VTermPos oldPos,
                                        int visible, void *user) {
  CAMLparam0();

  static value *reason_libvterm_onScreenMoveCursor = NULL;

  if (reason_libvterm_onScreenMoveCursor == NULL) {
    reason_libvterm_onScreenMoveCursor =
        (value *)caml_named_value("reason_libvterm_onScreenMoveCursor");
  }

  value *pArgs = (value *)malloc(sizeof(value) * 6);
  pArgs[0] = Val_int(user);
  pArgs[1] = Val_int(pos.row);
  pArgs[2] = Val_int(pos.col);
  pArgs[3] = Val_int(oldPos.row);
  pArgs[4] = Val_int(oldPos.col);
  pArgs[5] = Val_bool(visible);

  caml_callbackN(*reason_libvterm_onScreenMoveCursor, 6, pArgs);
  free(pArgs);

  CAMLreturn(0);
}

int reason_libvterm_onScreenSbPushLineF(int cols, const VTermScreenCell *cells,
                                        void *user) {
  CAMLparam0();
  CAMLlocal1(ret);

  ret = caml_alloc(cols, 0);

  for (int i = 0; i < cols; i++) {
    Store_field(ret, i, reason_libvterm_Val_screencell(&cells[i]));
  }

  static value *reason_libvterm_onScreenSbPushLine = NULL;

  if (reason_libvterm_onScreenSbPushLine == NULL) {
    reason_libvterm_onScreenSbPushLine =
        (value *)caml_named_value("reason_libvterm_onScreenSbPushLine");
  }

  caml_callback2(*reason_libvterm_onScreenSbPushLine, Val_int(user), ret);

  CAMLreturn(0);
}

int reason_libvterm_onScreenSbPopLineF(int cols, VTermScreenCell *cells,
                                       void *user) {
  CAMLparam0();
  CAMLlocal1(ret);

  ret = caml_alloc(cols, 0);

  for (int i = 0; i < cols; i++) {
    Store_field(ret, i, reason_libvterm_Val_screencell(&cells[i]));
  }

  static value *reason_libvterm_onScreenSbPopLine = NULL;

  if (reason_libvterm_onScreenSbPopLine == NULL) {
    reason_libvterm_onScreenSbPopLine =
        (value *)caml_named_value("reason_libvterm_onScreenSbPopLine");
  }

  caml_callback2(*reason_libvterm_onScreenSbPopLine, Val_int(user), ret);

  CAMLreturn(0);
}

int reason_libvterm_onScreenResizeF(int rows, int cols, void *user) {
  CAMLparam0();

  static value *reason_libvterm_onScreenResize = NULL;

  if (reason_libvterm_onScreenResize == NULL) {
    reason_libvterm_onScreenResize =
        (value *)caml_named_value("reason_libvterm_onScreenResize");
  }

  caml_callback3(*reason_libvterm_onScreenResize, Val_int(user), Val_int(rows),
                 Val_int(cols));
  CAMLreturn(0);
}

int reason_libvterm_onScreenDamageF(VTermRect rect, void *user) {
  CAMLparam0();
  CAMLlocal1(outRect);

  outRect = caml_alloc(4, 0);
  Store_field(outRect, 0, Val_int(rect.start_row));
  Store_field(outRect, 1, Val_int(rect.end_row));
  Store_field(outRect, 2, Val_int(rect.start_col));
  Store_field(outRect, 3, Val_int(rect.end_col));
  static value *reason_libvterm_onScreenDamage = NULL;

  if (reason_libvterm_onScreenDamage == NULL) {
    reason_libvterm_onScreenDamage =
        (value *)caml_named_value("reason_libvterm_onScreenDamage");
  }

  caml_callback2(*reason_libvterm_onScreenDamage, Val_int(user), outRect);
  CAMLreturn(0);
}

int VTermMod_val(value vMod) {
  switch (Int_val(vMod)) {
  case 0:
    return VTERM_MOD_NONE;
  case 1:
    return VTERM_MOD_SHIFT;
  case 2:
    return VTERM_MOD_ALT;
  case 3:
    return VTERM_MOD_CTRL;
  case 4:
    return VTERM_ALL_MODS_MASK;
  default:
    return VTERM_MOD_NONE;
  }
}

VTermKey VTermKey_val(value vKey) {

  if (Is_block(vKey)) {
    return VTERM_KEY_NONE;
  }

  switch (Int_val(vKey)) {
  case 0:
    return VTERM_KEY_ENTER;
  case 1:
    return VTERM_KEY_TAB;
  case 2:
    return VTERM_KEY_BACKSPACE;
  case 3:
    return VTERM_KEY_ESCAPE;
  case 4:
    return VTERM_KEY_UP;
  case 5:
    return VTERM_KEY_DOWN;
  case 6:
    return VTERM_KEY_LEFT;
  case 7:
    return VTERM_KEY_RIGHT;
  case 8:
    return VTERM_KEY_INS;
  case 9:
    return VTERM_KEY_DEL;
  case 10:
    return VTERM_KEY_HOME;
  case 11:
    return VTERM_KEY_END;
  case 12:
    return VTERM_KEY_PAGEUP;
  case 13:
    return VTERM_KEY_PAGEDOWN;
  default:
    return VTERM_KEY_NONE;
  }
}

CAMLprim value reason_libvterm_vterm_screen_enable_altscreen(value vTerm,
                                                             value vAlt) {
  CAMLparam2(vTerm, vAlt);

  VTerm *pTerm = (VTerm *)vTerm;
  int altScreenEnabled = Int_val(vAlt);
  VTermScreen *pScreen = vterm_obtain_screen(pTerm);

  vterm_screen_enable_altscreen(pScreen, altScreenEnabled);

  CAMLreturn(Val_unit);
}

CAMLprim value reason_libvterm_vterm_screen_get_cell(value vTerm, value vRow,
                                                     value vCol) {
  CAMLparam3(vTerm, vRow, vCol);
  CAMLlocal1(ret);

  int row = Int_val(vRow);
  int col = Int_val(vCol);
  VTerm *pTerm = (VTerm *)vTerm;
  VTermScreen *pScreen = vterm_obtain_screen(pTerm);

  VTermPos pos;
  pos.row = row;
  pos.col = col;

  VTermScreenCell cell;
  vterm_screen_get_cell(pScreen, pos, &cell);

  ret = reason_libvterm_Val_screencell(&cell);

  CAMLreturn(ret);
}

CAMLprim value reason_libvterm_vterm_keyboard_unichar(value vTerm, value vChar,
                                                      value vMod) {
  CAMLparam3(vTerm, vChar, vMod);

  VTerm *pTerm = (VTerm *)vTerm;
  uint32_t c = Int32_val(vChar);
  VTermModifier mod = VTermMod_val(vMod);
  vterm_keyboard_unichar(pTerm, c, mod);

  CAMLreturn(Val_unit);
}

CAMLprim value reason_libvterm_vterm_keyboard_key(value vTerm, value vKey,
                                                  value vMod) {
  CAMLparam3(vTerm, vKey, vMod);

  VTerm *pTerm = (VTerm *)vTerm;
  VTermKey key = VTermKey_val(vKey);
  VTermModifier mod = VTermMod_val(vMod);
  vterm_keyboard_key(pTerm, key, mod);

  CAMLreturn(Val_unit);
}

static VTermScreenCallbacks reason_libvterm_screen_callbacks = {
    .bell = &reason_libvterm_onScreenBellF,
    .resize = &reason_libvterm_onScreenResizeF,
    .damage = &reason_libvterm_onScreenDamageF,
    .moverect = &reason_libvterm_onScreenMoveRectF,
    .movecursor = &reason_libvterm_onScreenMoveCursorF,
    .settermprop = &reason_libvterm_onScreenSetTermPropF,
    .sb_pushline = &reason_libvterm_onScreenSbPushLineF,
    .sb_popline = &reason_libvterm_onScreenSbPopLineF,
};

CAMLprim value reason_libvterm_vterm_new(value vId, value vRows, value vCol) {
  CAMLparam3(vId, vRows, vCol);

  // Store the id of the terminal in the user data we pass to libvterm..
  // Some coercion gymnatics to convert from Int -> void *
  void *id = (void *)(uintptr_t)Int_val(vId);

  int rows = Int_val(vRows);
  int cols = Int_val(vCol);
  VTerm *pTerm = vterm_new(rows, cols);
  // vterm_set_utf8(pTerm, true);
  vterm_output_set_callback(pTerm, &reason_libvterm_onOutputF, id);
  VTermScreen *pScreen = vterm_obtain_screen(pTerm);
  vterm_screen_set_callbacks(pScreen, &reason_libvterm_screen_callbacks, id);
  vterm_screen_reset(pScreen, 1);
  CAMLreturn((value)pTerm);
}

CAMLprim value reason_libvterm_vterm_free(value vTerm) {
  CAMLparam1(vTerm);
  VTerm *pTerm = (VTerm *)vTerm;
  vterm_free(pTerm);
  CAMLreturn(Val_unit);
}

CAMLprim value reason_libvterm_vterm_set_utf8(value vTerm, value vUtf8) {
  CAMLparam2(vTerm, vUtf8);
  VTerm *pTerm = (VTerm *)vTerm;
  int isUtf8 = Bool_val(vUtf8);
  vterm_set_utf8(pTerm, isUtf8);
  CAMLreturn(Val_unit);
}

CAMLprim value reason_libvterm_vterm_get_utf8(value vTerm) {
  CAMLparam1(vTerm);
  VTerm *pTerm = (VTerm *)vTerm;
  int isUtf8 = vterm_get_utf8(pTerm);
  CAMLreturn(Val_int(isUtf8));
}

CAMLprim value reason_libvterm_vterm_get_size(value vTerm) {
  CAMLparam1(vTerm);
  CAMLlocal1(ret);
  VTerm *pTerm = (VTerm *)vTerm;
  int rows, cols;
  vterm_get_size(pTerm, &rows, &cols);

  ret = caml_alloc(2, 0);
  Store_field(ret, 0, Val_int(rows));
  Store_field(ret, 1, Val_int(cols));

  CAMLreturn(ret);
}

CAMLprim value reason_libvterm_vterm_set_size(value vTerm, value vSize) {
  CAMLparam2(vTerm, vSize);
  VTerm *pTerm = (VTerm *)vTerm;
  int rows = Int_val(Field(vSize, 0));
  int cols = Int_val(Field(vSize, 1));
  vterm_set_size(pTerm, rows, cols);
  CAMLreturn(Val_unit);
}

CAMLprim value reason_libvterm_vterm_input_write(value vTerm, value vStr) {
  CAMLparam2(vTerm, vStr);
  VTerm *pTerm = (VTerm *)vTerm;
  int len = caml_string_length(vStr);
  char *bytes = caml_stat_strdup(String_val(vStr));
  int ret = vterm_input_write(pTerm, bytes, len);
  free(bytes);
  CAMLreturn(Val_int(ret));
}
