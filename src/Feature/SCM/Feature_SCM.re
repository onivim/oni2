open Oni_Core;
open Utility;

module InputModel = Oni_Components.InputModel;

// MODEL

module Resource = Oni_Extensions.SCM.Resource;
module ResourceGroup = Oni_Extensions.SCM.ResourceGroup;
module Provider = Oni_Extensions.SCM.Provider;

[@deriving show({with_path: false})]
type model = {
  providers: list(Provider.t),
  inputBox,
}

and inputBox = {
  value: string,
  cursorPosition: int,
  placeholder: string,
};

let initial = {
  providers: [],
  inputBox: {
    value: "",
    cursorPosition: 0,
    placeholder: "Do the commit thing!",
  },
};

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
    })
  | AcceptInputCommandChanged({
      handle: int,
      id: string,
    })
  | KeyPressed({key: string})
  | InputBoxClicked({cursorPosition: int});

module Msg = {
  let keyPressed = key => KeyPressed({key: key});
};

type outmsg =
  | Focus;

let update = (model, msg) =>
  switch (msg) {
  | NewProvider({handle, id, label, rootUri}) => (
      {
        ...model,
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
            acceptInputCommand: None,
          },
          ...model.providers,
        ],
      },
      None,
    )

  | LostProvider({handle}) => (
      {
        ...model,
        providers:
          List.filter(
            (it: Provider.t) => it.handle != handle,
            model.providers,
          ),
      },
      None,
    )

  | QuickDiffProviderChanged({handle, available}) => (
      {
        ...model,
        providers:
          List.map(
            (it: Provider.t) =>
              it.handle == handle
                ? {...it, hasQuickDiffProvider: available} : it,
            model.providers,
          ),
      },
      None,
    )

  | CountChanged({handle, count}) => (
      {
        ...model,
        providers:
          List.map(
            (it: Provider.t) => it.handle == handle ? {...it, count} : it,
            model.providers,
          ),
      },
      None,
    )

  | CommitTemplateChanged({handle, template}) => (
      {
        ...model,
        providers:
          List.map(
            (it: Provider.t) =>
              it.handle == handle ? {...it, commitTemplate: template} : it,
            model.providers,
          ),
      },
      None,
    )

  | AcceptInputCommandChanged({handle, id}) => (
      {
        ...model,
        providers:
          List.map(
            (it: Provider.t) =>
              it.handle == handle
                ? {...it, acceptInputCommand: Some(id)} : it,
            model.providers,
          ),
      },
      None,
    )

  | NewResourceGroup({provider, handle, id, label}) => (
      {
        ...model,
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
      None,
    )

  | LostResourceGroup({provider, handle}) => (
      {
        ...model,
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
      None,
    )

  | ResourceStatesChanged({
      provider,
      group,
      spliceStart,
      deleteCount,
      additions,
    }) => (
      {
        ...model,
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
      None,
    )

  | KeyPressed({key}) =>
    let (value, cursorPosition) =
      InputModel.handleInput(
        ~text=model.inputBox.value,
        ~cursorPosition=model.inputBox.cursorPosition,
        key,
      );

    (
      {
        ...model,
        inputBox: {
          ...model.inputBox,
          value,
          cursorPosition,
        },
      },
      None,
    );

  | InputBoxClicked({cursorPosition}) => (
      {
        ...model,
        inputBox: {
          ...model.inputBox,
          cursorPosition,
        },
      },
      Some(Focus),
    )
  };

let handleExtensionMessage = (~dispatch, msg: Oni_Extensions.SCM.msg) =>
  switch (msg) {
  | RegisterSourceControl({handle, id, label, rootUri}) =>
    dispatch(NewProvider({handle, id, label, rootUri}))

  | UnregisterSourceControl({handle}) =>
    dispatch(LostProvider({handle: handle}))

  | RegisterSCMResourceGroup({provider, handle, id, label}) =>
    dispatch(NewResourceGroup({provider, handle, id, label}))

  | UnregisterSCMResourceGroup({provider, handle}) =>
    dispatch(LostResourceGroup({provider, handle}))

  | SpliceSCMResourceStates({provider, group, start, deleteCount, additions}) =>
    dispatch(
      ResourceStatesChanged({
        provider,
        group,
        spliceStart: start,
        deleteCount,
        additions,
      }),
    )

  | UpdateSourceControl({
      handle,
      hasQuickDiffProvider,
      count,
      commitTemplate,
      acceptInputCommand,
    }) =>
    Option.iter(
      available => dispatch(QuickDiffProviderChanged({handle, available})),
      hasQuickDiffProvider,
    );
    Option.iter(count => dispatch(CountChanged({handle, count})), count);
    Option.iter(
      template => dispatch(CommitTemplateChanged({handle, template})),
      commitTemplate,
    );
    Option.iter(
      id => dispatch(AcceptInputCommandChanged({handle, id})),
      acceptInputCommand,
    );
  };

// VIEW

open Revery;
open Revery.UI;
open Revery.UI.Components;

module Input = Oni_Components.Input;

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

    let input = (~font: UiFont.t) => [
      border(~width=2, ~color=Color.rgba(0., 0., 0., 0.1)),
      backgroundColor(Color.rgba(0., 0., 0., 0.3)),
      color(Colors.white),
      fontFamily(font.fontFile),
      fontSize(font.fontSize),
      flexGrow(1),
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

  let make =
      (
        ~model,
        ~workingDirectory,
        ~onItemClick,
        ~isFocused,
        ~theme,
        ~font,
        ~dispatch,
        (),
      ) => {
    let groups = {
      open Base.List.Let_syntax;

      let%bind provider = model.providers;
      let%bind group = provider.resourceGroups;

      return((provider, group));
    };

    <View style=Styles.container>
      <Input
        style={Styles.input(~font)}
        cursorColor=Colors.gray
        value={model.inputBox.value}
        cursorPosition={model.inputBox.cursorPosition}
        placeholder={model.inputBox.placeholder}
        isFocused
        onClick={pos => dispatch(InputBoxClicked({cursorPosition: pos}))}
      />
      {groups
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
       |> React.listToElement}
    </View>;
  };
};
