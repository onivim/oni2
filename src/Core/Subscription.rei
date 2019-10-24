module type Provider = {
  type action;
  type params;

  let start: (~id: string, ~params: params, ~dispatch: action => unit) => unit;
  let update:
    (~id: string, ~params: params, ~dispatch: action => unit) => unit;
  let dispose: (~id: string) => unit;
};

type provider('action, 'params) = (module Provider with
                                      type action = 'action and
                                      type params = 'params);

type t('action);

let create: (string, provider('action, 'params), 'params) => t('action);

module Runner:
  (Config: {
     type action;
     let id: string;
   }) =>
   {
    let run:
      (~dispatch: Config.action => unit, list(t(Config.action))) => unit;
  };
