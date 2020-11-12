type t = Types.cmdline;

let getCompletions = (~context: Context.t=Context.current(), ()) => {
  GlobalState.context := Some(context);
  let completions = Native.vimCommandLineGetCompletions();
  GlobalState.context := None;
  completions;
};

let getText = Native.vimCommandLineGetText;

let getPosition = Native.vimCommandLineGetPosition;

let getType = Native.vimCommandLineGetType;

let onEnter = f => Event.add(f, Listeners.commandLineEnter);
let onLeave = f => Event.add(f, Listeners.commandLineLeave);
let onUpdate = f => Event.add(f, Listeners.commandLineUpdate);
