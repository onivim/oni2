open Oni_Core;
open Utility;
open Oni_Components;

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
    inputVisible: bool,
    validationEnabled: bool,
    statusBarCommands: list(Exthost.Command.t),
  };

  let initial = (~handle, ~id, ~label, ~rootUri) => {
    handle,
    id,
    label,
    rootUri,
    resourceGroups: [],
    hasQuickDiffProvider: false,
    count: 0,
    commitTemplate: "",
    acceptInputCommand: None,
    inputVisible: true,
    validationEnabled: false,
    statusBarCommands: [],
  };
};

[@deriving show({with_path: false})]
type model = {
  providers: list(Provider.t),
  inputBox: Feature_InputText.model,
  textContentProviders: list((int, string)),
  originalLines: [@opaque] IntMap.t(array(string)),
};

let initial = {
  providers: [],
  inputBox: Feature_InputText.create(~placeholder="Do the commit thing!"),
  textContentProviders: [],
  originalLines: IntMap.empty,
};

let statusBarCommands = ({providers, _}: model) => {
  providers
  |> List.map(({statusBarCommands, _}: Provider.t) => statusBarCommands)
  |> List.flatten;
};

let getOriginalLines = (buffer, model) => {
  let bufferId = buffer |> Oni_Core.Buffer.getId;

  IntMap.find_opt(bufferId, model.originalLines);
};

let setOriginalLines = (buffer, lines, model) => {
  let bufferId = buffer |> Oni_Core.Buffer.getId;
  let originalLines = IntMap.add(bufferId, lines, model.originalLines);
  {...model, originalLines};
};

// UPDATE

[@deriving show({with_path: false})]
type msg =
  | GotOriginalUri({
      bufferId: int,
      uri: Uri.t,
    })
  | GotOriginalContent({
      bufferId: int,
      lines: array(string),
    })
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
  | GroupHideWhenEmptyChanged({
      provider: int,
      handle: int,
      hideWhenEmpty: bool,
    })
  | GroupLabelChanged({
      provider: int,
      handle: int,
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
  | StatusBarCommandsChanged({
      handle: int,
      statusBarCommands: list(Exthost.Command.t),
    })
  | CommitTemplateChanged({
      handle: int,
      template: string,
    })
  | AcceptInputCommandChanged({
      handle: int,
      command: Exthost.SCM.command,
    })
  //  | InputBoxPlaceholderChanged({
  //      handle: int,
  //      placeholder: string,
  //    })
  | InputBoxVisibilityChanged({
      handle: int,
      visible: bool,
    })
  | ValidationProviderEnabledChanged({
      handle: int,
      validationEnabled: bool,
    })
  | KeyPressed({key: string})
  | Pasted({text: string})
  | DocumentContentProvider(Exthost.Msg.DocumentContentProvider.msg)
  | InputBox(Feature_InputText.msg);

module Msg = {
  let paste = text => Pasted({text: text});
  let keyPressed = key => KeyPressed({key: key});

  let documentContentProvider = msg => DocumentContentProvider(msg);
};

type outmsg =
  | Effect(Isolinear.Effect.t(msg))
  | Focus
  | Nothing;

module Effects = {
  let getOriginalContent = (bufferId, uri, providers, client) => {
    let scheme = uri |> Uri.getScheme |> Uri.Scheme.toString;
    providers
    |> List.find_opt(((_, providerScheme)) => providerScheme == scheme)
    |> Option.map(provider => {
         let (handle, _) = provider;
         Service_Exthost.Effects.SCM.getOriginalContent(
           ~handle,
           ~uri,
           ~toMsg=lines => GotOriginalContent({bufferId, lines}),
           client,
         );
       })
    |> Option.value(~default=Isolinear.Effect.none);
  };
};

module Internal = {
  let updateProvider = (~handle, f, model) => {
    {
      ...model,
      providers:
        List.map(
          (it: Provider.t) => it.handle == handle ? f(it) : it,
          model.providers,
        ),
    };
  };

  let updateResourceGroup = (~provider, ~group, f, model) => {
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
                    (g: ResourceGroup.t) => g.handle == group ? f(g) : g,
                    p.resourceGroups,
                  ),
              }
              : p,
          model.providers,
        ),
    };
  };
};

