open Oni_Core.Utility;

module Internal = {
  let exceptionToString =
    fun
    | LuvEx.LuvException(exn) => Luv.Error.strerror(exn)
    | exn => Printexc.to_string(exn);
};

let uninstall = (~extensionsFolder, ~toMsg, extensionId) =>
  Isolinear.Effect.createWithDispatch(
    ~name="Service_Extensions.Effect.uninstall", dispatch => {
    let promise = Management.uninstall(~extensionsFolder?, extensionId);

    Lwt.on_success(promise, () => dispatch(Ok()));
    Lwt.on_failure(promise, exn => {
      dispatch(Error(Internal.exceptionToString(exn)))
    });
  })
  |> Isolinear.Effect.map(toMsg);

let install = (~proxy, ~extensionsFolder, ~toMsg, extensionId) =>
  Isolinear.Effect.createWithDispatch(
    ~name="Service_Extensions.Effect.install", dispatch => {
    let promise =
      Management.Internal.installFromOpenVSX(
        ~proxy,
        ~setup=Oni_Core.Setup.init(),
        ~extensionsFolder,
        extensionId,
      );

    Lwt.on_success(promise, scanResult => dispatch(Ok(scanResult)));
    Lwt.on_failure(promise, exn => {
      dispatch(Error(Internal.exceptionToString(exn)))
    });
  })
  |> Isolinear.Effect.map(toMsg);

let update = (~proxy, ~extensionsFolder, ~toMsg, extensionId) =>
  Isolinear.Effect.createWithDispatch(
    ~name="Service_Extensions.Effect.update", dispatch => {
    let promise =
      Management.update(
        ~proxy,
        ~setup=Oni_Core.Setup.init(),
        ~extensionsFolder?,
        extensionId,
      );

    Lwt.on_success(promise, scanResult => dispatch(Ok(scanResult)));
    Lwt.on_failure(promise, exn => {
      dispatch(Error(Internal.exceptionToString(exn)))
    });
  })
  |> Isolinear.Effect.map(toMsg);

let details = (~proxy, ~extensionId, ~toMsg) =>
  Isolinear.Effect.createWithDispatch(
    ~name="Service_Extensions.Effect.details", dispatch => {
    let maybeIdentifier = Catalog.Identifier.fromString(extensionId);
    switch (maybeIdentifier) {
    | None => dispatch(toMsg(Error("Invalid identifier: " ++ extensionId)))
    | Some(id) =>
      let promise = Catalog.details(~proxy, ~setup=Oni_Core.Setup.init(), id);

      Lwt.on_success(promise, details => {dispatch(toMsg(Ok(details)))});

      Lwt.on_failure(promise, exn => {
        dispatch(toMsg(Error(Printexc.to_string(exn))))
      });
    };
  });
