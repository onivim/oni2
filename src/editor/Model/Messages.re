open Oni_Core.Types;

let getMessageType = message =>
  switch (message) {
  | "emsg" => Emsg
  | "echo" => Echo
  | "echomsg" => Echomsg
  | "echoerr" => Echoerr
  | "return_prompt" => ReturnPrompt
  | "quickfix" => Quickfix
  | _ => Unknown
  };

[@deriving show]
type t = list(message);

let create = (): t => [];

let reduce = (state: t, action: Actions.t) =>
  switch (action) {
  | ShowMessage(message) => [message, ...state]
  | ClearMessages => []
  | _ => state
  };