let update = (extHostClient: Exthost.Client.t, model, msg) =>
  switch (msg) {
  | DocumentContentProvider(documentContentProviderMsg) =>
    Exthost.Msg.DocumentContentProvider.(
      {
        switch (documentContentProviderMsg) {
        | RegisterTextContentProvider({handle, scheme}) => (
            {
              ...model,
              textContentProviders: [
                (handle, scheme),
                ...model.textContentProviders,
              ],
            },
            Nothing,
          )

        | UnregisterTextContentProvider({handle}) => (
            {
              ...model,
              textContentProviders:
                List.filter(
                  ((h, _)) => h != handle,
                  model.textContentProviders,
                ),
            },
            Nothing,
          )
        | VirtualDocumentChange(_) => (model, Nothing)
        };
      }
    )

  | GotOriginalUri({bufferId, uri}) => (
      model,
      Effect(
        Effects.getOriginalContent(
          bufferId,
          uri,
          model.textContentProviders,
          extHostClient,
        ),
      ),
    )

  | GotOriginalContent({bufferId, lines}) => (
      {
        ...model,
        originalLines: IntMap.add(bufferId, lines, model.originalLines),
      },
      Nothing,
    )

  | NewProvider({handle, id, label, rootUri}) => (
      {
        ...model,
        providers: [
          Provider.initial(~handle, ~id, ~label, ~rootUri),
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
      model
      |> Internal.updateProvider(~handle, it =>
           {...it, hasQuickDiffProvider: available}
         ),
      Nothing,
    )

  | StatusBarCommandsChanged({handle, statusBarCommands}) => (
      model
      |> Internal.updateProvider(~handle, p => {...p, statusBarCommands}),
      Nothing,
    )

  // TODO: Handle replacing '{0}' character in placeholder text
  //  | InputBoxPlaceholderChanged({handle, placeholder}) => (
  //      {
  //        ...model,
  //        inputBox:
  //          Feature_InputText.setPlaceholder(~placeholder, model.inputBox),
  //      },
  //      Nothing,
  //    )

  | InputBoxVisibilityChanged({handle, visible}) => (
      model
      |> Internal.updateProvider(~handle, it =>
           {...it, inputVisible: visible}
         ),
      Nothing,
    )

  | ValidationProviderEnabledChanged({handle, validationEnabled}) => (
      model
      |> Internal.updateProvider(~handle, it => {...it, validationEnabled}),
      Nothing,
    )

  | CountChanged({handle, count}) => (
      model |> Internal.updateProvider(~handle, it => {...it, count}),
      Nothing,
    )

  | CommitTemplateChanged({handle, template}) => (
      model
      |> Internal.updateProvider(~handle, it =>
           {...it, commitTemplate: template}
         ),
      Nothing,
    )

  | AcceptInputCommandChanged({handle, command}) => (
      model
      |> Internal.updateProvider(~handle, it =>
           {...it, acceptInputCommand: Some(command)}
         ),
      Nothing,
    )

  | NewResourceGroup({provider, handle, id, label}) => (
      model
      |> Internal.updateProvider(~handle=provider, p =>
           {
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
         ),
      Nothing,
    )

  | LostResourceGroup({provider, handle}) => (
      model
      |> Internal.updateProvider(~handle=provider, p =>
           {
             ...p,
             resourceGroups:
               List.filter(
                 (g: ResourceGroup.t) => g.handle != handle,
                 p.resourceGroups,
               ),
           }
         ),
      Nothing,
    )

  | GroupHideWhenEmptyChanged({provider, handle, hideWhenEmpty}) => (
      model
      |> Internal.updateResourceGroup(~provider, ~group=handle, group =>
           {...group, hideWhenEmpty}
         ),
      Nothing,
    )

  | GroupLabelChanged({provider, handle, label}) => (
      model
      |> Internal.updateResourceGroup(~provider, ~group=handle, group =>
           {...group, label}
         ),
      Nothing,
    )

  | ResourceStatesChanged({
      provider,
      group,
      spliceStart,
      deleteCount,
      additions,
    }) => (
      model
      |> Internal.updateResourceGroup(~provider, ~group, g =>
           {
             ...g,
             resources:
               ListEx.splice(
                 ~start=spliceStart,
                 ~deleteCount,
                 ~additions,
                 g.resources,
               ),
           }
         ),
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
    let inputBox = Feature_InputText.handleInput(~key, model.inputBox);
    (
      {...model, inputBox},
      Effect(
        Isolinear.Effect.batch(
          model.providers
          |> List.map((provider: Provider.t) =>
               Service_Exthost.Effects.SCM.onInputBoxValueChange(
                 ~handle=provider.handle,
                 ~value=inputBox |> Feature_InputText.value,
                 extHostClient,
               )
             ),
        ),
      ),
    );

  | Pasted({text}) =>
    let inputBox = Feature_InputText.paste(~text, model.inputBox);
    (
      {...model, inputBox},
      Effect(
        Isolinear.Effect.batch(
          model.providers
          |> List.map((provider: Provider.t) =>
               Service_Exthost.Effects.SCM.onInputBoxValueChange(
                 ~handle=provider.handle,
                 ~value=inputBox |> Feature_InputText.value,
                 extHostClient,
               )
             ),
        ),
      ),
    );

  | InputBox(msg) => (
      {...model, inputBox: Feature_InputText.update(msg, model.inputBox)},
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

  | UpdateGroup({provider, handle, features}) =>
    Exthost.SCM.GroupFeatures.(
      dispatch(
        GroupHideWhenEmptyChanged({
          provider,
          handle,
          hideWhenEmpty: features.hideWhenEmpty,
        }),
      )
    )

  | UpdateGroupLabel({provider, handle, label}) =>
    dispatch(GroupLabelChanged({provider, handle, label}))

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
  | UpdateSourceControl({handle, features}) =>
    let {
      hasQuickDiffProvider,
      count,
      commitTemplate,
      acceptInputCommand,
      statusBarCommands,
    }: Exthost.SCM.ProviderFeatures.t = features;
    dispatch(
      QuickDiffProviderChanged({handle, available: hasQuickDiffProvider}),
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

    dispatch(StatusBarCommandsChanged({handle, statusBarCommands}));

  | SetInputBoxPlaceholder(_) =>
    // TODO: Set up replacement for '{0}'
    //dispatch(InputBoxPlaceholderChanged({handle, placeholder: value}))
    ()
  | SetInputBoxVisibility({handle, visible}) =>
    dispatch(InputBoxVisibilityChanged({handle, visible}))
  | SetValidationProviderIsEnabled({handle, enabled}) =>
    dispatch(
      ValidationProviderEnabledChanged({handle, validationEnabled: enabled}),
    )
  };

// SUBSCRIPTION

let sub = (~activeBuffer, ~client, model) => {
  let filePath =
    activeBuffer |> Oni_Core.Buffer.getUri |> Oni_Core.Uri.toFileSystemPath;

  let bufferId = activeBuffer |> Oni_Core.Buffer.getId;

  model.providers
  |> List.map((provider: Provider.t) =>
       Service_Exthost.Sub.SCM.originalUri(
         ~handle=provider.handle,
         ~filePath,
         ~toMsg=uri => GotOriginalUri({bufferId, uri}),
         client,
       )
     )
  |> Isolinear.Sub.batch;
};

// VIEW

open Revery;
open Revery.UI;
open Revery.UI.Components;

module Colors = Feature_Theme.Colors;

module Pane = {
  module Styles = {
    open Style;

    let container = [flexGrow(1)];

    let text = (~theme) => [
      color(Colors.SideBar.foreground.from(theme)),
      textWrap(TextWrapping.NoWrap),
      textOverflow(`Ellipsis),
    ];

    let input = [flexGrow(1), margin(12)];

    let item = (~isHovered, ~theme) => [
      isHovered
        ? backgroundColor(Colors.List.hoverBackground.from(theme))
        : backgroundColor(Colors.SideBar.background.from(theme)),
      paddingVertical(2),
      marginLeft(6),
      cursor(MouseCursors.pointer),
    ];
  };

  let%component itemView =
                (
                  ~provider: Provider.t,
                  ~resource: Resource.t,
                  ~theme,
                  ~font: UiFont.t,
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
        <Text
          style={Styles.text(~theme)}
          text=displayName
          fontFamily={font.family}
          fontSize={font.size}
        />
      </Clickable>
    </View>;
  };

  let groupView =
      (
        ~provider,
        ~group: ResourceGroup.t,
        ~theme,
        ~font: UiFont.t,
        ~workingDirectory,
        ~onItemClick,
        ~onTitleClick,
        ~expanded,
        (),
      ) => {
    let label = group.label;
    let items = Array.of_list(group.resources);
    let renderItem = (items, idx) => {
      let resource = items[idx];
      <itemView
        provider
        resource
        theme
        font
        workingDirectory
        onClick={() => onItemClick(resource)}
      />;
    };
    <Accordion
      title=label
      expanded
      uiFont=font
      rowHeight=20
      count={Array.length(items)}
      renderItem={renderItem(items)}
      focused=None
      theme
      onClick=onTitleClick
    />;
  };

  let%component make =
                (
                  ~model,
                  ~workingDirectory,
                  ~onItemClick,
                  ~isFocused,
                  ~theme,
                  ~font: UiFont.t,
                  ~dispatch,
                  (),
                ) => {
    let groups = {
      open Base.List.Let_syntax;

      let%bind provider = model.providers;
      let%bind group = provider.resourceGroups;

      return((provider, group));
    };

    let%hook (localState, localDispatch) =
      Hooks.reducer(~initialState=StringMap.empty, (msg, model) => {
        StringMap.update(
          msg,
          fun
          | None => Some(false)
          | Some(false) => Some(true)
          | Some(true) => Some(false),
          model,
        )
      });

    <View style=Styles.container>
      <Feature_InputText.View
        style=Styles.input
        model={model.inputBox}
        isFocused
        fontFamily={font.family}
        fontSize={font.size}
        dispatch={msg => dispatch(InputBox(msg))}
        theme
      />
      {groups
       |> List.filter_map(((provider, group: ResourceGroup.t)) => {
            let expanded =
              StringMap.find_opt(group.label, localState)
              |> Option.value(~default=true);

            if (group.resources == [] && group.hideWhenEmpty) {
              None;
            } else {
              Some(
                <groupView
                  provider
                  expanded
                  group
                  theme
                  font
                  workingDirectory
                  onItemClick
                  onTitleClick={() => localDispatch(group.label)}
                />,
              );
            };
          })
       |> React.listToElement}
    </View>;
  };
};
