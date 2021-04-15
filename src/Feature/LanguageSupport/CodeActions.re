open EditorCoreTypes;
open Oni_Core;
open Exthost;

type provider = {
  handle: int,
  selector: DocumentSelector.t,
  metadata: CodeAction.ProviderMetadata.t,
  displayName: string,
  supportsResolve: bool,
};

module Configuration = {
  open Config.Schema;

  let enabled = setting("editor.lightBulb.enabled", bool, ~default=false);
};

module CodeAction = {
  type t = {
    handle: int,
    action: Exthost.CodeAction.t,
  };

  let ofExthost = (~handle, action) => {handle, action};

  let intersects = (pos: CharacterPosition.t, {action, _}) => {
    let diagnostics: list(Exthost.Diagnostic.t) =
      Exthost.CodeAction.(action.diagnostics);
    diagnostics
    |> List.exists((diag: Exthost.Diagnostic.t) => {
         diag.range
         |> Exthost.OneBasedRange.toRange
         |> CharacterRange.contains(pos)
       });
  };
};

type positionOrSelection =
  | Position(CharacterPosition.t)
  | Selection({
      cursor: CharacterPosition.t,
      range: CharacterRange.t,
    });

module Session = {
  type t =
    | Idle
    | QueryingForFixes({
        editorId: int,
        handles: list(int),
        buffer: Oni_Core.Buffer.t,
        positionOrSelection,
        results: IntMap.t(list(CodeAction.t)),
      })
    | GatheringFixes({
        editorId: int,
        buffer: Oni_Core.Buffer.t,
        handles: list(int),
        positionOrSelection,
        results: IntMap.t(list(CodeAction.t)),
      })
    | ApplyingFixes({
        editorId: int,
        positionOrSelection,
        contextMenu: Component_EditorContextMenu.model(CodeAction.t),
      });

  let initial = Idle;

  [@deriving show]
  type msg =
    | ContextMenu([@opaque] Component_EditorContextMenu.msg(CodeAction.t))
    | CodeActionsAvailable({
        handle: int,
        codeActions: [@opaque] list(CodeAction.t),
      })
    | NoCodeActionsAvailable({handle: int})
    | ErrorRetrievingCodeActions({
        handle: int,
        msg: string,
      });

  let handles =
    fun
    | Idle => []
    | GatheringFixes({handles, _}) => handles
    | QueryingForFixes({handles, _}) => handles
    | ApplyingFixes(_) => [];

  let buffer =
    fun
    | Idle => None
    | GatheringFixes({buffer, _}) => Some(buffer)
    | QueryingForFixes({buffer, _}) => Some(buffer)
    | ApplyingFixes(_) => None;

  let toPosition =
    fun
    | Position(pos) => pos
    | Selection({cursor, _}) => cursor;

  let hasAnyFixes =
    fun
    | Idle => false
    | GatheringFixes({results, _})
    | QueryingForFixes({results, _}) =>
      results |> IntMap.exists((_handle, items) => items != [])
    | ApplyingFixes(_) => false;

  let lightBulb = session =>
    switch (session) {
    | QueryingForFixes({positionOrSelection, _}) when hasAnyFixes(session) =>
      Some(toPosition(positionOrSelection))
    | _ => None
    };

  let position =
    fun
    | Idle => None
    | GatheringFixes({positionOrSelection, _})
    | QueryingForFixes({positionOrSelection, _})
    | ApplyingFixes({positionOrSelection, _}) =>
      Some(toPosition(positionOrSelection));

  let contextMenu =
    fun
    | ApplyingFixes({contextMenu, _}) => Some(contextMenu)
    | _ => None;

  let sub = (~client, session) => {
    let toMsg = handle =>
      fun
      | Ok(Some(actionsList: Exthost.CodeAction.List.t)) =>
        CodeActionsAvailable({
          handle,
          codeActions:
            actionsList.actions |> List.map(CodeAction.ofExthost(~handle)),
        })
      | Ok(None) => NoCodeActionsAvailable({handle: handle})
      | Error(msg) => ErrorRetrievingCodeActions({handle, msg});

    let maybeBuffer = buffer(session);
    // TODO: Seleciton
    let maybePosition = position(session);
    let handles = handles(session);
    //let isLightBulbEnabled = Configuration.enabled.get(config);

    let context =
      Exthost.CodeAction.(Context.{only: None, trigger: TriggerType.Auto});

    Utility.OptionEx.map2(
      (buffer, position) => {
        handles
        |> List.map(handle => {
             let range =
               EditorCoreTypes.CharacterRange.{
                 start: position,
                 stop: position,
               };
             Service_Exthost.Sub.codeActionsByRange(
               ~handle,
               ~range,
               ~buffer,
               ~context,
               ~toMsg=toMsg(handle),
               client,
             );
           })
        |> Isolinear.Sub.batch
      },
      maybeBuffer,
      maybePosition,
    )
    |> Option.value(~default=Isolinear.Sub.none);
  };

