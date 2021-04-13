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
             prerr_endline(
               "Sub - querying handle: " ++ string_of_int(handle),
             );
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
    prerr_endline("All fixes available!");
    let allFixes = IntMap.fold((key, a, b) => {a @ b}, results, []);
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

        prerr_endline("Gathering fixes...");
        if (allFixesAreAvailable(handles, results')) {
          doFix(~editorId, ~positionOrSelection, results');
        } else {
          (GatheringFixes({...orig, results: results'}), Outmsg.Nothing);
        };
      };

  let update = (msg, model) => {
    prerr_endline("Session.update: " ++ show_msg(msg));
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
        | FocusChanged(_) => (model', Outmsg.Nothing)
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
      (~editorId, ~buffer, ~handles, ~positionOrSelection, session) => {
    prerr_endline("Checking light bulb...");
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
};

// module QuickFixes = {
//   type t = {
//     bufferId: int,
//     position: EditorCoreTypes.CharacterPosition.t,
//     fixes: list(CodeAction.t),
//   };

//   let addQuickFixes = (~handle, ~bufferId, ~position, ~newActions, quickFixes) => {
//     let actions = newActions |> List.map(CodeAction.ofExthost(~handle));
// If at the same location, just append!
//     let fixes =
//       if (bufferId == quickFixes.bufferId && position == quickFixes.position) {
//         quickFixes.fixes @ actions;
//       } else {
// If not, replace
//         actions;
//       };

//     {bufferId, position, fixes};
//   };

//   let initial = {
//     bufferId: (-1),
//     position: EditorCoreTypes.CharacterPosition.zero,
//     fixes: [],
//   };

//   let cursorMoved = (~bufferId, ~position, qf) =>
// Totally different buffer, reset...
//     if (bufferId != qf.bufferId) {
//       {...initial, bufferId, position};
//     } else if (position != qf.position) {
//       {
//         ...qf,
//         position,
//         fixes: qf.fixes |> List.filter(CodeAction.intersects(position)),
//       };
//     } else {
//       qf;
//     };

//   let any = ({fixes, _}) => fixes != [];

//   let appliesToBuffer = (buffer, {bufferId, _}) => {
//     bufferId == Buffer.getId(buffer);
//   };

//   let position = ({fixes, position, _}) => {
//     Base.List.nth(fixes, 0)
//     |> Utility.OptionEx.flatMap((fix: CodeAction.t) => {
//          Base.List.nth(fix.action.diagnostics, 0)
//        })
//     |> Option.map((diagnostic: Exthost.Diagnostic.t) => {
//          let range = diagnostic.range |> Exthost.OneBasedRange.toRange;

//          CharacterRange.(range.start);
//        })
//     |> Utility.OptionEx.or_lazy(() =>
//          if (fixes == []) {
//            None;
//          } else {
//            Some(position);
//          }
//        );
//   };

//   let all = ({fixes, _}) => fixes;
// };

type model = {
  providers: list(provider),
  // quickFixes: QuickFixes.t,
  session: Session.t,
  // lightBulbActiveEditorId: option(int),
  // quickFixContextMenu:
  //   option(Component_EditorContextMenu.model(CodeAction.t)),
};

[@deriving show]
type command =
  | QuickFix;

[@deriving show]
type msg =
  | Command(command)
  | Session(Session.msg);
// | QuickFixContextMenu(
//     [@opaque] Component_EditorContextMenu.msg(CodeAction.t),
//   )
// | QuickFixesAvailable({
//     handle: int,
//     actions: list(Exthost.CodeAction.t),
//   })
// | QuickFixesNotAvailable({handle: int})
// | QuickFixesError({
//     handle: int,
//     msg: string,
//   });

let initial = {
  providers: [],
  session: Session.initial,
  // quickFixes: QuickFixes.initial,
  // quickFixContextMenu: None,
  // lightBulbActiveEditorId: None,
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

let cursorMoved = (~editorId, ~buffer, ~cursor, model) => {
  let handles =
    model.providers
    |> List.filter(({selector, _}) => {
         Exthost.DocumentSelector.matchesBuffer(~buffer, selector)
       })
    |> List.map(({handle, _}) => handle);
  {
    ...model,
    session:
      Session.checkLightBulb(
        ~editorId,
        ~buffer,
        ~positionOrSelection=Position(cursor),
        ~handles,
        model.session,
      ),
  };
};

let unregister = (~handle, model) => {
  ...model,
  providers:
    model.providers |> List.filter(provider => {provider.handle != handle}),
};

let update = (~editorId, ~buffer, ~cursorLocation, msg, model) => {
  let bufferId = Buffer.getId(buffer);
  switch (msg) {
  | Session(sessionMsg) =>
    let (session', outmsg) = Session.update(sessionMsg, model.session);
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
  // TODO: Revive with session
  // let all = QuickFixes.all(model.quickFixes);
  // let allCount = List.length(all);
  // if (allCount == 0) {
  //   (model, Outmsg.NotifyFailure("No quickfixes available here."));
  // } else if (allCount == 1) {
  //   let maybeEdit =
  //     List.nth_opt(all, 0)
  //     |> Utility.OptionEx.flatMap((fix: CodeAction.t) =>
  //          Exthost.CodeAction.(fix.action.edit)
  //        );
  //   let outmsg =
  //     maybeEdit
  //     |> Option.map((edit: Exthost.WorkspaceEdit.t) =>
  //          Outmsg.ApplyWorkspaceEdit(edit)
  //        )
  //     |> Option.value(~default=Outmsg.Nothing);
  //   (model, outmsg);
  // } else {
  //   let toString = (codeAction: CodeAction.t) => {
  //     Exthost.CodeAction.(codeAction.action.title);
  //   };
  //   let renderer =
  //     Component_EditorContextMenu.Schema.Renderer.default(~toString);
  //   let schema = Component_EditorContextMenu.Schema.contextMenu(~renderer);
  //   let quickFixMenu =
  //     Component_EditorContextMenu.create(~schema, all) |> Option.some;
  //   ({...model, quickFixContextMenu: quickFixMenu}, Outmsg.Nothing);
  // };
  // | QuickFixContextMenu(contextMenuMsg) =>
  //   let (model', eff) =
  //     model.quickFixContextMenu
  //     |> Option.map(contextMenu => {
  //          let (model', outmsg) =
  //            Component_EditorContextMenu.update(contextMenuMsg, contextMenu);
  //          let (model'', eff) =
  //            switch (outmsg) {
  //            | Nothing => (Some(model'), Outmsg.Nothing)
  //            | Cancelled => (None, Outmsg.Nothing)
  //            | FocusChanged(_) => (Some(model'), Outmsg.Nothing)
  //            | Selected(item) => (
  //                None,
  //                Exthost.CodeAction.(item.action.edit)
  //                |> Option.map(edit => {Outmsg.ApplyWorkspaceEdit(edit)})
  //                |> Option.value(~default=Outmsg.Nothing),
  //              )
  //            };
  //          ({...model, quickFixContextMenu: model''}, eff);
  //        })
  //     |> Option.value(~default=(model, Outmsg.Nothing));
  //   (model', eff);
  // | QuickFixesAvailable({handle, actions}) => (
  //     {
  //       ...model,
  //       quickFixes:
  //         QuickFixes.addQuickFixes(
  //           ~handle,
  //           ~bufferId,
  //           ~position=cursorLocation,
  //           ~newActions=actions,
  //           model.quickFixes,
  //         ),
  //     },
  //     Outmsg.Nothing,
  //   )
  // | QuickFixesNotAvailable(_) => (model, Outmsg.Nothing)
  // | QuickFixesError(_) => (model, Outmsg.Nothing)
  };
};

let sub =
    (
      ~config,
      ~buffer,
      ~activeEditor,
      ~activePosition,
      ~topVisibleBufferLine as _,
      ~bottomVisibleBufferLine as _,
      ~lineHeightInPixels,
      ~positionToRelativePixel,
      ~client,
      codeActions,
    ) => {
  // TODO:
  Session.sub(~client, codeActions.session)
  |> Isolinear.Sub.map(msg => Session(msg));
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
};

module Keybindings = {
  open Feature_Input.Schema;

  let isMac = Revery.Environment.isMac;

  let condition = "editorTextFocus && normalMode" |> WhenExpr.parse;

  let quickFix =
    bind(
      ~key=isMac ? "<D-.>" : "<C-.>",
      ~command=Commands.quickFix.id,
      ~condition,
    );
};

module Contributions = {
  let commands = model => {
    let static = Commands.[quickFix];

    let dynamic = [];
    // model.quickFixContextMenu
    // |> Option.map(qf => {
    //      Component_EditorContextMenu.Contributions.commands(qf)
    //      |> List.map(Oni_Core.Command.map(msg => QuickFixContextMenu(msg)))
    //    })
    // |> Option.value(~default=[]);

    static @ dynamic;
  };

  let configuration = Configuration.[enabled.spec];

  let keybindings = Keybindings.[quickFix];

  let contextKeys = model => {
    WhenExpr.ContextKeys.empty;
    // model.quickFixContextMenu
    // |> Option.map(qf => {
    //      Component_EditorContextMenu.Contributions.contextKeys(qf)
    //    })
    // |> Option.value(~default=WhenExpr.ContextKeys.empty);
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
        switch (Session.lightBulb(model.session)) {
        | None => Revery.UI.React.empty
        | Some(_pos) =>
          let foregroundColor =
            Feature_Theme.Colors.Editor.lightBulbForeground.from(theme);

          let fontSize = editorFont.fontSize;
          <Revery.UI.View
            style=Revery.UI.Style.[position(`Absolute), top(y), left(x)]>
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
