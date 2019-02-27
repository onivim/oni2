type t = {
  input: in_channel,
  output: out_channel,
  mutable shouldClose: bool,
};

let _send = (rpc, json: Yojson.Safe.json) => {
  let str = Yojson.Safe.to_string(json);

  let length = String.length(str);
  let contentLengthString =
    "Content-Length: " ++ string_of_int(length) ++ "\r\n";
  output_string(rpc.output, contentLengthString);
  output_string(rpc.output, "\r\n");
  output_string(rpc.output, str);
  flush(rpc.output);
};

let _sendResponse = (rpc: t, msg: Yojson.Safe.json, id: int) => {
  let response = `Assoc([("id", `Int(id)), ("result", msg)]);
  _send(rpc, response);
};

let sendNotification = (rpc: t, method: string, msg: Yojson.Safe.json) => {
  let response = `Assoc([("method", `String(method)), ("params", msg)]);
  _send(rpc, response);
};

type message =
  | Request(int, Request.t)
  | Notification(Notification.t)
  | Response(Response.t);

let parse: string => message =
  msg => {
    let p = Yojson.Safe.from_string(msg);

    switch (Notification.is(p), Request.is(p), Response.is(p)) {
    | (true, _, _) => Notification.parse(p) |> Notification
    | (_, true, _) =>
      let id = p |> Yojson.Safe.Util.member("id") |> Yojson.Safe.Util.to_int;
      Request(id, Request.parse(p));
    | _ => Response(UnknownResponse)
    };
  };

let start =
    (~onNotification, ~onRequest, input: in_channel, output: out_channel) => {
  let rpc: t = {input, output, shouldClose: false};

  set_binary_mode_in(input, true);
  set_binary_mode_out(output, true);
  let id = Unix.descr_of_in_channel(stdin);
  while (!rpc.shouldClose) {
    Thread.wait_read(id);

    let preamble = Preamble.read(input);
    let len = preamble.contentLength;
    Log.debug("Message length: " ++ string_of_int(len));

    /* Read message */
    let buffer = Bytes.create(len);
    let read = ref(0);
    while (read^ < len) {
      let n = Pervasives.input(input, buffer, 0, len);
      read := read^ + n;
    };

    let str = Bytes.to_string(buffer);
    Log.debug("Received msg: " ++ str);

    let result = parse(str);

    switch (result) {
    | Notification(v) => onNotification(v, rpc)
    | Request(id, v) =>
      switch (onRequest(rpc, v)) {
      | Ok(result) => _sendResponse(rpc, result, id)
      | Error(msg) => Log.error(msg)
      | exception (Yojson.Json_error(msg)) => Log.error(msg)
      }
    | _ => Log.error("Unhandled message")
    };
  };
};

let stop = (rpc: t) => rpc.shouldClose = true;
