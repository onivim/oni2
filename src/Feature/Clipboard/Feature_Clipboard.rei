open Oni_Core;

type model;

type msg;

type outmsg =
| Pasted({ lines: list(string) });

module Commands: {
    let paste: Command.t(msg);
};

module Contributions: {
    let commands: list(Command.t(msg));
}
