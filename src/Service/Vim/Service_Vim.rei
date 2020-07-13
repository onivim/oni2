let forceReload: unit => Isolinear.Effect.t(_);
let forceOverwrite: unit => Isolinear.Effect.t(_);
let reload: unit => Isolinear.Effect.t(_);
let saveAllAndQuit: unit => Isolinear.Effect.t(_);
let quitAll: unit => Isolinear.Effect.t(_);

module Effects: {
  let paste:
    (~toMsg: list(Vim.Cursor.t) => 'msg, string) => Isolinear.Effect.t('msg);

  let getRegisterValue:
    (~toMsg: option(array(string)) => 'msg, char) => Isolinear.Effect.t('msg);

  let applyEdits:
    (
      ~bufferId: int,
      ~version: int,
      ~edits: list(Vim.Edit.t),
      result(unit, string) => 'msg
    ) =>
    Isolinear.Effect.t('msg);
};
