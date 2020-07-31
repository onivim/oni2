/*
 * StoreThread.re
 *
 * This is the 'state management' piece of Oni2.
 *
 * The state updates are run in a parallel thread to the rendering,
 * so that we can eek out as much perf as we can in this architecture.
 */

module Core = Oni_Core;

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
          Scanner.scan(
            // The extension host assumes bundled extensions start with 'vscode.'
            ~category=Bundled,
            setup.bundledExtensionsPath,
          );

        let developmentExtensions =
          switch (setup.developmentExtensionsPath) {
          | Some(p) => Scanner.scan(~category=Development, p)
          | None => []
          };

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
      ~getUserSettings,
      ~configurationFilePath=None,
      ~keybindingsFilePath=None,
      ~onAfterDispatch=_ => (),
      ~setup: Core.Setup.t,
      ~executingDirectory,
      ~getState,
      ~onStateChanged,
      ~getClipboardText,
      ~setClipboardText,
      ~getZoom,
      ~setZoom,
      ~quit,
      ~setTitle,
      ~setVsync,
      ~maximize,
      ~minimize,
      ~close,
      ~restore,
      ~raiseWindow,
      ~window: option(Revery.Window.t),
      ~filesToOpen=[],
      ~overriddenExtensionsDir=None,
      ~shouldLoadExtensions=true,
      ~shouldSyntaxHighlight=true,
      ~shouldLoadConfiguration=true,
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

  let (extHostClientResult, extHostStream) =
    ExtensionClient.create(~config=getState().config, ~extensions, ~setup);

  // TODO: How to handle this correctly?
  let extHostClient = extHostClientResult |> Result.get_ok;

  let extHostUpdater =
    ExtensionClientStoreConnector.start(extensions, extHostClient);

  let quickmenuUpdater = QuickmenuStoreConnector.start();

  let configurationUpdater =
    ConfigurationStoreConnector.start(
      ~configurationFilePath,
      ~getZoom,
      ~setZoom,
      ~setVsync,
      ~shouldLoadConfiguration,
      ~filesToOpen,
    );
  let keyBindingsUpdater =
    KeyBindingsStoreConnector.start(keybindingsFilePath);

  let fileExplorerUpdater = FileExplorerStore.start();

  let lifecycleUpdater = LifecycleStoreConnector.start(~quit, ~raiseWindow);
  let indentationUpdater = IndentationStoreConnector.start();
  let windowUpdater = WindowsStoreConnector.start();

  let completionUpdater = CompletionStoreConnector.start();

  let (inputUpdater, inputStream) =
    InputStoreConnector.start(window, runRunEffects);

  let titleUpdater =
    TitleStoreConnector.start(setTitle, maximize, minimize, restore, close);
  let contextMenuUpdater = ContextMenuStore.start();
  let updater =
    Isolinear.Updater.combine([
      Isolinear.Updater.ofReducer(Reducer.reduce),
      inputUpdater,
      quickmenuUpdater,
      vimUpdater,
      extHostUpdater,
      configurationUpdater,
      keyBindingsUpdater,
      commandUpdater,
      lifecycleUpdater,
      fileExplorerUpdater,
      indentationUpdater,
      windowUpdater,
      themeUpdater,
      completionUpdater,
      titleUpdater,
      Features.update(
        ~grammarRepository,
        ~extHostClient,
        ~getUserSettings,
        ~setup,
      ),
      PaneStore.update,
      contextMenuUpdater,
    ]);

  let subscriptions = (state: Model.State.t) => {
    let config = Feature_Configuration.resolver(state.config);
    let visibleBuffersAndRanges =
      state |> Model.EditorVisibleRanges.getVisibleBuffersAndRanges;

    let isInsertMode = Feature_Vim.mode(state.vim) == Vim.Types.Insert;

    let visibleRanges =
      visibleBuffersAndRanges
      |> List.map(((bufferId, ranges)) => {
           Model.Selectors.getBufferById(state, bufferId)
           |> Option.map(buffer => {(buffer, ranges)})
         })
      |> Core.Utility.OptionEx.values;

    let visibleBuffers =
      visibleBuffersAndRanges
      |> List.map(fst)
      |> Base.List.dedup_and_sort(~compare)
      |> List.map(bufferId => Model.Selectors.getBufferById(state, bufferId))
      |> Core.Utility.OptionEx.values;

    let syntaxSubscription =
      shouldSyntaxHighlight && !state.isQuitting
        ? Feature_Syntax.subscription(
            ~config,
            ~grammarInfo,
            ~languageInfo,
            ~setup,
            ~tokenTheme=state.tokenTheme,
            ~bufferVisibility=visibleRanges,
            state.syntaxHighlights,
          )
          |> Isolinear.Sub.map(msg => Model.Actions.Syntax(msg))
        : Isolinear.Sub.none;

    let terminalSubscription =
      Feature_Terminal.subscription(
        ~workspaceUri=Core.Uri.fromPath(state.workspace.workingDirectory),
        extHostClient,
        state.terminals,
      )
      |> Isolinear.Sub.map(msg => Model.Actions.Terminal(msg));

    let fontFamily =
      Oni_Core.Configuration.getValue(
        c => c.editorFontFile,
        state.configuration,
      );
    let fontSize =
      Oni_Core.Configuration.getValue(
        c => c.editorFontSize,
        state.configuration,
      );
    let fontSmoothing =
      Oni_Core.Configuration.getValue(
        c => c.editorFontSmoothing,
        state.configuration,
      );
    let fontLigatures =
      Oni_Core.Configuration.getValue(
        c => c.editorFontLigatures,
        state.configuration,
      );
    let editorFontSubscription =
      Service_Font.Sub.font(
        ~uniqueId="editorFont",
        ~fontFamily,
        ~fontSize,
        ~fontSmoothing,
        ~fontLigatures,
      )
      |> Isolinear.Sub.map(msg => Model.Actions.EditorFont(msg));

    let terminalFontFamily =
      Oni_Core.Configuration.getValue(
        c => c.terminalIntegratedFontFile,
        state.configuration,
      );
    let terminalFontSize =
      Oni_Core.Configuration.getValue(
        c => c.terminalIntegratedFontSize,
        state.configuration,
      );
    let terminalFontSmoothing =
      Oni_Core.Configuration.getValue(
        c => c.terminalIntegratedFontSmoothing,
        state.configuration,
      );
    let terminalFontSubscription =
      Service_Font.Sub.font(
        ~uniqueId="terminalFont",
        ~fontFamily=terminalFontFamily,
        ~fontSize=terminalFontSize,
        ~fontSmoothing=terminalFontSmoothing,
        ~fontLigatures,
      )
      |> Isolinear.Sub.map(msg => Model.Actions.TerminalFont(msg));

    let visibleEditors = Feature_Layout.visibleEditors(state.layout);

    let activeEditor = Feature_Layout.activeEditor(state.layout);
    let activeEditorId = Feature_Editor.Editor.getId(activeEditor);
    let activeBufferId = Feature_Editor.Editor.getBufferId(activeEditor);
    let activePosition = Feature_Editor.Editor.getPrimaryCursor(activeEditor);
    let maybeActiveBuffer =
      Model.Buffers.getBuffer(activeBufferId, state.buffers);

    let extHostSubscription =
      Feature_Exthost.subscription(
        ~buffers=visibleBuffers,
        ~editors=visibleEditors,
        ~activeEditorId=Some(activeEditorId),
        ~client=extHostClient,
      )
      |> Isolinear.Sub.map(() => Model.Actions.Noop);

    let fileExplorerActiveFileSub =
      Model.Sub.activeFile(
        ~id="activeFile.fileExplorer", ~state, ~toMsg=maybeFilePath =>
        Model.Actions.FileExplorer(ActiveFilePathChanged(maybeFilePath))
      );

    let languageSupportSub =
      maybeActiveBuffer
      |> Option.map(activeBuffer => {
           Feature_LanguageSupport.sub(
             ~isInsertMode,
             ~activeBuffer,
             ~activePosition,
             ~visibleBuffers,
             ~client=extHostClient,
             state.languageSupport,
           )
           |> Isolinear.Sub.map(msg => Model.Actions.LanguageSupport(msg))
         })
      |> Option.value(~default=Isolinear.Sub.none);

    let editorGlobalSub =
      Feature_Editor.Sub.global(~config)
      |> Isolinear.Sub.map(msg =>
           Model.Actions.Editor({scope: Model.EditorScope.All, msg})
         );

    let extensionsSub =
      Feature_Extensions.sub(~setup, state.extensions)
      |> Isolinear.Sub.map(msg => Model.Actions.Extensions(msg));

    let registersSub =
      Feature_Registers.sub(state.registers)
      |> Isolinear.Sub.map(msg => Model.Actions.Registers(msg));

    [
      languageSupportSub,
      syntaxSubscription,
      terminalSubscription,
      editorFontSubscription,
      terminalFontSubscription,
      extHostSubscription,
      Isolinear.Sub.batch(VimStoreConnector.subscriptions(state)),
      fileExplorerActiveFileSub,
      editorGlobalSub,
      extensionsSub,
      registersSub,
    ]
    |> Isolinear.Sub.batch;
  };

  module Store =
    Isolinear.Store.Make({
      type msg = Model.Actions.t;
      type model = Model.State.t;

      let initial = getState();
      let updater = updater;
      let subscriptions = subscriptions;
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
        if (Model.Buffers.anyModified(getState().buffers)) {
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
    Feature_Hover.Contributions.commands
    |> List.map(Core.Command.map(msg => Model.Actions.Hover(msg))),
    Feature_SignatureHelp.Contributions.commands
    |> List.map(Core.Command.map(msg => Model.Actions.SignatureHelp(msg))),
    Feature_Theme.Contributions.commands
    |> List.map(Core.Command.map(msg => Model.Actions.Theme(msg))),
    Feature_Clipboard.Contributions.commands
    |> List.map(Core.Command.map(msg => Model.Actions.Clipboard(msg))),
    Feature_Registers.Contributions.commands
    |> List.map(Core.Command.map(msg => Model.Actions.Registers(msg))),
    Feature_LanguageSupport.Contributions.commands
    |> List.map(Core.Command.map(msg => Model.Actions.LanguageSupport(msg))),
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
    Revery.Tick.interval(_ => runEffects(), Revery.Time.zero);

  (dispatch, runEffects);
};
