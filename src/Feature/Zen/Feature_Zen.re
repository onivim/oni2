open Oni_Core;

[@deriving show]
type command =
  | EnableZenMode
  | DisableZenMode;

[@deriving show]
type msg =
  | Command(command);

type model = {
  isSingleFile: bool,
  isZenMode: bool,
  shouldShowTabsInZenMode: bool,
};

let initial = (~isSingleFile) => {
  isSingleFile,
  isZenMode: true,
  shouldShowTabsInZenMode: false,
};

let isZen = ({isZenMode, _}) => isZenMode;

let shouldShowTabsInZenMode = ({shouldShowTabsInZenMode, _}) => shouldShowTabsInZenMode;

let exitZenMode = model => {...model, isZenMode: false};

let update = (msg, model) =>
  switch (msg) {
  | Command(EnableZenMode) => {...model, isZenMode: true}

  | Command(DisableZenMode) => {...model, isZenMode: false}
  };

// CONTEXT KEYS

module ContextKeys = {
  open WhenExpr.ContextKeys.Schema;

  let inZenMode = bool("zenMode", isZen);
};

module Commands = {
  open Feature_Commands.Schema;
  let enable =
    define(
      ~category="View",
      ~title="Enable Zen Mode",
      ~isEnabledWhen=WhenExpr.parse("!zenMode"),
      "workbench.action.enableZenMode",
      Command(EnableZenMode),
    );

  let disable =
    define(
      ~category="View",
      ~title="Disable Zen Mode",
      ~isEnabledWhen=WhenExpr.parse("zenMode"),
      "workbench.action.disableZenMode",
      Command(DisableZenMode),
    );
};

// CONTRIBUTIONS

module Contributions = {
  let commands = Commands.[enable, disable];

  let configuration = [];

  let contextKeys = model => {
    WhenExpr.ContextKeys.(
      ContextKeys.[inZenMode] |> Schema.fromList |> fromSchema(model)
    );
  };
};
