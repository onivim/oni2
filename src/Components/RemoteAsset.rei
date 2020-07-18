open Revery.UI;

type status =
  | Downloading
  | Downloaded({filePath: string})
  | DownloadFailed({errorMsg: string});

let make:
  (
    ~key: Brisk_reconciler.Key.t=?,
    ~url: string,
    ~children: status => React.element(React.node),
    unit
  ) =>
  React.element(React.node);
