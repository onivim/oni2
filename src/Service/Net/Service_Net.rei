open Oni_Core;

exception ConnectionFailed;
exception ResponseParseFailed;

module Proxy: {
  type t = {
    url: string,
    strictSSL: bool,
  };
};

module Request: {
  let json:
    (
      ~proxy: option(Proxy.t),
      ~setup: Oni_Core.Setup.t,
      ~decoder: Json.decoder('a),
      string
    ) =>
    Lwt.t('a);

  let download:
    (~dest: string=?, ~setup: Oni_Core.Setup.t, string) => Lwt.t(string);
};
