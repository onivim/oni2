[@deriving show({with_path: false})]
type command;

[@deriving show({with_path: false})]
type msg;

module Contributions: {let commands: list(Oni_Core.Command.t(msg));};
