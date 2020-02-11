open Oni_Core;

// MODEL

module ResourceGroup = {
  [@deriving show({with_path: false})]
  type t = {
    handle: int,
    id: string,
    label: string,
    hideWhenEmpty: bool,
    resources: list(SCMResource.t),
  };
};

module Provider = {
  [@deriving show({with_path: false})]
  type t = {
    handle: int,
    id: string,
    label: string,
    rootUri: option(Uri.t),
    resourceGroups: list(ResourceGroup.t),
    hasQuickDiffProvider: bool,
    count: int,
    commitTemplate: string,
  };
};

module DecorationProvider = {
  [@deriving show({with_path: false})]
  type t = {
    handle: int,
    label: string,
  };
};

[@deriving show({with_path: false})]
type t = {
  providers: list(Provider.t),
  decorationProviders: list(DecorationProvider.t),
};

let initial = {providers: [], decorationProviders: []};

[@deriving show({with_path: false})]
type msg =
  | NewProvider({
      handle: int,
      id: string,
      label: string,
      rootUri: option(Uri.t),
    })
  | LostProvider({handle: int})
  | NewResourceGroup({
      provider: int,
      handle: int,
      id: string,
      label: string,
    })
  | LostResourceGroup({
      provider: int,
      handle: int,
    })
  | ResourceStatesChanged({
      provider: int,
      group: int,
      spliceStart: int,
      deleteCount: int,
      additions: list(SCMResource.t),
    })
  | CountChanged({
      handle: int,
      count: int,
    })
  | QuickDiffProviderChanged({
      handle: int,
      available: bool,
    })
  | CommitTemplateChanged({
      handle: int,
      template: string,
    })
  | GotOriginalUri({
      bufferId: int,
      uri: Uri.t,
    })
  | GotOriginalContent({
      bufferId: int,
      lines: [@opaque] array(string),
    })
  | NewDecorationProvider({
      handle: int,
      label: string,
    })
  | LostDecorationProvider({handle: int})
  | DecorationsChanged({
      handle: int,
      uris: list(Uri.t),
    })
  | GotDecorations({
      handle: int,
      uri: Uri.t,
      decorations: list(SCMDecoration.t),
    });

// VIEW

open Revery;
open Revery.UI;
open Revery.UI.Components;
open Utility;

module Pane = {
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
                  ~provider: Provider.t,
                  ~resource: SCMResource.t,
                  ~theme,
                  ~font,
                  ~workingDirectory: option(string),
                  ~onClick,
                  (),
                ) => {
    open Base;
    let%hook (isHovered, setHovered) = Hooks.state(false);
    let onMouseOver = _ => setHovered(_ => true);
    let onMouseOut = _ => setHovered(_ => false);

    let base =
      Option.first_some(
        Option.map(provider.rootUri, ~f=Uri.toFileSystemPath),
        workingDirectory,
      )
      |> Option.value(~default="/");

    let path = Uri.toFileSystemPath(resource.uri);
    let displayName = Path.toRelative(~base, path);

    <View style={Styles.item(~isHovered, ~theme)} onMouseOver onMouseOut>
      <Clickable onClick>
        <Text style={Styles.text(~font, ~theme)} text=displayName />
      </Clickable>
    </View>;
  };

  let groupView =
      (~provider, ~group: ResourceGroup.t, ~theme, ~font, ~workingDirectory, ~onItemClick, ()) => {
    let label = String.uppercase_ascii(group.label);
    <View style=Styles.group>
      <View style=Styles.groupLabel>
        <Text style={Styles.groupLabelText(~font, ~theme)} text=label />
      </View>
      <View style=Styles.groupItems>
        ...{
            group.resources
            |> List.map(resource =>
                  <itemView provider resource theme font workingDirectory onClick={() => onItemClick(resource)} />
                )
            |> React.listToElement
          }
      </View>
    </View>;
  };

  let make = (~model, ~workingDirectory, ~onItemClick, ~theme, ~font, ()) => {
    let groups = {
      open Base.List.Let_syntax;

      let%bind provider = model.providers;
      let%bind group = provider.resourceGroups;

      return((provider, group));
    };

    <View style=Styles.container>
      ...{
          groups
          |> List.map(((provider, group)) =>
                <groupView provider group theme font workingDirectory onItemClick />
              )
          |> React.listToElement
        }
    </View>;
  };
};