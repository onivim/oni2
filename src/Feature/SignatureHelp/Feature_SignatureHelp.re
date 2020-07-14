/* Feature_SignatureHelp.re
   This feature project contains logic related to Signature Help
    */
open Oni_Core;
open EditorCoreTypes;

module Log = (val Log.withNamespace("Oni.Feature.SignatureHelp"));
module IDGenerator =
  Utility.IDGenerator.Make({});

[@deriving show({with_path: false})]
type provider = {
  handle: int,
  selector: list(Exthost.DocumentFilter.t),
  metadata: Exthost.SignatureHelp.ProviderMetadata.t,
};

type model = {
  shown: bool,
  providers: list(provider),
  triggeredFrom: option([ | `CommandPalette]),
  lastRequestID: option(int),
  signatures: list(Exthost.SignatureHelp.Signature.t),
  activeSignature: option(int),
  activeParameter: option(int),
  editorID: option(int),
};

let initial = {
  shown: false,
  providers: [],
  triggeredFrom: None,
  lastRequestID: None,
  signatures: [],
  activeSignature: None,
  activeParameter: None,
  editorID: None,
};

[@deriving show({with_path: false})]
type command =
  | Show
  | IncrementSignature
  | DecrementSignature;

[@deriving show({with_path: false})]
type msg =
  | Command(command)
  | ProviderRegistered(provider)
  | KeyPressed(option(string), bool)
  | InfoReceived({
      signatures: list(Exthost.SignatureHelp.Signature.t),
      activeSignature: int,
      activeParameter: int,
      requestID: int,
      editorID: int,
    })
  | EmptyInfoReceived(int)
  | RequestFailed(string)
  | SignatureIncrementClicked
  | SignatureDecrementClicked;

type outmsg =
  | Nothing
  | Effect(Isolinear.Effect.t(msg))
  | Error(string);

let isShown = model => model.shown;

module Commands = {
  open Feature_Commands.Schema;

  let show =
    define(
      ~category="Parameter Hints",
      ~title="Show parameter hints",
      "editor.action.triggerParameterHints",
      Command(Show),
    );

  let incrementSignature =
    define(
      ~category="Parameter Hints",
      ~title="Next signature",
      "editor.action.showNextParameterHint",
      Command(IncrementSignature),
    );

  let decrementSignature =
    define(
      ~category="Parameter Hints",
      ~title="Previous signature",
      "editor.action.showPrevParameterHint",
      Command(DecrementSignature),
    );
};

module Contributions = {
  let commands = Commands.[show, incrementSignature, decrementSignature];
};

let getEffectsForLocation =
    (
      ~buffer,
      ~location,
      ~extHostClient,
      ~model,
      ~context,
      ~requestID,
      ~editor,
    ) => {
  let filetype =
    buffer |> Buffer.getFileType |> Option.value(~default="plaintext");

  let matchingProviders =
    model.providers
    |> List.filter(({selector, _}) =>
         Exthost.DocumentSelector.matches(~filetype, selector)
       );

  matchingProviders
  |> List.map(provider =>
       Service_Exthost.Effects.LanguageFeatures.provideSignatureHelp(
         ~handle=provider.handle,
         ~uri=Buffer.getUri(buffer),
         ~position=location,
         ~context,
         extHostClient,
         res =>
         switch (res) {
         | Ok(Some({signatures, activeSignature, activeParameter, _})) =>
           InfoReceived({
             signatures,
             activeSignature,
             activeParameter,
             requestID,
             editorID: Feature_Editor.Editor.getId(editor),
           })
         | Ok(None) => EmptyInfoReceived(requestID)
         | Error(s) => RequestFailed(s)
         }
       )
     )
  |> Isolinear.Effect.batch;
};

let update = (~maybeBuffer, ~maybeEditor, ~extHostClient, model, msg) =>
  switch (msg) {
  | Command(Show) =>
    switch (maybeBuffer, maybeEditor) {
    | (Some(buffer), Some(editor)) =>
      let requestID = IDGenerator.get();
      let context =
        Exthost.SignatureHelp.RequestContext.{
          triggerKind: Exthost.SignatureHelp.TriggerKind.Invoke,
          triggerCharacter: None,
          isRetrigger: false,
        };

      let effects =
        getEffectsForLocation(
          ~buffer,
          ~editor,
          ~location=Feature_Editor.Editor.getPrimaryCursor(editor),
          ~extHostClient,
          ~model,
          ~context,
          ~requestID,
        );

      (
        {
          ...model,
          shown: true,
          triggeredFrom: Some(`CommandPalette),
          lastRequestID: Some(requestID),
        },
        Effect(effects),
      );
    | _ => (model, Nothing)
    }
  | ProviderRegistered(provider) => (
      {...model, providers: [provider, ...model.providers]},
      Nothing,
    )
  | InfoReceived({
      signatures,
      activeSignature,
      activeParameter,
      requestID,
      editorID,
    }) =>
    switch (model.lastRequestID) {
    | Some(reqID) when reqID == requestID => (
        {
          ...model,
          signatures,
          activeSignature: Some(activeSignature),
          activeParameter: Some(activeParameter),
          editorID: Some(editorID),
        },
        Nothing,
      )
    | _ => (model, Nothing)
    }
  | EmptyInfoReceived(requestID) =>
    switch (model.lastRequestID) {
    | Some(reqID) when reqID == requestID => (
        {
          ...model,
          signatures: [],
          activeSignature: None,
          activeParameter: None,
          shown: false,
          lastRequestID: None,
          triggeredFrom: None,
        },
        Nothing,
      )
    | _ => (model, Nothing)
    }
  | RequestFailed(str) =>
    Log.warnf(m => m("Request failed : %s", str));
    (model, Error(str));
  | KeyPressed(maybeKey, before) =>
    switch (maybeBuffer, maybeEditor, maybeKey) {
    | (Some(buffer), Some(editor), Some(key)) =>
      let filetype =
        buffer |> Buffer.getFileType |> Option.value(~default="plaintext");
      let matchingProviders =
        model.providers
        |> List.filter(({selector, _}) =>
             Exthost.DocumentSelector.matches(~filetype, selector)
           );
      let trigger =
        matchingProviders
        |> List.exists(({metadata, _}) =>
             List.mem(key, metadata.triggerCharacters)
           );
      let retrigger =
        matchingProviders
        |> List.exists(({metadata, _}) =>
             List.mem(key, metadata.retriggerCharacters)
           );
      let location =
        if (before) {
          open Index;
          let Location.{line, column: col} =
            Feature_Editor.Editor.getPrimaryCursor(editor);
          Location.create(~line, ~column=col + 1);
        } else {
          Feature_Editor.Editor.getPrimaryCursor(editor);
        };
      if (trigger) {
        Log.infof(m => m("Trigger character hit: %s", key));
        let requestID = IDGenerator.get();
        let context =
          Exthost.SignatureHelp.RequestContext.{
            triggerKind: Exthost.SignatureHelp.TriggerKind.TriggerCharacter,
            triggerCharacter: Some(key),
            isRetrigger: false,
          };
        let effects =
          getEffectsForLocation(
            ~buffer,
            ~editor,
            ~location,
            ~extHostClient,
            ~model,
            ~context,
            ~requestID,
          );
        (
          {...model, shown: true, lastRequestID: Some(requestID)},
          Effect(effects),
        );
      } else if (retrigger && model.shown) {
        Log.infof(m => m("Retrigger character hit: %s", key));
        let requestID = IDGenerator.get();
        let context =
          Exthost.SignatureHelp.RequestContext.{
            triggerKind: Exthost.SignatureHelp.TriggerKind.TriggerCharacter,
            triggerCharacter: Some(key),
            isRetrigger: true,
          };
        let effects =
          getEffectsForLocation(
            ~buffer,
            ~editor,
            ~location,
            ~extHostClient,
            ~model,
            ~context,
            ~requestID,
          );
        (
          {...model, shown: true, lastRequestID: Some(requestID)},
          Effect(effects),
        );
      } else if (key == "<ESC>") {
        (
          {
            ...model,
            shown: false,
            lastRequestID: None,
            activeSignature: None,
            activeParameter: None,
            triggeredFrom: None,
          },
          Nothing,
        );
      } else {
        (model, Nothing);
      };
    | _ => (model, Nothing)
    }
  | Command(IncrementSignature)
  | SignatureIncrementClicked => (
      {
        ...model,
        activeSignature:
          Option.map(
            i =>
              Oni_Core.Utility.IntEx.clamp(
                ~lo=0,
                ~hi=List.length(model.signatures) - 1,
                i + 1,
              ),
            model.activeSignature,
          ),
      },
      Nothing,
    )
  | Command(DecrementSignature)
  | SignatureDecrementClicked => (
      {
        ...model,
        activeSignature:
          Option.map(
            i =>
              Oni_Core.Utility.IntEx.clamp(
                ~lo=0,
                ~hi=List.length(model.signatures) - 1,
                i - 1,
              ),
            model.activeSignature,
          ),
      },
      Nothing,
    )
  };

