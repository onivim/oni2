type t = Types.cmdline;

let getCompletions = Native.vimCommandLineGetCompletions;

let getText = Native.vimCommandLineGetText;

let getPosition = Native.vimCommandLineGetPosition;

let getType = Native.vimCommandLineGetType;

let onEnter = f => Event.add(f, Listeners.commandLineEnter);
let onLeave = f => Event.add(f, Listeners.commandLineLeave);
let onUpdate = f => Event.add(f, Listeners.commandLineUpdate);
