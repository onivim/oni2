#include "libvim.h"
#include <caml/alloc.h>
#include <caml/bigarray.h>
#include <caml/callback.h>
#include <caml/memory.h>
#include <caml/mlvalues.h>
#include <caml/threads.h>

#define Val_none Val_int(0)

static value Val_some(value v) {
  CAMLparam1(v);
  CAMLlocal1(some);
  some = caml_alloc(1, 0);
  Store_field(some, 0, v);
  CAMLreturn(some);
}

static value Val_highlight(searchHighlight_T hl) {
  CAMLparam0();
  CAMLlocal1(ret);

  ret = caml_alloc(4, 0);
  Store_field(ret, 0, Val_int(hl.start.lnum));
  Store_field(ret, 1, Val_int(hl.start.col));
  Store_field(ret, 2, Val_int(hl.end.lnum));
  Store_field(ret, 3, Val_int(hl.end.col));

  CAMLreturn(ret);
}

void onBufferChanged(bufferUpdate_T bu) {
  static const value *lv_onBufferChanged = NULL;

  if (lv_onBufferChanged == NULL) {
    lv_onBufferChanged = caml_named_value("lv_onBufferChanged");
  }

  value *pArgs = (value *)malloc(sizeof(value) * 4);
  pArgs[0] = (value)bu.buf;
  pArgs[1] = Val_int(bu.lnum);
  pArgs[2] = Val_int(bu.lnume);
  pArgs[3] = Val_long(bu.xtra);

  caml_callbackN(*lv_onBufferChanged, 4, pArgs);

  free(pArgs);
}

int onAutoIndent(int lnum, buf_T *buf, char_u *prevLine, char_u *newLine) {
  CAMLparam0();
  CAMLlocal2(vPrevLine, vNewLine);
  static const value *lv_onAutoIndent = NULL;

  if (lv_onAutoIndent == NULL) {
    lv_onAutoIndent = caml_named_value("lv_onAutoIndent");
  }

  vPrevLine = caml_copy_string((const char *) prevLine);

  value vIndent = caml_callback2(*lv_onAutoIndent, Val_int(lnum), vPrevLine);

  int ret = Int_val(vIndent);

  CAMLreturnT(int, ret);
};

value Val_input_mode(int mode) {
  CAMLparam0();
  CAMLlocal1(vRet);

  if (mode == INSERT) {
    vRet = Val_int(0);
  } else if(mode == LANGMAP) {
    vRet = Val_int(1);
  } else if(mode == CMDLINE) {
    vRet = Val_int(2);
  } else if (mode == NORMAL) {
    vRet = Val_int(3);
  } else if (mode == VISUAL + SELECTMODE) {
    vRet = Val_int(4);
  } else if (mode == VISUAL) {
    vRet = Val_int(5);
  } else if (mode == SELECTMODE) {
    vRet = Val_int(6);
  } else if (mode == OP_PENDING) {
    vRet = Val_int(7);
  } else if (mode == TERMINAL) {
    vRet = Val_int(8);
  } else if (mode == INSERT + CMDLINE) {
    vRet = Val_int(9);
  } else {
    vRet = Val_int(10);
  }

  CAMLreturn(vRet);
}

void onInputMap(const mapblock_T* mapping) {
  CAMLparam0();
  CAMLlocal4(vRet, vMode, vFromKeys, vToKeys);

  static const value *lv_onInputMap = NULL;
  if (lv_onInputMap == NULL) {
    lv_onInputMap = caml_named_value("lv_onInputMap");
  }

  vRet = caml_alloc(7, 0);
  vMode = Val_input_mode(mapping->m_mode);
  vFromKeys = caml_copy_string((const char*)mapping->m_orig_keys);
  vToKeys = caml_copy_string((const char*)mapping->m_orig_str);

  Store_field(vRet, 0, vMode);
  Store_field(vRet, 1, vFromKeys);
  Store_field(vRet, 2, vToKeys);
  Store_field(vRet, 3, Val_bool(mapping->m_expr));
  Store_field(vRet, 4, Val_bool(mapping->m_noremap == 0));
  Store_field(vRet, 5, Val_bool(mapping->m_silent));
  Store_field(vRet, 6, Val_int(mapping->m_script_ctx.sc_sid));

  caml_callback(*lv_onInputMap, vRet);

  CAMLreturn0;
};

void onInputUnmap(int mode, const char_u* maybeKeys) {
  CAMLparam0();
  CAMLlocal2(vKeyStr, vMaybeKeys);

  static const value *lv_onInputUnmap = NULL;
  if (lv_onInputUnmap == NULL) {
    lv_onInputUnmap = caml_named_value("lv_onInputUnmap");
  }

  if (maybeKeys == NULL) {
    vMaybeKeys = Val_none;
  } else {
    vKeyStr = caml_copy_string((const char*)maybeKeys);
    vMaybeKeys = Val_some(vKeyStr);
  }

  caml_callback2(*lv_onInputUnmap, Val_input_mode(mode), vMaybeKeys);

  CAMLreturn0;
};

int getColorSchemesCallback(char_u *pat, int *num_schemes, char_u ***schemes) {
  CAMLparam0();
  CAMLlocal2(vPat, vSchemes);

  static const value *lv_getColorSchemesCallback = NULL;

  if (lv_getColorSchemesCallback == NULL) {
    lv_getColorSchemesCallback = caml_named_value("lv_getColorSchemesCallback");
  }

  vPat = caml_copy_string((const char*)pat);
  vSchemes = caml_callback(*lv_getColorSchemesCallback, vPat);

  int len = Wosize_val(vSchemes);
  *num_schemes = len;
  char_u **out = malloc(sizeof(char_u*) * len);

  for (int i = 0; i < len; i++) {
    const char *sz = String_val(Field(vSchemes, i));
    out[i] = malloc((sizeof(char) * strlen(sz)) + 1); 
    strcpy((char *)out[i], sz);
  }

  *schemes = out;

  CAMLreturnT(int, OK);
}

