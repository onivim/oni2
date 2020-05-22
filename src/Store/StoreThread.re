/*
 * StoreThread.re
 *
 * This is the 'state management' piece of Oni2.
 *
 * The state updates are run in a parallel thread to the rendering,
 * so that we can eek out as much perf as we can in this architecture.
 */

module Core = Oni_Core;

module Extensions = Oni_Extensions;
module Model = Oni_Model;

open Oni_Extensions;
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
          Utility.getUserExtensions(~overriddenExtensionsDir);

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
  let languageInfo = LanguageInfo.ofExtensions(extensions);
  let themeInfo = Model.ThemeInfo.ofExtensions(extensions);
  let grammarRepository = Oni_Syntax.GrammarRepository.create(languageInfo);

  let commandUpdater = CommandStoreConnector.start();
  let (vimUpdater, vimStream) =
    VimStoreConnector.start(
      ~showUpdateChangelog,
      languageInfo,
      getState,
      getClipboardText,
      setClipboardText,
    );

  let themeUpdater = ThemeStoreConnector.start(themeInfo);

  let (extHostClientResult, extHostStream) =
    ExtensionClient.create(~config=getState().config, ~extensions, ~setup);

  // TODO: How to handle this correctly?
  let extHostClient = extHostClientResult |> Result.get_ok;

  let extHostUpdater =
    ExtensionClientStoreConnector.start(extensions, extHostClient);

  let quickmenuUpdater = QuickmenuStoreConnector.start(themeInfo);

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

  let lifecycleUpdater = LifecycleStoreConnector.start(quit);
  let indentationUpdater = IndentationStoreConnector.start();
  let windowUpdater = WindowsStoreConnector.start();

  let completionUpdater = CompletionStoreConnector.start();

  let languageFeatureUpdater = LanguageFeatureConnector.start();

  let (inputUpdater, inputStream) =
    InputStoreConnector.start(window, runRunEffects);

  let titleUpdater =
    TitleStoreConnector.start(setTitle, maximize, minimize, restore, close);
  let sneakUpdater = SneakStore.start();
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
      languageFeatureUpdater,
      completionUpdater,
      titleUpdater,
      sneakUpdater,
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
    let visibleRanges =
      state
      |> Model.EditorVisibleRanges.getVisibleBuffersAndRanges
      |> List.map(((bufferId, ranges)) => {
           Model.Selectors.getBufferById(state, bufferId)
           |> Option.map(buffer => {(buffer, ranges)})
         })
      |> Core.Utility.OptionEx.values;
    let syntaxSubscription =
      shouldSyntaxHighlight && !state.isQuitting
        ? Feature_Syntax.subscription(
            ~config,
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
        c => c.editorFontFamily,
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
    let editorFontSubscription =
      Service_Font.Sub.font(
        ~uniqueId="editorFont",
        ~fontFamily,
        ~fontSize,
        ~fontSmoothing,
      )
      |> Isolinear.Sub.map(msg => Model.Actions.EditorFont(msg));

    let terminalFontFamily =
      Oni_Core.Configuration.getValue(
        c => c.terminalIntegratedFontFamily,
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
      )
      |> Isolinear.Sub.map(msg => Model.Actions.TerminalFont(msg));

    [
      syntaxSubscription,
      terminalSubscription,
      editorFontSubscription,
      terminalFontSubscription,
      Isolinear.Sub.batch(VimStoreConnector.subscriptions(state)),
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
    Store.onBeforeMsg(msg => {DispatchLog.info(Model.Actions.show(msg))});

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

  registerCommands(~dispatch, Model.GlobalCommands.registrations());
  registerCommands(
    ~dispatch,
    Feature_Terminal.Contributions.commands
    |> List.map(Core.Command.map(msg => Model.Actions.Terminal(msg))),
  );

  // TODO: These should all be replaced with isolinear subscriptions.
  let _: Isolinear.unsubscribe =
    Isolinear.Stream.connect(dispatch, inputStream);
  let _: Isolinear.unsubscribe =
    Isolinear.Stream.connect(dispatch, vimStream);
  let _: Isolinear.unsubscribe =
    Isolinear.Stream.connect(dispatch, extHostStream);

  dispatch(Model.Actions.SetLanguageInfo(languageInfo));

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
