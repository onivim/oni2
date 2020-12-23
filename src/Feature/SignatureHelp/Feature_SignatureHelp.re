/* Feature_SignatureHelp.re
   This feature project contains logic related to Signature Help
    */
open Oni_Core;
open EditorCoreTypes;

module Log = (val Log.withNamespace("Oni2.Feature.SignatureHelp"));
module IDGenerator =
  Utility.IDGenerator.Make({});

[@deriving show({with_path: false})]
type provider = {
  handle: int,
  selector: Exthost.DocumentSelector.t,
  metadata: Exthost.SignatureHelp.ProviderMetadata.t,
};

type signatureHelp = {
  signatures: list(Exthost.SignatureHelp.Signature.t),
  activeSignature: int,
  activeParameter: int,
};

// SESSION

// A session models the state of an individual signature help provider
// (there may be multiple signature help providers registered)
module Session = {
  // SignatureHelpMeet.t, plus some extra info
  type meet = {
    triggerKind: Exthost.SignatureHelp.TriggerKind.t,
    triggerCharacter: option(string),
    bufferId: int,
    position: CharacterPosition.t,
    isRetrigger: bool,
  };

  let meetToRequestContext = (meet: meet) =>
    Exthost.SignatureHelp.RequestContext.{
      triggerKind: meet.triggerKind,
      triggerCharacter: meet.triggerCharacter,
      isRetrigger: meet.isRetrigger,
    };

  type model = {
    handle: int,
    triggerCharacters: list(string),
    retriggerCharacters: list(string),
    latestSignatureHelpResult: option(signatureHelp),
    latestMeet: option(meet),
  };

  let start = (provider: provider) => {
    handle: provider.handle,
    triggerCharacters: provider.metadata.triggerCharacters,
    retriggerCharacters: provider.metadata.retriggerCharacters,
    latestSignatureHelpResult: None,
    latestMeet: None,
  };

  let handle = ({handle, _}) => handle;

  [@deriving show]
  type msg =
    | InfoReceived({
        signatures: list(Exthost.SignatureHelp.Signature.t),
        activeSignature: int,
        activeParameter: int,
      })
    | EmptyInfoReceived
    | RequestFailed(string);

  let update = (_msg: msg, model: model) => model;

  let get = ({latestSignatureHelpResult, _}) => latestSignatureHelpResult;
  let sub = (~buffer, ~client, model) => {
    switch (model.latestMeet) {
    | None => Isolinear.Sub.none
    | Some(meet) =>
      // Our meet is out-of-date... we're now in a different buffer
      if (Buffer.getId(buffer) != meet.bufferId) {
        Isolinear.Sub.none;
      } else {
        let toMsg = (
          fun
          | _ => EmptyInfoReceived
        );
        let context = meetToRequestContext(meet);
        Service_Exthost.Sub.signatureHelp(
          ~handle=model.handle,
          ~buffer,
          ~position=meet.position,
          ~context,
          ~toMsg,
          client,
        );
      }
    };
  };
};

