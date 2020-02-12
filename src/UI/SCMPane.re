open Oni_Core;
open Oni_Model;
open Utility;
open Revery;
open Revery.UI;

module Styles = {
  open Style;

  let container = [padding(10), flexGrow(1)];

  let text = (~theme: Theme.t, ~font: UiFont.t) => [
    fontSize(font.fontSize),
    fontFamily(font.fontFile),
    color(theme.sideBarForeground),
    textWrap(TextWrapping.NoWrap),
    textOverflow(`Ellipsis),
  ];

  let group = [];

  let groupLabel = [paddingVertical(3)];

  let groupLabelText = (~theme: Theme.t, ~font: UiFont.t) => [
    fontSize(font.fontSize *. 0.85),
    fontFamily(font.fontFileBold),
    color(theme.sideBarForeground),
    textWrap(TextWrapping.NoWrap),
    textOverflow(`Ellipsis),
  ];

  let groupItems = [marginLeft(6)];

  let item = (~isHovered, ~theme: Theme.t) => [
    isHovered
      ? backgroundColor(theme.listHoverBackground)
      : backgroundColor(theme.sideBarBackground),
    paddingVertical(2),
    cursor(MouseCursors.pointer),
  ];
};

let%component itemView =
              (
                ~provider: SCM.Provider.t,
                ~resource: SCMResource.t,
                ~theme,
                ~font,
                ~workspace: Workspace.t,
                (),
              ) => {
  open Base;
  let%hook (isHovered, setHovered) = Hooks.state(false);
  let onMouseOver = _ => setHovered(_ => true);
  let onMouseOut = _ => setHovered(_ => false);

  let base =
    Option.first_some(
      Option.map(provider.rootUri, ~f=Uri.toFileSystemPath),
      Option.map(workspace, ~f=w => w.workingDirectory),
    )
    |> Option.value(~default="/");

  let path = Uri.toFileSystemPath(resource.uri);
  let displayName = Path.toRelative(~base, path);

  let onClick = () =>
    GlobalContext.current().dispatch(
      Actions.OpenFileByPath(path, None, None),
    );

  <View style={Styles.item(~isHovered, ~theme)} onMouseOver onMouseOut>
    <Sneakable onClick>
      <Text style={Styles.text(~font, ~theme)} text=displayName />
    </Sneakable>
  </View>;
};

let groupView =
    (~provider, ~group: SCM.ResourceGroup.t, ~theme, ~font, ~workspace, ()) => {
  let label = String.uppercase_ascii(group.label);
  <View style=Styles.group>
    <View style=Styles.groupLabel>
      <Text style={Styles.groupLabelText(~font, ~theme)} text=label />
    </View>
    <View style=Styles.groupItems>
      ...{
           group.resources
           |> List.map(resource =>
                <itemView provider resource theme font workspace />
              )
           |> React.listToElement
         }
    </View>
  </View>;
};

let make = (~state: State.t, ()) => {
  [@warning "-27"]
  let {theme, uiFont as font, workspace, _}: State.t = state;

  let groups = {
    open Base.List.Let_syntax;

    let%bind provider = state.scm.providers;
    let%bind group = provider.resourceGroups;

    return((provider, group));
  };

  <View style=Styles.container>
    ...{
         groups
         |> List.map(((provider, group)) =>
              <groupView provider group theme font workspace />
            )
         |> React.listToElement
       }
  </View>;
};
