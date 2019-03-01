/*
 * TextmateClient.re
 *
 * This is a client-side API for integrating with our textmate 'server'. 
 *
 * Long-term, it'd be ideal to instead implement this API in an in-proc fashion -
 * for example, if vscode-textmate was ported to native ReasonML
 */

open Reason_jsonrpc;

type scopeInfo = {
    scopeName: string,
    path: string,
};

type initializationInfo = list(scopeInfo);

type t = {
  process: NodeProcess.t,  
  rpc: Rpc.t,
};

type simpleCallback = unit => unit;
let defaultCallback: simpleCallback = () => ();

type onScopeLoaded = (string) => unit;
let defaultScopeLoaded: onScopeLoaded = (_) => ();

let emptyJsonValue = `Assoc([]);

let start = (
    ~onClosed=defaultCallback,
    ~onInitialized=defaultCallback,
    ~onScopeLoaded=defaultScopeLoaded,
    setup: Setup.t, 
    initializationInfo) => {
   let process = NodeProcess.start(setup, setup.textmateServicePath); 

   let onNotification = (n: Notification.t, _) => {
       switch (n.method, n.params) {
       | ("textmate/initialized", _) => onInitialized()
       | ("textmate/scopeLoaded", `String(s)) => onScopeLoaded(s)
       | _ => ();
       }
   };

   let onRequest = (_, _) => Ok(emptyJsonValue);

   let rpc = Rpc.start(
    ~onNotification,
    ~onRequest,
    ~onClose=onClosed,
    process.stdout,
    process.stdin,
   );

   let mapScopeInfoToJson = (v: scopeInfo) => {
    (v.scopeName, `String(v.path))
   };

   let initInfo = initializationInfo |> List.map(mapScopeInfoToJson);
   Rpc.sendNotification(rpc, "initialize", `Assoc(initInfo));

    { process, rpc }
}

let getpid = (v: t) => v.process.pid;

let preloadScope = (v: t, scopeName: string) => {
    Rpc.sendNotification(
      v.rpc,
      "textmate/preloadScope",
      `String(scopeName),
    );
};

let pump = (v:t) => Rpc.pump(v.rpc);

let close = (v: t) => {
  Rpc.sendNotification(v.rpc, "exit", emptyJsonValue);

  let result = Unix.waitpid([], v.process.pid);
  result;
};