int onColorSchemeChanged(char_u *colorscheme) {
  CAMLparam0();
  CAMLlocal2(vThemeStr, vThemeOpt);
  static const value *lv_onColorSchemeChanged = NULL;

  if (lv_onColorSchemeChanged == NULL) {
    lv_onColorSchemeChanged = caml_named_value("lv_onColorSchemeChanged");
  }

  if (colorscheme == NULL) {
    vThemeOpt = Val_none;
  } else {
    vThemeStr = caml_copy_string((const char *)colorscheme);
    vThemeOpt = Val_some(vThemeStr);
  }
  
  caml_callback(*lv_onColorSchemeChanged, vThemeOpt);

  
  CAMLreturnT(int, OK);
};

int onGetChar(int mode, char* c, int *modMask) {
  CAMLparam0();
  CAMLlocal1(vRet);
  *modMask = 0;
  *c = 0;

  static const value *lv_onGetChar = NULL;

  if (lv_onGetChar == NULL) {
    lv_onGetChar = caml_named_value("lv_onGetChar");
  }

  vRet = caml_callback(*lv_onGetChar, Val_int(mode));
  *c = Int_val(Field(vRet, 0));
  *modMask = Int_val(Field(vRet, 1));

  CAMLreturnT(int, OK);
}

void onSettingChanged(optionSet_T *options) {
  CAMLparam0();
  CAMLlocal2(innerValue, settingValue);
  static const value *lv_onSettingChanged = NULL;

  if (lv_onSettingChanged == NULL) {
    lv_onSettingChanged = caml_named_value("lv_onSettingChanged");
  }
  
  if (options->type == 1 || options-> type == 0) {
    // String value
    if (options->type == 0) {
      innerValue = caml_alloc(1, 0);
      Store_field(innerValue, 0, caml_copy_string((const char *)options->stringval));
    } else {
      innerValue = caml_alloc(1, 1);
      Store_field(innerValue, 0, Val_int(options->numval));
    }

    settingValue = caml_alloc(3, 0);
    Store_field(settingValue, 0, caml_copy_string((const char *)options->fullname));
    if (options->shortname == NULL) {
      Store_field(settingValue, 1, Val_none);
    } else {
      Store_field(settingValue, 1, Val_some(caml_copy_string((const char *)options->shortname)));
    }
    Store_field(settingValue, 2, innerValue);

    caml_callback(*lv_onSettingChanged, settingValue);
  };

  CAMLreturn0;
};

int onGoto(gotoRequest_T gotoInfo) {
  static const value *lv_onGoto = NULL;

  if (lv_onGoto == NULL) {
    lv_onGoto = caml_named_value("lv_onGoto");
  }

  int line = gotoInfo.location.lnum;
  int col = gotoInfo.location.col;
  int target = 0;
  switch (gotoInfo.target) {
  case DEFINITION:
    target = 0;
    break;
  case DECLARATION:
    target = 1;
    break;
  case HOVER:
    target = 2;
    break;
  case OUTLINE:
    target = 3;
    break;
  case MESSAGES:
    target = 4;
    break;
  default:
    target = 0;
  }

  caml_callback3(*lv_onGoto, Val_int(line), Val_int(col), Val_int(target));

  return target;
}

void onClear(clearRequest_T clearRequest) {
  static const value *lv_onClear = NULL;

  if (lv_onClear == NULL) {
    lv_onClear = caml_named_value("lv_onClear");
  }

  int count = clearRequest.count;
  int target = 0;
  switch (clearRequest.target) {
  case CLEAR_MESSAGES:
    target = 0;
    break;
  default:
    target = 0;
  }

  caml_callback2(*lv_onClear, Val_int(target), Val_int(count));

  return;
}

int onTabPage(tabPageRequest_T request) {
  CAMLparam0();
  CAMLlocal1(msg);
  static const value *tabPageCallback = NULL;

  if (tabPageCallback == NULL) {
    tabPageCallback = caml_named_value("lv_onTabPage");
  }

  switch (request.kind) {
  case GOTO:
    if (request.relative == 0) {
      msg = caml_alloc(1, 0);
      Store_field(msg, 0, Val_int(request.arg));
    } else {
      msg = caml_alloc(1, 1);
      Store_field(msg, 0, Val_int(request.arg * request.relative));
    }
    break;
    
  case MOVE:
    if (request.relative == 0) {
      msg = caml_alloc(1, 2);
      Store_field(msg, 0, Val_int(request.arg));
    } else {
      msg = caml_alloc(1, 3);
      Store_field(msg, 0, Val_int(request.arg * request.relative));
    }
    break;
  
  case CLOSE:
    if (request.relative == 0) {
      msg = caml_alloc(1, 4);
      Store_field(msg, 0, Val_int(request.arg));
    } else {
      msg = caml_alloc(1, 5);
      Store_field(msg, 0, Val_int(request.arg * request.relative));
    }
    break;

  case ONLY:
    if (request.relative == 0) {
      
      msg = caml_alloc(1, 6);
      Store_field(msg, 0, Val_int(request.arg));
    } else {
      msg = caml_alloc(1, 7);
      Store_field(msg, 0, Val_int(request.arg * request.relative));
    }
    break;
  }

  caml_callback(*tabPageCallback, msg);
  CAMLreturn(1);
}

void onAutocommand(event_T event, buf_T *buf) {
  static const value *lv_onAutocmd = NULL;

  if (lv_onAutocmd == NULL) {
    lv_onAutocmd = caml_named_value("lv_onAutocommand");
  }

  caml_callback2(*lv_onAutocmd, Val_int(event), (value)buf);
}

void onDirectoryChanged(char_u *path) {
  CAMLparam0();
  CAMLlocal1(pathString);
  static const value *lv_onDirectoryChanged = NULL;

  if (lv_onDirectoryChanged == NULL) {
    lv_onDirectoryChanged = caml_named_value("lv_onDirectoryChanged");
  }

  pathString = caml_copy_string((const char *) path);
  caml_callback(*lv_onDirectoryChanged, pathString);
  CAMLreturn0;
}

