open Oni_Core;
open Utility;

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
    viewModel: Component_VimList.model(Resource.t),
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
    statusBarCommands: list(command),
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

  let appliesToPath = (~path: string, {rootUri, _}) => {
    let maybePath =
      path
      |> Oni_Core.Uri.fromPath
      |> Oni_Core.Uri.toFileSystemPath
      |> Fp.absoluteCurrentPlatform;

    let maybeScmPath =
      rootUri
      |> Option.map(Oni_Core.Uri.toFileSystemPath)
      |> OptionEx.flatMap(Fp.absoluteCurrentPlatform);

    OptionEx.map2(
      (path, scmPath) => {Fp.isDescendent(~ofPath=scmPath, path)},
      maybePath,
      maybeScmPath,
    )
    |> Option.value(~default=false);
  };

  let%test_module "Provider" =
    (module
     {
       let make = (~rootUri) => {
         initial(~handle=0, ~id="test-id", ~label="test", ~rootUri);
       };
       module Uri = Oni_Core.Uri;
       let%test "appliesToPath -no path should be false" = {
         let provider = make(~rootUri=None);
         appliesToPath(~path="/test", provider) == false;
       };

       let%test "appliesToPath - same path should be true" =
         if (Sys.win32) {
           let provider = make(~rootUri=Some("D:/test" |> Uri.fromPath));
           appliesToPath(~path="D:/test", provider) == true;
         } else {
           let provider = make(~rootUri=Some("/test" |> Uri.fromPath));
           appliesToPath(~path="/test", provider) == true;
         };
       let%test "appliesToPath - nested path should be true" =
         if (Sys.win32) {
           let provider = make(~rootUri=Some("D:/test" |> Uri.fromPath));
           appliesToPath(~path="D:/test/dir", provider) == true;
         } else {
           let provider = make(~rootUri=Some("/test" |> Uri.fromPath));
           appliesToPath(~path="/test/dir", provider) == true;
         };
       let%test "appliesToPath - root path should be false" =
         if (Sys.win32) {
           let provider =
             make(~rootUri=Some("D:/test/project" |> Uri.fromPath));
           appliesToPath(~path="D:/test/", provider) == false;
         } else {
           let provider =
             make(~rootUri=Some("/test/project" |> Uri.fromPath));
           appliesToPath(~path="/test", provider) == false;
         };
     });
};

[@deriving show({with_path: false})]
module Focus = {
  [@deriving show]
  type t =
    | CommitText
    | Group({
        providerHandle: int,
        handle: int,
        id: string,
      });

  let initial = CommitText;

  let idx = (~handle, ~id, visibleGroups) => {
    let rec loop = (currentIndex, remainingGroups) => {
      switch (remainingGroups) {
      | [] => None
      | [(_provider, group), ...tail] =>
        ResourceGroup.(
          if (group.handle == handle && group.id == id) {
            Some(currentIndex);
          } else {
            loop(currentIndex + 1, tail);
          }
        )
      };
    };

    loop(0, visibleGroups);
  };

  let group = (~idx, visibleGroups) => {
    let (provider: Provider.t, group: ResourceGroup.t) =
      List.nth(visibleGroups, idx);
    Group({
      providerHandle: provider.handle,
      handle: group.handle,
      id: group.id,
    });
  };

  let focusUp = (visibleGroups, focus) => {
    switch (focus) {
    | CommitText => None

    | Group({handle, id, _}) =>
      let maybeIdx = idx(~handle, ~id, visibleGroups);

      switch (maybeIdx) {
      // Somehow, our group isn't visible.. focus commit text
      | None => Some(CommitText)

      // Below top - focus group up
      | Some(idx) when idx >= 1 => Some(group(~idx=idx - 1, visibleGroups))

      // At top, focus text now
      | Some(_) => Some(CommitText)
      };
    };
  };

  let focusDown = (visibleGroups, focus) => {
    let groupCount = List.length(visibleGroups);
    switch (focus) {
    | CommitText when groupCount >= 1 => Some(group(~idx=0, visibleGroups))

    | CommitText => None

    | Group({handle, id, _}) =>
      let maybeIdx = idx(~handle, ~id, visibleGroups);

      switch (maybeIdx) {
      | None when groupCount >= 1 => Some(group(~idx=0, visibleGroups))

      // No groups, unhandled
      | None => None

      // Not at bottom - focus downward
      | Some(idx) when idx < groupCount - 1 =>
        Some(group(~idx=idx + 1, visibleGroups))

      // At bottom, unhandled
      | Some(_) => None
      };
    };
  };

