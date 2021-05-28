exception LuvException(Luv.Error.t);

module Log = (val Kernel.Log.withNamespace("LuvEx"));

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

let allocator = name => {
  let maybeAllocatedBuffer = ref(None);

  let createNewBuffer = size => {
    let newBuffer = Luv.Buffer.create(size);
    Log.infof(m =>
      m("Allocating new buffer for %s with size %d", name, size)
    );
    maybeAllocatedBuffer := Some(newBuffer);
    newBuffer;
  };

  size => {
    switch (maybeAllocatedBuffer^) {
    | Some(buffer) =>
      if (Luv.Buffer.size(buffer) >= size) {
        buffer;
      } else {
        createNewBuffer(size);
      }
    | None => createNewBuffer(size)
    };
  };
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
        Luv.Buffer.blit_to_bytes(
          hd,
          bytes,
          ~destination_offset=offset - size,
        );
        loop(offset - size, tail);
      };
    };

    loop(totalSize, buffers);
    bytes;
  };
};
