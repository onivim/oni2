type t = Native.buffer;

module IntMap =
  Map.Make({
    let compare = Stdlib.compare;
    type t = int;
  });

let knownBuffers: ref(IntMap.t(int)) = ref(IntMap.empty);
let currentBuffer: ref(option(Native.buffer)) = ref(None);
let lastFilename = ref(None);
let lastFiletype = ref(None);

let notifyUpdate = (buffer: t) => {
  let id = Native.vimBufferGetId(buffer);
  let version = Native.vimBufferGetChangedTick(buffer);
  knownBuffers :=
    IntMap.update(
      id,
      curr =>
        switch (curr) {
        | None => Some(version)
        | Some(_) => Some(version)
        },
      knownBuffers^,
    );
};

let string_opt = s =>
  switch (s) {
  | None => ""
  | Some(v) => v
  };

let doFullUpdate = (buffer: t) => {
  let bu = BufferUpdate.createFull(buffer);
  notifyUpdate(buffer);
  Event.dispatch(bu, Listeners.bufferUpdate);
};
let checkBufferForUpdate = buffer => {
  let id = Native.vimBufferGetId(buffer);

  switch (IntMap.find_opt(id, knownBuffers^)) {
  | None =>
    let update = BufferUpdate.createInitial(buffer);
    notifyUpdate(buffer);
    Event.dispatch(buffer, Listeners.bufferEnter);
    Event.dispatch(update, Listeners.bufferUpdate);
  | Some(lastVersion) =>
    /* Check if the current buffer changed */
    switch (currentBuffer^) {
    | Some(v) =>
      if (v != buffer) {
        Event.dispatch(v, Listeners.bufferLeave);
        Event.dispatch(buffer, Listeners.bufferEnter);
        currentBuffer := Some(buffer);
        lastFilename := Native.vimBufferGetFilename(buffer);
        lastFiletype := Native.vimBufferGetFiletype(buffer);
      } else {
        let lastF = lastFilename^;
        let lastFt = lastFiletype^;
        let newFileName = Native.vimBufferGetFilename(buffer);
        let newFiletype = Native.vimBufferGetFiletype(buffer);

        if (!String.equal(string_opt(lastF), string_opt(newFileName))) {
          lastFilename := newFileName;
          Event.dispatch(
            BufferMetadata.ofBuffer(buffer),
            Listeners.bufferFilenameChanged,
          );
        };

        if (!String.equal(string_opt(lastFt), string_opt(newFiletype))) {
          lastFiletype := newFiletype;
          Event.dispatch(
            BufferMetadata.ofBuffer(buffer),
            Listeners.bufferFiletypeChanged,
          );
        };
      }
    | None =>
      Event.dispatch(buffer, Listeners.bufferEnter);
      currentBuffer := Some(buffer);
    };
    let newVersion = Native.vimBufferGetChangedTick(buffer);

    if (newVersion > lastVersion) {
      doFullUpdate(buffer);
    };
  };
};

let checkCurrentBufferForUpdate = () => {
  let buffer = Native.vimBufferGetCurrent();
  checkBufferForUpdate(buffer);
};
