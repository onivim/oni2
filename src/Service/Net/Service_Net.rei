open Oni_Core;

exception ConnectionFailed;
exception ResponseParseFailed;

module Proxy: {
  type t = {
    httpUrl: option(string),
    httpsUrl: option(string),
    strictSSL: bool,
  };

  let none: t;
};

module Request: {
  let json:
    (
      ~proxy: Proxy.t,
      ~setup: Oni_Core.Setup.t,
      ~decoder: Json.decoder('a),
      string
    ) =>
    Lwt.t('a);

  let download:
    (~dest: string=?, ~proxy: Proxy.t, ~setup: Oni_Core.Setup.t, string) =>
    Lwt.t(string);
};