void onFormat(formatRequest_T *pRequest) {
  CAMLparam0();
  CAMLlocal3(ret, commandString, commandOpt);

  static const value *lv_onFormat = NULL;
  if (lv_onFormat == NULL) {
    lv_onFormat = caml_named_value("lv_onFormat");
  }

  if (pRequest->cmd != NULL) {
    commandString = caml_copy_string((const char *) pRequest->cmd);
    commandOpt = Val_some(commandString);
  } else {
    commandOpt = Val_none;
  }

  int id = vimBufferGetId(pRequest->buf);
  int lineCount = vimBufferGetLineCount(pRequest->buf);
  int formatType = 0;

  if (pRequest->formatType == FORMATTING) {
    formatType = 1;
  }

  ret = caml_alloc(6, 0);
  Store_field(ret, 0, Val_int(pRequest->start.lnum));
  Store_field(ret, 1, Val_int(pRequest->end.lnum));
  Store_field(ret, 2, Val_int(id));
  Store_field(ret, 3, Val_int(pRequest->returnCursor));
  Store_field(ret, 4, Val_int(formatType));
  Store_field(ret, 5, Val_int(lineCount));

  caml_callback(*lv_onFormat, ret);
  CAMLreturn0;
}

void onMacroStartRecord(int regname) {
  CAMLparam0();

  static const value *lv_onMacroStartRecording = NULL;

  if (lv_onMacroStartRecording == NULL) {
    lv_onMacroStartRecording = caml_named_value("lv_onMacroStartRecording");
  }

  caml_callback(*lv_onMacroStartRecording, Val_int(regname));
  CAMLreturn0;
}

void onMacroStopRecord(int regname, char_u *regvalue) {
  CAMLparam0();
  CAMLlocal2(vRegister, vStr);

  static const value *lv_onMacroStopRecording = NULL;

  if (lv_onMacroStopRecording == NULL) {
    lv_onMacroStopRecording = caml_named_value("lv_onMacroStopRecording");
  }

  vRegister = Val_none;

  if (regvalue != NULL) {
    vStr = caml_copy_string((const char *)regvalue);
    vRegister = Val_some(vStr);
  }

  caml_callback2(*lv_onMacroStopRecording, Val_int(regname), vRegister);
  CAMLreturn0;
}

void onMessage(char_u *title, char_u *contents, msgPriority_T priority) {
  CAMLparam0();
  CAMLlocal2(titleString, contentsString);

  static const value *lv_onMessage = NULL;

  if (lv_onMessage == NULL) {
    lv_onMessage = caml_named_value("lv_onMessage");
  }

  titleString = caml_copy_string((const char *) title);
  contentsString = caml_copy_string((const char *) contents);
  caml_callback3(*lv_onMessage, Val_int(priority), titleString, contentsString);
  CAMLreturn0;
}

void onTerminal(terminalRequest_t *pRequest) {
  CAMLparam0();
  CAMLlocal3(ret, commandString, commandOpt);

  static const value *lv_onTerminal = NULL;

  if (lv_onTerminal == NULL) {
    lv_onTerminal = caml_named_value("lv_onTerminal");
  }

  if (pRequest->cmd != NULL) {
    commandString = caml_copy_string((const char *) pRequest->cmd);
    commandOpt = Val_some(commandString);
  } else {
    commandOpt = Val_none;
  }

  ret = caml_alloc(6, 0);
  Store_field(ret, 0, Val_int(pRequest->rows));
  Store_field(ret, 1, Val_int(pRequest->cols));
  Store_field(ret, 2, Val_bool(pRequest->finish == 'c'));
  Store_field(ret, 3, Val_bool(pRequest->curwin));
  Store_field(ret, 4, Val_bool(pRequest->hidden));
  Store_field(ret, 5, commandOpt);

  caml_callback(*lv_onTerminal, ret);
  CAMLreturn0;
}

void onQuit(buf_T *buf, int isForced) {
  CAMLparam0();
  CAMLlocal1(quitResult);

  static const value *lv_onQuit = NULL;

  if (lv_onQuit == NULL) {
    lv_onQuit = caml_named_value("lv_onQuit");
  }

  if (buf == NULL) {
    quitResult = Val_none;
  } else {
    quitResult = Val_some((value)buf);
  }
  caml_callback2(*lv_onQuit, quitResult,
                 isForced == TRUE ? Val_true : Val_false);

  CAMLreturn0;
}

void onUnhandledEscape() {
  CAMLparam0();

  static const value *lv_onUnhandledEscape = NULL;

  if (lv_onUnhandledEscape == NULL) {
    lv_onUnhandledEscape = caml_named_value("lv_onUnhandledEscape");
  }
  caml_callback(*lv_onUnhandledEscape, Val_unit);
  CAMLreturn0;
}

void onStopSearch(void) {
  CAMLparam0();

  static const value *lv_onStopSearch = NULL;

  if (lv_onStopSearch == NULL) {
    lv_onStopSearch = caml_named_value("lv_onStopSearch");
  }

  caml_callback(*lv_onStopSearch, Val_unit);
  CAMLreturn0;
}

void onWindowMovement(windowMovement_T movementType, int count) {
  CAMLparam0();

  static const value *lv_onWindowMovement = NULL;

  if (lv_onWindowMovement == NULL) {
    lv_onWindowMovement = caml_named_value("lv_onWindowMovement");
  }

  caml_callback2(*lv_onWindowMovement, Val_int(movementType), Val_int(count));
  CAMLreturn0;
}

void onIntro() {
  CAMLparam0();

  static const value *lv_onIntro = NULL;

  if (lv_onIntro == NULL) {
    lv_onIntro = caml_named_value("lv_onIntro");
  }

  caml_callback(*lv_onIntro, Val_unit);
  CAMLreturn0;
}

void onVersion() {
  CAMLparam0();

  static const value *lv_onVersion = NULL;

  if (lv_onVersion == NULL) {
    lv_onVersion = caml_named_value("lv_onVersion");
  }

  caml_callback(*lv_onVersion, Val_unit);
  CAMLreturn0;
}