  let isGroupFocused = (group: ResourceGroup.t, focus) => {
    switch (focus) {
    | CommitText => false
    | Group({handle, id, _}) => group.handle == handle && group.id == id
    };
  };
};

open Focus;

[@deriving show({with_path: false})]
type model = {
  providers: list(Provider.t),
  inputBox: Component_InputText.model,
  textContentProviders: list((int, string)),
  vimWindowNavigation: Component_VimWindows.model,
  focus: Focus.t,
};

let count = ({providers, _}) => {
  providers
  |> List.fold_left(
       (count, provider: Provider.t) => {count + provider.count},
       0,
     );
};

let resetFocus = model => {...model, focus: Focus.initial};

let initial = {
  providers: [],
  inputBox: Component_InputText.create(~placeholder="Do the commit thing!"),
  textContentProviders: [],
  vimWindowNavigation: Component_VimWindows.initial,
  focus: Focus.initial,
};

let visibleGroups = ({providers, _}) => {
  let groups = {
    open Base.List.Let_syntax;

    let%bind provider = providers;
    let%bind group = provider.resourceGroups;
    return((provider, group));
  };

  groups
  |> List.filter(((_provider, group: ResourceGroup.t)) => {
       !(group.resources == [] && group.hideWhenEmpty)
     });
};

let selectedGroup = model => {
  switch (model.focus) {
  | CommitText => None
  | Group({providerHandle, handle, _}) =>
    visibleGroups(model)
    |> List.filter(((provider: Provider.t, group: ResourceGroup.t)) => {
         provider.handle == providerHandle && group.handle == handle
       })
    |> (list => List.nth_opt(list, 0))
  };
};

