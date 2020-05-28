exception LuvException(string);

let wrapPromise = f => {
  let (promise: Lwt.t('a), resolver: Lwt.u('a)) = Lwt.task();

  let handler =
    fun
    | Ok(v) => Lwt.wakeup(resolver, v)
    | Error(err) =>
      Lwt.wakeup_exn(resolver, LuvException(Luv.Error.strerror(err)));

  f(handler);

  promise;
};
