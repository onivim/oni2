let all =
    (
      ~initial: 'acc,
      join: ('acc, 'cur) => 'acc,
      promises: list(Lwt.t('cur)),
    ) => {
  List.fold_left(
    (accPromise, promise) => {
      let%lwt acc = accPromise;
      let%lwt curr = promise;
      Lwt.return(join(acc, curr));
    },
    Lwt.return(initial),
    promises,
  );
};

let fromOption = (~errorMsg) =>
  fun
  | Some(v) => Lwt.return(v)
  | None => Lwt.fail_with(errorMsg);

let some = (~default: 'a, join: ('a, 'a) => 'a, promises: list(Lwt.t('a))) => {
  promises
  |> List.map(p => {
       try%lwt(p) {
       | _exn => Lwt.return(default)
       }
     })
  |> all(~initial=[], join);
};

let flatMap = (f, promise) => Lwt.bind(promise, f);

exception Timeout;

let sync: (~timeout: float=?, Lwt.t('a)) => result('a, exn) =
  (~timeout=10.0, promise) => {
    let completed = ref(None);

    Lwt.on_success(promise, v => {completed := Some(Ok(v))});

    Lwt.on_failure(promise, v => {completed := Some(Error(v))});

    ThreadEx.waitForCondition(~timeout, () => {completed^ != None});

    Option.value(~default=Error(Timeout), completed^);
  };

let tap: ('a => unit, Lwt.t('a)) => Lwt.t('a) =
  (f, promise) => {
    Lwt.on_success(promise, f);
    promise;
  };
