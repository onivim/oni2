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

module Process = {
  let spawn =
    Luv.Process.spawn(
      ~windows_hide=true,
      ~windows_hide_console=true,
      ~windows_hide_gui=true,
    );
};
