type t = Types.mode;

let show = (mode: t) => {
  switch (mode) {
  | Normal => "Normal"
  | Visual => "Visual"
  | CommandLine => "CommandLine"
  | Replace => "Replace"
  | Operator => "Operator"
  | Insert => "Insert"
  | Select => "Select"
  };
};

let getCurrent = Native.vimGetMode;

let onChanged = (f: Listeners.modeChangedListener) => {
  Event.add(f, Listeners.modeChanged);
};
