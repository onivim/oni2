open Types;

let getState: unit => Types.cmdline =
  () => {
    let text =
      switch (Native.vimCommandLineGetText()) {
      | Some(v) => v
      | None => ""
      };
    let position = Native.vimCommandLineGetPosition();
    let cmdType = Native.vimCommandLineGetType();
    {cmdType, text, position};
  };
