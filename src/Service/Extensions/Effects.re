let uninstall = (~extensionsFolder, ~toMsg, extensionId) =>
  Isolinear.Effect.createWithDispatch(
    ~name="Service_Extensions.Effect.uninstall", dispatch => {
    let promise = Management.uninstall(~extensionsFolder?, extensionId);

    Lwt.on_success(promise, () => dispatch(Ok()));
    Lwt.on_failure(promise, exn => {
      dispatch(Error(Printexc.to_string(exn)))
    });
  })
  |> Isolinear.Effect.map(toMsg);

let install = (~extensionsFolder, ~toMsg, extensionId) =>
  Isolinear.Effect.createWithDispatch(
    ~name="Service_Extensions.Effect.install", dispatch => {
    let promise = Management.Internal.installFromOpenVSX(
      ~setup=Oni_Core.Setup.init(),
    ~extensionsFolder, extensionId);

    Lwt.on_success(promise, (scanResult) => dispatch(Ok(scanResult)));
    Lwt.on_failure(promise, exn => {
      dispatch(Error(Printexc.to_string(exn)))
    });
  })
  |> Isolinear.Effect.map(toMsg);
