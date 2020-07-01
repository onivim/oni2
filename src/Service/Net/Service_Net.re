open Oni_Core;

module Internal = {
  let getTemporaryFilePath = () => {
    open Base.Result.Let_syntax;

    // TODO: De-syncify
    let ret = {
      let%bind tempDir = Luv.Path.tmpdir();
      prerr_endline ("TEMPORARY FILE: " ++ tempDir);
      let%bind (tempFile, _) = Luv.File.Sync.mkstemp(
        tempDir ++ "/oni2-download-XXXXXX"
      );
      Ok(tempFile);
    };

    ret
    |> Result.map_error(Luv.Error.strerror);
  };
}

module Request = {
  let json = (~setup, ~decoder: Json.decoder('a), url) => {
    let promise = NodeTask.run(~args=[url], ~setup, "request.js");
    Lwt.bind(
      promise,
      (output: string) => {
        let result: result('a, string) =
          output
          |> Yojson.Safe.from_string
          |> Json.Decode.decode_value(decoder)
          |> Result.map_error(Json.Decode.string_of_error);

        switch (result) {
        | Ok(v) => Lwt.return(v)
        | Error(msg) => Lwt.fail_with(msg)
        };
      },
    );
  };
  
  let download = (
    ~dest=?,
    ~setup, url) => {
      let maybeDest = switch (dest) {
      | None => Internal.getTemporaryFilePath()
      | Some(path) => Ok(path);
      };

      switch(maybeDest) {
      | Error(msg) => Lwt.fail_with(msg)
      | Ok(dest) => 
        NodeTask.run(~args=[url, dest], ~setup, "download.js")
        |> Lwt.map(_ => dest);
      }
  }
};
