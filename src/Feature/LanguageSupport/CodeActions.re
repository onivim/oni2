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

module QuickFixes = {
  type t = {
    bufferId: int,
    position: EditorCoreTypes.CharacterPosition.t,
    fixes: list(CodeAction.t),
  };

  let addQuickFixes = (~handle, ~bufferId, ~position, ~newActions, quickFixes) => {
    let actions = newActions |> List.map(CodeAction.ofExthost(~handle));
    // If at the same location, just append!
    let fixes =
      if (bufferId == quickFixes.bufferId && position == quickFixes.position) {
        quickFixes.fixes @ actions;
      } else {
        // If not, replace
        actions;
      };

    {bufferId, position, fixes};
  };

  let initial = {
    bufferId: (-1),
    position: EditorCoreTypes.CharacterPosition.zero,
    fixes: [],
  };

  let cursorMoved = (~bufferId, ~position, qf) =>
    // Totally different buffer, reset...
    if (bufferId != qf.bufferId) {
      {...initial, bufferId, position};
    } else if (position != qf.position) {
      {
        ...qf,
        position,
        fixes: qf.fixes |> List.filter(CodeAction.intersects(position)),
      };
    } else {
      qf;
    };

  let any = ({fixes, _}) => fixes != [];

  let appliesToBuffer = (buffer, {bufferId, _}) => {
    bufferId == Buffer.getId(buffer);
  };

  let position = ({fixes, position, _}) => {
    Base.List.nth(fixes, 0)
    |> Utility.OptionEx.flatMap((fix: CodeAction.t) => {
         Base.List.nth(fix.action.diagnostics, 0)
       })
    |> Option.map((diagnostic: Exthost.Diagnostic.t) => {
         let range = diagnostic.range |> Exthost.OneBasedRange.toRange;

         CharacterRange.(range.start);
       })
    |> Utility.OptionEx.or_lazy(() =>
         if (fixes == []) {
           None;
         } else {
           Some(position);
         }
       );
  };

  let all = ({fixes, _}) => fixes;
};

type model = {
  providers: list(provider),
  quickFixes: QuickFixes.t,
  lightBulbActiveEditorId: option(int),
  quickFixContextMenu:
    option(Component_EditorContextMenu.model(CodeAction.t)),
};

[@deriving show]
type command =
  | QuickFix;

[@deriving show]
type msg =
  | Command(command)
  | QuickFixContextMenu(
      [@opaque] Component_EditorContextMenu.msg(CodeAction.t),
    )
  | QuickFixesAvailable({
      handle: int,
      actions: list(Exthost.CodeAction.t),
    })
  | QuickFixesNotAvailable({handle: int})
  | QuickFixesError({
      handle: int,
      msg: string,
    });

