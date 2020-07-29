open Oni_Core;
open Exthost;

[@deriving show]
type msg =
  | CompletionResultAvailable({
      handle: int,
      suggestResult: Exthost.SuggestResult.t,
    })
  | CompletionError({
      handle: int,
      errorMsg: string,
    });

[@deriving show]
type provider = {
  handle: int,
  selector: DocumentSelector.t,
  triggerCharacters: list(string),
  supportsResolveDetails: bool,
  extensionId: string,
};

module Session = {
  [@deriving show]
  type state =
    | Waiting
    | Completed({
        allItems: list(Exthost.SuggestItem.t),
        filteredItems: [@opaque] list(Filter.result(Exthost.SuggestItem.t)),
      })
    // TODO
    //| Incomplete(list(Exthost.SuggestItem.t))
    | Failure(string);

  [@deriving show]
  type t = {
    state,
    buffer: [@opaque] Oni_Core.Buffer.t,
    base: string,
    location: EditorCoreTypes.Location.t,
  };

  let filter:
    (~query: string, list(Exthost.SuggestItem.t)) =>
    list(Filter.result(Exthost.SuggestItem.t)) =
    (~query, items) => {
      let toString = (item: Exthost.SuggestItem.t, ~shouldLower) =>
        shouldLower ? String.lowercase_ascii(item.label) : item.label;

      prerr_endline("-- running filter with query: " ++ query);

      items
      |> List.filter((item: Exthost.SuggestItem.t) => {
           let query = Zed_utf8.explode(query);
           let filterText = Exthost.SuggestItem.filterText(item);
           Filter.fuzzyMatches(query, filterText);
         })
      |> Filter.rank(query, toString);
    };

  let filteredItems = ({state, _}) => {
    switch (state) {
    | Waiting => []
    | Failure(_) => []
    | Completed({filteredItems, _}) => filteredItems
    };
  };

  let create = (~buffer, ~base, ~location) => {
    state: Waiting,
    buffer,
    base,
    location,
  };