let statusBarCommands = (~workingDirectory, {providers, _}: model) => {
  providers
  |> List.filter(provider => {
       Provider.appliesToPath(~path=workingDirectory, provider)
     })
  |> List.map(({statusBarCommands, _}: Provider.t) => statusBarCommands)
  |> List.flatten;
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
  | GetOriginalContentFailed({bufferId: int})
  | NewProvider({
      handle: int,
      id: string,
      label: string,
      rootUri: option(Uri.t),
    })
  | LostProvider({handle: int})
  | NewResourceGroups({
      provider: int,
      groups: list(Exthost.SCM.Group.t),
      splices: [@opaque] list(Exthost.SCM.Resource.Splices.t),
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
  | SpliceResourceStates({
      handle: int,
      splices: [@opaque] list(Exthost.SCM.Resource.Splices.t),
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
      statusBarCommands: list(Exthost.SCM.command),
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
  | InputBox(Component_InputText.msg)
  | VimWindowNav(Component_VimWindows.msg)
  | List({
      provider: int,
      group: int,
      msg: Component_VimList.msg,
    });

module Msg = {
  let paste = text => Pasted({text: text});
  let keyPressed = key => KeyPressed({key: key});

  let documentContentProvider = msg => DocumentContentProvider(msg);
};

type outmsg =
  | Effect(Isolinear.Effect.t(msg))
  | EffectAndFocus(Isolinear.Effect.t(msg))
  | Focus
  | OpenFile(string)
  | PreviewFile(string)
  | UnhandledWindowMovement(Component_VimWindows.outmsg)
  | OriginalContentLoaded({
      bufferId: int,
      originalLines: array(string),
    })
  | Nothing;

module Effects = {
  let getOriginalContent = (fileSystem, bufferId, uri, providers, client) => {
    let scheme = uri |> Uri.getScheme |> Uri.Scheme.toString;

    // Is there a file system provider?

    let maybeFileSystem =
      Feature_FileSystem.getFileSystem(~scheme, fileSystem);
    switch (maybeFileSystem) {
    // No file system - fall back to text content provider,
    // if available...
    | None =>
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
      |> Option.value(~default=Isolinear.Effect.none)

    | Some(handle) =>
      Feature_FileSystem.Effects.readFile(
        ~handle,
        ~uri,
        ~toMsg=
          resultLines =>
            switch (resultLines) {
            | Error(_) => GetOriginalContentFailed({bufferId: bufferId})
            | Ok(lines) =>
              // If the file is empty, it's either untracked, ignored, or totally empty when adding -
              // in any of these cases, we don't want to render diff markers.
              // https://github.com/onivim/oni2/issues/3355
              if (lines != [|""|]) {
                GotOriginalContent({bufferId, lines});
              } else {
                GetOriginalContentFailed({bufferId: bufferId});
              }
            },
        fileSystem,
        client,
      )
    };
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

let update =
    (
      ~previewEnabled,
      ~fileSystem: Feature_FileSystem.model,
      extHostClient: Exthost.Client.t,
      model,
      msg,
    ) =>
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
          fileSystem,
          bufferId,
          uri,
          model.textContentProviders,
          extHostClient,
        ),
      ),
    )

  | GotOriginalContent({bufferId, lines}) => (
      model,
      OriginalContentLoaded({bufferId, originalLines: lines}),
    )

  | GetOriginalContentFailed(_) => (model, Nothing)

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
  //          Component_InputText.setPlaceholder(~placeholder, model.inputBox),
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

  | NewResourceGroups({provider, groups, splices}) =>
    let model' =
      groups
      |> List.fold_left(
           (acc, group: Exthost.SCM.Group.t) => {
             acc
             |> Internal.updateProvider(~handle=provider, p =>
                  {
                    ...p,
                    resourceGroups: [
                      ResourceGroup.{
                        handle: group.handle,
                        id: group.id,
                        label: group.label,
                        hideWhenEmpty:
                          Exthost.SCM.GroupFeatures.(
                            group.features.hideWhenEmpty
                          ),
                        resources: [],
                        viewModel: Component_VimList.create(~rowHeight=20),
                      },
                      ...p.resourceGroups,
                    ],
                  }
                )
           },
           model,
         );

    let model'' =
      splices
      |> List.fold_left(
           (acc, {handle: group, resourceSplices}: Resource.Splices.t) => {
             resourceSplices
             |> List.fold_left(
                  (
                    innerAcc,
                    {start, deleteCount, resources}: Resource.Splice.t,
                  ) => {
                    innerAcc
                    |> Internal.updateResourceGroup(
                         ~provider,
                         ~group,
                         g => {
                           let resources =
                             ListEx.splice(
                               ~start,
                               ~deleteCount,
                               ~additions=resources,
                               g.resources,
                             );

                           let viewModel =
                             Component_VimList.set(
                               resources |> Array.of_list,
                               g.viewModel,
                             );
                           {...g, resources, viewModel};
                         },
                       )
                  },
                  acc,
                )
           },
           model',
         );
    (model'', Nothing);

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

  | SpliceResourceStates({handle, splices}) =>
    let provider = handle;
    open Exthost.SCM;
    let model' =
      splices
      |> List.fold_left(
           (acc, {handle: group, resourceSplices}: Resource.Splices.t) => {
             resourceSplices
             |> List.fold_left(
                  (
                    innerAcc,
                    {start, deleteCount, resources}: Resource.Splice.t,
                  ) => {
                    innerAcc
                    |> Internal.updateResourceGroup(
                         ~provider,
                         ~group,
                         g => {
                           let resources =
                             ListEx.splice(
                               ~start,
                               ~deleteCount,
                               ~additions=resources,
                               g.resources,
                             );

                           let viewModel =
                             Component_VimList.set(
                               resources |> Array.of_list,
                               g.viewModel,
                             );
                           {...g, resources, viewModel};
                         },
                       )
                  },
                  acc,
                )
           },
           model,
         );
    (model', Nothing);

  | KeyPressed({key: "<CR>"}) => (
      model,
      EffectAndFocus(
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
    let inputBox = Component_InputText.handleInput(~key, model.inputBox);
    (
      {...model, inputBox},
      EffectAndFocus(
        Isolinear.Effect.batch(
          model.providers
          |> List.map((provider: Provider.t) =>
               Service_Exthost.Effects.SCM.onInputBoxValueChange(
                 ~handle=provider.handle,
                 ~value=inputBox |> Component_InputText.value,
                 extHostClient,
               )
             ),
        ),
      ),
    );

  | Pasted({text}) =>
    let inputBox = Component_InputText.paste(~text, model.inputBox);
    (
      {...model, inputBox},
      EffectAndFocus(
        Isolinear.Effect.batch(
          model.providers
          |> List.map((provider: Provider.t) =>
               Service_Exthost.Effects.SCM.onInputBoxValueChange(
                 ~handle=provider.handle,
                 ~value=inputBox |> Component_InputText.value,
                 extHostClient,
               )
             ),
        ),
      ),
    );

  | InputBox(msg) =>
    let (inputBox', inputOutmsg) =
      Component_InputText.update(msg, model.inputBox);
    let outmsg =
      switch (inputOutmsg) {
      | Component_InputText.Nothing => Nothing
      | Component_InputText.Focus => Focus
      };

    ({...model, inputBox: inputBox'}, outmsg);

  | VimWindowNav(navMsg) =>
    let (windowNav, outmsg) =
      Component_VimWindows.update(navMsg, model.vimWindowNavigation);

    let model' = {...model, vimWindowNavigation: windowNav};
    let (focus, outmsg) =
      switch (outmsg) {
      | Nothing => (model'.focus, Nothing)
      | FocusLeft => (model'.focus, UnhandledWindowMovement(outmsg))
      | FocusRight => (model'.focus, UnhandledWindowMovement(outmsg))
      | FocusDown =>
        switch (Focus.focusDown(visibleGroups(model'), model'.focus)) {
        | None => (model'.focus, UnhandledWindowMovement(outmsg))
        | Some(focus) => (focus, Nothing)
        }
      | FocusUp =>
        switch (Focus.focusUp(visibleGroups(model'), model'.focus)) {
        | None => (model'.focus, UnhandledWindowMovement(outmsg))
        | Some(focus) => (focus, Nothing)
        }

      | PreviousTab
      | NextTab => (model'.focus, Nothing)
      };
    ({...model', focus}, outmsg);

  | List({provider, group, msg}) =>
    // DIRTY IMPURE HACK: Capture the outmsg during the update
    // This assumes that there is a unique (provider, group).
    let capturedOutmsg = ref(None);

    let model =
      model
      |> Internal.updateResourceGroup(
           ~provider,
           ~group,
           g => {
             let (viewModel, outmsg) =
               Component_VimList.update(msg, g.viewModel);

             let outmsg =
               switch (outmsg) {
               | Component_VimList.Nothing => Some(Nothing)
               | Component_VimList.Touched({index}) =>
                 Component_VimList.get(index, viewModel)
                 |> Option.map((item: Resource.t) =>
                      previewEnabled
                        ? PreviewFile(
                            item.uri |> Oni_Core.Uri.toFileSystemPath,
                          )
                        : OpenFile(item.uri |> Oni_Core.Uri.toFileSystemPath)
                    )
               | Component_VimList.Selected({index}) =>
                 Component_VimList.get(index, viewModel)
                 |> Option.map((item: Resource.t) =>
                      OpenFile(item.uri |> Oni_Core.Uri.toFileSystemPath)
                    )
               };

             capturedOutmsg := outmsg;
             {...g, viewModel};
           },
         );

    (model, capturedOutmsg^ |> Option.value(~default=Nothing));
  };

let handleExtensionMessage = (~dispatch, msg: Exthost.Msg.SCM.msg) =>
  switch (msg) {
  | RegisterSourceControl({handle, id, label, rootUri}) =>
    dispatch(NewProvider({handle, id, label, rootUri}))

  | UnregisterSourceControl({handle}) =>
    dispatch(LostProvider({handle: handle}))

  | RegisterSCMResourceGroups({provider, groups, splices}) =>
    dispatch(NewResourceGroups({provider, groups, splices}))

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
    dispatch(SpliceResourceStates({handle, splices}))

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

    Option.iter(
      statusBarCommands => {
        dispatch(StatusBarCommandsChanged({handle, statusBarCommands}))
      },
      statusBarCommands,
    );

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
  |> List.filter(Provider.appliesToPath(~path=filePath))
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

module Colors = Feature_Theme.Colors;

module Pane = {
  module Styles = {
    open Style;

    let container = [flexGrow(1)];

    let inputContainer = [margin(12)];

    let text = (~theme) => [
      color(Colors.SideBar.foreground.from(theme)),
      textWrap(TextWrapping.NoWrap),
      textOverflow(`Ellipsis),
    ];

    let item = [
      flexDirection(`Row),
      paddingVertical(2),
      marginLeft(6),
      cursor(MouseCursors.pointer),
    ];
  };

  let itemView =
      (
        ~provider: Provider.t,
        ~resource: Resource.t,
        ~theme,
        ~iconTheme,
        ~languageInfo,
        ~font: UiFont.t,
        ~workingDirectory,
        (),
      ) => {
    open Base;

    let base =
      provider.rootUri
      |> Option.map(~f=Uri.toFileSystemPath)
      |> Option.value(~default=workingDirectory);

    let path = Uri.toFileSystemPath(resource.uri);
    let displayName = Path.toRelative(~base, path);

    <View style=Styles.item>
      <Oni_Components.FileIcon font iconTheme languageInfo path />
      <Text
        style={Styles.text(~theme)}
        text=displayName
        fontFamily={font.family}
        fontSize={font.size}
      />
    </View>;
  };

  let groupView =
      (
        ~provider,
        ~group: ResourceGroup.t,
        ~iconTheme,
        ~languageInfo,
        ~theme,
        ~isFocused: bool,
        ~font: UiFont.t,
        ~dispatch,
        ~workingDirectory,
        ~onTitleClick,
        ~expanded,
        (),
      ) => {
    let label = group.label;
    let renderItem =
        (
          ~availableWidth as _,
          ~index as _,
          ~hovered as _,
          ~selected as _,
          item,
        ) => {
      <itemView
        provider
        resource=item
        iconTheme
        languageInfo
        theme
        font
        workingDirectory
      />;
    };
    <Component_Accordion.VimList
      title=label
      expanded
      uiFont=font
      model={group.viewModel}
      dispatch={msg =>
        dispatch(List({provider: provider.handle, group: group.handle, msg}))
      }
      isFocused
      render=renderItem
      theme
      //focused=None
      onClick=onTitleClick
    />;
  };

  let%component make =
                (
                  ~model,
                  ~workingDirectory,
                  ~isFocused,
                  ~iconTheme,
                  ~languageInfo,
                  ~theme,
                  ~font: UiFont.t,
                  ~dispatch,
                  (),
                ) => {
    let groups = visibleGroups(model);

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
      <View style=Styles.inputContainer>
        <Component_InputText.View
          model={model.inputBox}
          isFocused={isFocused && model.focus == CommitText}
          fontFamily={font.family}
          fontSize={font.size}
          dispatch={msg => dispatch(InputBox(msg))}
          theme
        />
      </View>
      {groups
       |> List.map(((provider, group: ResourceGroup.t)) => {
            let expanded =
              StringMap.find_opt(group.label, localState)
              |> Option.value(~default=true);

            <groupView
              provider
              expanded
              group
              dispatch
              isFocused={isFocused && isGroupFocused(group, model.focus)}
              iconTheme
              languageInfo
              theme
              font
              workingDirectory
              onTitleClick={() => localDispatch(group.label)}
            />;
          })
       |> React.listToElement}
    </View>;
  };
};

module Contributions = {
  let commands = (~isFocused, model) => {
    let listCommands =
      switch (model.focus) {
      | CommitText => []
      | Group({providerHandle, handle, _}) =>
        Component_VimList.Contributions.commands
        |> List.map(
             Oni_Core.Command.map(msg =>
               List({provider: providerHandle, group: handle, msg})
             ),
           )
      };

    !isFocused
      ? []
      : (
          Component_VimWindows.Contributions.commands
          |> List.map(Oni_Core.Command.map(msg => VimWindowNav(msg)))
        )
        @ listCommands;
  };

  let contextKeys = (~isFocused, model) => {
    open WhenExpr.ContextKeys;
    let inputKeys =
      isFocused && model.focus == CommitText
        ? Component_InputText.Contributions.contextKeys(model.inputBox)
        : empty;

    let listKeys =
      isFocused && model.focus != CommitText
        ? selectedGroup(model)
          |> Option.map(((_provider, group: ResourceGroup.t)) => {
               Component_VimList.Contributions.contextKeys(group.viewModel)
             })
          |> Option.value(~default=empty)
        : empty;

    let vimNavKeys =
      isFocused
        ? Component_VimWindows.Contributions.contextKeys(
            model.vimWindowNavigation,
          )
        : empty;

    [inputKeys, listKeys, vimNavKeys] |> unionMany;
  };
};
