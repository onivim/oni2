/* Feature_Hover.re
     This feature project contains logic related to Hover
   */
open Oni_Core;
module OptionEx = Utility.OptionEx;
open Revery;
open Revery.UI;
open EditorCoreTypes;

module Log = (val Log.withNamespace("Oni.Feature.Hover"));

[@deriving show({with_path: false})]
type provider = {
  handle: int,
  selector: Exthost.DocumentSelector.t,
};

module Session = {
  type t = {
    range: option(CharacterRange.t),
    triggeredFrom: [
      | `CommandPalette(CharacterPosition.t)
      | `Mouse(CharacterPosition.t)
    ],
    requestId: int,
    editorId: int,
    contents: list(Exthost.MarkdownString.t),
    diagnostics: list(Feature_Diagnostics.Diagnostic.t),
  };

  let start = (~diagnostics, ~requestId, ~editorId, ~triggeredFrom) => {
    range: None,
    triggeredFrom,
    requestId,
    editorId,
    contents: [],
    diagnostics,
  };

  let hasContent = session =>
    session.contents != [] || session.diagnostics != [];

  let position = ({triggeredFrom, _}) =>
    switch (triggeredFrom) {
    | `CommandPalette(pos) => pos
    | `Mouse(pos) => pos
    };

  let containsPosition = (~position as pos, model) => {
    pos == position(model)
    || Option.map(CharacterRange.contains(pos), model.range)
    |> Option.value(~default=false);
  };
};

type model = {
  shown: bool,
  providers: list(provider),
  activeSession: option(Session.t),
};

let initial = {shown: false, providers: [], activeSession: None};

module IDGenerator =
  Oni_Core.Utility.IDGenerator.Make({});

[@deriving show({with_path: false})]
type command =
  | Show;

[@deriving show({with_path: false})]
type msg =
  | Command(command)
  | KeyPressed(string)
  | HoverInfoReceived({
      contents: list(Exthost.MarkdownString.t),
      range: option(EditorCoreTypes.CharacterRange.t),
      requestId: int,
      editorId: int,
    })
  | HoverRequestFailed(string)
  | MouseHovered(option(EditorCoreTypes.CharacterPosition.t))
  | MouseMoved(option(EditorCoreTypes.CharacterPosition.t));

type outmsg =
  | Nothing
  | Effect(Isolinear.Effect.t(msg));

let getEffectsForLocation =
    (~buffer, ~location, ~extHostClient, ~model, ~requestId, ~editorId) => {
  let matchingProviders =
    model.providers
    |> List.filter(({selector, _}) =>
         Exthost.DocumentSelector.matchesBuffer(~buffer, selector)
       );

  matchingProviders
  |> List.map(provider =>
       Service_Exthost.Effects.LanguageFeatures.provideHover(
         ~handle=provider.handle,
         ~uri=Oni_Core.Buffer.getUri(buffer),
         ~position=location,
         extHostClient,
         res =>
         switch (res) {
         | Ok({contents, range}) =>
           HoverInfoReceived({
             contents,
             range: Option.map(Exthost.OneBasedRange.toRange, range),
             requestId,
             editorId,
           })
         | Error(s) => HoverRequestFailed(s)
         }
       )
     )
  |> Isolinear.Effect.batch;
};

let getDiagnosticsForPosition =
    (~buffer, ~languageConfiguration, ~diagnostics, ~position) => {
  let maybeRange = Buffer.tokenAt(~languageConfiguration, position, buffer);

  maybeRange
  |> Option.map(range => {
       Feature_Diagnostics.getDiagnosticsInRange(diagnostics, buffer, range)
     })
  |> Option.value(~default=[]);
};

