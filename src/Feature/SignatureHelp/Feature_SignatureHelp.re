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
    triggerCharacters: list(Uchar.t),
    retriggerCharacters: list(Uchar.t),
    latestSignatureHelpResult: option(signatureHelp),
    latestMeet: option(meet),
  };

  let stringToTriggerCharacter = str =>
    try(Some(ZedBundled.get(str, 0))) {
    | _exn => None
    };

  let triggerCharacters = strings => {
    strings |> List.filter_map(stringToTriggerCharacter);
  };

  let start = (provider: provider) => {
    handle: provider.handle,
    triggerCharacters:
      provider.metadata.triggerCharacters |> triggerCharacters,
    retriggerCharacters:
      provider.metadata.retriggerCharacters |> triggerCharacters,
    latestSignatureHelpResult: None,
    latestMeet: None,
  };

  let incrementSignature = model => {
    let latestResult' =
      model.latestSignatureHelpResult
      |> Option.map(sigHelp => {
           let activeSignature' =
             Oni_Core.Utility.IntEx.clamp(
               ~lo=0,
               ~hi=List.length(sigHelp.signatures) - 1,
               sigHelp.activeSignature + 1,
             );
           {...sigHelp, activeSignature: activeSignature'};
         });
    {...model, latestSignatureHelpResult: latestResult'};
  };

  let decrementSignature = model => {
    let latestResult' =
      model.latestSignatureHelpResult
      |> Option.map(sigHelp => {
           let activeSignature' =
             Oni_Core.Utility.IntEx.clamp(
               ~lo=0,
               ~hi=List.length(sigHelp.signatures) - 1,
               sigHelp.activeSignature - 1,
             );
           {...sigHelp, activeSignature: activeSignature'};
         });
    {...model, latestSignatureHelpResult: latestResult'};
  };

  let bufferUpdated =
      (
        ~languageConfiguration,
        ~buffer,
        ~activeCursor,
        ~triggerKey as _,
        model,
      ) => {
    let maybeMeet =
      SignatureHelpMeet.fromBufferPosition(
        ~languageConfiguration,
        ~triggerCharacters=model.triggerCharacters,
        ~retriggerCharacters=model.retriggerCharacters,
        ~position=activeCursor,
        buffer,
      );

    let maybeMeet =
      maybeMeet
      |> Option.map((meet: SignatureHelpMeet.t) =>
           {
             bufferId: Buffer.getId(buffer),
             position: meet.location,
             isRetrigger: meet.isRetrigger,
             triggerCharacter:
               Some(ZedBundled.make(1, meet.triggerCharacter)),
             triggerKind: TriggerCharacter,
           }
         );

    if (maybeMeet == model.latestMeet) {
      model;
    } else {
      let latestSignatureHelpResult =
        switch (maybeMeet, model.latestSignatureHelpResult) {
        // If we had a previous result, keep the signature help result around, until it's refreshed...
        | (Some(_newMeet), Some(lastSignatureHelpResult)) =>
          Some(lastSignatureHelpResult)
        // Otherwise, close it out
        | (Some(_), None) => None
        | (None, Some(_)) => None
        | (None, None) => None
        };
      {...model, latestMeet: maybeMeet, latestSignatureHelpResult};
    };
  };

  let close = model => {...model, latestSignatureHelpResult: None};

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

  let update = (msg: msg, model: model) =>
    switch (msg) {
    | InfoReceived({signatures, activeSignature, activeParameter}) =>
      // Some providers will submit an empty signature help... not very useful to show this!
      if (signatures == []) {
        {...model, latestSignatureHelpResult: None};
      } else {
        {
          ...model,
          latestSignatureHelpResult:
            Some({signatures, activeSignature, activeParameter}),
        };
      }
    | EmptyInfoReceived => {...model, latestSignatureHelpResult: None}
    | RequestFailed(_) => {...model, latestSignatureHelpResult: None}
    };

  let get = ({latestSignatureHelpResult, _}) => latestSignatureHelpResult;
  let sub = (~buffer, ~client, model) => {
    switch (model.latestMeet) {
    | None => Isolinear.Sub.none
    | Some(meet) =>
      // Our meet is out-of-date... we're now in a different buffer
      if (Buffer.getId(buffer) != meet.bufferId) {
        Isolinear.Sub.none;
      } else {
        let toMsg = msg => {
          switch (msg) {
          | Ok(
              Some(
                (
                  {signatures, activeSignature, activeParameter, _}: Exthost.SignatureHelp.Response.t
                ),
              ),
            ) =>
            InfoReceived({signatures, activeSignature, activeParameter})
          | Ok(None) => EmptyInfoReceived
          | Error(s) => RequestFailed(s)
          };
        };
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
  providers: list(provider),
  sessions: list(Session.model),
  triggeredFrom: option([ | `CommandPalette]),
  location: option(CharacterPosition.t),
  context: option(Exthost.SignatureHelp.RequestContext.t),
};

let initial = {
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

let isShown = model => model |> getSignatureHelp |> Option.is_some;

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

let stopInsert = (~maybeBuffer as _, model) => {
  {...model, sessions: []};
};

let bufferUpdated =
    (~languageConfiguration, ~buffer, ~activeCursor, ~triggerKey, model) => {
  let sessions' =
    model.sessions
    |> List.map(session =>
         Session.bufferUpdated(
           ~languageConfiguration,
           ~buffer,
           ~activeCursor,
           ~triggerKey,
           session,
         )
       );
  {...model, sessions: sessions'};
};

[@deriving show({with_path: false})]
type command =
  | Show
  | IncrementSignature
  | DecrementSignature
  | Close;

[@deriving show({with_path: false})]
type msg =
  | Command(command)
  | ProviderRegistered(provider)
  // | KeyPressed(option(string), bool)
  | SignatureIncrementClicked
  | SignatureDecrementClicked
  | Session({
      handle: int,
      msg: Session.msg,
    });
// | CursorMoved(int);

module Msg = {
  let providerAvailable = provider => ProviderRegistered(provider);
};

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

  let close =
    define(
      ~category="Parameter Hints",
      ~title="Close",
      "editor.action.hideParameterHints",
      Command(Close),
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

module Keybindings = {
  open Feature_Input.Schema;

  let decrementSignature = {
    key: "<A-DOWN>",
    command: Commands.decrementSignature.id,
    condition: "editorTextFocus && parameterHintsVisible" |> WhenExpr.parse,
  };

  let incrementSignature = {
    key: "<A-UP>",
    command: Commands.incrementSignature.id,
    condition: "editorTextFocus && parameterHintsVisible" |> WhenExpr.parse,
  };

  let close = {
    key: "<ESC>",
    command: Commands.close.id,
    condition: "editorTextFocus && parameterHintsVisible" |> WhenExpr.parse,
  };
};

module Contributions = {
  let commands =
    Commands.[show, incrementSignature, decrementSignature, close];

  let keybindings =
    Keybindings.[incrementSignature, decrementSignature, close];
};

let sub = (~buffer, ~isInsertMode, ~activePosition as _, ~client, model) =>
  if (!isInsertMode) {
    Isolinear.Sub.none;
  } else {
    model.sessions
    |> List.map(session => {
         let handle = Session.handle(session);
         Session.sub(~buffer, ~client, session)
         |> Isolinear.Sub.map(msg => Session({handle, msg}));
       })
    |> Isolinear.Sub.batch;
  };

let update = (~maybeBuffer, ~maybeEditor, ~extHostClient as _, model, msg) =>
  switch (msg) {
  | Command(Show) =>
    switch (maybeBuffer, maybeEditor) {
    | (Some(_buffer), Some(_editor)) =>
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

      ({...model, triggeredFrom: Some(`CommandPalette)}, Effect(effects));
    | _ => (model, Nothing)
    }

  | Command(Close) =>
    let sessions' = model.sessions |> List.map(Session.close);
    ({...model, sessions: sessions'}, Nothing);

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
  | Command(IncrementSignature)
  | SignatureIncrementClicked =>
    let sessions' = model.sessions |> List.map(Session.incrementSignature);
    ({...model, sessions: sessions'}, Nothing);
  | Command(DecrementSignature)
  | SignatureDecrementClicked =>
    let sessions' = model.sessions |> List.map(Session.decrementSignature);
    ({...model, sessions: sessions'}, Nothing);
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
        ~editor as _,
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
      {
        let cursorLocation = Feature_Editor.Editor.getPrimaryCursor(editor);
        Some(cursorLocation);
      }
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
