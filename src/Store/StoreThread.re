/*
 * StoreThread.re
 *
 * This is the 'state management' piece of Oni2.
 *
 * The state updates are run in a parallel thread to the rendering,
 * so that we can eek out as much perf as we can in this architecture.
 */

module Core = Oni_Core;
module FpExp = Oni_Core.FpExp;

module Model = Oni_Model;

open Exthost.Extension;

module Log = (val Core.Log.withNamespace("Oni2.Store.StoreThread"));
module DispatchLog = (val Core.Log.withNamespace("Oni2.Store.dispatch"));

let discoverExtensions =
    (setup: Core.Setup.t, ~shouldLoadExtensions, ~overriddenExtensionsDir) =>
  if (shouldLoadExtensions) {
    let extensions =
      Core.Log.perf("Discover extensions", () => {
        let extensions =
          setup.bundledExtensionsPath
          |> FpExp.absoluteCurrentPlatform
          |> Option.map(
               Scanner.scan(
                 // The extension host assumes bundled extensions start with 'vscode.'
                 ~category=Bundled,
               ),
             )
          |> Option.value(~default=[]);

        let developmentExtensions =
          setup.developmentExtensionsPath
          |> Core.Utility.OptionEx.flatMap(FpExp.absoluteCurrentPlatform)
          |> Option.map(Scanner.scan(~category=Development))
          |> Option.value(~default=[]);

        let userExtensions =
          Service_Extensions.Management.get(
            ~extensionsFolder=?overriddenExtensionsDir,
            (),
          )
          // TODO: De-syncify!
          |> Core.Utility.LwtEx.sync
          |> Result.value(~default=[]);

        Log.infof(m =>
          m("Discovered %n user extensions.", List.length(userExtensions))
        );

        [extensions, developmentExtensions, userExtensions] |> List.flatten;
      });

    Log.infof(m =>
      m("-- Discovered: %n extensions", List.length(extensions))
    );

    extensions;
  } else {
    Log.info("Not loading extensions; disabled via CLI");
    [];
  };

let registerCommands = (~dispatch, commands) => {
  List.iter(
    command => dispatch(Model.Actions.Commands(NewCommand(command))),
    commands,
  );
};

