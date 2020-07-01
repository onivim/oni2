open Oni_Core;

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
};
