/* Feature_Hover.re
     This feature project contains logic related to Hover
   */
open Oni_Core;
open Revery;
open Revery.UI;
open EditorCoreTypes;

module Log = (val Log.withNamespace("Oni.Feature.Hover"));

[@deriving show({with_path: false})]
type provider = {
  handle: int,
  selector: list(Exthost.DocumentFilter.t),
};

type model = {
  shown: bool,
  providers: list(provider),
  contents: list(Exthost.MarkdownString.t),
  range: option(EditorCoreTypes.Range.t),
  triggeredFrom:
    option(
      [
        | `CommandPalette(EditorCoreTypes.Location.t)
        | `Mouse(EditorCoreTypes.Location.t)
      ],
    ),
  lastRequestID: option(int),
  editorID: option(int),
};

let initial = {
  shown: false,
  providers: [],
  contents: [],
  range: None,
  triggeredFrom: None,
  lastRequestID: None,
  editorID: None,
};

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
      range: option(EditorCoreTypes.Range.t),
      requestID: int,
      editorID: int,
    })
  | HoverRequestFailed(string)
  | MouseHovered(EditorCoreTypes.Location.t)
  | MouseMoved(EditorCoreTypes.Location.t);

type outmsg =
  | Nothing
  | Effect(Isolinear.Effect.t(msg));

let getEffectsForLocation =
    (~buffer, ~location, ~extHostClient, ~model, ~requestID, ~editorId) => {
  let filetype =
    buffer
    |> Oni_Core.Buffer.getFileType
    |> Option.value(~default="plaintext");

  let matchingProviders =
    model.providers
    |> List.filter(({selector, _}) =>
         Exthost.DocumentSelector.matches(~filetype, selector)
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
             requestID,
             editorID: editorId,
           })
         | Error(s) => HoverRequestFailed(s)
         }
       )
     )
  |> Isolinear.Effect.batch;
};

let update =
    (
      ~cursorLocation: Location.t,
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
      let requestID = IDGenerator.get();
      let effects =
        getEffectsForLocation(
          ~buffer,
          ~location=cursorLocation,
          ~extHostClient,
          ~model,
          ~requestID,
          ~editorId,
        );
      (
        {
          ...model,
          shown: true,
          triggeredFrom: Some(`CommandPalette(cursorLocation)),
          lastRequestID: Some(requestID),
        },
        Effect(effects),
      );

    | _ => (model, Nothing)
    }
  | MouseHovered(location) =>
    switch (maybeBuffer) {
    | Some(buffer) =>
      let requestID = IDGenerator.get();
      let effects =
        getEffectsForLocation(
          ~buffer,
          ~location,
          ~extHostClient,
          ~model,
          ~requestID,
          ~editorId,
        );
      (
        {
          ...model,
          shown: true,
          triggeredFrom: Some(`Mouse(location)),
          lastRequestID: Some(requestID),
        },
        Effect(effects),
      );
    | _ => (model, Nothing)
    }
  | MouseMoved(location) =>
    let newModel =
      switch (model.range) {
      | Some(range) =>
        if (EditorCoreTypes.Range.contains(location, range)) {
          model;
        } else {
          {
            ...model,
            shown: false,
            range: None,
            contents: [],
            triggeredFrom: None,
          };
        }

      | None => {
          ...model,
          shown: false,
          range: None,
          contents: [],
          triggeredFrom: None,
        }
      };
    (newModel, Nothing);
  | KeyPressed(_) => (
      {
        ...model,
        shown: false,
        contents: [],
        range: None,
        triggeredFrom: None,
      },
      Nothing,
    )
  | HoverInfoReceived({contents, range, requestID, editorID}) =>
    switch (model.lastRequestID) {
    | Some(id) when requestID == id => (
        {
          ...model,
          contents,
          range,
          lastRequestID: None,
          editorID: Some(editorID),
        },
        Nothing,
      )
    | _ => (model, Nothing)
    }
  | _ => (model, Nothing)
  };

let keyPressed = (_key, model) => {
  ...model,
  shown: false,
  contents: [],
  range: None,
  triggeredFrom: None,
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
    textOverflow(`Ellipsis),
    color(Colors.Editor.foreground.from(theme)),
    backgroundColor(Colors.EditorHoverWidget.background.from(theme)),
  ];
};

module Popup = {
  let make =
      (
        ~diagnostics,
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
    if (model.editorID != editorId) {
      None;
    } else {
      let maybeLocation: option(EditorCoreTypes.Location.t) =
        switch (model.triggeredFrom, model.shown) {
        | (Some(trigger), true) =>
          switch (trigger) {
          | `Mouse(location) => Some(location)
          | `CommandPalette(location) => Some(location)
          }
        | _ => None
        };

      let defaultLanguage =
        Option.value(
          ~default=Exthost.LanguageInfo.defaultLanguage,
          Buffer.getFileType(buffer),
        );

      let hoverDiagnostic = (~diagnostic: Diagnostic.t, ()) => {
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

      maybeLocation
      |> Option.map(location => {
           let hoverElement = {
             List.map(markdown => <hoverMarkdown markdown />, model.contents)
             |> React.listToElement;
           };

           let diagnostics =
             Diagnostics.getDiagnosticsAtPosition(
               diagnostics,
               buffer,
               location,
             );

           let diagnosticsSection =
             if (diagnostics == []) {
               [];
             } else {
               let element =
                 List.map(
                   diag => <hoverDiagnostic diagnostic=diag />,
                   diagnostics,
                 )
                 |> React.listToElement;
               [Oni_Components.Popup.Section.{element, position: `Below}];
             };

           let hoverSection = [
             Oni_Components.Popup.Section.{
               element: hoverElement,
               position: `Below,
             },
           ];
           (location, diagnosticsSection @ hoverSection);
         });
    };
};