void onWindowSplit(windowSplit_T splitType, char_u *path) {
  CAMLparam0();
  CAMLlocal1(pathString);

  static const value *lv_onWindowSplit = NULL;

  if (lv_onWindowSplit == NULL) {
    lv_onWindowSplit = caml_named_value("lv_onWindowSplit");
  }

  pathString = caml_copy_string((const char *) path);
  caml_callback2(*lv_onWindowSplit, Val_int(splitType), pathString);
  CAMLreturn0;
}

int getClipboardCallback(int regname, int *num_lines, char_u ***lines,
                         int *blockType) {
  CAMLparam0();
  CAMLlocal3(vRecord, clipboardArray, vBlockType);

  static const value *lv_clipboardGet = NULL;

  if (lv_clipboardGet == NULL) {
    lv_clipboardGet = caml_named_value("lv_clipboardGet");
  }

  value v = caml_callback(*lv_clipboardGet, Val_int(regname));

  int ret = 0;
  // Some
  if (Is_block(v)) {
    vRecord = Field(v, 0);
    clipboardArray = Field(vRecord, 0);
    vBlockType = Int_val(Field(vRecord, 1));

    if (vBlockType == 0) {
      *blockType = MLINE;
    } else {
      *blockType = MCHAR;
    }

    int len = Wosize_val(clipboardArray);

    *num_lines = len;
    char_u **out = malloc(sizeof(char_u *) * len);

    for (int i = 0; i < len; i++) {
      const char *sz = String_val(Field(clipboardArray, i));
      out[i] = malloc((sizeof(char) * strlen(sz)) + 1);
      strcpy((char *)out[i], sz);
    }
    *lines = out;

    ret = 1;
    // None
  } else {
    ret = 0;
  }

  CAMLreturn(ret);
}

void onYank(yankInfo_T *yankInfo) {
  CAMLparam0();
  CAMLlocal1(lines);

  static const value *lv_onYank = NULL;
  if (lv_onYank == NULL) {
    lv_onYank = caml_named_value("lv_onYank");
  }

  if (yankInfo->numLines == 0) {
    lines = Atom(0);
  } else {
    lines = caml_alloc(yankInfo->numLines, 0);
    for (int i = 0; i < yankInfo->numLines; i++) {
      Store_field(lines, i, caml_copy_string((const char *) yankInfo->lines[i]));
    }
  }

  value *pArgs = (value *)malloc(sizeof(value) * 8);
  pArgs[0] = lines;
  pArgs[1] = Val_int(yankInfo->blockType);
  pArgs[2] = Val_int(yankInfo->op_char);
  pArgs[3] = Val_int(yankInfo->regname);
  pArgs[4] = Val_int(yankInfo->start.lnum);
  pArgs[5] = Val_int(yankInfo->start.col);
  pArgs[6] = Val_int(yankInfo->end.lnum);
  pArgs[7] = Val_int(yankInfo->end.col);

  caml_callbackN(*lv_onYank, 8, pArgs);
  free(pArgs);

  CAMLreturn0;
}

void onWriteFailure(writeFailureReason_T reason, buf_T *buf) {
  CAMLparam0();

  static const value *lv_onWriteFailure = NULL;
  if (lv_onWriteFailure == NULL) {
    lv_onWriteFailure = caml_named_value("lv_onWriteFailure");
  }

  caml_callback2(*lv_onWriteFailure, reason, (value)buf);

  CAMLreturn0;
}

void onCursorMoveScreenLine(screenLineMotion_T motion, int count, linenr_T startLine, linenr_T *outLine) {
    CAMLparam0();
    CAMLlocal1(valDestLine);

    int iMotion = 0;
    switch (motion) {
    case MOTION_M:
        iMotion = 1;
        break;
    case MOTION_L:
        iMotion = 2;
        break;
    case MOTION_H:
    default:
        iMotion = 0;
        break;
    }

   static const value *lv_onCursorMoveScreenLine = NULL;
   if (lv_onCursorMoveScreenLine == NULL) {
     lv_onCursorMoveScreenLine = caml_named_value("lv_onCursorMoveScreenLine");
   }

   valDestLine = caml_callback3(*lv_onCursorMoveScreenLine, Val_int(iMotion),
   Val_int(count), Val_int(startLine));
   *outLine = Int_val(valDestLine);
   CAMLreturn0;

}

void onOutput(char_u *cmd, char_u* output) {
  CAMLparam0();
  CAMLlocal2(vStr, vMaybeOutput);

  // `cmd` shouldn't be NULL, but if it is, don't bother calling back..
  if (cmd == NULL) {
    CAMLreturn0;
  }

  vStr = caml_copy_string((const char*) cmd);

  if (output == NULL) {
    vMaybeOutput = Val_none;
  } else {
    vMaybeOutput = Val_some(caml_copy_string((const char *)output));
  }

   static const value *lv_onOutput = NULL;
   if (lv_onOutput == NULL) {
     lv_onOutput = caml_named_value("lv_onOutput");
   }
  caml_callback2(*lv_onOutput, vStr, vMaybeOutput);

  CAMLreturn0;
}

int onToggleComments(buf_T *buf, linenr_T start, linenr_T end,
linenr_T *outCount, char_u ***outLines
) {
  CAMLparam0();
  CAMLlocal1(vArray);

  int count = end - start + 1;

  if (count <= 0) {
    CAMLreturnT(int, FAIL);
  } else {
    
    *outCount = count;
   static const value *lv_onToggleComments = NULL;
   if (lv_onToggleComments == NULL) {
     lv_onToggleComments = caml_named_value("lv_onToggleComments");
   }

   vArray = caml_callback3(*lv_onToggleComments, 
  // TODO: Naked pointer
    (value)buf, Val_int(start), Val_int(end));

    int count = Wosize_val(vArray);
    *outCount = count;

    char_u **newLines = malloc(sizeof(char_u *) * count);
    for (int i = 0; i < count; i++) {
      const char *sz = String_val(Field(vArray, i));
      newLines[i] = malloc((sizeof(char) * strlen(sz)) + 1);
      strcpy((char *)newLines[i], sz);
    }
    *outLines = newLines;
    CAMLreturnT(int, OK);
  }
}

