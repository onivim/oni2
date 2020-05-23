open Oni_Core;
open Utility;

module InputModel = Oni_Components.InputModel;
module Selection = Oni_Components.Selection;

// MODEL

module Resource = Exthost.SCM.Resource;

module ResourceGroup = {
  [@deriving show({with_path: false})]
  type t = {
    handle: int,
    id: string,
    label: string,
    hideWhenEmpty: bool,
    resources: list(Resource.t),
  };
};

[@deriving show]
type command = Exthost.SCM.command;

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
    acceptInputCommand: option(command),
  };
};

[@deriving show({with_path: false})]
type model = {
  providers: list(Provider.t),
  inputBox,
}

and inputBox = {
  value: string,
  selection: Selection.t,
  placeholder: string,
};

let initial = {
  providers: [],
  inputBox: {
    value: "",
    selection: Selection.initial,
    placeholder: "Do the commit thing!",
  },
};

// EFFECTS

module Effects = {
  let getOriginalUri = (extHostClient, model, path, toMsg) => {
    let handles =
      model.providers |> List.map((provider: Provider.t) => provider.handle);
    Service_Exthost.Effects.SCM.provideOriginalResource(
      ~handles,
      extHostClient,
      path,
      toMsg,
    );
  };
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
      command: Exthost.SCM.command,
    })
  | KeyPressed({key: string})
  | InputBoxClicked({selection: Selection.t});

module Msg = {
  let keyPressed = key => KeyPressed({key: key});
};

type outmsg =
  | Effect(Isolinear.Effect.t(msg))
  | Focus
  | Nothing;

let update = (extHostClient: Exthost.Client.t, model, msg) =>
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
      Nothing,
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
      Nothing,
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
      Nothing,
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
      Nothing,
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
      Nothing,
    )

  | AcceptInputCommandChanged({handle, command}) => (
      {
        ...model,
        providers:
          List.map(
            (it: Provider.t) =>
              it.handle == handle
                ? {...it, acceptInputCommand: Some(command)} : it,
            model.providers,
          ),
      },
      Nothing,
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
      Nothing,
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
      Nothing,
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
      Nothing,
    )

  | KeyPressed({key: "<CR>"}) => (
      model,
      Effect(
        Isolinear.Effect.batch(
          model.providers
          |> List.map((provider: Provider.t) =>
               switch (provider.acceptInputCommand) {
               | Some(command) =>
                 Isolinear.Effect.create(~name="acceptInputCommand", () => {
                   Exthost.Request.Commands.executeContributedCommand(
                     ~command=command.id,
                     ~arguments=command.arguments,
                     extHostClient,
                   )
                 })
               | None => Isolinear.Effect.none
               }
             ),
        ),
      ),
    )

  | KeyPressed({key}) =>
    let (value, selection) =
      InputModel.handleInput(
        ~text=model.inputBox.value,
        ~selection=model.inputBox.selection,
        key,
      );

    (
      {
        ...model,
        inputBox: {
          ...model.inputBox,
          value,
          selection,
        },
      },
      Effect(
        Isolinear.Effect.batch(
          model.providers
          |> List.map((provider: Provider.t) =>
               Service_Exthost.Effects.SCM.onInputBoxValueChange(
                 ~handle=provider.handle,
                 ~value,
                 extHostClient,
               )
             ),
        ),
      ),
    );

  | InputBoxClicked({selection}) => (
      {
        ...model,
        inputBox: {
          ...model.inputBox,
          selection,
        },
      },
      Focus,
    )
  };

let handleExtensionMessage = (~dispatch, msg: Exthost.Msg.SCM.msg) =>
  switch (msg) {
  | RegisterSourceControl({handle, id, label, rootUri}) =>
    dispatch(NewProvider({handle, id, label, rootUri}))

  | UnregisterSourceControl({handle}) =>
    dispatch(LostProvider({handle: handle}))

  | RegisterSCMResourceGroup({provider, handle, id, label}) =>
    dispatch(NewResourceGroup({provider, handle, id, label}))

  | UnregisterSCMResourceGroup({provider, handle}) =>
    dispatch(LostResourceGroup({provider, handle}))

  | SpliceSCMResourceStates({handle, splices}) =>
    open Exthost.SCM;

    let provider = handle;
    splices
    |> List.iter(({handle as group, resourceSplices}: Resource.Splices.t) => {
         ignore(handle);

         resourceSplices
         |> List.iter(({start, deleteCount, resources}: Resource.Splice.t) => {
              dispatch(
                ResourceStatesChanged({
                  provider,
                  group,
                  spliceStart: start,
                  deleteCount,
                  additions: resources,
                }),
              )
            });
       });
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
      command => dispatch(AcceptInputCommandChanged({handle, command})),
      acceptInputCommand,
    );
  };

// VIEW

open Revery;
open Revery.UI;
open Revery.UI.Components;

module Input = Oni_Components.Input;

module Colors = Feature_Theme.Colors;

module Pane = {
  module Styles = {
    open Style;

    let container = [padding(10), flexGrow(1)];

    let text = (~theme, ~font: UiFont.t) => [
      fontSize(font.fontSize),
      fontFamily(font.fontFile),
      color(Colors.SideBar.foreground.from(theme)),
      textWrap(TextWrapping.NoWrap),
      textOverflow(`Ellipsis),
    ];

    let input = (~font: UiFont.t) => [
      fontFamily(font.fontFile),
      fontSize(font.fontSize),
      flexGrow(1),
    ];

    let group = [];

    let groupLabel = [paddingVertical(3)];

    let groupLabelText = (~theme, ~font: UiFont.t) => [
      fontSize(font.fontSize *. 0.85),
      fontFamily(font.fontFileBold),
      color(Colors.SideBar.foreground.from(theme)),
      textWrap(TextWrapping.NoWrap),
      textOverflow(`Ellipsis),
    ];

    let groupItems = [marginLeft(6)];

    let item = (~isHovered, ~theme) => [
      isHovered
        ? backgroundColor(Colors.List.hoverBackground.from(theme))
        : backgroundColor(Colors.SideBar.background.from(theme)),
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
                  ~workingDirectory,
                  ~onClick,
                  (),
                ) => {
    open Base;
    let%hook (isHovered, setHovered) = Hooks.state(false);
    let onMouseOver = _ => setHovered(_ => true);
    let onMouseOut = _ => setHovered(_ => false);

    let base =
      provider.rootUri
      |> Option.map(~f=Uri.toFileSystemPath)
      |> Option.value(~default=workingDirectory);

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

    <ScrollView style=Styles.container>
      <Input
        style={Styles.input(~font)}
        value={model.inputBox.value}
        selection={model.inputBox.selection}
        placeholder={model.inputBox.placeholder}
        isFocused
        onClick={selection =>
          dispatch(InputBoxClicked({selection: selection}))
        }
        theme
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
    </ScrollView>;
  };
};
