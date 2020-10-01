type pane =
  | FileExplorer
  | SCM
  | Extensions
  | Search;

type location =
  | Left
  | Right;

[@deriving show]
type command =
  | OpenExplorerPane
  | OpenSearchPane
  | OpenSCMPane
  | OpenExtensionsPane
  | ToggleVisibility;

[@deriving show]
type msg =
  | ResizeInProgress(int)
  | ResizeCommitted
  | Command(command)
  | FileExplorerClicked
  | SearchClicked
  | SCMClicked
  | ExtensionsClicked;

module Constants = {
  let defaultWidth = 225;
  let minWidth = 50;
  let maxWidth = 800;
};

type model = {
  // Track the last value of 'workbench.sideBar.visible'
  // If it changes, we should update
  openByDefault: bool,
  isOpen: bool,
  selected: pane,
  width: int,
  resizeDelta: int,
  shouldSnapShut: bool,
  location,
};

type outmsg =
  | Nothing
  | Focus
  | PopFocus;

let selected = ({selected, _}) => selected;
let isOpen = ({isOpen, _}) => isOpen;
let location = ({location, _}) => location;

let initial = {
  openByDefault: false,
  isOpen: false,
  selected: FileExplorer,
  width: Constants.defaultWidth,
  resizeDelta: 0,
  shouldSnapShut: true,
  location: Left,
};

let width = ({width, resizeDelta, isOpen, location, shouldSnapShut, _}) =>
  if (!isOpen) {
    0;
  } else {
    let candidate =
      switch (location) {
      | Left => width + resizeDelta
      | Right => width - resizeDelta
      };

    if (candidate < Constants.minWidth && shouldSnapShut) {
      0;
    } else if (candidate > Constants.maxWidth) {
      Constants.maxWidth;
    } else {
      max(0, candidate);
    };
  };