  let refine = (~base, current) => {
    let state' =
      switch (current.state) {
      | Waiting => Waiting
      | Failure(_) as failure => failure
      | Completed({allItems, _}) =>
        Completed({allItems, filteredItems: filter(~query=base, allItems)})
      };

    {...current, base, state: state'};
  };

  let receivedItems = (items: list(Exthost.SuggestItem.t), model) => {
    ...model,
    state:
      Completed({
        allItems: items,
        filteredItems: filter(~query=model.base, items),
      }),
  };

  let error = (~errorMsg: string, model) => {
    ...model,
    state: Failure(errorMsg),
  };

  let update = (~buffer, ~base, ~location, previous) =>
    // If different buffer or location... start over!
    if (Oni_Core.Buffer.getId(buffer)
        != Oni_Core.Buffer.getId(previous.buffer)
        || location != previous.location) {
      prerr_endline("RECREATING!");
      create(~buffer, ~base, ~location);
    } else {
      // Refine results
      prerr_endline("REFINING!");
      refine(~base, previous);
    };
};

type model = {
  handleToSession: IntMap.t(Session.t),
  providers: list(provider),
};

let initial = {handleToSession: IntMap.empty, providers: []};

let isActive = (model: model) => true;

let allItems = (model: model) => {
  let compare =
      (
        a: Filter.result(Exthost.SuggestItem.t),
        b: Filter.result(Exthost.SuggestItem.t),
      ) => {
    String.compare(
      a.item |> Exthost.SuggestItem.sortText,
      b.item |> Exthost.SuggestItem.sortText,
    );
  };

  model.handleToSession
  |> IntMap.bindings
  |> List.map(((handle, session)) => {session |> Session.filteredItems})
  |> List.flatten
  |> List.fast_sort(compare);
};

let register =
    (
      ~handle,
      ~selector,
      ~triggerCharacters,
      ~supportsResolveDetails,
      ~extensionId,
      model,
    ) => {
  ...model,
  providers: [
    {
      handle,
      selector,
      triggerCharacters,
      supportsResolveDetails,
      extensionId,
    },
    ...model.providers,
  ],
};

let unregister = (~handle, model) => {
  ...model,
  providers: List.filter(prov => prov.handle != handle, model.providers),
};

let bufferUpdated = (~buffer, ~activeCursor, ~syntaxScope, ~triggerKey, model) => {
  let candidateProviders =
    model.providers
    |> List.filter(prov =>
         Exthost.DocumentSelector.matchesBuffer(~buffer, prov.selector)
       );

  let handleToSession =
    List.fold_left(
      (acc: IntMap.t(Session.t), curr: provider) => {
        let maybeMeet =
          CompletionMeet.fromBufferLocation(
            // TODO: triggerCharacters
            ~location=activeCursor,
            buffer,
          );

        prerr_endline(
          "Trying to update handle: " ++ string_of_int(curr.handle),
        );

        switch (maybeMeet) {
        | None =>
          prerr_endline("No meet");
          acc |> IntMap.remove(curr.handle);
        | Some({base, location, _}) =>
          prerr_endline("Got meet");
          acc
          |> IntMap.update(
               curr.handle,
               fun
               | None => {
                   prerr_endline(
                     "No previous results for handle, recreating",
                   );
                   Some(Session.create(~buffer, ~base, ~location));
                 }
               | Some(previous) => {
                   prerr_endline("Some previous results - refining");
                   Some(Session.update(~buffer, ~base, ~location, previous));
                 },
             );
        };
      },
      model.handleToSession,
      candidateProviders,
    );

  {...model, handleToSession};
};

let update = (msg, model) => {
  switch (msg) {
  | CompletionResultAvailable({handle, suggestResult}) => (
      {
        ...model,
        handleToSession:
          IntMap.update(
            handle,
            Option.map(prev =>
              Session.receivedItems(
                Exthost.SuggestResult.(suggestResult.completions),
                prev,
              )
            ),
            model.handleToSession,
          ),
      },
      Outmsg.Nothing,
    )
  | CompletionError({handle, errorMsg}) => (
      {
        ...model,
        handleToSession:
          IntMap.update(
            handle,
            Option.map(prev => Session.error(~errorMsg, prev)),
            model.handleToSession,
          ),
      },
      Outmsg.Nothing,
    )
  };
};

let sub = (~client, model) => {
  model.handleToSession
  |> IntMap.bindings
  |> List.map(((handle: int, meet: Session.t)) => {
       Service_Exthost.Sub.completionItems(
         // TODO: proper trigger kind
         ~context=
           Exthost.CompletionContext.{
             triggerKind: Invoke,
             triggerCharacter: None,
           },
         ~handle,
         ~buffer=meet.buffer,
         ~position=meet.location,
         ~toMsg=
           suggestResult => {
             //             prerr_endline(
             //             );
             switch (suggestResult) {
             //               "Got result for handle: " ++ string_of_int(handle),

             | Ok(v) =>
               //prerr_endline(Exthost.SuggestResult.show(v));
               CompletionResultAvailable({handle, suggestResult: v})
             | Error(errorMsg) =>
               //prerr_endline(errorMsg);
               CompletionError({handle, errorMsg})
             }
           },
         client,
       )
     })
  |> Isolinear.Sub.batch;
};

module ContextKeys = {
  open WhenExpr.ContextKeys.Schema;

  let suggestWidgetVisible = bool("suggestWidgetVisible", isActive);
};

module Contributions = {
  let contextKeys = ContextKeys.[suggestWidgetVisible];
};

module View = {
  open Revery;
  open Revery.UI;
  open Oni_Syntax;
  open Oni_Components;

  module Zed_utf8 = Oni_Core.ZedBundled;
  open Exthost.CompletionKind;

  module Constants = {
    let maxCompletionWidth = 225;
    let maxDetailWidth = 225;
    let itemHeight = 22;
    let maxHeight = itemHeight * 5;
    let opacity = 1.0;
    let padding = 8;
  };