module View = {
  open Oni_Components;
  open Revery;
  open Revery.UI;
  open Exthost.SignatureHelp;

  module Styles = {
    open Style;
    module Colors = Feature_Theme.Colors;

    let signatureLine = [
      flexDirection(`Row),
      flexWrap(`Wrap),
      alignItems(`FlexStart),
      justifyContent(`FlexStart),
    ];

    let hr = (~theme) => [
      flexGrow(1),
      height(1),
      backgroundColor(Colors.EditorHoverWidget.border.from(theme)),
      marginTop(3),
      marginBottom(3),
    ];

    let activeParameter = (~theme) => [
      backgroundColor(
        Colors.EditorSuggestWidget.selectedBackground.from(theme),
      ),
    ];

    let parameterText = (~theme) => [
      color(Colors.Editor.foreground.from(theme)),
    ];

    let text = (~theme) => [color(Colors.foreground.from(theme))];

    let button = [cursor(MouseCursors.pointer)];
  };

  let horizontalRule = (~theme, ()) =>
    <UI.Components.Row>
      <View style={Styles.hr(~theme)} />
    </UI.Components.Row>;

  let signatureHelp =
      (
        ~x,
        ~y,
        ~colorTheme,
        ~tokenTheme,
        ~languageInfo,
        ~uiFont: UiFont.t,
        ~editorFont: Service_Font.font,
        ~model,
        ~editor,
        ~grammars,
        ~signatureIndex,
        ~parameterIndex,
        ~dispatch,
        (),
      ) => {
    let signatureHelpMarkdown = (~markdown) =>
      Oni_Components.Markdown.make(
        ~colorTheme,
        ~tokenTheme,
        ~languageInfo,
        ~fontFamily=uiFont.family,
        ~codeFontFamily=editorFont.fontFamily,
        ~grammars,
        ~markdown=Exthost.MarkdownString.toString(markdown),
        ~baseFontSize=uiFont.size,
        ~codeBlockFontSize=editorFont.fontSize,
      );
    let maybeSignature: option(Signature.t) =
      List.nth_opt(model.signatures, signatureIndex);
    let maybeParameter: option(ParameterInformation.t) =
      Option.bind(maybeSignature, signature =>
        List.nth_opt(signature.parameters, parameterIndex)
      );
    let renderLabel = () =>
      switch (maybeSignature, maybeParameter) {
      | (Some(signature), Some(parameter)) =>
        switch (parameter.label) {
        | `Range(start, end_) =>
          let s1 = String.sub(signature.label, 0, start);
          let s2 = String.sub(signature.label, start, end_ - start);
          let s3 =
            String.sub(
              signature.label,
              end_,
              String.length(signature.label) - end_,
            );
          [
            <Text
              text=s1
              fontFamily={editorFont.fontFamily}
              fontSize={editorFont.fontSize}
              style={Styles.parameterText(~theme=colorTheme)}
            />,
            <View style={Styles.activeParameter(~theme=colorTheme)}>
              <Text
                text=s2
                fontFamily={editorFont.fontFamily}
                fontSize={editorFont.fontSize}
                style={Styles.parameterText(~theme=colorTheme)}
              />
            </View>,
            <Text
              text=s3
              fontFamily={editorFont.fontFamily}
              fontSize={editorFont.fontSize}
              style={Styles.parameterText(~theme=colorTheme)}
            />,
          ]
          |> React.listToElement;
        | `String(str) =>
          let regex = Str.quote(str) |> Str.regexp;
          let strList = Str.full_split(regex, signature.label);
          List.map(
            res =>
              switch (res) {
              | Str.Text(s) =>
                <Text
                  text=s
                  fontFamily={editorFont.fontFamily}
                  fontSize={editorFont.fontSize}
                  style={Styles.parameterText(~theme=colorTheme)}
                />
              | Str.Delim(s) =>
                <View style={Styles.activeParameter(~theme=colorTheme)}>
                  <Text
                    text=s
                    fontFamily={editorFont.fontFamily}
                    fontSize={editorFont.fontSize}
                    style={Styles.parameterText(~theme=colorTheme)}
                  />
                </View>
              },
            strList,
          )
          |> React.listToElement;
        }
      | _ => React.empty
      };
    switch (model.editorID) {
    | Some(editorID) when editorID == Feature_Editor.Editor.getId(editor) =>
      <HoverView x y displayAt=`Top theme=colorTheme>
        <View style=Styles.signatureLine> {renderLabel()} </View>
        <UI.Components.Row>
          <View
            style=Styles.button
            onMouseUp={_ => dispatch(SignatureDecrementClicked)}>
            <Codicon
              icon=Codicon.chevronLeft
              color={Feature_Theme.Colors.foreground.from(colorTheme)}
              fontSize={uiFont.size *. 0.9}
            />
          </View>
          <Text
            text={Printf.sprintf(
              "%d/%d",
              signatureIndex + 1,
              List.length(model.signatures),
            )}
            style={Styles.text(~theme=colorTheme)}
            fontFamily={uiFont.family}
            fontSize={uiFont.size *. 0.8}
          />
          <View
            style=Styles.button
            onMouseUp={_ => dispatch(SignatureIncrementClicked)}>
            <Codicon
              icon=Codicon.chevronRight
              color={Feature_Theme.Colors.foreground.from(colorTheme)}
              fontSize={uiFont.size *. 0.9}
            />
          </View>
        </UI.Components.Row>
        {switch (maybeParameter) {
         | Some({documentation: Some(docs), _})
             when Exthost.MarkdownString.toString(docs) != "" =>
           [
             <horizontalRule theme=colorTheme />,
             <signatureHelpMarkdown markdown=docs />,
           ]
           |> React.listToElement
         | _ => React.empty
         }}
        {switch (maybeSignature) {
         | Some({documentation: Some(docs), _})
             when Exthost.MarkdownString.toString(docs) != "" =>
           [
             <horizontalRule theme=colorTheme />,
             <signatureHelpMarkdown markdown=docs />,
           ]
           |> React.listToElement
         | _ => React.empty
         }}
      </HoverView>
    | _ => React.empty
    };
  };

  let make =
      (
        ~colorTheme,
        ~tokenTheme,
        ~languageInfo,
        ~uiFont: UiFont.t,
        ~editorFont: Service_Font.font,
        ~model,
        ~editor,
        ~gutterWidth,
        ~grammars,
        ~dispatch,
        (),
      ) => {
    let maybeCoords =
      (
        if (model.shown) {
          let cursorLocation = Feature_Editor.Editor.getPrimaryCursor(editor);
          Some(cursorLocation);
        } else {
          None;
        }
      )
      |> Option.map((Location.{line, column}) => {
           let ({pixelX, pixelY}: Feature_Editor.Editor.pixelPosition, _) =
             Feature_Editor.Editor.bufferLineCharacterToPixel(
               ~line=line |> Index.toZeroBased,
               ~characterIndex=column |> Index.toZeroBased,
               editor,
             );
           (pixelX +. gutterWidth |> int_of_float, pixelY |> int_of_float);
         });
    switch (maybeCoords, model.activeSignature, model.activeParameter) {
    | (Some((x, y)), Some(signatureIndex), Some(parameterIndex)) =>
      <signatureHelp
        x
        y
        colorTheme
        tokenTheme
        languageInfo
        uiFont
        editorFont
        model
        editor
        grammars
        signatureIndex
        parameterIndex
        dispatch
      />
    | _ => React.empty
    };
  };
};
