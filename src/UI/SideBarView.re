open Revery.UI;

open Oni_Model;
module Core = Oni_Core;

open Feature_SideBar;
open Oni_Components;

module Colors = Feature_Theme.Colors;

module Styles = {
  open Style;

  let sidebar = (~opacity, ~theme) => [
    Style.opacity(opacity),
    flexDirection(`Row),
    backgroundColor(Colors.SideBar.background.from(theme)),
  ];

  let contents = (~width) => [
    Style.width(width),
    flexDirection(`Column),
    flexGrow(1),
  ];

  let resizer = [flexGrow(0), width(4), position(`Relative)];

  let title = (~theme) => [color(Colors.SideBar.foreground.from(theme))];

  let titleContainer = [paddingLeft(23)];

  let heading = theme => [
    flexDirection(`Row),
    alignItems(`Center),
    backgroundColor(Colors.SideBar.background.from(theme)),
    height(Core.Constants.tabHeight),
  ];

  let separator = [
    width(4),
    backgroundColor(Revery.Color.rgba(0., 0., 0., 0.1)),
  ];
};

let separator = () => {
  <View style=Styles.separator />;
};

let animation =
  Revery.UI.Animation.(
    animate(Revery.Time.milliseconds(150))
    |> ease(Easing.easeIn)
    |> tween(-50.0, 0.)
    |> delay(Revery.Time.milliseconds(0))
  );

let make = (~key=?, ~config, ~theme, ~state: State.t, ~dispatch, ()) => {
  let State.{sideBar, uiFont: font, _} = state;

  let title =
    switch (sideBar |> selected) {
    | FileExplorer => "Explorer"
    | SCM => "Source Control"
    | Extensions => "Extensions"
    | Search => "Search"
    };

  let languageInfo =
    state.languageSupport |> Feature_LanguageSupport.languageInfo;
  let maybeBuffer = Selectors.getActiveBuffer(state);
  let maybeSymbols =
    maybeBuffer
    |> Option.map(buffer => Oni_Core.Buffer.getId(buffer))
    |> Utility.OptionEx.flatMap(bufferId =>
         Feature_LanguageSupport.DocumentSymbols.get(
           ~bufferId,
           state.languageSupport,
         )
       );

  let isOpen =
    Feature_SideBar.isOpen(state.sideBar)
    || Focus.isSidebarFocused(FocusManager.current(state));
  let width = Feature_SideBar.width(state.sideBar);

  let elem =
    width > 25 && isOpen
      ? switch (sideBar |> selected) {
        | FileExplorer =>
          let dispatch = msg => dispatch(Actions.FileExplorer(msg));
          <Feature_Explorer.View
            config
            isFocused={FocusManager.current(state) == Focus.FileExplorer}
            languageInfo
            iconTheme={state.iconTheme}
            decorations={state.decorations}
            documentSymbols=maybeSymbols
            model={state.fileExplorer}
            editorFont={state.editorFont}
            theme
            font
            dispatch
          />;

        | SCM =>
          <Feature_SCM.Pane
            model={state.scm}
            workingDirectory={Feature_Workspace.workingDirectory(
              state.workspace,
            )}
            isFocused={FocusManager.current(state) == Focus.SCM}
            languageInfo
            iconTheme={state.iconTheme}
            theme
            font
            dispatch={msg => dispatch(Actions.SCM(msg))}
          />

        | Search =>
          let dispatch = msg =>
            GlobalContext.current().dispatch(Actions.Search(msg));

          <Feature_Search
            config
            isFocused={FocusManager.current(state) == Focus.Search}
            theme
            languageInfo
            iconTheme={state.iconTheme}
            uiFont={state.uiFont}
            model={state.searchPane}
            dispatch
            workingDirectory={Feature_Workspace.workingDirectory(
              state.workspace,
            )}
          />;

        | Extensions =>
          let extensionDispatch = msg => dispatch(Actions.Extensions(msg));
          <Feature_Extensions.ListView
            model={state.extensions}
            proxy={state.proxy |> Feature_Proxy.proxy}
            theme
            font
            isFocused={FocusManager.current(state) == Focus.Extensions}
            dispatch=extensionDispatch
          />;
        }
      : React.empty;

  let separator = isOpen && width > 4 ? <separator /> : React.empty;

  let focus = FocusManager.current(state);
  let isFocused =
    focus == Focus.FileExplorer
    || focus == Focus.SCM
    || focus == Focus.Extensions
    || focus == Focus.Search;

  let widthTakingIntoAccountOpen = isOpen ? width : 0;

  let content =
    <View ?key style={Styles.contents(~width=widthTakingIntoAccountOpen)}>
      <View style={Styles.heading(theme)}>
        <View style=Styles.titleContainer>
          <Text
            text=title
            style={Styles.title(~theme)}
            fontFamily={font.family}
            fontWeight=Revery.Font.Weight.SemiBold
            fontSize=13.
            italic=isFocused
          />
        </View>
      </View>
      elem
    </View>;

  let resizer =
    <View style=Styles.resizer>
      <ResizeHandle.Vertical
        onDrag={delta =>
          dispatch(
            Actions.SideBar(
              Feature_SideBar.ResizeInProgress(int_of_float(delta)),
            ),
          )
        }
        onDragComplete={() =>
          dispatch(Actions.SideBar(Feature_SideBar.ResizeCommitted))
        }
      />
    </View>;

  let defaultOrder = [content, resizer];
  let items =
    switch (Feature_SideBar.location(sideBar)) {
    | Feature_SideBar.Left => defaultOrder
    | Feature_SideBar.Right => List.rev(defaultOrder)
    };

  let opacity =
    isFocused
      ? 1.0
      : Feature_Configuration.GlobalConfiguration.inactiveWindowOpacity.get(
          config,
        );
  <View style={Styles.sidebar(~opacity, ~theme)}>
    separator
    {React.listToElement(items)}
  </View>;
};