  let kindToIcon =
    fun
    | Method => Codicon.symbolMethod
    | Function => Codicon.symbolFunction
    | Constructor => Codicon.symbolConstructor
    | Field => Codicon.symbolField
    | Variable => Codicon.symbolVariable
    | Class => Codicon.symbolClass
    | Struct => Codicon.symbolStruct
    | Interface => Codicon.symbolInterface
    | Module => Codicon.symbolModule
    | Property => Codicon.symbolProperty
    | Event => Codicon.symbolEvent
    | Operator => Codicon.symbolOperator
    | Unit => Codicon.symbolUnit
    | Value => Codicon.symbolValue
    | Constant => Codicon.symbolConstant
    | Enum => Codicon.symbolEnum
    | EnumMember => Codicon.symbolEnumMember
    | Keyword => Codicon.symbolKeyword
    | Text => Codicon.symbolText
    | Color => Codicon.symbolColor
    | File => Codicon.symbolFile
    | Reference => Codicon.symbolReference
    | Customcolor => Codicon.symbolColor
    | Folder => Codicon.symbolFolder
    | TypeParameter => Codicon.symbolTypeParameter
    | User => Codicon.symbolMisc
    | Issue => Codicon.symbolMisc
    | Snippet => Codicon.symbolText;

  let kindToColor = (tokenTheme: TokenTheme.t) =>
    fun
    | Text => Some(tokenTheme.textColor)
    | Method => Some(tokenTheme.functionColor)
    | Function => Some(tokenTheme.functionColor)
    | Constructor => Some(tokenTheme.entityColor)
    | Struct => Some(tokenTheme.typeColor)
    | Module => Some(tokenTheme.entityColor)
    | Unit => Some(tokenTheme.entityColor)
    | Keyword => Some(tokenTheme.keywordColor)
    | Enum => Some(tokenTheme.entityColor)
    | Constant => Some(tokenTheme.constantColor)
    | Property => Some(tokenTheme.entityColor)
    | Interface => Some(tokenTheme.entityColor)
    | _ => None;

  module Colors = {
    type t = {
      suggestWidgetSelectedBackground: Revery.Color.t,
      suggestWidgetBackground: Revery.Color.t,
      suggestWidgetBorder: Revery.Color.t,
      editorForeground: Revery.Color.t,
      normalModeBackground: Revery.Color.t,
    };
  };

  let colors: Colors.t = {
    suggestWidgetSelectedBackground: Revery.Colors.red,
    suggestWidgetBackground: Revery.Colors.magenta,
    suggestWidgetBorder: Revery.Colors.white,
    editorForeground: Revery.Colors.white,
    normalModeBackground: Revery.Colors.blue,
  };

  module Styles = {
    open Style;

    let outerPosition = (~x, ~y) => [
      position(`Absolute),
      top(y - 4),
      left(x + 4),
    ];

    let innerPosition = (~height, ~width, ~lineHeight, ~colors: Colors.t) => [
      position(`Absolute),
      top(int_of_float(lineHeight +. 0.5)),
      left(0),
      Style.width(width),
      Style.height(height),
      border(~color=colors.suggestWidgetBorder, ~width=1),
      backgroundColor(colors.suggestWidgetBackground),
    ];

    let item = (~isFocused, ~colors: Colors.t) => [
      isFocused
        ? backgroundColor(colors.suggestWidgetSelectedBackground)
        : backgroundColor(colors.suggestWidgetBackground),
      flexDirection(`Row),
    ];

    let icon = (~color) => [
      flexDirection(`Row),
      justifyContent(`Center),
      alignItems(`Center),
      flexGrow(0),
      backgroundColor(color),
      width(25),
      padding(4),
    ];

    let label = [flexGrow(1), margin(4)];

    let text = (~highlighted=false, ~colors: Colors.t, ()) => [
      textOverflow(`Ellipsis),
      textWrap(Revery.TextWrapping.NoWrap),
      color(
        highlighted ? colors.normalModeBackground : colors.editorForeground,
      ),
    ];

    let highlightedText = (~colors) => text(~highlighted=true, ~colors, ());

    let detail = (~width, ~lineHeight, ~colors: Colors.t) => [
      position(`Absolute),
      left(width),
      top(int_of_float(lineHeight +. 0.5)),
      Style.width(Constants.maxDetailWidth),
      flexDirection(`Column),
      alignItems(`FlexStart),
      justifyContent(`Center),
      border(~color=colors.suggestWidgetBorder, ~width=1),
      backgroundColor(colors.suggestWidgetBackground),
    ];

    let detailText = (~tokenTheme: TokenTheme.t) => [
      textOverflow(`Ellipsis),
      color(tokenTheme.commentColor),
      margin(3),
    ];
  };

