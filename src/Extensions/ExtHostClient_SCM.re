open Oni_Core;

// EFFECTS

module Effects = {
  let provideOriginalResource = (~handles, extHostClient, path, toMsg) =>
    Isolinear.Effect.createWithDispatch(~name="scm.getOriginalUri", dispatch => {
      // Try our luck with every provider. If several returns Last-Writer-Wins
      // TODO: Is there a better heuristic? Perhaps use rootUri to choose the "nearest" provider?
      handles
      |> List.iter(handle => {
           let promise =
             Exthost.Request.SCM.provideOriginalResource(
               ~handle,
               ~uri=Uri.fromPath(path),
               extHostClient,
             );

           Lwt.on_success(
             promise,
             Option.iter(uri => dispatch(toMsg(uri)))
           );
         });
    });

  let onInputBoxValueChange = (~handle, ~value, extHostClient) =>
    Isolinear.Effect.create(~name="scm.onInputBoxValueChange", () =>
      Exthost.Request.SCM.onInputBoxValueChange(
        ~handle,
        ~value,
        extHostClient,
      )
    );
};