let start =
    (
      ~showUpdateChangelog=true,
      ~onAfterDispatch=_ => (),
      ~setup: Core.Setup.t,
      ~executingDirectory,
      ~getState,
      ~onStateChanged,
      ~getClipboardText,
      ~setClipboardText,
      ~quit,
      ~setVsync,
      ~maximize,
      ~minimize,
      ~close,
      ~restore,
      ~raiseWindow,
      ~window: option(Revery.Window.t),
      ~overriddenExtensionsDir=None,
      ~shouldLoadExtensions=true,
      ~shouldSyntaxHighlight=true,
      (),
    ) => {
  ignore(executingDirectory);

  let latestRunEffects: ref(option(unit => unit)) = ref(None);

  let runRunEffects = () =>
    switch (latestRunEffects^) {
    | Some(v) => v()
    | None => ()
    };

  let extensions =
    discoverExtensions(
      setup,
      ~shouldLoadExtensions,
      ~overriddenExtensionsDir,
    );
  let languageInfo = Exthost.LanguageInfo.ofExtensions(extensions);
  let grammarInfo = Exthost.GrammarInfo.ofExtensions(extensions);
  let grammarRepository = Oni_Syntax.GrammarRepository.create(grammarInfo);

  let commandUpdater = CommandStoreConnector.start();
  let (vimUpdater, vimStream) =
    VimStoreConnector.start(
      ~showUpdateChangelog,
      languageInfo,
      getState,
      getClipboardText,
      setClipboardText,
    );

  let themeUpdater = ThemeStoreConnector.start();

  let initialState = getState();

  let attachExthostStdio =
    Oni_CLI.(
      {
        initialState.cli.attachToForeground
        && (
          Option.is_some(initialState.cli.logLevel)
          || initialState.cli.logExthost
        );
      }
    );

  let initialWorkspace =
    Feature_Workspace.openedFolder(initialState.workspace)
    |> Option.map(Exthost.WorkspaceData.fromPath);

  let (extHostClientResult, extHostStream) =
    ExtensionClient.create(
      ~initialWorkspace,
      ~attachStdio=attachExthostStdio,
      ~config=getState().config,
      ~extensions,
      ~setup,
    );

  // TODO: How to handle this correctly?
  let extHostClient = extHostClientResult |> Result.get_ok;

  let extHostUpdater =
    ExtensionClientStoreConnector.start(extensions, extHostClient);

  let quickmenuUpdater = QuickmenuStoreConnector.start();

  let keyBindingsUpdater = KeyBindingsStoreConnector.start();

  let lifecycleUpdater = LifecycleStoreConnector.start(~quit, ~raiseWindow);

  let (inputUpdater, inputStream) =
    InputStoreConnector.start(window, runRunEffects);

  let updater =
    Isolinear.Updater.combine([
      Isolinear.Updater.ofReducer(Reducer.reduce),
      inputUpdater,
      quickmenuUpdater,
      vimUpdater,
      extHostUpdater,
      keyBindingsUpdater,
      commandUpdater,
      lifecycleUpdater,
      themeUpdater,
      Features.update(
        ~grammarRepository,
        ~extHostClient,
        ~maximize,
        ~minimize,
        ~close,
        ~restore,
        ~setVsync,
      ),
    ]);

  let subscriptions = (~setup, state: Model.State.t) => {
    let config = Model.Selectors.configResolver(state);
    let contextKeys = Model.ContextKeys.all(state);
    let commands = Model.CommandManager.current(state);

    let menuBarSub =
      Feature_MenuBar.sub(
        ~config,
        ~contextKeys,
        ~commands,
        ~input=state.input,
        state.menuBar,
      )
      |> Isolinear.Sub.map(msg => Model.Actions.MenuBar(msg));

    let visibleBuffersAndRanges =
      state |> Model.EditorVisibleRanges.getVisibleBuffersAndRanges;
    let activeEditor = state.layout |> Feature_Layout.activeEditor;

    let isInsertOrSelectMode =
      activeEditor |> Feature_Editor.Editor.mode |> Vim.Mode.isInsertOrSelect;

    let isAnimatingScroll =
      activeEditor |> Feature_Editor.Editor.isAnimatingScroll;

    let visibleRanges =
      visibleBuffersAndRanges
      |> List.map(((bufferId, ranges)) => {
           Model.Selectors.getBufferById(state, bufferId)
           |> Option.map(buffer => {(buffer, ranges)})
         })
      |> Core.Utility.OptionEx.values;

    let topVisibleBufferLine =
      activeEditor |> Feature_Editor.Editor.getTopVisibleBufferLine;

    let bottomVisibleBufferLine =
      activeEditor |> Feature_Editor.Editor.getBottomVisibleBufferLine;

    let visibleBuffers =
      visibleBuffersAndRanges
      |> List.map(fst)
      |> Base.List.dedup_and_sort(~compare)
      |> List.map(bufferId => Model.Selectors.getBufferById(state, bufferId))
      |> Core.Utility.OptionEx.values;

    let syntaxSubscription =
      shouldSyntaxHighlight && !state.isQuitting
        ? Feature_Syntax.subscription(
            ~buffers=state.buffers,
            ~config,
            ~grammarInfo,
            ~languageInfo,
            ~setup,
            ~tokenTheme=state.colorTheme |> Feature_Theme.tokenColors,
            ~bufferVisibility=visibleRanges,
            state.syntaxHighlights,
          )
          |> Isolinear.Sub.map(msg => Model.Actions.Syntax(msg))
        : Isolinear.Sub.none;

    let terminalSubscription =
      Feature_Terminal.subscription(
        ~workspaceUri=
          Core.Uri.fromPath(
            Feature_Workspace.workingDirectory(state.workspace),
          ),
        extHostClient,
        state.terminals,
      )
      |> Isolinear.Sub.map(msg => Model.Actions.Terminal(msg));

    let fontFamily = Feature_Editor.Configuration.fontFamily.get(config);
    let fontSize = Feature_Editor.Configuration.fontSize.get(config);
    let fontWeight = Feature_Editor.Configuration.fontWeight.get(config);
    let fontLigatures =
      Feature_Editor.Configuration.fontLigatures.get(config);

    let fontSmoothing =
      Feature_Editor.Configuration.fontSmoothing.get(config);

    let editorFontSubscription =
      Service_Font.Sub.font(
        ~uniqueId="editorFont",
        ~fontFamily,
        ~fontSize,
        ~fontWeight,
        ~fontSmoothing,
        ~fontLigatures,
      )
      |> Isolinear.Sub.map(msg => Model.Actions.EditorFont(msg));

    let terminalFontFamily =
      Feature_Terminal.Configuration.fontFamily.get(config)
      |> Option.value(~default=fontFamily);

    let terminalFontSize =
      Feature_Terminal.Configuration.fontSize.get(config)
      |> Option.value(~default=fontSize);

    let terminalFontWeight =
      Feature_Terminal.Configuration.fontWeight.get(config)
      |> Option.value(~default=fontWeight);

    let terminalFontLigatures =
      Feature_Terminal.Configuration.fontLigatures.get(config)
      |> Option.value(~default=fontLigatures);

    let terminalFontSmoothing =
      Feature_Terminal.Configuration.fontSmoothing.get(config)
      |> Option.value(~default=fontSmoothing);

    let terminalFontSubscription =
      Service_Font.Sub.font(
        ~uniqueId="terminalFont",
        ~fontFamily=terminalFontFamily,
        ~fontSize=terminalFontSize,
        ~fontWeight=terminalFontWeight,
        ~fontSmoothing=terminalFontSmoothing,
        ~fontLigatures=terminalFontLigatures,
      )
      |> Isolinear.Sub.map(msg => Model.Actions.TerminalFont(msg));

    let visibleEditors = Feature_Layout.visibleEditors(state.layout);

    let activeEditor = Feature_Layout.activeEditor(state.layout);
    let activeEditorId = Feature_Editor.Editor.getId(activeEditor);
    let activeBufferId = Feature_Editor.Editor.getBufferId(activeEditor);
    let activePosition = Feature_Editor.Editor.getPrimaryCursor(activeEditor);
    let maybeActiveBuffer =
      Feature_Buffers.get(activeBufferId, state.buffers);

    let extHostSubscription =
      Feature_Exthost.subscription(
        ~buffers=state.buffers,
        ~editors=visibleEditors,
        ~activeEditorId=Some(activeEditorId),
        ~client=extHostClient,
        state.exthost,
      )
      |> Isolinear.Sub.map(msg => Model.Actions.Exthost(msg));

    // TODO: Move sub inside Explorer feature
    let fileExplorerActiveFileSub =
      Model.Sub.activeFile(
        ~id="activeFile.fileExplorer",
        ~state,
        ~toMsg=maybeFilePathStr => {
          let maybeFilePath =
            maybeFilePathStr
            |> Utility.OptionEx.flatMap(FpExp.absoluteCurrentPlatform);
          Model.Actions.FileExplorer(
            Feature_Explorer.Msg.activeFileChanged(maybeFilePath),
          );
        },
      );

    let fileExplorerSub =
      Feature_Explorer.sub(~configuration=state.config, state.fileExplorer)
      |> Isolinear.Sub.map(msg => Model.Actions.FileExplorer(msg));

    let languageSupportSub =
      maybeActiveBuffer
      |> Option.map(activeBuffer => {
           Feature_LanguageSupport.sub(
             ~config,
             ~isInsertMode=isInsertOrSelectMode,
             ~isAnimatingScroll,
             ~activeBuffer,
             ~activePosition,
             ~topVisibleBufferLine,
             ~bottomVisibleBufferLine,
             ~visibleBuffers,
             ~client=extHostClient,
             state.languageSupport,
           )
           |> Isolinear.Sub.map(msg => Model.Actions.LanguageSupport(msg))
         })
      |> Option.value(~default=Isolinear.Sub.none);

    let isSideBarOpen = Feature_SideBar.isOpen(state.sideBar);
    let isExtensionsFocused =
      Feature_SideBar.selected(state.sideBar) == Feature_SideBar.Extensions;
    let extensionsSub =
      Feature_Extensions.sub(
        ~isVisible=isSideBarOpen && isExtensionsFocused,
        ~setup,
        state.extensions,
      )
      |> Isolinear.Sub.map(msg => Model.Actions.Extensions(msg));

    let registersSub =
      Feature_Registers.sub(state.registers)
      |> Isolinear.Sub.map(msg => Model.Actions.Registers(msg));

    let scmSub =
      maybeActiveBuffer
      |> Option.map(buffer => {
           Feature_SCM.sub(
             ~activeBuffer=buffer,
             ~client=extHostClient,
             state.scm,
           )
           |> Isolinear.Sub.map(msg => Model.Actions.SCM(msg))
         })
      |> Option.value(~default=Isolinear.Sub.none);

    let autoUpdateSub =
      Feature_AutoUpdate.sub(~config)
      |> Isolinear.Sub.map(msg => Model.Actions.AutoUpdate(msg));

    let visibleEditorsSubscription =
      visibleEditors
      |> List.map(editor =>
           Feature_Editor.Sub.editor(~config, editor)
           |> Isolinear.Sub.map(msg =>
                Model.Actions.Editor({
                  scope:
                    Model.EditorScope.Editor(
                      editor |> Feature_Editor.Editor.getId,
                    ),
                  msg,
                })
              )
         )
      |> Isolinear.Sub.batch;

    let inputSubscription =
      state.input
      |> Feature_Input.sub(~config)
      |> Isolinear.Sub.map(msg => Model.Actions.Input(msg));

    let notificationSub =
      state.notifications
      |> Feature_Notification.sub
      |> Isolinear.Sub.map(msg => Model.Actions.Notification(msg));

    let vimBufferSub =
      visibleBuffersAndRanges
      |> List.map(bufferAndRanges => {
           let (bufferId, ranges) = bufferAndRanges;

           let maybeTopVisibleLine = ranges |> EditorCoreTypes.Range.minLine;
           let maybeBottomVisibleLine =
             ranges |> EditorCoreTypes.Range.maxLine;

           switch (Feature_Buffers.get(bufferId, state.buffers)) {
           | None => Isolinear.Sub.none
           | Some(buffer) =>
             Utility.OptionEx.map2(
               (topVisibleLine, bottomVisibleLine) => {
                 Feature_Vim.sub(
                   ~buffer,
                   ~topVisibleLine,
                   ~bottomVisibleLine,
                   state.vim,
                 )
                 |> Isolinear.Sub.map(msg => Model.Actions.Vim(msg))
               },
               maybeTopVisibleLine,
               maybeBottomVisibleLine,
             )
             |> Option.value(~default=Isolinear.Sub.none)
           };
         })
      |> Isolinear.Sub.batch;

    let bufferSub =
      state.buffers
      |> Feature_Buffers.sub
      |> Isolinear.Sub.map(msg => Model.Actions.Buffers(msg));

    let quickmenuSub =
      state.newQuickmenu
      |> Feature_Quickmenu.sub
      |> Isolinear.Sub.map(msg => Model.Actions.Quickmenu(msg));

    let isExthostInitialized = Feature_Exthost.isInitialized(state.exthost);
    let configurationSub =
      state.config
      |> Feature_Configuration.sub(
           ~client=extHostClient,
           ~isExthostInitialized,
         )
      |> Isolinear.Sub.map(msg => Model.Actions.Configuration(msg));

    let themeSub =
      if (Feature_Extensions.hasCompletedDiscovery(state.extensions)) {
        // If discovery hasn't been completed, theme contributions aren't meaningful.
        let getThemeContribution = themeId =>
          Feature_Extensions.themeById(~id=themeId, state.extensions);

        state.colorTheme
        |> Feature_Theme.sub(~getThemeContribution)
        |> Isolinear.Sub.map(msg => Model.Actions.Theme(msg));
      } else {
        Isolinear.Sub.none;
      };

    [
      menuBarSub,
      extHostSubscription,
      languageSupportSub,
      syntaxSubscription,
      terminalSubscription,
      editorFontSubscription,
      terminalFontSubscription,
      Isolinear.Sub.batch(VimStoreConnector.subscriptions(state)),
      fileExplorerActiveFileSub,
      fileExplorerSub,
      extensionsSub,
      registersSub,
      scmSub,
      autoUpdateSub,
      visibleEditorsSubscription,
      inputSubscription,
      notificationSub,
      bufferSub,
      configurationSub,
      quickmenuSub,
      themeSub,
      vimBufferSub,
    ]
    |> Isolinear.Sub.batch;
  };

  module Store =
    Isolinear.Store.Make({
      type msg = Model.Actions.t;
      type model = Model.State.t;

      let initial = getState();
      let updater = updater;
      let subscriptions = subscriptions(~setup);
    });

  let _unsubscribe: unit => unit = Store.onModelChanged(onStateChanged);

  let _unsubscribe: unit => unit =
    Store.onBeforeMsg(msg =>
      DispatchLog.infof(m => m("dispatch: %s", Model.Actions.show(msg)))
    );

  let dispatch = Store.dispatch;

  let _unsubscribe: unit => unit =
    Store.onAfterMsg((msg, model) => {
      Features.updateSubscriptions(setup, model, dispatch);
      onAfterDispatch(msg);
      DispatchLog.debugf(m => m("After: %s", Model.Actions.show(msg)));
    });

  let _unsubscribe: unit => unit =
    Store.onBeforeEffectRan(e => {
      Log.debugf(m => m("Running effect: %s", Isolinear.Effect.name(e)))
    });
  let _unsubscribe: unit => unit =
    Store.onAfterEffectRan(e => {
      Log.debugf(m => m("Effect complete: %s", Isolinear.Effect.name(e)))
    });

  let runEffects = Store.runPendingEffects;
  latestRunEffects := Some(runEffects);

  Option.iter(
    window =>
      Revery.Window.setCanQuitCallback(window, () =>
        if (Feature_Buffers.anyModified(getState().buffers)) {
          dispatch(Model.Actions.WindowCloseBlocked);
          false;
        } else {
          true;
        }
      ),
    window,
  );

  // Commands
  [
    Model.GlobalCommands.registrations(),
    Feature_Sneak.Contributions.commands
    |> List.map(Core.Command.map(msg => Model.Actions.Sneak(msg))),
    Feature_Terminal.Contributions.commands
    |> List.map(Core.Command.map(msg => Model.Actions.Terminal(msg))),
    Feature_Sneak.Contributions.commands
    |> List.map(Core.Command.map(msg => Model.Actions.Sneak(msg))),
    Feature_Layout.Contributions.commands
    |> List.map(Core.Command.map(msg => Model.Actions.Layout(msg))),
    Feature_Theme.Contributions.commands
    |> List.map(Core.Command.map(msg => Model.Actions.Theme(msg))),
    Feature_Clipboard.Contributions.commands
    |> List.map(Core.Command.map(msg => Model.Actions.Clipboard(msg))),
    Feature_Registers.Contributions.commands
    |> List.map(Core.Command.map(msg => Model.Actions.Registers(msg))),
    Feature_LanguageSupport.Contributions.commands
    |> List.map(Core.Command.map(msg => Model.Actions.LanguageSupport(msg))),
    Feature_Input.Contributions.commands
    |> List.map(Core.Command.map(msg => Model.Actions.Input(msg))),
    Feature_AutoUpdate.Contributions.commands
    |> List.map(Core.Command.map(msg => Model.Actions.AutoUpdate(msg))),
    Feature_Registration.Contributions.commands
    |> List.map(Core.Command.map(msg => Model.Actions.Registration(msg))),
    Feature_Snippets.Contributions.commands
    |> List.map(Core.Command.map(msg => Model.Actions.Snippets(msg))),
    Feature_Zoom.Contributions.commands
    |> List.map(Core.Command.map(msg => Model.Actions.Zoom(msg))),
  ]
  |> List.flatten
  |> registerCommands(~dispatch);

  // TODO: These should all be replaced with isolinear subscriptions.
  let _: Isolinear.unsubscribe =
    Isolinear.Stream.connect(dispatch, inputStream);
  let _: Isolinear.unsubscribe =
    Isolinear.Stream.connect(dispatch, vimStream);
  let _: Isolinear.unsubscribe =
    Isolinear.Stream.connect(dispatch, extHostStream);

  dispatch(Model.Actions.SetLanguageInfo(languageInfo));
  dispatch(Model.Actions.SetGrammarRepository(grammarRepository));

  /* Set icon theme */

  let setIconTheme = s => {
    let iconThemeInfo =
      extensions
      |> List.map((ext: Scanner.ScanResult.t) =>
           ext.manifest.contributes.iconThemes
         )
      |> List.flatten
      |> List.filter((iconTheme: Contributions.IconTheme.t) =>
           String.equal(iconTheme.id, s)
         );

    let iconThemeInfo = List.nth_opt(iconThemeInfo, 0);

    switch (iconThemeInfo) {
    | Some(iconThemeInfo) =>
      let iconTheme =
        Yojson.Safe.from_file(iconThemeInfo.path) |> Core.IconTheme.ofJson;

      switch (iconTheme) {
      | Some(iconTheme) => dispatch(Model.Actions.SetIconTheme(iconTheme))
      | None => ()
      };
    | None => ()
    };
  };

  setIconTheme("vs-seti");

  let _: unit => unit =
    Revery.Tick.interval(
      ~name="Store: Run Effects",
      _ => runEffects(),
      Revery.Time.zero,
    );

  (dispatch, runEffects);
};
