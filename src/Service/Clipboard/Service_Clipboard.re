module Effects = {
  let getClipboardText = (~toMsg) => {
    Isolinear.Effect.createWithDispatch(
      ~name="Service_Clipboard.getClipboardText", dispatch => {
      let clipboardText = Sdl2.Clipboard.getText();
      dispatch(toMsg(clipboardText));
    });
  };
};
