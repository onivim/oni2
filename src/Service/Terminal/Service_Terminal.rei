open Oni_Core;
open Oni_Extensions;


  type msg =
    // TODO:
    | Resized(unit)
    | Updated(unit)
    | ProcessStarted({
        id: int,
        pid: int,
      })
    | ProcessTitleSet({
        id: int,
        title: string,
      });

module Sub: {
  let terminal:
    (
      ~id: int,
      ~cmd: string,
      ~columns: int,
      ~rows: int,
      ~workspaceUri: Uri.t,
      ~extHostClient: ExtHostClient.t
    ) =>
    Isolinear.Sub.t(msg);
};

module Effect: {
  let input:
    (~id: int, ~input: string, ExtHostClient.t) => Isolinear.Effect.t(msg);
};

let handleExtensionMessage: ExtHostClient.Terminal.msg => unit;
