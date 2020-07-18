open Oni_Core;
open Oni_Core.Utility;

open Revery;
open Revery.UI;

module Log = (val Log.withNamespace("Oni2.Components.RemoteAsset"));

type status =
  | Downloading
  | Downloaded({filePath: string})
  | DownloadFailed({errorMsg: string});

module LocalState = {
  type state = status;

  let initial = Downloading;

  type action =
    | Reset
    | DownloadSuccess(string)
    | DownloadFailed(string);

  let reducer = (action, model) =>
    switch (action) {
    | Reset => Downloading
    | DownloadSuccess(asset) => Downloaded({filePath: asset})
    | DownloadFailed(msg) => DownloadFailed({errorMsg: msg})
    };
};

module Internal {
   let allowedDomains = [
      "https://open-vsx.org/",
      "https://www.open-vsx.org/",
      "https://github.com/",
      "https://raw.githubusercontent.com/",
   ];

   let isUrlAllowed = (url: string) => {
      allowedDomains
      |> List.exists(prefix => {
         StringEx.startsWith(~prefix, url)
      });
   };
}

let%component make =
              (
                ~src: string,
                ~children: status => React.element(React.node),
                (),
              ) => {
  open LocalState;
   let renderItem = children;

  let%hook (state, localDispatch) =
    Hooks.reducer(~initialState=LocalState.initial, LocalState.reducer);

  let%hook () =
    Hooks.effect(
      OnMountAndIf((!=), src),
      () => {
        localDispatch(Reset);
        Log.infof(m => m("Mounted or src changed: %s", src));
        
        let promise =
          src
          |> Internal.isUrlAllowed
          |> (fun
          | true => 
          Service_Net.Request.download(~setup=Oni_Core.Setup.init(), src)
          | false => Lwt.fail_with(src ++ " is not an allowed domain")
          );

        Lwt.on_success(
          promise,
          res => {
            Log.infof(m => m("Download succeeded: %s to %s", src, res));
            localDispatch(DownloadSuccess(res));
          },
        );

        Lwt.on_failure(
          promise,
          exn => {
            let errMsg = exn |> Printexc.to_string;
            Log.errorf(m => m("Download failed %s with %s", src, errMsg));
            localDispatch(DownloadFailed(errMsg));
          },
        );

        None;
      },
    );

  renderItem(state);
};