  let itemView =
      (
        ~isFocused,
        ~text,
        ~kind,
        ~highlight,
        ~colors: Colors.t,
        ~tokenTheme,
        ~editorFont: Service_Font.font,
        (),
      ) => {
    let icon = kind |> kindToIcon;

    let iconColor =
      kind
      |> kindToColor(tokenTheme)
      |> Option.value(~default=colors.editorForeground);

    <View style={Styles.item(~isFocused, ~colors)}>
      <View style={Styles.icon(~color=iconColor)}>
        <Codicon
          icon
          color={colors.suggestWidgetBackground}
          // Not sure why, but specifying a font size fails to render the icon!
          // Might be a bug with Revery font loading / re - rendering in this case?
        />
      </View>
      <View style=Styles.label>
        <HighlightText
          highlights=highlight
          style={Styles.text(~colors, ())}
          highlightStyle={Styles.highlightedText(~colors)}
          fontFamily={editorFont.fontFamily}
          fontSize={editorFont.fontSize}
          text
        />
      </View>
    </View>;
  };

  let detailView =
      (
        ~text,
        ~width,
        ~lineHeight,
        ~editorFont: Service_Font.font,
        ~colors,
        ~tokenTheme,
        (),
      ) =>
    <View style={Styles.detail(~width, ~lineHeight, ~colors)}>
      <Text
        style={Styles.detailText(~tokenTheme)}
        fontFamily={editorFont.fontFamily}
        fontSize={editorFont.fontSize}
        text
      />
    </View>;

  let make =
      (
        ~x: int,
        ~y: int,
        ~lineHeight: float,
        // TODO
        // ~colors,
        ~theme,
        ~tokenTheme,
        ~editorFont,
        ~completions: model,
        (),
      ) => {
    /*let hoverEnabled =
      Configuration.getValue(c => c.editorHoverEnabled, state.configuration);*/
    let items = completions |> allItems |> Array.of_list;

    // TODO
    let focused = None;

    let maxWidth =
      items
      |> Array.fold_left(
           (maxWidth, this: Filter.result(Exthost.SuggestItem.t)) => {
             let textWidth =
               Service_Font.measure(~text=this.item.label, editorFont);
             let thisWidth =
               int_of_float(textWidth +. 0.5) + Constants.padding;
             max(maxWidth, thisWidth);
           },
           Constants.maxCompletionWidth,
         );

    let width = maxWidth + Constants.padding * 2;
    let height =
      min(Constants.maxHeight, Array.length(items) * Constants.itemHeight);

    let detail =
      switch (focused) {
      | Some(index) =>
        let focused: Filter.result(Exthost.SuggestItem.t) = items[index];
        switch (focused.item.detail) {
        | Some(text) =>
          <detailView text width lineHeight colors tokenTheme editorFont />
        | None => React.empty
        };
      | None => React.empty
      };

    <View style={Styles.outerPosition(~x, ~y)}>
      <Opacity opacity=Constants.opacity>
        <View
          style={Styles.innerPosition(~height, ~width, ~lineHeight, ~colors)}>
          <FlatList
            rowHeight=Constants.itemHeight
            initialRowsToRender=5
            count={Array.length(items)}
            theme
            focused>
            ...{index => {
              let Filter.{highlight, item} = items[index];
              let Exthost.SuggestItem.{label: text, kind, _} = item;
              <itemView
                isFocused={Some(index) == focused}
                text
                kind
                highlight
                colors
                tokenTheme
                editorFont
              />;
            }}
          </FlatList>
        </View>
        detail
      </Opacity>
    </View>;
  };
};
