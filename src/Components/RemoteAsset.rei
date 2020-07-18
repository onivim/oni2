open Oni_Core;
open Oni_Core.Utility;

open Revery;
open Revery.UI;

type status =
  | Downloading
  | Downloaded({ filePath: string })
  | DownloadFailed({ errorMsg: string});

let make:
  (
    ~key: Brisk_reconciler.Key.t=?,
    ~src: string,
    ~children: status => React.element(React.node),
    unit
  ) =>
  React.element(React.node);
