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