type model = {
  shown: bool,
  providers: list(provider),
  sessions: list(Session.model),
  triggeredFrom: option([ | `CommandPalette]),
  location: option(CharacterPosition.t),
  context: option(Exthost.SignatureHelp.RequestContext.t),
};

let initial = {
  shown: false,
  providers: [],
  sessions: [],
  triggeredFrom: None,
  location: None,
  context: None,
};

let getSignatureHelp = ({sessions, _}) => {
  let candidates =
    sessions |> List.filter_map(session => Session.get(session));

  // If there are multiple, just return the first one
  List.nth_opt(candidates, 0);
};

let startInsert = (~maybeBuffer, model) => {
  switch (maybeBuffer) {
  | None => model
  | Some(buffer) =>
    let sessions =
      model.providers
      |> List.filter(provider =>
           Exthost.DocumentSelector.matchesBuffer(~buffer, provider.selector)
         )
      |> List.map(provider => Session.start(provider));

    {...model, sessions};
  };
};

let stopInsert = (~maybeBuffer, model) => {
  {...model, sessions: []};
};

let bufferUpdated = (
  ~languageConfiguration,
  ~buffer,
  ~activeCursor,
  ~triggerKey,
  model
) => model;

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
  | SignatureIncrementClicked
  | SignatureDecrementClicked
  | Session({
      handle: int,
      msg: Session.msg,
    })
  | CursorMoved(int);

module Msg = {
  let providerAvailable = provider => ProviderRegistered(provider);
};

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

let sub = (~buffer, ~isInsertMode, ~activePosition, ~client, model) =>
  if (!isInsertMode) {
    Isolinear.Sub.none;
  } else {
    // TODO: Wire this back up when starting a session!
    // |> List.filter(({selector, _}) =>
    //      Exthost.DocumentSelector.matchesBuffer(~buffer, selector)
    //    )
    model.sessions
    |> List.map(session => {
         let handle = Session.handle(session);
         Session.sub(~buffer, ~client, session)
         |> Isolinear.Sub.map(msg => Session({handle, msg}));
       })
    |> Isolinear.Sub.batch;
  };

// let getEffectsForLocation =
//     (
//       ~buffer,
//       ~location,
//       ~extHostClient,
//       ~model,
//       ~context,
//       ~requestID,
//       ~editor,
//     ) => {
//   let matchingProviders =
//     model.providers
//     |> List.filter(({selector, _}) =>
//          Exthost.DocumentSelector.matchesBuffer(~buffer, selector)
//        );

//   matchingProviders
//   |> List.map(provider =>
//        Service_Exthost.Effects.LanguageFeatures.provideSignatureHelp(
//          ~handle=provider.handle,
//          ~uri=Buffer.getUri(buffer),
//          ~position=location,
//          ~context,
//          extHostClient,
//          res =>
//          switch (res) {
//          | Ok(Some({signatures, activeSignature, activeParameter, _})) =>
//            InfoReceived({
//              signatures,
//              activeSignature,
//              activeParameter,
//              requestID,
//              editorID: Feature_Editor.Editor.getId(editor),
//              location,
//              context,
//            })
//          | Ok(None) => EmptyInfoReceived(requestID)
//          | Error(s) => RequestFailed(s)
//          }
//        )
//      )
//   |> Isolinear.Effect.batch;
// };

let update = (~maybeBuffer, ~maybeEditor, ~extHostClient, model, msg) =>
  switch (msg) {
  | Command(Show) =>
    switch (maybeBuffer, maybeEditor) {
    | (Some(buffer), Some(editor)) =>
      // let context =
      //   Exthost.SignatureHelp.RequestContext.{
      //     triggerKind: Exthost.SignatureHelp.TriggerKind.Invoke,
      //     triggerCharacter: None,
      //     isRetrigger: false,
      //   };

      let effects = Isolinear.Effect.none;
      // getEffectsForLocation(
      //   ~buffer,
      //   ~editor,
      //   ~location=Feature_Editor.Editor.getPrimaryCursor(editor),
      //   ~extHostClient,
      //   ~model,
      //   ~context,
      //   ~requestID,
      // );

      (
        {...model, shown: true, triggeredFrom: Some(`CommandPalette)},
        Effect(effects),
      );
    | _ => (model, Nothing)
    }
  | ProviderRegistered(provider) => (
      {...model, providers: [provider, ...model.providers]},
      Nothing,
    )
  | Session({handle, msg}) =>
    let sessions' =
      model.sessions
      |> List.map(session =>
           if (Session.handle(session) == handle) {
             Session.update(msg, session);
           } else {
             session;
           }
         );
    ({...model, sessions: sessions'}, Nothing);
  // | InfoReceived({
  //     signatures,
  //     activeSignature,
  //     activeParameter,
  //     editorID,
  //     location,
  //     context,
  //   }) => (
  //     {
  //       ...model,
  //       signatures,
  //       activeSignature: Some(activeSignature),
  //       activeParameter: Some(activeParameter),
  //       editorID: Some(editorID),
  //       location: Some(location),
  //       context: Some(context),
  //     },
  //     Nothing,
  //   )
  // | EmptyInfoReceived =>
  // TODO:
  //   ({
  //     ...model,
  //     signatures: [],
  //     activeSignature: None,
  //     activeParameter: None,
  //     shown: false,
  //     //lastRequestID: None,
  //     triggeredFrom: None,
  //   },
  //   Nothing,
  // )
  //   (model, Nothing)
  // | RequestFailed(str) =>
  //   Log.warnf(m => m("Request failed : %s", str));
  //   (model, Error(str));
  | KeyPressed(maybeKey, before) => (model, Nothing)
  // switch (maybeBuffer, maybeEditor, maybeKey) {
  // | (Some(buffer), Some(editor), Some(key)) =>
  //   let matchingProviders =
  //     model.providers
  //     |> List.filter(({selector, _}) =>
  //          Exthost.DocumentSelector.matchesBuffer(~buffer, selector)
  //        );
  //   let trigger =
  //     matchingProviders
  //     |> List.exists(({metadata, _}) =>
  //          List.mem(key, metadata.triggerCharacters)
  //        );
  //   let retrigger =
  //     matchingProviders
  //     |> List.exists(({metadata, _}) =>
  //          List.mem(key, metadata.retriggerCharacters)
  //        );
  //   let location =
  //     if (before) {
  //       let CharacterPosition.{line, character: col} =
  //         Feature_Editor.Editor.getPrimaryCursor(editor);
  //       CharacterPosition.{line, character: CharacterIndex.(col + 1)};
  //     } else {
  //       Feature_Editor.Editor.getPrimaryCursor(editor);
  //     };
  //   if (trigger) {
  //     Log.infof(m => m("Trigger character hit: %s", key));
  //     let context =
  //       Exthost.SignatureHelp.RequestContext.{
  //         triggerKind: Exthost.SignatureHelp.TriggerKind.TriggerCharacter,
  //         triggerCharacter: Some(key),
  //         isRetrigger: false,
  //       };
  // let effects =
  //   getEffectsForLocation(
  //     ~buffer,
  //     ~editor,
  //     ~location,
  //     ~extHostClient,
  //     ~model,
  //     ~context,
  //     ~requestID,
  //   );
  //     ({...model, shown: true}, Effect(Isolinear.Effect.none));
  //   } else if (retrigger && model.shown) {
  //     Log.infof(m => m("Retrigger character hit: %s", key));
  //     let context =
  //       Exthost.SignatureHelp.RequestContext.{
  //         triggerKind: Exthost.SignatureHelp.TriggerKind.TriggerCharacter,
  //         triggerCharacter: Some(key),
  //         isRetrigger: true,
  //       };
  // let effects =
  //   getEffectsForLocation(
  //     ~buffer,
  //     ~editor,
  //     ~location,
  //     ~extHostClient,
  //     ~model,
  //     ~context,
  //     ~requestID,
  //   );
  //     ({...model, shown: true}, Effect(Isolinear.Effect.none));
  //   } else if (key == "<ESC>") {
  //     (
  //       {
  //         ...model,
  //         shown: false,
  // lastRequestID: None,
  //         activeSignature: None,
  //         activeParameter: None,
  //         triggeredFrom: None,
  //       },
  //       Nothing,
  //     );
  //   } else {
  //     (model, Nothing);
  //   };
  // | _ => (model, Nothing)
  // }
  | Command(IncrementSignature)
  | SignatureIncrementClicked => (
      // TODO:
      model,
      Nothing,
      // {
      //   ...model,
      //   activeSignature:
      //     Option.map(
      //       i =>
      //         Oni_Core.Utility.IntEx.clamp(
      //           ~lo=0,
      //           ~hi=List.length(model.signatures) - 1,
      //           i + 1,
      //         ),
      //       model.activeSignature,
      //     ),
      // },
      // Nothing,
    )
  | Command(DecrementSignature)
  | SignatureDecrementClicked => (
      // TODO:
      // {
      // ...model,
      // activeSignature:
      //   Option.map(
      //     i =>
      //       Oni_Core.Utility.IntEx.clamp(
      //         ~lo=0,
      //         ~hi=List.length(model.signatures) - 1,
      //         i - 1,
      //       ),
      //     model.activeSignature,
      //   ),
      // },
      model,
      Nothing,
    )
  | CursorMoved(editorID) =>
    // TODO
    (model, Nothing)
  // switch (model.editorID, maybeEditor, maybeBuffer, model.context) {
  // | (Some(editorID'), Some(editor), Some(buffer), Some(context))
  //     when
  //       editorID == editorID'
  //       && editorID == Feature_Editor.Editor.getId(editor) =>
  //   let cursorLocation = Feature_Editor.Editor.getPrimaryCursor(editor);
  //   let loc =
  //     CharacterPosition.{
  //       line: cursorLocation.line,
  //       character: CharacterIndex.(cursorLocation.character + 1),
  //     };
  //   switch (model.location) {
  //   | Some(location) when location == loc =>
  // let effects =
  //   getEffectsForLocation(
  //     ~buffer,
  //     ~editor,
  //     ~location=cursorLocation,
  //     ~extHostClient,
  //     ~model,
  //     ~context,
  //     ~requestID,
  //   );
  //     ({...model, shown: true}, Effect(Isolinear.Effect.none));
  //   | _ => (model, Nothing)
  //   };
  // | _ => (model, Nothing)
  // }
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
        ~signatures,
        ~buffer,
        ~editor,
        ~grammars,
        ~signatureIndex,
        ~parameterIndex,
        ~dispatch,
        (),
      ) => {
    let defaultLanguage =
      Buffer.getFileType(buffer) |> Buffer.FileType.toString;
    let signatureHelpMarkdown = (~markdown) => {
      Oni_Components.Markdown.make(
        ~colorTheme,
        ~tokenTheme,
        ~languageInfo,
        ~defaultLanguage,
        ~fontFamily=uiFont.family,
        ~codeFontFamily=editorFont.fontFamily,
        ~grammars,
        ~markdown=Exthost.MarkdownString.toString(markdown),
        ~baseFontSize=uiFont.size,
        ~codeBlockFontSize=editorFont.fontSize,
      );
    };
    let maybeSignature: option(Signature.t) =
      Base.List.nth(signatures, signatureIndex);
    let maybeParameter: option(ParameterInformation.t) =
      Option.bind(maybeSignature, signature =>
        Base.List.nth(signature.parameters, parameterIndex)
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
            List.length(signatures),
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
        ~buffer,
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
      |> Option.map((characterPosition: CharacterPosition.t) => {
           let ({x: pixelX, y: pixelY}: PixelPosition.t, _) =
             Feature_Editor.Editor.bufferCharacterPositionToPixel(
               ~position=characterPosition,
               editor,
             );
           (pixelX +. gutterWidth |> int_of_float, pixelY |> int_of_float);
         });

    let maybeSignatureHelp = getSignatureHelp(model);
    switch (maybeCoords, maybeSignatureHelp) {
    | (
        Some((x, y)),
        Some({signatures, activeSignature, activeParameter, _}),
      ) =>
      <signatureHelp
        x
        y
        colorTheme
        tokenTheme
        languageInfo
        uiFont
        editorFont
        buffer
        editor
        grammars
        signatures
        signatureIndex=activeSignature
        parameterIndex=activeParameter
        dispatch
      />
    | _ => React.empty
    };
  };
};