let update = (msg, model) =>
  switch (msg) {
  | ResizeInProgress(delta) =>
    let model' =
      if (model.isOpen) {
        {...model, resizeDelta: delta};
      } else {
        {
          ...model,
          width: 0,
          resizeDelta: delta,
          isOpen: true,
          shouldSnapShut: false,
        };
      };

    (model', Nothing);

  | ResizeCommitted =>
    let newWidth = width(model);
    let model' =
      if (newWidth == 0) {
        {...model, isOpen: false, shouldSnapShut: true, resizeDelta: 0};
      } else {
        {...model, width: newWidth, shouldSnapShut: true, resizeDelta: 0};
      };
    (model', Nothing);

  | FileExplorerClicked
  | Command(OpenExplorerPane) => (
      {...model, isOpen: true, selected: FileExplorer},
      Focus,
    )

  | SCMClicked
  | Command(OpenSCMPane) => ({...model, isOpen: true, selected: SCM}, Focus)

  | ExtensionsClicked
  | Command(OpenExtensionsPane) => (
      {...model, isOpen: true, selected: Extensions},
      Focus,
    )

  | SearchClicked
  | Command(OpenSearchPane) => (
      {...model, isOpen: true, selected: Search},
      Focus,
    )

  | Command(ToggleVisibility) =>
    // If we were open, and we are going to close, we should pop focus...
    // Otherwise, if we are opening, we need to acquire focus.
    let eff = model.isOpen ? PopFocus : Focus;
    ({...model, shouldSnapShut: true, isOpen: !model.isOpen}, eff);
  };

let isVisible = (pane, model) => {
  model.isOpen && model.selected == pane;
};

let setDefaultVisibility = (pane, defaultVisibility) =>
  if (pane.openByDefault == defaultVisibility) {
    pane;
  } else {
    {...pane, openByDefault: defaultVisibility, isOpen: defaultVisibility};
  };

let toggle = (pane, state) =>
  if (pane == state.selected) {
    {...state, shouldSnapShut: true, isOpen: !state.isOpen};
  } else {
    {...state, shouldSnapShut: true, isOpen: true, selected: pane};
  };

let setDefaultLocation = (state, setting) => {
  let location = setting == "right" ? Right : Left;
  {...state, location};
};

type settings = {
  sideBarLocation: string,
  sideBarVisibility: bool,
};

let setDefaults = (state, settings) => {
  let {sideBarVisibility, sideBarLocation} = settings;
  let state = setDefaultLocation(state, sideBarLocation);
  setDefaultVisibility(state, sideBarVisibility);
};

module Commands = {
  open Feature_Commands.Schema;

  let openSearchPane =
    define(
      ~category="Search",
      ~title="Find in Files",
      "workbench.action.findInFiles",
      Command(OpenSearchPane),
    );

  let openExtensionsPane =
    define(
      ~category="Extensions",
      ~title="Open Extensions Pane",
      "workbench.view.extensions",
      Command(OpenExtensionsPane),
    );

  let openExplorerPane =
    define(
      ~category="Explorer",
      ~title="Open File Explorer Pane",
      "workbench.view.explorer",
      Command(OpenExplorerPane),
    );

  let openSCMPane =
    define(
      ~category="Source Control",
      ~title="Open Source Control Pane",
      "workbench.view.scm",
      Command(OpenSCMPane),
    );

  let toggleSidebar =
    define(
      ~category="View",
      ~title="Toggle Sidebar Visibility",
      "workbench.action.toggleSidebarVisibility",
      Command(ToggleVisibility),
    );
};

module Keybindings = {
  open Oni_Input.Keybindings;

  let findInFiles = {
    key: "<S-C-F>",
    command: Commands.openSearchPane.id,
    condition: "!isMac" |> WhenExpr.parse,
  };

  let findInFilesMac = {
    key: "<D-S-F>",
    command: Commands.openSearchPane.id,
    condition: "isMac" |> WhenExpr.parse,
  };

  let openExtensions = {
    key: "<S-C-X>",
    command: Commands.openExtensionsPane.id,
    condition: "!isMac" |> WhenExpr.parse,
  };

  let openExtensionsMac = {
    key: "<D-S-X>",
    command: Commands.openExtensionsPane.id,
    condition: "isMac" |> WhenExpr.parse,
  };

  let openExplorer = {
    key: "<S-C-E>",
    command: Commands.openExplorerPane.id,
    condition: "!isMac" |> WhenExpr.parse,
  };

  let openExplorerMac = {
    key: "<D-S-E>",
    command: Commands.openExplorerPane.id,
    condition: "isMac" |> WhenExpr.parse,
  };

  // This keybinding is used in both MacOS & Windows
  let openSCM = {
    key: "<S-C-G>",
    command: Commands.openSCMPane.id,
    condition: WhenExpr.Value(True),
  };

  let toggleSidebar = {
    key: "<C-B>",
    command: Commands.toggleSidebar.id,
    condition: "!isMac" |> WhenExpr.parse,
  };

  let toggleSidebarMac = {
    key: "<D-B>",
    command: Commands.toggleSidebar.id,
    condition: "isMac" |> WhenExpr.parse,
  };
};

module ContextKeys = {
  open WhenExpr.ContextKeys.Schema;

  let sideBarVisible = bool("sideBarVisible", isOpen);

  module Focused = {
    let sideBarFocus = bool("sideBarFocus", _ => true);
  };
};

module Contributions = {
  let commands =
    Commands.[
      openSearchPane,
      openExplorerPane,
      openExtensionsPane,
      openSCMPane,
      toggleSidebar,
    ];

  let keybindings =
    Keybindings.[
      findInFiles,
      findInFilesMac,
      openExtensions,
      openExtensionsMac,
      openExplorer,
      openExplorerMac,
      toggleSidebar,
      toggleSidebarMac,
      openSCM,
    ];

  let contextKeys = (~isFocused) => {
    let common = ContextKeys.[sideBarVisible];

    let focused = ContextKeys.Focused.[sideBarFocus];

    if (isFocused) {
      common @ focused;
    } else {
      common;
    };
  };
};
