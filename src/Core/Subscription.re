module type Provider = {
  type action;
  type params;

  let start : (~id: string, ~params: params, ~dispatch: action => unit) => unit;
  let update : (~id: string, ~params: params, ~dispatch: action => unit) => unit;
  let dispose : (~id: string) => unit;
};

type provider('action, 'params) =
  (module Provider with type action = 'action and type params = 'params);

// We need this to be a GADT because we want to "hide" the `'params` type
// variable. A GADT allows us to use "existential" type variables which are
// carried in the value itself instead of being a parameter on the type. That
// way we can make a kind of hetereogenous list with subscriptions containing
// types for `'params`.
type t('action) = 
  Subscription({
    id: string,
    provider: provider('action, 'params),
    params: 'params,
  }) : t('action);

let create = (id: string, provider: provider('action, 'params), params: 'params) =>
  Subscription({ id, provider, params });

module Runner = (Config: { type action; let id: string }) => {
  let jobs = Hashtbl.create(10);

  let applyNamespaceToIds =
    List.map(
      (Subscription(sub)) =>
        Subscription({ ...sub, id: Config.id ++ "$" ++ sub.id })
    );

  let diff = subscriptions => {
    let removed = 
      Hashtbl.fold(
        (id, subscription, acc) =>
          if (List.exists((Subscription(sub)) => sub.id == id, subscriptions)) {
            acc
          } else {
            [subscription, ...acc]
          },
        jobs, []);

    let (added, remaining) =
      List.fold_left(
        ((added, remaining), subscription: t(Config.action)) => {
          let Subscription({ id, params, provider:(module Provider)}) = subscription;

          switch (Hashtbl.find_opt(jobs, id)) {
            | None =>
              ([subscription, ...added], remaining)

            | Some(_) =>
              (added, [subscription, ...remaining])
          }
      }, ([], []), subscriptions);

      (added, removed, remaining);
  };


  let start = (dispatch, subscription: t(Config.action)) => {
    let Subscription{ id, params, provider:(module Provider)} = subscription;

    Hashtbl.add(jobs, id, subscription);
    Provider.start(id, params, dispatch);
  };

  let dispose = (subscription: t(Config.action)) => {
    let Subscription({ id, provider:(module Provider)}) = subscription;

    Hashtbl.remove(jobs, id);
    Provider.dispose(id);
  };

  let update = (dispatch, subscription: t(Config.action)) => {
    let Subscription({ id, params, provider:(module Provider)}) = subscription;

    Hashtbl.replace(jobs, id, subscription);
    Provider.update(id, params, dispatch);
  };


  let run = (~dispatch, subscriptions) => {
    let (added, removed, remaining) =
      subscriptions
        |> applyNamespaceToIds
        |> diff;

    List.iter(start(dispatch), added);
    List.iter(dispose, removed);
    List.iter(update(dispatch), remaining);
  };
}