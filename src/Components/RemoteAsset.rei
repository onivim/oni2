open Revery.UI;

module Make:
  (
    Config: {
      type asset;
      let mapper: (~filePath: string) => Lwt.t(asset);
    },
  ) =>
   {
    type status =
      | Downloading
      | Downloaded(Config.asset)
      | DownloadFailed({errorMsg: string});

    let make:
      (
        ~key: Brisk_reconciler.Key.t=?,
        ~proxy: Service_Net.Proxy.t,
        ~url: string,
        ~children: status => React.element(React.node),
        unit
      ) =>
      React.element(React.node);
  };
