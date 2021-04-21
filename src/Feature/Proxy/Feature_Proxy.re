open Oni_Core;

type model = Service_Net.Proxy.t;

let default =
  Service_Net.Proxy.{httpUrl: None, httpsUrl: None, strictSSL: true};

let proxy = Fun.id;

module Configuration = {
  open Config.Schema;

  let httpProxy = setting("http.proxy", nullable(string), ~default=None);

  let httpsProxy = setting("https.proxy", nullable(string), ~default=None);

  let httpProxyStrictSSL =
    setting("http.proxyStrictSSL", bool, ~default=true);

  let httpProxySupport =
    setting("http.proxySupport", string, ~default="override");
};

module Contributions = {
  let configuration =
    Configuration.[
      httpProxy.spec,
      httpsProxy.spec,
      httpProxyStrictSSL.spec,
      httpProxySupport.spec,
    ];
};

let configurationChanged = (config, _model) => {
  let maybeHttpUrl = Configuration.httpProxy.get(config);
  let maybeHttpsUrl = Configuration.httpsProxy.get(config);
  let strictSSL = Configuration.httpProxyStrictSSL.get(config);

  Service_Net.Proxy.{
    httpUrl: maybeHttpUrl,
    httpsUrl: maybeHttpsUrl,
    strictSSL,
  };
};
