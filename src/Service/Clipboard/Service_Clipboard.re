module GlobalState = {
  type provider = {
    get: unit => option(string),
    set: string => unit,
  };

  let providerInstance =
    ref({get: Sdl2.Clipboard.getText, set: Sdl2.Clipboard.setText});
};

module Effects = {
  let getClipboardText = (~toMsg) => {
    Isolinear.Effect.createWithDispatch(
      ~name="Service_Clipboard.getClipboardText", dispatch => {
      let clipboardText = GlobalState.providerInstance^.get();
      dispatch(toMsg(clipboardText));
    });
  };

  let setClipboardText = text =>
    Isolinear.Effect.create(~name="Service_Clipboard.setClipboardText", () => {
      GlobalState.providerInstance^.set(text)
    });
};

module Testing = {
  let setClipboardProvider = (~get, ~set) => {
    GlobalState.providerInstance := {get, set};
  };
};
