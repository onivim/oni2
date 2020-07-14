exception LuvException(Luv.Error.t);

let wrapPromise = (f, arg) => {
  let (promise, resolver) = Lwt.task();

  f(arg, result => {
    switch (result) {
    | Ok(v) => Lwt.wakeup(resolver, v)
    | Error(err) => Lwt.wakeup_exn(resolver, LuvException(err))
    }
  });
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