  let allFixesAreAvailable = (handles, results) => {
    handles |> List.for_all(handle => IntMap.mem(handle, results));
  };

  let doFix = (~editorId, ~positionOrSelection, results) => {
    let allFixes = IntMap.fold((_key, a, b) => {a @ b}, results, []);
    let len = List.length(allFixes);
    if (len == 0) {
      (Idle, Outmsg.NotifyFailure("No code actions available."));
    } else if (len == 1) {
      let maybeEdit =
        List.nth_opt(allFixes, 0)
        |> Utility.OptionEx.flatMap((fix: CodeAction.t) =>
             Exthost.CodeAction.(fix.action.edit)
           );
      let outmsg =
        maybeEdit
        |> Option.map((edit: Exthost.WorkspaceEdit.t) =>
             Outmsg.ApplyWorkspaceEdit(edit)
           )
        |> Option.value(~default=Outmsg.Nothing);
      (Idle, outmsg);
    } else {
      let toString = (codeAction: CodeAction.t) => {
        Exthost.CodeAction.(codeAction.action.title);
      };

      let renderer =
        Component_EditorContextMenu.Schema.Renderer.default(~toString);
      let schema = Component_EditorContextMenu.Schema.contextMenu(~renderer);
      let contextMenu = Component_EditorContextMenu.create(~schema, allFixes);
      (
        ApplyingFixes({positionOrSelection, editorId, contextMenu}),
        Outmsg.Nothing,
      );
    };
  };

