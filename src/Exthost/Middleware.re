let download = (msg: Msg.DownloadService.msg) => {
  switch (msg) {
  | Download({uri, dest}) =>
    let uri = uri |> Oni_Core.Uri.toString;
    let dest = dest |> Oni_Core.Uri.toFileSystemPath;

    Service_Net.Request.download(~dest, ~setup=Oni_Core.Setup.init(), uri)
    |> Lwt.map(_ => Reply.okEmpty);
  };
};
