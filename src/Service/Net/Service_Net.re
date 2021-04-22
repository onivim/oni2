open Oni_Core;

module Log = (val Log.withNamespace("Oni2.Service.Net"));

exception ConnectionFailed;
exception ResponseParseFailed;

module Proxy = {
  type t = {
    httpUrl: option(string),
    httpsUrl: option(string),
    strictSSL: bool,
  };

  let none = {httpUrl: None, httpsUrl: None, strictSSL: false};
};

module Internal = {
  let getTemporaryFilePath = () => {
    open Base.Result.Let_syntax;

    // TODO: De-syncify
    let ret = {
      let%bind tempDir = Luv.Path.tmpdir();
      let%bind (tempFile, _) =
        Luv.File.Sync.mkstemp(tempDir ++ "/oni2-download-XXXXXX");
      Ok(tempFile);
    };

    ret |> Result.map_error(Luv.Error.strerror);
  };

  let proxyToEnvironmentVariables = (proxy: Proxy.t) => {
    let initial = [
      ("PROXY_ALLOW_INSECURE_SSL", proxy.strictSSL ? "0" : "1"),
    ];

    let addHttpProxy = current => {
      switch (proxy.httpUrl) {
      | None => current
      | Some(url) => [("HTTP_PROXY", url), ...current]
      };
    };

    let addHttpsProxy = current => {
      switch (proxy.httpUrl) {
      | None => current
      | Some(url) => [("HTTPS_PROXY", url), ...current]
      };
    };

    initial |> addHttpProxy |> addHttpsProxy;
  };
};

module Cache = {
  let cache = Hashtbl.create(32);
  let check = (url: string) => {
    let ret = Hashtbl.find_opt(cache, url);
    switch (ret) {
    | None => Log.tracef(m => m("Cache miss: %s", url))
    | Some(_) => Log.tracef(m => m("Cache hit: %s", url))
    };
    ret;
  };

  let add = (~url: string, dest: string) => {
    Log.tracef(m => m("Adding cache entry: %s -> %s", url, dest));
    Hashtbl.add(cache, url, dest);
  };
};

module Request = {
  let json = (~proxy, ~setup, ~decoder: Json.decoder('a), url) => {
    let additionalEnvironment = Internal.proxyToEnvironmentVariables(proxy);
    Lwt.try_bind(
      () =>
        NodeTask.run(
          ~additionalEnvironment,
          ~args=[url],
          ~setup,
          "request.js",
        ),
      (output: string) => {
        let result: result('a, string) =
          output
          |> Yojson.Safe.from_string
          |> Json.Decode.decode_value(decoder)
          |> Result.map_error(Json.Decode.string_of_error);

        switch (result) {
        | Ok(v) => Lwt.return(v)
        | Error(_msg) => Lwt.fail(ResponseParseFailed)
        };
      },
      _exn => Lwt.fail(ConnectionFailed),
    );
  };

  let download = (~dest=?, ~proxy, ~setup, url) => {
    let run = maybeDest => {
      let destResult =
        switch (maybeDest) {
        | None => Internal.getTemporaryFilePath()
        | Some(path) => Ok(path)
        };

      switch (destResult) {
      | Error(msg) => Lwt.fail_with(msg)
      | Ok(dest) =>
        let additionalEnvironment =
          Internal.proxyToEnvironmentVariables(proxy);
        NodeTask.run(
          ~additionalEnvironment,
          ~args=[url, dest],
          ~setup,
          "download.js",
        )
        |> Utility.LwtEx.tap(msg => {Log.info(msg)})
        |> Lwt.map(_ => {
             Cache.add(~url, dest);
             dest;
           });
      };
    };

    let maybeCacheResult = Cache.check(url);

    switch (maybeCacheResult, dest) {
    // There was a result cached, and dest was not specified - return cached dest
    | (Some(cachedDest), None) => Lwt.return(cachedDest)
    // There was a result cached, and a dest was specified... We don't need
    // a request, we can just copy locally.
    | (Some(cachedDest), Some(dest)) =>
      Service_OS.Api.copy(~source=cachedDest, ~target=dest, ~overwrite=true)
      |> Lwt.map(_ => dest)
    // No cache result - run the request
    | (None, maybeDest) => run(maybeDest)
    };
  };
};