void onCursorMoveScreenPosition(int dir, int count, linenr_T srcLine,
colnr_T srcColumn, colnr_T wantColumn, linenr_T *destLine, colnr_T *destColumn) {
    CAMLparam0();
    CAMLlocal2(vDirection, vResult);

    if (dir == BACKWARD) {
        vDirection = hash_variant("Up");
    } else {
        vDirection = hash_variant("Down");
    }

   static const value *lv_onCursorMoveScreenPosition = NULL;
   if (lv_onCursorMoveScreenPosition == NULL) {
     lv_onCursorMoveScreenPosition = caml_named_value("lv_onCursorMoveScreenPosition");
   }
  value *pArgs = (value *)malloc(sizeof(value) * 5);
  pArgs[0] = vDirection,
  pArgs[1] = Val_int(count);
  pArgs[2] = Val_int(srcLine);
  pArgs[3] = Val_long(srcColumn);
  pArgs[4] = Val_long(wantColumn);

    vResult = caml_callbackN(*lv_onCursorMoveScreenPosition,
    5,
    pArgs
    );
    free(pArgs);

  if (Is_block(vResult)) {
    *destLine = Int_val(Field(vResult, 0));
    *destColumn = Int_val(Field(vResult, 1));
  } else {
    *destLine = srcLine;
    *destColumn = srcColumn;
  }

   CAMLreturn0;
}

void onCursorAdd(pos_T newCursorPosition) {
   CAMLparam0();
   static const value *lv_onCursorAdd = NULL;
   if (lv_onCursorAdd == NULL) {
     lv_onCursorAdd = caml_named_value("lv_onCursorAdd");
   }
    caml_callback2(*lv_onCursorAdd,
    Val_int(newCursorPosition.lnum),
    Val_int(newCursorPosition.col)
    );
  CAMLreturn0;
}

void onScrollCallback(scrollDirection_T dir, long count) {
   CAMLparam0();

   int outScroll = 0;
   switch (dir) {
    case SCROLL_CURSOR_CENTERH:
        outScroll = 1;
        break;
    case SCROLL_CURSOR_TOP:
        outScroll = 2;
        break;
    case SCROLL_CURSOR_BOTTOM:
        outScroll = 3;
        break;
    case SCROLL_CURSOR_LEFT:
        outScroll = 4;
        break;
    case SCROLL_CURSOR_RIGHT:
        outScroll = 5;
        break;
    case SCROLL_LINE_UP:
        outScroll = 6;
        break;
    case SCROLL_LINE_DOWN:
        outScroll = 7;
        break;
    case SCROLL_HALFPAGE_DOWN:
        outScroll = 8;
        break;
    case SCROLL_HALFPAGE_UP:
        outScroll = 9;
        break;
    case SCROLL_PAGE_DOWN:
        outScroll = 10;
        break;
    case SCROLL_PAGE_UP:
        outScroll = 11;
        break;
    case SCROLL_HALFPAGE_LEFT:
        outScroll = 12;
        break;
    case SCROLL_HALFPAGE_RIGHT:
        outScroll = 13;
        break;
    case SCROLL_COLUMN_LEFT:
        outScroll = 14;
        break;
    case SCROLL_COLUMN_RIGHT:
        outScroll = 15;
        break;
    case SCROLL_CURSOR_CENTERV:
    default:
        outScroll = 0;
        break;
   }

   static const value *lv_onScroll = NULL;
   if (lv_onScroll == NULL) {
     lv_onScroll = caml_named_value("lv_onScroll");
   }

   caml_callback2(*lv_onScroll, Val_int(outScroll), Val_int(count));
   CAMLreturn0;
}

CAMLprim value libvim_vimInit(value unit) {
  vimMacroSetStartRecordCallback(&onMacroStartRecord);
  vimMacroSetStopRecordCallback(&onMacroStopRecord);
  vimSetAutoCommandCallback(&onAutocommand);
  vimSetAutoIndentCallback(&onAutoIndent);
  vimSetBufferUpdateCallback(&onBufferChanged);
  vimSetClipboardGetCallback(&getClipboardCallback);
  vimColorSchemeSetChangedCallback(&onColorSchemeChanged);
  vimColorSchemeSetCompletionCallback(&getColorSchemesCallback);
  vimSetDirectoryChangedCallback(&onDirectoryChanged);
  vimSetDisplayIntroCallback(&onIntro);
  vimSetDisplayVersionCallback(&onVersion);
  vimSetFormatCallback(&onFormat);
  vimSetClearCallback(&onClear);
  vimSetGotoCallback(&onGoto);
  vimSetOptionSetCallback(&onSettingChanged);
  vimSetTabPageCallback(&onTabPage);
  vimSetMessageCallback(&onMessage);
  vimSetQuitCallback(&onQuit);
  vimSetTerminalCallback(&onTerminal);
  vimSetStopSearchHighlightCallback(&onStopSearch);
  vimSetUnhandledEscapeCallback(&onUnhandledEscape);
  vimSetWindowMovementCallback(&onWindowMovement);
  vimSetWindowSplitCallback(&onWindowSplit);
  vimSetYankCallback(&onYank);
  vimSetFileWriteFailureCallback(&onWriteFailure);
  vimSetCursorMoveScreenLineCallback(&onCursorMoveScreenLine);
  vimSetCursorMoveScreenPositionCallback(&onCursorMoveScreenPosition);
  vimSetScrollCallback(&onScrollCallback);
  vimSetInputMapCallback(&onInputMap);
  vimSetInputUnmapCallback(&onInputUnmap);
  vimSetToggleCommentsCallback(&onToggleComments);
  vimSetCursorAddCallback(&onCursorAdd);
  vimSetFunctionGetCharCallback(&onGetChar);
  vimSetOutputCallback(&onOutput);
  char *args[0];
  vimInit(0, args);
  return Val_unit;
}

