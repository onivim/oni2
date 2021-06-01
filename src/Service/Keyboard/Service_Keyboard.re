module Effects = {
  let showEmojiAndSymbols: Isolinear.Effect.t(unit) =
    Isolinear.Effect.create(~name="keyboard.showEmojiAndSymbols", () => {
      Revery.Native.Input.openEmojiPanel()
    });
};
