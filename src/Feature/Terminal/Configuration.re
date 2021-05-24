open Oni_Core;

open Config.Schema;
module Codecs = Feature_Configuration.GlobalConfiguration.Codecs;

let shellCmd = ShellUtility.getDefaultShell();

module Shell = {
  let windows =
    setting("terminal.integrated.shell.windows", string, ~default=shellCmd);
  let linux =
    setting("terminal.integrated.shell.linux", string, ~default=shellCmd);
  let osx =
    setting("terminal.integrated.shell.osx", string, ~default=shellCmd);
};

module ShellArgs = {
  let windows =
    setting(
      "terminal.integrated.shellArgs.windows",
      list(string),
      ~default=[],
    );
  let linux =
    setting(
      "terminal.integrated.shellArgs.linux",
      list(string),
      ~default=[],
    );
  let osx =
    setting(
      "terminal.integrated.shellArgs.osx",
      list(string),
      // ~/.[bash|zsh}_profile etc is not sourced when logging in on macOS.
      // Instead, terminals on macOS should run as a login shell (which in turn
      // sources these files).
      // See more at http://unix.stackexchange.com/a/119675/115410.
      ~default=["-l"],
    );
};

let fontFamily =
  setting("terminal.integrated.fontFamily", nullable(string), ~default=None);

let fontSize =
  setting(
    "terminal.integrated.fontSize",
    nullable(Codecs.fontSize),
    ~default=None,
  );
let fontWeight =
  setting(
    "terminal.integrated.fontWeight",
    nullable(Codecs.fontWeight),
    ~default=None,
  );

let fontLigatures =
  setting(
    "terminal.integrated.fontLigatures",
    nullable(Codecs.fontLigatures),
    ~default=None,
  );

let fontSmoothing =
  setting(
    "terminal.integrated.fontSmoothing",
    nullable(
      custom(~encode=FontSmoothing.encode, ~decode=FontSmoothing.decode),
    ),
    ~default=None,
  );

let lineHeight =
  setting(
    "terminal.integrated.lineHeight",
    nullable(custom(~encode=LineHeight.encode, ~decode=LineHeight.decode)),
    ~default=None,
  );
