module GlobalState = {
  type provider = {get: unit => option(string)};

  let providerInstance = ref({get: Sdl2.Clipboard.getText});
};

module Effects = {
  let getClipboardText = (~toMsg) => {
    Isolinear.Effect.createWithDispatch(
      ~name="Service_Clipboard.getClipboardText", dispatch => {
      let clipboardText = GlobalState.providerInstance^.get();
      dispatch(toMsg(clipboardText));
    });
  };
};

module Testing = {
  let setClipboardProvider = (~get) => {
    GlobalState.providerInstance := {get: get};
  };
};
