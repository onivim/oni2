open Oni_Core;
open Oni_Model;
open Revery;
open Revery.UI;
open Utility;

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

  let item = [marginVertical(2)];
};

let make = (~state: State.t, ()) => {
  let {theme, uiFont, _}: State.t = state;

  let groups = {
    open Base.List.Let_syntax;

    let%bind provider = state.scm.providers;
    let%bind group = provider.resourceGroups;

    return((provider, group));
  };

  let itemView = (~provider: SCM.Provider.t, ~resource: SCMResource.t, ()) => {
    open Base;

    let base =
      Option.first_some(
        Option.map(provider.rootUri, ~f=Uri.toFileSystemPath),
        Option.map(state.workspace, ~f=w => w.workingDirectory),
      )
      |> Option.value(~default="/");

    let displayName =
      Path.toRelative(~base, Uri.toFileSystemPath(resource.uri));

    <View style=Styles.item>
      <Text style={Styles.text(~font=uiFont, ~theme)} text=displayName />
    </View>;
  };

  let groupView = (~provider, ~group: SCM.ResourceGroup.t, ()) => {
    let label = String.uppercase_ascii(group.label);
    <View style=Styles.group>
      <View style=Styles.groupLabel>
        <Text
          style={Styles.groupLabelText(~font=uiFont, ~theme)}
          text=label
        />
      </View>
      <View style=Styles.groupItems>
        ...{
             group.resources
             |> List.map(resource => <itemView provider resource />)
             |> React.listToElement
           }
      </View>
    </View>;
  };

  <View style=Styles.container>
    ...{
         groups
         |> List.map(((provider, group)) => <groupView provider group />)
         |> React.listToElement
       }
  </View>;
};
