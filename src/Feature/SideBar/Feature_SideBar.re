open Oni_Core;

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
  | ToggleExplorerPane
  | ToggleSearchPane
  | ToggleSCMPane
  | ToggleExtensionsPane
  | ToggleVisibility
  | GotoOutline;

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
  // Adjusted minWidth for empty experience - don't let the text get
  // too clipped!
  let minWidth = 75;
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

type subFocus =
  | Outline;

type outmsg =
  | Nothing
  | Focus(option(subFocus))
  | PopFocus;

let selected = ({selected, _}) => selected;
let isOpen = ({isOpen, resizeDelta, _}) => isOpen || resizeDelta != 0;
let location = ({location, _}) => location;

let isOpenByDefault = ({openByDefault, _}) => openByDefault;

let initial = {
  openByDefault: false,
  isOpen: false,
  selected: FileExplorer,
  width: Constants.defaultWidth,
  resizeDelta: 0,
  shouldSnapShut: true,
  location: Left,
};

let width = ({width, resizeDelta, location, shouldSnapShut, _}) => {
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

let update = (~isFocused, msg, model) => {
  // Focus a pane if it is not open, or close it
  let selectOrClosePane = (~pane, model) =>
    if (!model.isOpen) {
      ({...model, isOpen: true, selected: pane}, Focus(None));
    } else if (model.selected == pane) {
      ({...model, isOpen: false}, PopFocus);
    } else {
      ({...model, selected: pane}, Focus(None));
    };

  let togglePane = (~pane: pane, ~subFocus=None, model) =>
    // Not open - open the sidebar, and focus
    if (!model.isOpen || model.selected != pane) {
      (
        {...model, isOpen: true, selected: pane},
        Focus(subFocus),
        // Sidebar is open, and with the selected pane...
        // If not open by default, we should close.
      );
    } else if (!model.openByDefault && model.isOpen) {
      (
        {...model, isOpen: false, shouldSnapShut: true},
        PopFocus,
        // We should leave focus, but sidebar should stay open
      );
    } else if (isFocused) {
      (model, PopFocus);
    } else {
      (model, Focus(subFocus));
    };

  switch (msg) {
  | ResizeInProgress(delta) =>
    let model' =
      if (model.isOpen) {
        {...model, resizeDelta: delta};
      } else {
        {...model, resizeDelta: delta, isOpen: true, shouldSnapShut: false};
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

  | FileExplorerClicked => selectOrClosePane(~pane=FileExplorer, model)
  | Command(ToggleExplorerPane) => togglePane(~pane=FileExplorer, model)

  | SCMClicked => selectOrClosePane(~pane=SCM, model)
  | Command(ToggleSCMPane) => togglePane(~pane=SCM, model)

  | ExtensionsClicked => selectOrClosePane(~pane=Extensions, model)
  | Command(ToggleExtensionsPane) => togglePane(~pane=Extensions, model)

  | SearchClicked => selectOrClosePane(~pane=Search, model)
  | Command(ToggleSearchPane) => togglePane(~pane=Search, model)

  | Command(GotoOutline) =>
    togglePane(~pane=FileExplorer, ~subFocus=Some(Outline), model)

  | Command(ToggleVisibility) =>
    // If we were open, and we are going to close, we should pop focus...
    // Otherwise, if we are opening, we need to acquire focus.
    let eff = model.isOpen ? PopFocus : Focus(None);
    ({...model, shouldSnapShut: true, isOpen: !model.isOpen}, eff);
  };
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

let show = state => {
  {...state, shouldSnapShut: true, isOpen: true};
};

let setDefaultLocation = (state, setting) => {
  let location = setting == `Right ? Right : Left;
  {...state, location};
};

module Commands = {
  open Feature_Commands.Schema;

  let openSearchPane =
    define(
      ~category="Search",
      ~title="Find in Files",
      "workbench.action.findInFiles",
      Command(ToggleSearchPane),
    );

  let openExtensionsPane =
    define(
      ~category="Extensions",
      ~title="Open Extensions Pane",
      "workbench.view.extensions",
      Command(ToggleExtensionsPane),
    );

  let openExplorerPane =
    define(
      ~category="Explorer",
      ~title="Open File Explorer Pane",
      "workbench.view.explorer",
      Command(ToggleExplorerPane),
    );

  let gotoOutline =
    define(
      ~category="Explorer",
      ~title="Go-to file outline",
      "workbench.action.gotoOutline",
      Command(GotoOutline),
    );

  let openSCMPane =
    define(
      ~category="Source Control",
      ~title="Open Source Control Pane",
      "workbench.view.scm",
      Command(ToggleSCMPane),
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
  open Feature_Input.Schema;

  let findInFiles =
    bind(
      ~key="<S-C-F>",
      ~command=Commands.openSearchPane.id,
      ~condition="!isMac" |> WhenExpr.parse,
    );

  let findInFilesMac =
    bind(
      ~key="<D-S-F>",
      ~command=Commands.openSearchPane.id,
      ~condition="isMac" |> WhenExpr.parse,
    );

  let openExtensions =
    bind(
      ~key="<S-C-X>",
      ~command=Commands.openExtensionsPane.id,
      ~condition="!isMac" |> WhenExpr.parse,
    );

  let openExtensionsMac =
    bind(
      ~key="<D-S-X>",
      ~command=Commands.openExtensionsPane.id,
      ~condition="isMac" |> WhenExpr.parse,
    );

  let openExplorer =
    bind(
      ~key="<S-C-E>",
      ~command=Commands.openExplorerPane.id,
      ~condition="!isMac" |> WhenExpr.parse,
    );

  let openExplorerMac =
    bind(
      ~key="<D-S-E>",
      ~command=Commands.openExplorerPane.id,
      ~condition="isMac" |> WhenExpr.parse,
    );

  // This keybinding is used in both MacOS & Windows
  let openSCM =
    bind(
      ~key="<S-C-G>",
      ~command=Commands.openSCMPane.id,
      ~condition=WhenExpr.Value(True),
    );

  let toggleSidebar =
    bind(
      ~key="<C-S-B>",
      ~command=Commands.toggleSidebar.id,
      ~condition="!isMac" |> WhenExpr.parse,
    );

  let toggleSidebarMac =
    bind(
      ~key="<D-B>",
      ~command=Commands.toggleSidebar.id,
      ~condition="isMac" |> WhenExpr.parse,
    );
};

module ContextKeys = {
  open WhenExpr.ContextKeys.Schema;

  let sideBarVisible = bool("sideBarVisible", isOpen);

  module Focused = {
    let sideBarFocus = bool("sideBarFocus", _ => true);
  };

  let activeViewlet =
    string("activeViewlet", model => {
      switch (model.selected) {
      | FileExplorer => "workbench.view.explorer"
      | Search => "workbench.view.search"
      | SCM => "workbench.view.scm"
      | Extensions => "workbench.view.extensions"
      }
    });
};

module Configuration = {
  open Config.Schema;
  module CustomDecoders: {let location: codec([ | `Left | `Right]);} = {
    let location =
      custom(
        ~decode=
          Json.Decode.(
            string
            |> and_then(
                 fun
                 | "left" => succeed(`Left)
                 | "right" => succeed(`Right)
                 | other => fail("Unknown location: " ++ other),
               )
          ),
        ~encode=
          Json.Encode.(
            location =>
              switch (location) {
              | `Left => string("left")
              | `Right => string("right")
              }
          ),
      );
  };

  /* Onivim2 specific setting */
  let visible = setting("workbench.sideBar.visible", bool, ~default=true);

  let location =
    setting(
      "workbench.sideBar.location",
      CustomDecoders.location,
      ~default=`Left,
    );
};

let configurationChanged = (~hasWorkspace, ~config, model) => {
  let model' = setDefaultLocation(model, Configuration.location.get(config));
  setDefaultVisibility(
    model',
    hasWorkspace && Configuration.visible.get(config),
  );
};

module MenuItems = {
  open ContextMenu.Schema;
  module View = {
    let explorer = command(~title="Explorer", Commands.openExplorerPane);

    let extensions =
      command(~title="Extensions", Commands.openExtensionsPane);

    let scm = command(~title="Source Control", Commands.openSCMPane);

    let toggle = command(~title="Toggle Sidebar", Commands.toggleSidebar);
  };

  module Edit = {
    let findInFiles =
      command(~title="Find in files", Commands.openSearchPane);
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
      gotoOutline,
    ];

  let configuration = Configuration.[visible.spec, location.spec];

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

  let menuItems = commands |> List.map(ContextMenu.Schema.command);

  let menuGroups =
    ContextMenu.Schema.[
      group(
        ~order=200,
        ~parent=Feature_MenuBar.Global.view,
        MenuItems.View.[explorer, scm, extensions],
      ),
      group(
        ~order=900,
        ~parent=Feature_MenuBar.Global.view,
        MenuItems.View.[toggle],
      ),
      group(
        ~order=100,
        ~parent=Feature_MenuBar.Global.edit,
        MenuItems.Edit.[findInFiles],
      ),
    ];
  ContextMenu.Schema.group(~parent=Feature_MenuBar.Global.view, menuItems);

  let contextKeys = (~isFocused) => {
    let common = ContextKeys.[sideBarVisible, activeViewlet];

    let focused = ContextKeys.Focused.[sideBarFocus];

    if (isFocused) {
      common @ focused;
    } else {
      common;
    };
  };
};

// TEST

let%test_module "SideBar" =
  (module
   {
     let%test "#2594: explorer should toggle closed when not visible by default" = {
       let sideBar = {
         ...setDefaultVisibility(initial, false),
         selected: FileExplorer,
         isOpen: true,
       };

       let (sideBar', outmsg) =
         update(~isFocused=true, Command(ToggleExplorerPane), sideBar);

       sideBar'.isOpen == false && outmsg == PopFocus;
     };
   });
