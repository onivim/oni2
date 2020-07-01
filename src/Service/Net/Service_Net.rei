open Oni_Core;
module Request: {
  let json:
    (~setup: Oni_Core.Setup.t, ~decoder: Json.decoder('a), string) =>
    Lwt.t('a);
};
