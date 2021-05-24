open Oni_Core;
open Feature_Pane.Schema;

type model = {
  tree: Component_VimTree.model(string, Oni_Components.LocationListItem.t),
};

let initial = {tree: Component_VimTree.create(~rowHeight=20)};

[@deriving show]
type msg =
  | DiagnosticsTree(Component_VimTree.msg)
  | KeyPress(string);

type outmsg =
  Component_VimTree.outmsg(string, Oni_Components.LocationListItem.t);

let update = (msg, model) =>
  switch (msg) {
  | DiagnosticsTree(treeMsg) =>
    let (tree', outmsg) = Component_VimTree.update(treeMsg, model.tree);
    ({tree: tree'}, outmsg);
  | KeyPress(key) => (
      {tree: Component_VimTree.keyPress(key, model.tree)},
      Nothing,
    )
  };

let setDiagnostics = (locations, model) => {
  let diagLocList = locations |> Oni_Components.LocationListItem.toTrees;

  let tree' =
    Component_VimTree.set(
      ~uniqueId=path => path,
      ~searchText=
        Component_VimTree.(
          fun
          | Node({data, _}) => data
          | Leaf({data, _}) => Oni_Components.LocationListItem.(data.text)
        ),
      diagLocList,
      model.tree,
    );
  {tree: tree'};
};

let contextKeys = (~isFocused, model) =>
  if (isFocused) {
    Component_VimTree.Contributions.contextKeys(model.tree);
  } else {
    WhenExpr.ContextKeys.empty;
  };

module PaneView = {
  open Revery.UI;
  open Oni_Components;

  module Colors = Feature_Theme.Colors;

  module Styles = {
    open Style;

    let pane = [flexGrow(1), flexDirection(`Row)];

    let noResultsContainer = [
      flexGrow(1),
      alignItems(`Center),
      justifyContent(`Center),
    ];

    let title = (~theme) => [
      color(Colors.PanelTitle.activeForeground.from(theme)),
      margin(8),
    ];
  };

  let make =
      (
        ~config,
        ~isFocused: bool,
        ~diagnosticsList: Component_VimTree.model(string, LocationListItem.t),
        ~theme,
        ~iconTheme,
        ~languageInfo,
        ~uiFont: UiFont.t,
        ~workingDirectory,
        ~dispatch,
        (),
      ) => {
    let innerElement =
      if (Component_VimTree.count(diagnosticsList) == 0) {
        <View style=Styles.noResultsContainer>
          <Text
            style={Styles.title(~theme)}
            fontFamily={uiFont.family}
            fontSize={uiFont.size}
            text="No problems, yet!"
          />
        </View>;
      } else {
        <Component_VimTree.View
          config
          font=uiFont
          isActive=isFocused
          focusedIndex=None
          theme
          model=diagnosticsList
          dispatch
          render={(
            ~availableWidth,
            ~index as _,
            ~hovered as _,
            ~selected as _,
            item,
          ) =>
            switch (item) {
            | Component_VimTree.Node({data, _}) =>
              <FileItemView.View
                theme
                uiFont
                iconTheme
                languageInfo
                item=data
                workingDirectory
              />
            | Component_VimTree.Leaf({data, _}) =>
              <LocationListItem.View
                showPosition=true
                width=availableWidth
                theme
                uiFont
                item=data
              />
            }
          }
        />;
      };

    <View style=Styles.pane> innerElement </View>;
  };
};

let pane: Feature_Pane.Schema.t(model, msg) =
  panel(
    ~title="Problems",
    ~id=Some("workbench.panel.markers"),
    ~buttons=
      (~font as _, ~theme as _, ~dispatch as _, ~model as _) =>
        Revery.UI.React.empty,
    ~commands=
      _pane => {
        Component_VimTree.Contributions.commands
        |> List.map(Oni_Core.Command.map(msg => DiagnosticsTree(msg)))
      },
    ~contextKeys,
    ~sub=(~isFocused as _, _model) => Isolinear.Sub.none,
    ~view=
      (
        ~config,
        ~editorFont as _,
        ~font,
        ~isFocused,
        ~iconTheme,
        ~languageInfo,
        ~workingDirectory,
        ~theme,
        ~dispatch,
        ~model,
      ) => {
        <PaneView
          uiFont=font
          config
          isFocused
          diagnosticsList={model.tree}
          theme
          iconTheme
          languageInfo
          workingDirectory
          dispatch={msg => dispatch(DiagnosticsTree(msg))}
        />
      },
    ~keyPressed=key => KeyPress(key),
  );