CAMLprim value libvim_vimInput(value v) {
  CAMLparam1(v);
  char_u *s;
  s = (char_u *)String_val(v);
  vimInput(s);
  CAMLreturn(Val_unit);
}

CAMLprim value libvim_vimKey(value v) {
  CAMLparam1(v);
  char_u *s;
  s = (char_u *)String_val(v);
  vimKey(s);
  CAMLreturn(Val_unit);
}

CAMLprim value libvim_vimEval(value vStr) {
  CAMLparam1(vStr);
  CAMLlocal2(vOut, vRet);

  char_u *result = vimEval((char_u *) String_val(vStr));

  if (result == NULL) {
    vRet = Val_none;
  } else {
    vOut = caml_copy_string((const char *) result);
    vRet = Val_some(vOut);
    free(result);
  }
  CAMLreturn(vRet);
}

CAMLprim value libvim_vimCommand(value v) {
  char_u *s;
  s = (char_u *)String_val(v);
  vimExecute(s);
  return Val_unit;
}

CAMLprim value libvim_vimGetMode(value unit) {
  int mode = vimGetMode();

  int val = 0;

  if ((mode & INSERT) == INSERT) {
    if ((mode & REPLACE_FLAG) == REPLACE_FLAG) {
      val = 3;
    } else {
      val = 1;
    }
  } else if ((mode & CMDLINE) == CMDLINE) {
    val = 2;
  } else if ((mode & VISUAL) == VISUAL) {
    val = 4;
  } else if ((mode & SELECTMODE) == SELECTMODE) {
    val = 6;
  } else if ((mode & OP_PENDING) == OP_PENDING) {
    val = 5;
  }

  return Val_int(val);
}

CAMLprim value libvim_vimGetSubMode(value unit) {
  CAMLparam0();
  subMode_T submode = vimGetSubMode();

  int val = 0;
  switch (submode) {
    case SM_NONE:
      val = 0;
      break;
    case SM_INSERT_LITERAL:
      val = 1;
      break;
    default:
      val = 0;
      break;
  }

  CAMLreturn(Val_int(val));
}

CAMLprim value libvim_vimBufferGetId(value v) {
  buf_T *buf = (buf_T *)v;
  int id = vimBufferGetId(buf);
  return Val_int(id);
}

CAMLprim value libvim_vimBufferGetReadOnly(value vBuf) {
  buf_T *buf = (buf_T *)vBuf;
  int readonly = vimBufferGetReadOnly(buf);
  return Val_bool(readonly);
}

CAMLprim value libvim_vimBufferSetReadOnly(value vReadOnly, value vBuf) {
  buf_T *buf = (buf_T *)vBuf;
  int readOnly = Bool_val(vReadOnly);
  vimBufferSetReadOnly(buf, readOnly);
  return Val_unit;
}

CAMLprim value libvim_vimBufferGetModifiable(value vBuf) {
  buf_T *buf = (buf_T *)vBuf;
  int modifiable = vimBufferGetModifiable(buf);
  return Val_bool(modifiable);
}

CAMLprim value libvim_vimGetPendingOperator(value unit) {
  CAMLparam0();
  CAMLlocal2(inner, ret);

  pendingOp_T pendingOp;
  if (vimGetPendingOperator(&pendingOp)) {
    inner = caml_alloc(3, 0);
    Store_field(inner, 0, Val_int(pendingOp.op_type));
    Store_field(inner, 1, Val_int(pendingOp.regname));
    Store_field(inner, 2, Val_int(pendingOp.count));
    ret = Val_some(inner);
  } else {
    ret = Val_none;
  }

  CAMLreturn(ret);
}

CAMLprim value libvim_vimBufferSetModifiable(value vMod, value vBuf) {
  buf_T *buf = (buf_T *)vBuf;
  int modifiable = Bool_val(vMod);
  vimBufferSetModifiable(buf, modifiable);
  return Val_unit;
}

CAMLprim value libvim_vimBufferOpen(value v) {
  CAMLparam1(v);
  char_u *s;
  s = (char_u *)String_val(v);
  buf_T *buf = vimBufferOpen(s, 1, 0);
  value vbuf = (value)buf;
  CAMLreturn(vbuf);
}

CAMLprim value libvim_vimBufferLoad(value v) {
  CAMLparam1(v);
  char_u *s;
  s = (char_u *)String_val(v);
  buf_T *buf = vimBufferLoad(s, 1, 0);
  value vbuf = (value)buf;
  CAMLreturn(vbuf);
}

CAMLprim value libvim_vimBufferNew(value vUnit) {
  CAMLparam1(vUnit);

  buf_T *buf = vimBufferNew(BLN_NEW);
  value vbuf = (value)buf;
  CAMLreturn(vbuf);
}

CAMLprim value libvim_vimBufferGetById(value v) {
  CAMLparam1(v);
  CAMLlocal1(ret);
  buf_T *buf = vimBufferGetById(Int_val(v));

  if (!buf) {
    ret = Val_none;
  } else {
    value b = (value)buf;
    ret = Val_some(b);
  }

  CAMLreturn(ret);
}

CAMLprim value libvim_vimBufferGetFilename(value v) {
  CAMLparam1(v);
  CAMLlocal1(ret);
  buf_T *buf = (buf_T *)v;

  if (buf == NULL) {
    ret = Val_none;
  } else {
    char_u *fname = vimBufferGetFilename(buf);
    if (fname == NULL) {
      ret = Val_none;
    } else {
      ret = Val_some(caml_copy_string((const char *) fname));
    }
  }

  CAMLreturn(ret);
}

CAMLprim value libvim_vimBufferGetFileFormat(value v) {
  CAMLparam1(v);
  CAMLlocal1(ret);
  buf_T *buf = (buf_T *)v;

  if (buf == NULL) {
    ret = Val_none;
  } else {
    int format = vimBufferGetFileFormat(buf);
    switch (format) {
    case EOL_UNIX:
      ret = Val_some(Val_int(1));
      break;
    case EOL_DOS:
      ret = Val_some(Val_int(2));
      break;
    case EOL_MAC:
      ret = Val_some(Val_int(0));
      break;
    default:
      ret = Val_none;
      break;
    }
  }

  CAMLreturn(ret);
}

