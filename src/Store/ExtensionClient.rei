open Oni_Core;
open Oni_Model;
open Exthost;
open Exthost.Extension;

let create:
  (
    ~initialWorkspace: option(WorkspaceData.t),
    ~attachStdio: bool,
    ~config: Feature_Configuration.model,
    ~extensions: list(Scanner.ScanResult.t),
    ~setup: Setup.t,
    ~proxy: Service_Net.Proxy.t
  ) =>
  (result(Exthost.Client.t, string), Isolinear.Stream.t(Actions.t));
