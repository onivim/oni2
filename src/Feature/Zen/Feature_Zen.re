open Oni_Core;

// CONFIGURATION

module Configuration = {
  open Config.Schema;

  let hideTabs = setting("editor.zenMode.hideTabs", bool, ~default=true);

  let singleFile = setting("editor.zenMode.singleFile", bool, ~default=true);
};

[@deriving show]
type command =
  | EnableZenMode
  | DisableZenMode;

[@deriving show]
type msg =
  | Command(command);

type model = {
  // Keep track of the first configuration change...
  // When the configuration is first changed, we should
  // apply 'single-file' zen mode, if applicable.
  hasGottenFirstConfigurationChange: bool,
  isSingleFile: bool,
  isZenMode: bool,
  shouldHideTabsInZenMode: bool,
};

let initial = (~isSingleFile) => {
  hasGottenFirstConfigurationChange: false,
  isSingleFile,
  isZenMode: false,
  shouldHideTabsInZenMode: false,
};

let isZen = ({isZenMode, _}) => isZenMode;

let shouldShowTabsInZenMode = ({shouldHideTabsInZenMode, _}) =>
  !shouldHideTabsInZenMode;

let exitZenMode = model => {...model, isZenMode: false};

let configurationChanged = (config, model) => {
  let isSingleFileAllowed = Configuration.singleFile.get(config);

  let model' = {
    ...model,
    hasGottenFirstConfigurationChange: true,
    shouldHideTabsInZenMode: Configuration.hideTabs.get(config),
  };
  if (isSingleFileAllowed
      && !model.hasGottenFirstConfigurationChange
      && model.isSingleFile) {
    {...model', isZenMode: true};
  } else {
    model';
  };
};

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

// COMMANDS

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

  let configuration = Configuration.[hideTabs.spec, singleFile.spec];

  let contextKeys = model => {
    WhenExpr.ContextKeys.(
      ContextKeys.[inZenMode] |> Schema.fromList |> fromSchema(model)
    );
  };
};

// TESTING

module Testing = {
  let enableZenMode = Command(EnableZenMode);
};