CAMLprim value libvim_vimBufferSetFileFormat(value v, value vFormat) {
  CAMLparam2(v, vFormat);
  buf_T *buf = (buf_T *)v;

  if (buf != NULL) {
    int format = Int_val(vFormat);

    switch (format) {
    case 0:
      vimBufferSetFileFormat(buf, EOL_MAC);
      break;
    case 1:
      vimBufferSetFileFormat(buf, EOL_UNIX);
      break;
    case 2:
      vimBufferSetFileFormat(buf, EOL_DOS);
      break;
    default:
      // no-op
      break;
    }
  }

  CAMLreturn(Val_unit);
}

CAMLprim value libvim_vimBufferGetModified(value v) {
  buf_T *buf = (buf_T *)v;

  if (vimBufferGetModified(buf) == TRUE) {
    return Val_true;
  } else {
    return Val_false;
  }
}

CAMLprim value libvim_vimBufferGetChangedTick(value v) {
  buf_T *buf = (buf_T *)v;

  long tick = vimBufferGetLastChangedTick(buf);
  return Val_long(tick);
}

CAMLprim value libvim_vimBufferGetFiletype(value v) {
  CAMLparam1(v);
  CAMLlocal1(ret);
  buf_T *buf = (buf_T *)v;

  if (buf == NULL) {
    ret = Val_none;
  } else {
    char_u *ftype = vimBufferGetFiletype(buf);
    if (ftype == NULL) {
      ret = Val_none;
    } else {
      ret = Val_some(caml_copy_string((const char *) ftype));
    }
  }

  CAMLreturn(ret);
}

CAMLprim value libvim_vimSearchGetHighlights(value vBuf, value startLine, value endLine) {
  CAMLparam3(vBuf, startLine, endLine);
  CAMLlocal1(ret);

  buf_T *buf = (buf_T *)vBuf;

  int start = Int_val(startLine);
  int end = Int_val(endLine);

  int num_highlights;
  searchHighlight_T *highlights;

  vimSearchGetHighlights(buf, start, end, &num_highlights, &highlights);

  ret = caml_alloc(num_highlights, 0);

  for (int i = 0; i < num_highlights; i++) {
    Store_field(ret, i, Val_highlight(highlights[i]));
  }

  vim_free(highlights);
  CAMLreturn(ret);
}

CAMLprim value libvim_vimSearchGetPattern(value unit) {
  CAMLparam0();
  CAMLlocal2(ret, v);

  char_u *szSearchPattern = vimSearchGetPattern();

  if (szSearchPattern == NULL) {
    ret = Val_none;
  } else {
    v = caml_copy_string((char *)szSearchPattern);
    ret = Val_some(v);
  }

  CAMLreturn(ret);
}

CAMLprim value libvim_vimSearchGetMatchingPair(value unit) {
  CAMLparam0();
  CAMLlocal2(ret, v);

  v = caml_alloc(2, 0);

  pos_T *result = vimSearchGetMatchingPair(0);
  if (result == NULL) {
    ret = Val_none;
  } else {
    Store_field(v, 0, Val_int(result->lnum));
    Store_field(v, 1, Val_int(result->col));
    ret = Val_some(v);
  }

  CAMLreturn(ret);
}

CAMLprim value libvim_vimBufferGetCurrent(value unit) {
  return (value)vimBufferGetCurrent();
}

CAMLprim value libvim_vimBufferGetLineCount(value v) {
  buf_T *buf = (buf_T *)v;
  size_t count = vimBufferGetLineCount(buf);
  return Val_long(count);
}

CAMLprim value libvim_vimBufferGetLine(value vBuf, value vLine) {
  CAMLparam2(vBuf, vLine);
  CAMLlocal1(ret);
  buf_T *buf = (buf_T *)vBuf;
  int line = Int_val(vLine);

  char_u *c = vimBufferGetLine(buf, line);
  ret = caml_copy_string((const char *) c);

  CAMLreturn(ret);
}

CAMLprim value libvim_vimBufferSetLines(value vBuf, value vStart, value vEnd,
                                        value vLines) {
  CAMLparam4(vBuf, vStart, vEnd, vLines);

  buf_T *buf = (buf_T *)vBuf;
  int start = Int_val(vStart);
  int end = Int_val(vEnd);

  int len = Wosize_val(vLines);

  char_u **newLines = malloc(sizeof(char_u *) * len);
  for (int i = 0; i < len; i++) {
    const char *sz = String_val(Field(vLines, i));
    newLines[i] = (char_u *) sz;
  }
  vimBufferSetLines(buf, start, end, newLines, len);
  free(newLines);

  CAMLreturn(Val_unit);
};

CAMLprim value libvim_vimBufferSetCurrent(value v) {
  buf_T *buf = (buf_T *)v;

  vimBufferSetCurrent(buf);
  return Val_unit;
}

CAMLprim value libvim_vimCommandLineGetCompletions(value unit) {
  CAMLparam0();
  CAMLlocal1(ret);

  char_u **completions = NULL;
  int count = 0;

  vimCommandLineGetCompletions(&completions, &count);

  if (count == 0) {
    ret = Atom(0);
  } else {
    ret = caml_alloc(count, 0);
    for (int i = 0; i < count; i++) {
      Store_field(ret, i, caml_copy_string((const char *) completions[i]));
      vim_free(completions[i]);
    }

    vim_free(completions);
  }

  CAMLreturn(ret);
}

CAMLprim value libvim_vimCommandLineGetPosition(value unit) {
  CAMLparam0();

  int pos = vimCommandLineGetPosition();
  CAMLreturn(Val_int(pos));
}

