open Oni_Core.Utility;

open Revery;
open Revery.UI;

module Log = (val Log.withNamespace("Oni2.Components.RemoteAsset"));

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

module Make =
       (
         Config: {
           type asset;
           let mapper: (~filePath: string) => Lwt.t(asset);
         },
       ) => {
  type asset = Config.asset;

  type status =
    | Downloading
    | Downloaded(asset)
    | DownloadFailed({errorMsg: string});

  module LocalState = {
    let initial = Downloading;

    type action =
      | Reset
      | DownloadSuccess(asset)
      | DownloadFailed(string);

    let reducer = (action, _model) =>
      switch (action) {
      | Reset => Downloading
      | DownloadSuccess(asset) => Downloaded(asset)
      | DownloadFailed(msg) => DownloadFailed({errorMsg: msg})
      };
  };

  let%component make =
                (
                  ~proxy,
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
          let downloadPromise =
            if (Internal.isLocal(url)) {
              // If this is a local file path, just treat it as
              // a successful download.
              Lwt.return(
                url,
              );
            } else {
              localDispatch(Reset);
              Log.infof(m => m("Mounted or src changed: %s", url));

              url
              |> Internal.isUrlAllowed
              |> (
                fun
                | true =>
                  Service_Net.Request.download(
                    ~proxy,
                    ~setup=Oni_Core.Setup.init(),
                    url,
                  )
                | false => Lwt.fail_with(url ++ " is not an allowed domain")
              );
            };

          let promise =
            downloadPromise
            |> LwtEx.tap(path => {
                 Log.infof(m => m("Download succeeded: %s to %s", url, path))
               })
            |> LwtEx.flatMap(filePath => Config.mapper(~filePath));

          Lwt.on_success(promise, res => {
            localDispatch(DownloadSuccess(res))
          });

          Lwt.on_failure(
            promise,
            exn => {
              let errMsg = exn |> Printexc.to_string;
              Log.errorf(m => m("Download failed %s with %s", url, errMsg));
              localDispatch(DownloadFailed(errMsg));
            },
          );
          None;
        },
      );

    renderItem(state);
  };
};
