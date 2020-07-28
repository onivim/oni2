open Oni_Core.Utility;

open Revery;
open Revery.UI;

module Log = (val Log.withNamespace("Oni2.Components.RemoteAsset"));

type status =
  | Downloading
  | Downloaded({filePath: string})
  | DownloadFailed({errorMsg: string});

module LocalState = {
  let initial = Downloading;

  type action =
    | Reset
    | DownloadSuccess(string)
    | DownloadFailed(string);

  let reducer = (action, _model) =>
    switch (action) {
    | Reset => Downloading
    | DownloadSuccess(asset) => Downloaded({filePath: asset})
    | DownloadFailed(msg) => DownloadFailed({errorMsg: msg})
    };
};

module Internal = {
  let allowedDomains = [
    "https://open-vsx.org/",
    "https://www.open-vsx.org/",
    "https://github.com/",
    "https://raw.githubusercontent.com/",
  ];

  let isUrlAllowed = (url: string) => {
    allowedDomains
    |> List.exists(prefix => {StringEx.startsWith(~prefix, url)});
  };

  let isLocal = (url: string) => {
    !StringEx.startsWith(~prefix="https://", url)
    && !StringEx.startsWith(~prefix="http://", url);
  };
};

let%component make =
              (
                ~url: string,
                ~children: status => React.element(React.node),
                (),
              ) => {
  open LocalState;
  let renderItem = children;

  let%hook (state, localDispatch) =
    Hooks.reducer(~initialState=LocalState.initial, LocalState.reducer);

  let%hook () =
    Hooks.effect(
      OnMountAndIf((!=), url),
      () => {
        if (Internal.isLocal(url)) {
          // If this is a local file path, just treat it as
          // a successful download.
          localDispatch(
            DownloadSuccess(url),
          );
        } else {
          localDispatch(Reset);
          Log.infof(m => m("Mounted or src changed: %s", url));

          let promise =
            url
            |> Internal.isUrlAllowed
            |> (
              fun
              | true =>
                Service_Net.Request.download(
                  ~setup=Oni_Core.Setup.init(),
                  url,
                )
              | false => Lwt.fail_with(url ++ " is not an allowed domain")
            );

          Lwt.on_success(
            promise,
            res => {
              Log.infof(m => m("Download succeeded: %s to %s", url, res));
              localDispatch(DownloadSuccess(res));
            },
          );

          Lwt.on_failure(
            promise,
            exn => {
              let errMsg = exn |> Printexc.to_string;
              Log.errorf(m => m("Download failed %s with %s", url, errMsg));
              localDispatch(DownloadFailed(errMsg));
            },
          );
        };
        None;
      },
    );

  renderItem(state);
};
