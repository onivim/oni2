open EditorCoreTypes;

let forceReload: unit => Isolinear.Effect.t(_);
let forceOverwrite: unit => Isolinear.Effect.t(_);
let reload: unit => Isolinear.Effect.t(_);
let saveAllAndQuit: unit => Isolinear.Effect.t(_);
let quitAll: unit => Isolinear.Effect.t(_);

module Effects: {
  let paste: (~toMsg: Vim.Mode.t => 'msg, string) => Isolinear.Effect.t('msg);

  let getRegisterValue:
    (~toMsg: option(array(string)) => 'msg, char) =>
    Isolinear.Effect.t('msg);

  let applyEdits:
    (
      ~bufferId: int,
      ~version: int,
      ~edits: list(Vim.Edit.t),
      result(unit, string) => 'msg
    ) =>
    Isolinear.Effect.t('msg);

  let setLines:
    (
      ~bufferId: int,
      ~start: LineNumber.t=?,
      ~stop: LineNumber.t=?,
      ~lines: array(string),
      result(unit, string) => 'msg
    ) =>
    Isolinear.Effect.t('msg);

  let loadBuffer:
    (~filePath: string, (~bufferId: int) => 'msg) => Isolinear.Effect.t('msg);

  let applyCompletion:
    (
      ~meetColumn: CharacterIndex.t,
      ~insertText: string,
      ~toMsg: Vim.Mode.t => 'msg,
      ~additionalEdits: list(Vim.Edit.t)
    ) =>
    Isolinear.Effect.t('msg);
};

module Sub: {
  let eval:
    (~toMsg: result(string, string) => 'msg, string) => Isolinear.Sub.t('msg);

  let searchHighlights:
    (
      ~bufferId: int,
      ~version: int,
      ~searchPattern: string,
      ~topVisibleLine: LineNumber.t,
      ~bottomVisibleLine: LineNumber.t,
      array(ByteRange.t) => 'msg
    ) =>
    Isolinear.Sub.t('msg);
};