  let addToResults = (~handle, codeActions) =>
    fun
    | Idle => (Idle, Outmsg.Nothing)
    | ApplyingFixes(_) as af => (af, Outmsg.Nothing)
    | QueryingForFixes({results, _} as orig) => (
        QueryingForFixes({
          ...orig,
          results: IntMap.add(handle, codeActions, results),
        }),
        Outmsg.Nothing,
      )
    | GatheringFixes(
        {results, handles, positionOrSelection, editorId, _} as orig,
      ) => {
        let results' = IntMap.add(handle, codeActions, results);

        if (allFixesAreAvailable(handles, results')) {
          doFix(~editorId, ~positionOrSelection, results');
        } else {
          (GatheringFixes({...orig, results: results'}), Outmsg.Nothing);
        };
      };

  let update = (msg, model) => {
    switch (msg) {
    | CodeActionsAvailable({handle, codeActions}) =>
      addToResults(~handle, codeActions, model)

    | NoCodeActionsAvailable({handle}) => addToResults(~handle, [], model)

    | ErrorRetrievingCodeActions({handle, _}) =>
      addToResults(~handle, [], model)

    | ContextMenu(contextMenuMsg) =>
      switch (model) {
      | ApplyingFixes({contextMenu, _} as orig) =>
        let (contextMenu, outmsg) =
          Component_EditorContextMenu.update(contextMenuMsg, contextMenu);

        let model' = ApplyingFixes({...orig, contextMenu});
        switch (outmsg) {
        | Nothing => (model', Outmsg.Nothing)
        | Cancelled => (Idle, Outmsg.Nothing)
        | Selected(item) => (
            Idle,
            Exthost.CodeAction.(item.action.edit)
            |> Option.map(edit => {Outmsg.ApplyWorkspaceEdit(edit)})
            |> Option.value(~default=Outmsg.Nothing),
          )
        };
      | other => (other, Outmsg.Nothing)
      }
    };
  };

  let editorId =
    fun
    | Idle => None
    | ApplyingFixes({editorId, _})
    | GatheringFixes({editorId, _})
    | QueryingForFixes({editorId, _}) => Some(editorId);

  let stop = _session => Idle;

  let checkLightBulb =
      (~editorId, ~buffer, ~handles, ~positionOrSelection, _session) => {
    QueryingForFixes({
      editorId,
      buffer,
      positionOrSelection,
      handles,
      results: IntMap.empty,
    });
  };

  let expandOrApply =
      (~editorId, ~buffer, ~handles, ~positionOrSelection, session) => {
    let results =
      switch (session) {
      | QueryingForFixes({
          editorId as prevEditorId,
          buffer as prevBuffer,
          positionOrSelection as prevPosition,
          results,
          _,
        })
          when
            prevEditorId == editorId
            && Buffer.getId(buffer) == Buffer.getId(prevBuffer)
            && positionOrSelection == prevPosition => results
      | _ => IntMap.empty
      };

    if (allFixesAreAvailable(handles, results)) {
      doFix(~editorId, ~positionOrSelection, results);
    } else {
      (
        GatheringFixes({
          editorId,
          buffer,
          handles,
          positionOrSelection,
          results,
        }),
        Outmsg.Nothing,
      );
    };
  };

  let next =
    fun
    | ApplyingFixes({contextMenu, _} as orig) => {
        let contextMenu' = Component_EditorContextMenu.next(contextMenu);
        (
          ApplyingFixes({...orig, contextMenu: contextMenu'}),
          Outmsg.Nothing,
        );
      }
    | model => (model, Outmsg.Nothing);

  let previous =
    fun
    | ApplyingFixes({contextMenu, _} as orig) => {
        let contextMenu' = Component_EditorContextMenu.previous(contextMenu);
        (
          ApplyingFixes({...orig, contextMenu: contextMenu'}),
          Outmsg.Nothing,
        );
      }
    | model => (model, Outmsg.Nothing);

  let select =
    fun
    | ApplyingFixes({contextMenu, _}) => {
        let maybeSelected = Component_EditorContextMenu.selected(contextMenu);
        switch (maybeSelected) {
        | None => (Idle, Outmsg.Nothing)
        | Some(item: CodeAction.t) =>
          let maybeEdit = Exthost.CodeAction.(item.action.edit);

          let outmsg =
            maybeEdit
            |> Option.map(edit => Outmsg.ApplyWorkspaceEdit(edit))
            |> Option.value(~default=Outmsg.Nothing);
          (Idle, outmsg);
        };
      }
    | model => (model, Outmsg.Nothing);
};

module ViewModel = {
  open Component_Animation;
  type t = {
    popupAnimation: Animator.t(Session.t, [ | `PopupOpen | `PopupClosed]),
  };

  let initial = {
    popupAnimation:
      Animator.create(~initial=`PopupClosed, session => {
        switch (Session.lightBulb(session)) {
        | None => `PopupClosed
        | Some(_) => `PopupOpen
        }
      }),
  };

  type msg =
    | PopupAnimation(Animator.msg);

  let sub = ({popupAnimation}) => {
    popupAnimation
    |> Animator.sub
    |> Isolinear.Sub.map(msg => PopupAnimation(msg));
  };

  let sync = (~isAnimationEnabled, session, viewModel) => {
    {
      popupAnimation:
        Animator.set(
          ~instant=!isAnimationEnabled,
          session,
          viewModel.popupAnimation,
        ),
    };
  };

  let update = (msg, viewModel) => {
    switch (msg) {
    | PopupAnimation(popupAnimationMsg) => {
        popupAnimation:
          Animator.update(popupAnimationMsg, viewModel.popupAnimation),
      }
    };
  };
};

type model = {
  providers: list(provider),
  session: Session.t,
  isAnimationEnabled: bool,
  isLightBulbEnabled: bool,
  viewModel: ViewModel.t,
};

[@deriving show]
type command =
  | QuickFix
  | AcceptSelected
  | SelectPrevious
  | SelectNext
  | Cancel;

[@deriving show]
type msg =
  | Command(command)
  | Session(Session.msg)
  | ViewModel([@opaque] ViewModel.msg);

let initial = {
  providers: [],
  session: Session.initial,
  isLightBulbEnabled: true,
  isAnimationEnabled: true,
  viewModel: ViewModel.initial,
};

let configurationChanged = (~config, model) => {
  ...model,
  isLightBulbEnabled: Configuration.enabled.get(config),
  isAnimationEnabled:
    Feature_Configuration.GlobalConfiguration.animation.get(config),
};

let register =
    (~handle, ~selector, ~metadata, ~displayName, ~supportsResolve, model) => {
  {
    ...model,
    providers: [
      {handle, selector, metadata, displayName, supportsResolve},
      ...model.providers,
    ],
  };
};

let cursorMoved = (~editorId, ~buffer, ~cursor, model) =>
  if (model.isLightBulbEnabled) {
    let handles =
      model.providers
      |> List.filter(({selector, _}) => {
           Exthost.DocumentSelector.matchesBuffer(~buffer, selector)
         })
      |> List.map(({handle, _}) => handle);

    let session' =
      Session.checkLightBulb(
        ~editorId,
        ~buffer,
        ~positionOrSelection=Position(cursor),
        ~handles,
        model.session,
      );
    {
      ...model,
      session: session',
      viewModel:
        ViewModel.sync(
          ~isAnimationEnabled=model.isAnimationEnabled,
          session',
          model.viewModel,
        ),
    };
  } else {
    model;
  };

let unregister = (~handle, model) => {
  ...model,
  providers:
    model.providers |> List.filter(provider => {provider.handle != handle}),
};

let update = (~editorId, ~buffer, ~cursorLocation, msg, model) => {
  let (model', outmsg) =
    switch (msg) {
    | ViewModel(msg) => (
        {...model, viewModel: ViewModel.update(msg, model.viewModel)},
        Outmsg.Nothing,
      )

    | Session(sessionMsg) =>
      let (session', outmsg) = Session.update(sessionMsg, model.session);
      ({...model, session: session'}, outmsg);

    | Command(Cancel) => (
        {...model, session: Session.stop(model.session)},
        Outmsg.Nothing,
      )

    | Command(AcceptSelected) =>
      let (session', outmsg) = Session.select(model.session);
      ({...model, session: session'}, outmsg);

    | Command(SelectNext) =>
      let (session', outmsg) = Session.next(model.session);
      ({...model, session: session'}, outmsg);

    | Command(SelectPrevious) =>
      let (session', outmsg) = Session.previous(model.session);
      ({...model, session: session'}, outmsg);

    | Command(QuickFix) =>
      let handles =
        model.providers
        |> List.filter(({selector, _}) => {
             Exthost.DocumentSelector.matchesBuffer(~buffer, selector)
           })
        |> List.map(({handle, _}) => handle);

      // TODO: if handles are empty, show error
      let (session', outmsg) =
        Session.expandOrApply(
          ~editorId,
          ~buffer,
          ~handles,
          ~positionOrSelection=Position(cursorLocation),
          model.session,
        );
      ({...model, session: session'}, outmsg);
    };

  (
    {
      ...model',
      viewModel:
        ViewModel.sync(
          ~isAnimationEnabled=model.isAnimationEnabled,
          model'.session,
          model'.viewModel,
        ),
    },
    outmsg,
  );
};

let sub =
    (
      ~config as _,
      ~buffer as _,
      ~activeEditor as _,
      ~activePosition as _,
      ~topVisibleBufferLine as _,
      ~bottomVisibleBufferLine as _,
      ~lineHeightInPixels as _,
      ~positionToRelativePixel as _,
      ~client,
      codeActions,
    ) => {
  let viewModelSub =
    ViewModel.sub(codeActions.viewModel)
    |> Isolinear.Sub.map(msg => ViewModel(msg));

  let sessionSub =
    Session.sub(~client, codeActions.session)
    |> Isolinear.Sub.map(msg => Session(msg));

  [viewModelSub, sessionSub] |> Isolinear.Sub.batch;
};

module Commands = {
  open Feature_Commands.Schema;

  let quickFix =
    define(
      ~category="Language Support",
      ~title="Quick Fix",
      "editor.action.quickFix",
      Command(QuickFix),
    );

  let acceptContextItem = define("acceptQuickFix", Command(AcceptSelected));

  let selectPrevContextItem =
    define("selectPrevQuickFix", Command(SelectPrevious));

  let selectNextContextItem =
    define("selectNextQuickFix", Command(SelectNext));

  let cancel = define("cancelQuickFix", Command(Cancel));
};

module Keybindings = {
  open Feature_Input.Schema;

  let isMac = Revery.Environment.isMac;

  let condition = "editorTextFocus && normalMode" |> WhenExpr.parse;

  let contextMenuCondition =
    WhenExpr.And([
      WhenExpr.Defined("editorTextFocus"),
      WhenExpr.Defined("quickFixWidgetVisible"),
      WhenExpr.Or([
        WhenExpr.Defined("normalMode"),
        WhenExpr.Defined("visualMode"),
      ]),
    ]);

  let quickFix =
    bind(
      ~key=isMac ? "<D-.>" : "<C-.>",
      ~command=Commands.quickFix.id,
      ~condition,
    );

  let nextSuggestion =
    bind(
      ~key="<C-N>",
      ~command=Commands.selectNextContextItem.id,
      ~condition=contextMenuCondition,
    );

  let nextSuggestionArrow =
    bind(
      ~key="<DOWN>",
      ~command=Commands.selectNextContextItem.id,
      ~condition=contextMenuCondition,
    );

  let previousSuggestion =
    bind(
      ~key="<C-P>",
      ~command=Commands.selectPrevContextItem.id,
      ~condition=contextMenuCondition,
    );
  let previousSuggestionArrow =
    bind(
      ~key="<UP>",
      ~command=Commands.selectPrevContextItem.id,
      ~condition=contextMenuCondition,
    );

  let acceptSuggestionEnter =
    bind(
      ~key="<CR>",
      ~command=Commands.acceptContextItem.id,
      ~condition=contextMenuCondition,
    );

  let acceptSuggestionTab =
    bind(
      ~key="<TAB>",
      ~command=Commands.acceptContextItem.id,
      ~condition=contextMenuCondition,
    );

  let cancelEscape =
    bind(
      ~key="<ESC>",
      ~command=Commands.cancel.id,
      ~condition=contextMenuCondition,
    );
};

module Contributions = {
  let commands = _model => {
    Commands.[
      quickFix,
      acceptContextItem,
      selectNextContextItem,
      selectPrevContextItem,
      cancel,
    ];
  };

  let configuration = Configuration.[enabled.spec];

  let keybindings =
    Keybindings.[
      quickFix,
      acceptSuggestionEnter,
      acceptSuggestionTab,
      nextSuggestionArrow,
      nextSuggestion,
      previousSuggestion,
      previousSuggestionArrow,
      cancelEscape,
    ];

  let contextKeys = model => {
    WhenExpr.ContextKeys.(
      [
        Schema.bool("quickFixWidgetVisible", ({session, _}) => {
          Session.contextMenu(session) != None
        }),
      ]
      |> Schema.fromList
      |> fromSchema(model)
    );
  };
};

module View = {
  module EditorWidgets = {
    let make =
        (
          ~x,
          ~y,
          ~editorId,
          ~theme,
          ~editorFont: Service_Font.font,
          ~model,
          (),
        ) =>
      if (Some(editorId) == Session.editorId(model.session)) {
        model.viewModel.popupAnimation
        |> Component_Animation.Animator.render((~prev, ~next, v) => {
             let render = opac =>
               if (opac < 0.1) {
                 Revery.UI.React.empty;
               } else {
                 let foregroundColor =
                   Feature_Theme.Colors.Editor.lightBulbForeground.from(theme)
                   |> Revery.Color.multiplyAlpha(opac);

                 let fontSize = editorFont.fontSize;
                 <Revery.UI.View
                   style=Revery.UI.Style.[
                     position(`Absolute),
                     top(y),
                     left(x),
                     transform([
                       Revery.UI.Transform.TranslateY(8.0 *. (1.0 -. opac)),
                       Revery.UI.Transform.RotateX(
                         Revery.Math.Angle.Degrees((-60.0) *. (1.0 -. opac)),
                       ),
                     ]),
                   ]>
                   <Revery.UI.Components.Container
                     width=24 height=24 color=Revery.Colors.transparentWhite>
                     <Oni_Core.Codicon
                       fontSize
                       color=foregroundColor
                       icon=Codicon.lightbulb
                     />
                   </Revery.UI.Components.Container>
                 </Revery.UI.View>;
               };

             switch (prev, next) {
             | (`PopupOpen, `PopupOpen) => render(1.0)
             | (`PopupClosed, `PopupClosed) => render(0.0)
             | (`PopupOpen, `PopupClosed) => render(1.0 -. v)
             | (`PopupClosed, `PopupOpen) => render(v)
             };
           });
      } else {
        Revery.UI.React.empty;
      };
  };

  module Overlay = {
    let make =
        (~toPixel, ~dispatch, ~theme, ~uiFont, ~editorFont as _, ~model, ()) => {
      let maybeEditorId = Session.editorId(model.session);
      let maybePosition = Session.position(model.session);

      let maybePixel =
        Utility.OptionEx.map2(
          (editorId, position) => {(editorId, position)},
          maybeEditorId,
          maybePosition,
        )
        |> Utility.OptionEx.flatMap(((editorId, position)) => {
             toPixel(~editorId, position)
           });

      let maybeContextMenu = Session.contextMenu(model.session);
      Utility.OptionEx.map2(
        (pixelPosition, contextMenu) => {
          <Component_EditorContextMenu.View
            pixelPosition
            theme
            model=contextMenu
            dispatch={msg => dispatch(Session(ContextMenu(msg)))}
            uiFont
          />
        },
        maybePixel,
        maybeContextMenu,
      )
      |> Option.value(~default=Revery.UI.React.empty);
    };
  };
};
