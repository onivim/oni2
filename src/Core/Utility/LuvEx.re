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

module Buffer = {
  
  let toBytesRev = (buffers: list(Luv.Buffer.t)) => {
    let totalSize = Luv.Buffer.total_size(buffers);

    let bytes = Bytes.create(totalSize);
    let rec loop = (offset, buffers) => {
    switch (buffers) {
    | [] => ()
    | [hd, ...tail] =>
      let size = Luv.Buffer.size(hd);
      Luv.Buffer.blit_to_bytes(hd, bytes, ~destination_offset=offset - size);
      loop(offset - size, tail);
    } 
    };

    loop(totalSize, buffers);
    bytes;
  };
}