let initial = {
  providers: [],
  quickFixes: QuickFixes.initial,
  quickFixContextMenu: None,
  lightBulbActiveEditorId: None,
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

let cursorMoved = (~buffer, ~cursor, model) => {
  ...model,
  quickFixes:
    QuickFixes.cursorMoved(
      ~bufferId=Oni_Core.Buffer.getId(buffer),
      ~position=cursor,
      model.quickFixes,
    ),
};

let unregister = (~handle, model) => {
  ...model,
  providers:
    model.providers |> List.filter(provider => {provider.handle != handle}),
};

let update = (~buffer, ~cursorLocation, msg, model) => {
  let bufferId = Buffer.getId(buffer);
  switch (msg) {
  | Command(QuickFix) =>
    let all = QuickFixes.all(model.quickFixes);
    let allCount = List.length(all);
    if (allCount == 0) {
      (model, Outmsg.NotifyFailure("No quickfixes available here."));
    } else if (allCount == 1) {
      let maybeEdit =
        List.nth_opt(all, 0)
        |> Utility.OptionEx.flatMap((fix: CodeAction.t) =>
             Exthost.CodeAction.(fix.action.edit)
           );
      let outmsg =
        maybeEdit
        |> Option.map((edit: Exthost.WorkspaceEdit.t) =>
             Outmsg.ApplyWorkspaceEdit(edit)
           )
        |> Option.value(~default=Outmsg.Nothing);
      (model, outmsg);
    } else {
      let toString = (codeAction: CodeAction.t) => {
        Exthost.CodeAction.(codeAction.action.title);
      };
      let renderer =
        Component_EditorContextMenu.Schema.Renderer.default(~toString);
      let schema = Component_EditorContextMenu.Schema.contextMenu(~renderer);
      let quickFixMenu =
        Component_EditorContextMenu.create(~schema, all) |> Option.some;

      ({...model, quickFixContextMenu: quickFixMenu}, Outmsg.Nothing);
    };

  | QuickFixContextMenu(contextMenuMsg) =>
    let (model', eff) =
      model.quickFixContextMenu
      |> Option.map(contextMenu => {
           let (model', outmsg) =
             Component_EditorContextMenu.update(contextMenuMsg, contextMenu);

           let (model'', eff) =
             switch (outmsg) {
             | Nothing => (Some(model'), Outmsg.Nothing)
             | Cancelled => (None, Outmsg.Nothing)
             | FocusChanged(_) => (Some(model'), Outmsg.Nothing)
             | Selected(item) => (
                 None,
                 Exthost.CodeAction.(item.action.edit)
                 |> Option.map(edit => {Outmsg.ApplyWorkspaceEdit(edit)})
                 |> Option.value(~default=Outmsg.Nothing),
               )
             };

           ({...model, quickFixContextMenu: model''}, eff);
         })
      |> Option.value(~default=(model, Outmsg.Nothing));

    (model', eff);

  | QuickFixesAvailable({handle, actions}) => (
      {
        ...model,
        quickFixes:
          QuickFixes.addQuickFixes(
            ~handle,
            ~bufferId,
            ~position=cursorLocation,
            ~newActions=actions,
            model.quickFixes,
          ),
      },
      Outmsg.Nothing,
    )
  | QuickFixesNotAvailable(_) => (model, Outmsg.Nothing)
  | QuickFixesError(_) => (model, Outmsg.Nothing)
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
  let context =
    Exthost.CodeAction.(Context.{only: None, trigger: TriggerType.Auto});

  let toMsg = handle =>
    fun
    | Ok(Some(actionsList: Exthost.CodeAction.List.t)) =>
      QuickFixesAvailable({handle, actions: actionsList.actions})
    | Ok(None) => QuickFixesNotAvailable({handle: handle})
    | Error(msg) => QuickFixesError({handle, msg});

  let range =
    EditorCoreTypes.CharacterRange.{
      start: activePosition,
      stop: activePosition,
    };

  let isLightBulbEnabled = Configuration.enabled.get(config);

  let codeActionsSubs =
    codeActions.providers
    |> List.filter(({selector, _}) => {
         Exthost.DocumentSelector.matchesBuffer(~buffer, selector)
       })
    |> List.map(({handle, _}) => {
         Service_Exthost.Sub.codeActionsByRange(
           ~handle,
           ~range,
           ~buffer,
           ~context,
           ~toMsg=toMsg(handle),
           client,
         )
       });

  let quickFixContextMenuSub =
    codeActions.quickFixContextMenu
    |> Option.map(qf => {
         Component_EditorContextMenu.sub(
           ~isVisible=true,
           ~pixelPosition=Some(PixelPosition.{x: 250., y: 250.}),
           qf,
         )
       })
    |> Option.value(~default=Isolinear.Sub.none)
    |> Isolinear.Sub.map(msg => QuickFixContextMenu(msg));

  [quickFixContextMenuSub, ...codeActionsSubs] |> Isolinear.Sub.batch;
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

    let dynamic =
      model.quickFixContextMenu
      |> Option.map(qf => {
           Component_EditorContextMenu.Contributions.commands(qf)
           |> List.map(Oni_Core.Command.map(msg => QuickFixContextMenu(msg)))
         })
      |> Option.value(~default=[]);

    static @ dynamic;
  };

  let configuration = Configuration.[enabled.spec];

  let keybindings = Keybindings.[quickFix];

  let contextKeys = model => {
    model.quickFixContextMenu
    |> Option.map(qf => {
         Component_EditorContextMenu.Contributions.contextKeys(qf)
       })
    |> Option.value(~default=WhenExpr.ContextKeys.empty);
  };
};

module View = {
  module EditorWidgets = {
    let make = (~editorId, ~theme, ~editorFont: Service_Font.font, ~model, ()) =>
      if (Some(editorId) == model.lightBulbActiveEditorId) {
        let foregroundColor =
          Feature_Theme.Colors.Editor.lightBulbForeground.from(theme);

        let fontSize = editorFont.fontSize;
        <Revery.UI.Components.Container
          width=24 height=24 color=Revery.Colors.transparentWhite>
          <Oni_Core.Codicon
            fontSize
            color=foregroundColor
            icon=Codicon.lightbulb
          />
        </Revery.UI.Components.Container>;
      } else {
        Revery.UI.React.empty;
      };
  };

  module Overlay = {
    let make = (~dispatch, ~theme, ~uiFont, ~editorFont as _, ~model, ()) => {
      model.quickFixContextMenu
      |> Option.map(model => {
           <Component_EditorContextMenu.View
             dispatch={msg => dispatch(QuickFixContextMenu(msg))}
             theme
             model
             // <Revery.UI.Components.Clickable
             //   onClick={_ => prerr_endline("clicked")}
             //   style=Revery.UI.Style.[
             //     pointerEvents(`Allow),
             //     position(`Absolute),
             //     top(0),
             //     left(0),
             //   ]>
             //   <Revery.UI.Components.Container
             //     width=32
             //     height=32
             //     color=Revery.Colors.magenta
             //   />
             uiFont
             // </Revery.UI.Components.Clickable>;
           />
         })
      |> Option.value(~default=Revery.UI.React.empty);
    };
  };
};
