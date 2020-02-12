open Oni_Core;
open SCMModels;
open Utility;

// MODEL

[@deriving show({with_path: false})]
type t = {providers: list(Provider.t)};

let initial = {providers: []};

// EFFECTS

module Effects = {
  let getOriginalUri = (extHostClient, model, path, toMsg) =>
    Isolinear.Effect.createWithDispatch(~name="scm.getOriginalUri", dispatch => {
      // Try our luck with every provider. If several returns Last-Writer-Wins
      // TODO: Is there a better heuristic? Perhaps use rootUri to choose the "nearest" provider?
      model.providers
      |> List.iter((provider: Provider.t) => {
           let promise =
             Oni_Extensions.SCM.provideOriginalResource(
               provider.handle,
               Uri.fromPath(path),
               extHostClient,
             );

           Lwt.on_success(promise, uri => dispatch(toMsg(uri)));
         })
    });
};

// UPDATE

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
      additions: list(Resource.t),
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
    });

let update = (action, model) =>
  switch (action) {
  | NewProvider({handle, id, label, rootUri}) => (
      {
        providers: [
          Provider.{
            handle,
            id,
            label,
            rootUri,
            resourceGroups: [],
            hasQuickDiffProvider: false,
            count: 0,
            commitTemplate: "",
          },
          ...model.providers,
        ],
      },
      Isolinear.Effect.none,
    )

  | LostProvider({handle}) => (
      {
        providers:
          List.filter(
            (it: Provider.t) => it.handle != handle,
            model.providers,
          ),
      },
      Isolinear.Effect.none,
    )

  | QuickDiffProviderChanged({handle, available}) => (
      {
        providers:
          List.map(
            (it: Provider.t) =>
              it.handle == handle
                ? {...it, hasQuickDiffProvider: available} : it,
            model.providers,
          ),
      },
      Isolinear.Effect.none,
    )

  | CountChanged({handle, count}) => (
      {
        providers:
          List.map(
            (it: Provider.t) => it.handle == handle ? {...it, count} : it,
            model.providers,
          ),
      },
      Isolinear.Effect.none,
    )

  | CommitTemplateChanged({handle, template}) => (
      {
        providers:
          List.map(
            (it: Provider.t) =>
              it.handle == handle ? {...it, commitTemplate: template} : it,
            model.providers,
          ),
      },
      Isolinear.Effect.none,
    )

  | NewResourceGroup({provider, handle, id, label}) => (
      {
        providers:
          List.map(
            (p: Provider.t) =>
              p.handle == provider
                ? {
                  ...p,
                  resourceGroups: [
                    ResourceGroup.{
                      handle,
                      id,
                      label,
                      hideWhenEmpty: false,
                      resources: [],
                    },
                    ...p.resourceGroups,
                  ],
                }
                : p,
            model.providers,
          ),
      },
      Isolinear.Effect.none,
    )

  | LostResourceGroup({provider, handle}) => (
      {
        providers:
          List.map(
            (p: Provider.t) =>
              p.handle == provider
                ? {
                  ...p,
                  resourceGroups:
                    List.filter(
                      (g: ResourceGroup.t) => g.handle != handle,
                      p.resourceGroups,
                    ),
                }
                : p,
            model.providers,
          ),
      },
      Isolinear.Effect.none,
    )

  | ResourceStatesChanged({
      provider,
      group,
      spliceStart,
      deleteCount,
      additions,
    }) => (
      {
        providers:
          List.map(
            (p: Provider.t) =>
              p.handle == provider
                ? {
                  ...p,
                  resourceGroups:
                    List.map(
                      (g: ResourceGroup.t) =>
                        g.handle == group
                          ? {
                            ...g,
                            resources:
                              ListEx.splice(
                                ~start=spliceStart,
                                ~deleteCount,
                                ~additions,
                                g.resources,
                              ),
                          }
                          : g,
                      p.resourceGroups,
                    ),
                }
                : p,
            model.providers,
          ),
      },
      Isolinear.Effect.none,
    )
  };

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
                  ~resource: Resource.t,
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
      (
        ~provider,
        ~group: ResourceGroup.t,
        ~theme,
        ~font,
        ~workingDirectory,
        ~onItemClick,
        (),
      ) => {
    let label = String.uppercase_ascii(group.label);
    <View style=Styles.group>
      <View style=Styles.groupLabel>
        <Text style={Styles.groupLabelText(~font, ~theme)} text=label />
      </View>
      <View style=Styles.groupItems>
        ...{
             group.resources
             |> List.map(resource =>
                  <itemView
                    provider
                    resource
                    theme
                    font
                    workingDirectory
                    onClick={() => onItemClick(resource)}
                  />
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
                <groupView
                  provider
                  group
                  theme
                  font
                  workingDirectory
                  onItemClick
                />
              )
           |> React.listToElement
         }
    </View>;
  };
};
