module Effect = {
  let openURL = url =>
    Isolinear.Effect.create(~name="os.openurl", () =>
      Revery.Native.Shell.openURL(url) |> (ignore: bool => unit)
    );
};
