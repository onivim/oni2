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

// CONTRIBUTIONS

module Contributions = {
  let commands = [];

  let configuration = [];

  let contextKeys = model => {
    WhenExpr.ContextKeys.(
      ContextKeys.[inZenMode] |> Schema.fromList |> fromSchema(model)
    );
  };
};
