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
};

let initial = {
  shown: false,
  providers: [],
  triggeredFrom: None,
  lastRequestID: None,
  signatures: [],
  activeSignature: None,
  activeParameter: None,
};

[@deriving show({with_path: false})]
type command =
  | Show;

[@deriving show({with_path: false})]
type msg =
  | Command(command)
  | ProviderRegistered(provider)
  | InfoReceived({
      signatures: list(Exthost.SignatureHelp.Signature.t),
      activeSignature: int,
      activeParameter: int,
      requestID: int,
    })
  | RequestFailed(string);

type outmsg =
  | Nothing
  | Effect(Isolinear.Effect.t(msg))
  | Error(string);

module Commands = {
  open Feature_Commands.Schema;

  let show =
    define(
      ~category="Parameter Hints",
      ~title="Show parameter hints",
      "editor.action.triggerParameterHints",
      Command(Show),
    );
};

module Contributions = {
  let commands = Commands.[show];
};

let getEffectsForLocation =
    (~buffer, ~location, ~extHostClient, ~model, ~context, ~requestID) => {
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
         | Ok({signatures, activeSignature, activeParameter, _}) =>
           InfoReceived({
             signatures,
             activeSignature,
             activeParameter,
             requestID,
           })
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
          isRetrigger: false // TODO: actually determine if it's a retrigger
        };

      let effects =
        getEffectsForLocation(
          ~buffer,
          ~location=Feature_Editor.Editor.getPrimaryCursor(~buffer, editor),
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
  | InfoReceived({signatures, activeSignature, activeParameter, requestID}) =>
    switch (model.lastRequestID) {
    | Some(reqID) when reqID == requestID => (
        {
          ...model,
          signatures,
          activeSignature: Some(activeSignature),
          activeParameter: Some(activeParameter),
        },
        Nothing,
      )
    | _ => (model, Nothing)
    }
  | RequestFailed(str) =>
    Log.warnf(m => m("Request failed : %s", str));
    (model, Error(str));
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
        ~grammars,
        ~signatureIndex,
        ~parameterIndex,
        (),
      ) => {
    let signatureHelpMarkdown = (~markdown) =>
      Markdown.make(
        ~colorTheme,
        ~tokenTheme,
        ~languageInfo,
        ~fontFamily=uiFont.family,
        ~codeFontFamily=editorFont.fontFamily,
        ~grammars,
        ~markdown=Exthost.MarkdownString.toString(markdown),
        ~baseFontSize=uiFont.size,
      );
    let signature: Signature.t = List.nth(model.signatures, signatureIndex);
    let parameter: ParameterInformation.t =
      List.nth(signature.parameters, parameterIndex);
    let renderLabel = () => {
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
          />,
          <View style={Styles.activeParameter(~theme=colorTheme)}>
            <Text
              text=s2
              fontFamily={editorFont.fontFamily}
              fontSize={editorFont.fontSize}
            />
          </View>,
          <Text
            text=s3
            fontFamily={editorFont.fontFamily}
            fontSize={editorFont.fontSize}
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
              />
            | Str.Delim(s) =>
              <View style={Styles.activeParameter(~theme=colorTheme)}>
                <Text
                  text=s
                  fontFamily={editorFont.fontFamily}
                  fontSize={editorFont.fontSize}
                />
              </View>
            },
          strList,
        )
        |> React.listToElement;
      };
    };
    <HoverView x y theme=colorTheme>
      <View style=Styles.signatureLine> {renderLabel()} </View>
      {switch (parameter.documentation) {
       | Some(docs) when Exthost.MarkdownString.toString(docs) != "" =>
         [
           <horizontalRule theme=colorTheme />,
           <signatureHelpMarkdown markdown=docs />,
         ]
         |> React.listToElement
       | _ => React.empty
       }}
      {switch (signature.documentation) {
       | Some(docs) when Exthost.MarkdownString.toString(docs) != "" =>
         [
           <horizontalRule theme=colorTheme />,
           <signatureHelpMarkdown markdown=docs />,
         ]
         |> React.listToElement
       | _ => React.empty
       }}
      <Text
        text={Printf.sprintf(
          "%d/%d",
          signatureIndex + 1,
          List.length(model.signatures),
        )}
        fontFamily={uiFont.family}
        fontSize={uiFont.size *. 0.8}
      />
    </HoverView>;
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
        ~buffer,
        ~gutterWidth,
        ~grammars,
        (),
      ) => {
    let maybeCoords =
      (
        if (model.shown) {
          let cursorLocation =
            Feature_Editor.Editor.getPrimaryCursor(~buffer, editor);
          Some(cursorLocation);
        } else {
          None;
        }
      )
      |> Option.map((Location.{line, column}) => {
           let y =
             int_of_float(
               editorFont.measuredHeight
               *. float(Index.toZeroBased(line) + 1)
               -. editor.scrollY
               +. 0.5,
             );

           let x =
             int_of_float(
               gutterWidth
               +. editorFont.measuredWidth
               *. float(Index.toZeroBased(column))
               -. editor.scrollX
               +. 0.5,
             );
           (x, y);
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
        grammars
        signatureIndex
        parameterIndex
      />
    | _ => React.empty
    };
  };
};
