open Oni_Core;

type model = option(Service_Net.Proxy.t);

let initial = None;

let proxy = Fun.id;

module Configuration = {
  open Config.Schema;

  let httpProxy = setting("http.proxy", nullable(string), ~default=None);

  let httpProxyStrictSSL =
    setting("http.proxyStrictSSL", bool, ~default=true);

  let httpProxySupport =
    setting("http.proxySupport", string, ~default="override");
};

module Contributions = {
  let configuration =
    Configuration.[
      httpProxy.spec,
      httpProxyStrictSSL.spec,
      httpProxySupport.spec,
    ];
};

let configurationChanged = (config, model) => {
  let maybeProxyUrl = Configuration.httpProxy.get(config);

  maybeProxyUrl
  |> Option.map(url => {
       Service_Net.Proxy.{
         url,
         strictSSL: Configuration.httpProxyStrictSSL.get(config),
       }
     });
};