let update =
    (
      ~languageConfiguration,
      ~cursorLocation: CharacterPosition.t,
      ~diagnostics,
      ~maybeBuffer,
      ~editorId,
      ~extHostClient,
      model,
      msg,
    ) =>
  switch (msg) {
  | Command(Show) =>
    switch (maybeBuffer) {
    | Some(buffer) =>
      let requestId = IDGenerator.get();
      let effects =
        getEffectsForLocation(
          ~buffer,
          ~location=cursorLocation,
          ~extHostClient,
          ~model,
          ~requestId,
          ~editorId,
        );

      (
        {
          ...model,
          shown: true,
          activeSession:
            Some(
              Session.start(
                ~diagnostics=
                  getDiagnosticsForPosition(
                    ~buffer,
                    ~diagnostics,
                    ~position=cursorLocation,
                    ~languageConfiguration,
                  ),
                ~requestId,
                ~editorId,
                ~triggeredFrom=`CommandPalette(cursorLocation),
              ),
            ),
        },
        Effect(effects),
      );

    | _ => (model, Nothing)
    }
  | MouseHovered(maybeLocation) =>
    let requestNewHover = (buffer, location) => {
      let requestId = IDGenerator.get();
      let effects =
        getEffectsForLocation(
          ~buffer,
          ~location,
          ~extHostClient,
          ~model,
          ~requestId,
          ~editorId,
        );
      (
        {
          ...model,
          shown: true,
          activeSession:
            Some(
              Session.start(
                ~diagnostics=
                  getDiagnosticsForPosition(
                    ~buffer,
                    ~diagnostics,
                    ~position=cursorLocation,
                    ~languageConfiguration,
                  ),
                ~requestId,
                ~editorId,
                ~triggeredFrom=`Mouse(location),
              ),
            ),
        },
        Effect(effects),
      );
    };
    switch (maybeBuffer) {
    | Some(buffer) =>
      switch (maybeLocation, model.activeSession) {
      | (Some(location), None) => requestNewHover(buffer, location)
      | (Some(location), Some(session))
          when !Session.containsPosition(~position=location, session) =>
        requestNewHover(buffer, location)
      | _ => (model, Nothing)
      }
    | _ => (model, Nothing)
    };
  | MouseMoved(maybeLocation) =>
    let newModel =
      switch (model.activeSession, maybeLocation) {
      | (Some(session), Some(location)) =>
        if (Session.containsPosition(~position=location, session)) {
          model;
        } else {
          {...model, shown: false, activeSession: None};
        }

      | _ => {...model, shown: false, activeSession: None}
      };
    (newModel, Nothing);
  | KeyPressed(_) => ({...model, shown: false, activeSession: None}, Nothing)
  | HoverInfoReceived({contents, range, requestId, editorId}) =>
    let activeSession' =
      model.activeSession
      |> Option.map((session: Session.t) =>
           if (requestId == session.requestId && editorId == session.editorId) {
             Session.{...session, contents, range};
           } else {
             session;
           }
         );
    ({...model, activeSession: activeSession'}, Nothing);
  | HoverRequestFailed(_) => (model, Nothing)
  };

let keyPressed = (_key, model) => {
  ...model,
  shown: false,
  activeSession: None,
};

let register = (~handle, ~selector, model) => {
  ...model,
  providers: [{handle, selector}, ...model.providers],
};

let unregister = (~handle, model) => {
  ...model,
  providers:
    List.filter(provider => provider.handle != handle, model.providers),
};

module Commands = {
  open Feature_Commands.Schema;

  let show =
    define(
      ~category="Hover",
      ~title="Show hover panel",
      "editor.action.showHover",
      Command(Show),
    );
};

module Contributions = {
  let commands = Commands.[show];
};

module Styles = {
  open Style;
  module Colors = Feature_Theme.Colors;

  let diagnostic = (~theme) => [
    color(Colors.Editor.foreground.from(theme)),
    backgroundColor(Colors.EditorHoverWidget.background.from(theme)),
  ];
};

module Popup = {
  let make =
      (
        ~theme,
        ~tokenTheme,
        ~languageInfo,
        ~uiFont: Oni_Core.UiFont.t,
        ~editorFont: Service_Font.font,
        ~grammars,
        ~model,
        ~buffer,
        ~editorId,
      ) =>
    if (!model.shown) {
      None;
    } else {
      model.activeSession
      |> OptionEx.filter((session: Session.t) =>
           session.editorId == editorId
         )
      |> OptionEx.filter(Session.hasContent)
      |> Option.map((session: Session.t) => {
           let position = Session.position(session);
           let defaultLanguage =
             buffer |> Buffer.getFileType |> Buffer.FileType.toString;

           let hoverDiagnostic =
               (~diagnostic: Feature_Diagnostics.Diagnostic.t, ()) => {
             <Text
               text={diagnostic.message}
               fontFamily={editorFont.fontFamily}
               fontSize={editorFont.fontSize}
               style={Styles.diagnostic(~theme)}
             />;
           };

           let hoverMarkdown = (~markdown) => {
             Oni_Components.Markdown.make(
               ~colorTheme=theme,
               ~tokenTheme,
               ~languageInfo,
               ~defaultLanguage,
               ~fontFamily={
                 uiFont.family;
               },
               ~codeFontFamily={
                 editorFont.fontFamily;
               },
               ~grammars,
               ~markdown=Exthost.MarkdownString.toString(markdown),
               ~baseFontSize=uiFont.size,
               ~codeBlockStyle=Style.[flexGrow(1)],
               ~codeBlockFontSize=editorFont.fontSize,
             );
           };

           let hoverElement = {
             List.map(
               markdown => <hoverMarkdown markdown />,
               session.contents,
             )
             |> React.listToElement;
           };

           let diagnosticsSection =
             if (session.diagnostics == []) {
               [];
             } else {
               let element =
                 session.diagnostics
                 |> List.map(diag => <hoverDiagnostic diagnostic=diag />)
                 |> React.listToElement;
               [Oni_Components.Popup.Section.{element, position: `Below}];
             };

           let hoverSection = [
             Oni_Components.Popup.Section.{
               element: hoverElement,
               position: `Below,
             },
           ];
           (position, diagnosticsSection @ hoverSection);
         });
    };
};