CAMLprim value libvim_vimCommandLineGetText(value unit) {
  CAMLparam0();
  CAMLlocal1(ret);

  char_u *c = vimCommandLineGetText();
  if (c == NULL) {
    ret = Val_none;
  } else {
    ret = Val_some(caml_copy_string((const char *) c));
  }

  CAMLreturn(ret);
}

CAMLprim value libvim_vimCommandLineGetType(value unit) {
  CAMLparam0();
  int type = vimCommandLineGetType();

  int ret;
  switch (type) {
  case ':':
    ret = 0;
    break;
  case '/':
    ret = 1;
    break;
  case '?':
    ret = 2;
    break;
  default:
    ret = 3;
  }
  CAMLreturn(Val_int(ret));
}

CAMLprim value libvim_vimCursorGetLine(value unit) {
  int line = vimCursorGetLine();
  return Val_int(line);
}

CAMLprim value libvim_vimCursorGetColumn(value unit) {
  int column = vimCursorGetColumn();
  return Val_int(column);
}

CAMLprim value libvim_vimCursorSetPosition(value l, value c) {
  int line = Int_val(l);
  int column = Int_val(c);

  pos_T pos;
  pos.lnum = line;
  pos.col = column;

  vimCursorSetPosition(pos);

  return Val_unit;
}

CAMLprim value libvim_vimOptionSetTabSize(value ts) {
  int tabSize = Int_val(ts);
  vimOptionSetTabSize(tabSize);
  return Val_unit;
}

CAMLprim value libvim_vimOptionSetInsertSpaces(value v) {
  int insertSpaces = Bool_val(v);
  vimOptionSetInsertSpaces(insertSpaces);
  return Val_unit;
}

CAMLprim value libvim_vimOptionGetInsertSpaces(value unit) {
  int insertSpaces = vimOptionGetInsertSpaces();
  return Val_bool(insertSpaces);
}

CAMLprim value libvim_vimOptionGetTabSize(value unit) {
  int tabSize = vimOptionGetTabSize();
  return Val_int(tabSize);
}

CAMLprim value libvim_vimVisualSetStart(value vLine, value vByte) {
    CAMLparam2(vLine, vByte);

    pos_T start;
    start.lnum = Int_val(vLine);
    start.col = Int_val(vByte);
    vimVisualSetStart(start);

    CAMLreturn(Val_unit);
}

CAMLprim value libvim_vimVisualGetRange(value unit) {
  CAMLparam0();
  CAMLlocal1(ret);

  pos_T start;
  pos_T end;

  vimVisualGetRange(&start, &end);

  ret = caml_alloc(4, 0);
  Store_field(ret, 0, Val_int(start.lnum));
  Store_field(ret, 1, Val_int(start.col));
  Store_field(ret, 2, Val_int(end.lnum));
  Store_field(ret, 3, Val_int(end.col));

  CAMLreturn(ret);
}

CAMLprim value libvim_vimRegisterGet(value vChar) {
  CAMLparam1(vChar);
  CAMLlocal2(ret, vArray);

  
  int reg = Int_val(vChar);
  int numLines = 0;
  char_u **lines = NULL;
  vimRegisterGet(reg, &numLines, &lines);

  if (numLines == 0 || lines == NULL) {
    ret = Val_none;
  } else {
    vArray = caml_alloc(numLines, 0);

    for (int i = 0; i < numLines; i++) {
      Store_field(vArray, i, caml_copy_string((const char *) lines[i]));
    }

    ret = Val_some(vArray);
  }

  CAMLreturn(ret);
}

CAMLprim value libvim_vimWindowGetWidth(value unit) {
  int width = vimWindowGetWidth();
  return Val_int(width);
}

CAMLprim value libvim_vimWindowGetHeight(value unit) {
  int height = vimWindowGetHeight();
  return Val_int(height);
}

CAMLprim value libvim_vimWindowGetTopLine(value unit) {
  int topline = vimWindowGetTopLine();
  return Val_int(topline);
}

CAMLprim value libvim_vimWindowGetLeftColumn(value unit) {
  int left = vimWindowGetLeftColumn();
  return Val_int(left);
}

CAMLprim value libvim_vimWindowSetWidth(value width) {
  int w = Int_val(width);
  vimWindowSetWidth(w);
  return Val_unit;
}

CAMLprim value libvim_vimWindowSetHeight(value height) {
  int h = Int_val(height);
  vimWindowSetHeight(h);
  return Val_unit;
}

CAMLprim value libvim_vimWindowSetTopLeft(value top, value left) {
  int t = Int_val(top);
  int l = Int_val(left);
  vimWindowSetTopLeft(t, l);
  return Val_unit;
}

CAMLprim value libvim_vimUndoSync(value force) {
  CAMLparam0();

  vimUndoSync(Int_val(force));

  CAMLreturn(Val_unit);
}

CAMLprim value libvim_vimUndoSaveRegion(value startLine, value endLine) {
  CAMLparam2(startLine, endLine);
  CAMLlocal1(ret);
  
  int start = Int_val(startLine);
  int end = Int_val(endLine);

  int success = vimUndoSaveRegion(start, end);

  CAMLreturn(Val_bool(success != FAIL));
}

CAMLprim value libvim_vimVisualSetType(value vType) {
    CAMLparam1(vType);    

    char visualType = 0;
    switch(Int_val(vType)) {
    // character
    case 0:
      visualType ='v';
      break;
        // line
    case 1:
      visualType = 'V';
      break;
    case 2:
       visualType = Ctrl_V;
       break;
    }

    if (visualType != 0) {
        vimVisualSetType(visualType);
    }

    CAMLreturn(Val_unit);
}

CAMLprim value libvim_vimVisualGetType(value unit) {
  int ret;
  char v = vimVisualGetType();

  if (vimVisualIsActive() == FALSE) {
    ret = 3;
  } else {
    switch (v) {
    case 'v':
      ret = 0;
      break;
    case 'V':
      ret = 1;
      break;
    case Ctrl_V:
      ret = 2;
      break;
    default:
      ret = 3;
      break;
    };
  }

  return Val_int(ret);
}
