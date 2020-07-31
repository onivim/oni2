open Oni_Core;
open Utility;
open Exthost;

[@deriving show]
type command =
  | AcceptSelected
  | SelectPrevious
  | SelectNext;

[@deriving show]
type msg =
  | Command(command)
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

// CONFIGURATION

module QuickSuggestionsSetting = {
  type t = {
    comments: bool,
    strings: bool,
    other: bool,
  };

  let initial = {comments: false, strings: false, other: true};

  let enabledFor = (~syntaxScope: SyntaxScope.t, {comments, strings, other}) => {
    syntaxScope.isComment
    && comments
    || syntaxScope.isString
    && strings
    || !syntaxScope.isComment
    && !syntaxScope.isString
    && other;
  };

  module Decode = {
    open Json.Decode;
    let decodeBool =
      bool
      |> map(
           fun
           | false => {comments: false, strings: false, other: false}
           | true => {comments: true, strings: true, other: true},
         );

    let decodeObj =
      obj(({field, _}) =>
        {
          comments: field.withDefault("comments", initial.comments, bool),
          strings: field.withDefault("strings", initial.strings, bool),
          other: field.withDefault("other", initial.other, bool),
        }
      );

    let decode = one_of([("bool", decodeBool), ("obj", decodeObj)]);
  };

  let decode = Decode.decode;

  let encode = setting =>
    Json.Encode.(
      {
        obj([
          ("comments", setting.comments |> bool),
          ("strings", setting.strings |> bool),
          ("other", setting.other |> bool),
        ]);
      }
    );
};

// CONFIGURATION

module Configuration = {
  open Config.Schema;

  let quickSuggestions =
    setting(
      "editor.quickSuggestions",
      custom(
        ~decode=QuickSuggestionsSetting.decode,
        ~encode=QuickSuggestionsSetting.encode,
      ),
      ~default=QuickSuggestionsSetting.initial,
    );
};

module CompletionItem = {
  type t = {
    handle: int,
    label: string,
    kind: Exthost.CompletionKind.t,
    detail: option(string),
    documentation: option(string),
    insertText: string,
    sortText: string,
  };

  let create = (~handle, item: Exthost.SuggestItem.t) => {
    handle,
    label: item.label,
    kind: item.kind,
    detail: item.detail,
    documentation: item.documentation,
    insertText: item |> Exthost.SuggestItem.insertText,
    sortText: item |> Exthost.SuggestItem.sortText,
  };
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
    | Failure(string)
    | Accepted;

  [@deriving show]
  type t = {
    state,
    buffer: [@opaque] Oni_Core.Buffer.t,
    base: string,
    location: EditorCoreTypes.Location.t,
  };

  let complete = session => {...session, state: Accepted};

  let filter:
    (~query: string, list(Exthost.SuggestItem.t)) =>
    list(Filter.result(Exthost.SuggestItem.t)) =
    (~query, items) => {
      let toString = (item: Exthost.SuggestItem.t, ~shouldLower) =>
        shouldLower ? String.lowercase_ascii(item.label) : item.label;

      let explodedQuery = Zed_utf8.explode(query);

      items
      |> List.filter((item: Exthost.SuggestItem.t) => {
           let filterText = Exthost.SuggestItem.filterText(item);
           if (String.length(filterText) <= String.length(query)) {
             false;
           } else {
             Filter.fuzzyMatches(explodedQuery, filterText);
           };
         })
      |> Filter.rank(query, toString);
    };

  let filteredItems = ({state, _}) => {
    switch (state) {
    | Accepted => []
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
      | Accepted => Accepted
      | Waiting => Waiting
      | Failure(_) as failure => failure
      | Completed({allItems, _}) =>
        Completed({allItems, filteredItems: filter(~query=base, allItems)})
      };

    {...current, base, state: state'};
  };

  let receivedItems = (items: list(Exthost.SuggestItem.t), model) =>
    if (model.state == Accepted) {
      model;
    } else {
      {
        ...model,
        state:
          Completed({
            allItems: items,
            filteredItems: filter(~query=model.base, items),
          }),
      };
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
      create(~buffer, ~base, ~location);
    } else {
      // Refine results
      refine(~base, previous);
    };
};

module Selection = {
  type t = option(int);
  let initial = None;

  let ensureValidFocus = (~count, selection) =>
    if (count == 0) {
      None;
    } else {
      switch (selection) {
      | None => Some(0)
      | Some(index) =>
        index |> IntEx.clamp(~lo=0, ~hi=count - 1) |> Option.some
      };
    };

  let focusPrevious = (~count, selection) => {
    IndexEx.prevRollOverOpt(~last=count - 1, selection);
  };

  let focusNext = (~count, selection) => {
    IndexEx.nextRollOverOpt(~last=count - 1, selection);
  };
};

type model = {
  handleToSession: IntMap.t(Session.t),
  providers: list(provider),
  allItems: array(Filter.result(CompletionItem.t)),
  selection: Selection.t,
};

let initial = {
  handleToSession: IntMap.empty,
  providers: [],
  allItems: [||],
  selection: Selection.initial,
};

let providerCount = ({providers, _}) => List.length(providers);
let availableCompletionCount = ({allItems, _}) => Array.length(allItems);

let getMeetLocation = (~handle, model) => {
  IntMap.find_opt(handle, model.handleToSession)
  |> Option.map(({location, _}: Session.t) => location);
};

let recomputeAllItems = (sessions: IntMap.t(Session.t)) => {
  let compare =
      (
        a: Filter.result(CompletionItem.t),
        b: Filter.result(CompletionItem.t),
      ) => {
    String.compare(a.item.sortText, b.item.sortText);
  };

  sessions
  |> IntMap.bindings
  |> List.map(((handle, session)) => {
       session
       |> Session.filteredItems
       |> List.map(Filter.map(CompletionItem.create(~handle)))
     })
  |> List.flatten
  |> List.fast_sort(compare)
  |> Array.of_list;
};

let allItems = ({allItems, _}) => allItems;

let isActive = (model: model) => {
  model.allItems |> Array.length > 0;
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

let bufferUpdated =
    (~buffer, ~config, ~activeCursor, ~syntaxScope, ~triggerKey, model) => {
  // TODO: Account for syntax scope
  ignore(syntaxScope);

  let quickSuggestionsSetting = Configuration.quickSuggestions.get(config);

  if (!
        QuickSuggestionsSetting.enabledFor(
          ~syntaxScope,
          quickSuggestionsSetting,
        )) {
    // TODO: Do we need to clear in this case?
    model;
  } else {
    // TODO: Account for trigger key
    ignore(triggerKey);

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

          switch (maybeMeet) {
          | None => acc |> IntMap.remove(curr.handle)
          | Some({base, location, _}) =>
            acc
            |> IntMap.update(
                 curr.handle,
                 fun
                 | None => {
                     Some(Session.create(~buffer, ~base, ~location));
                   }
                 | Some(previous) => {
                     Some(
                       Session.update(~buffer, ~base, ~location, previous),
                     );
                   },
               )
          };
        },
        model.handleToSession,
        candidateProviders,
      );

    let allItems = recomputeAllItems(handleToSession);
    let selection =
      Selection.ensureValidFocus(
        ~count=Array.length(allItems),
        model.selection,
      );

    {...model, handleToSession, allItems, selection};
  };
};

let update = (msg, model) => {
  switch (msg) {
  | Command(AcceptSelected) =>
    let allItems = allItems(model);

    let default = (model, Outmsg.Nothing);
    if (allItems == [||]) {
      default;
    } else {
      let result: Filter.result(CompletionItem.t) = allItems[0];

      let handle = result.item.handle;

      getMeetLocation(~handle, model)
      |> Option.map((location: EditorCoreTypes.Location.t) => {
           (
             {
               ...model,
               handleToSession:
                 IntMap.map(
                   session => {session |> Session.complete},
                   model.handleToSession,
                 ),
               allItems: [||],
               selection: None,
             },
             Outmsg.ApplyCompletion({
               meetColumn: location.column,
               insertText: result.item.insertText,
             }),
           )
         })
      |> Option.value(~default);
    };

  | Command(SelectNext) =>
    let count = Array.length(model.allItems);
    (
      {...model, selection: Selection.focusNext(~count, model.selection)},
      Nothing,
    );

  | Command(SelectPrevious) =>
    let count = Array.length(model.allItems);
    (
      {...model, selection: Selection.focusPrevious(~count, model.selection)},
      Nothing,
    );

  | CompletionResultAvailable({handle, suggestResult}) =>
    let handleToSession =
      IntMap.update(
        handle,
        Option.map(prev =>
          Session.receivedItems(
            Exthost.SuggestResult.(suggestResult.completions),
            prev,
          )
        ),
        model.handleToSession,
      );
    let allItems = recomputeAllItems(handleToSession);
    (
      {
        ...model,
        handleToSession,
        allItems,
        selection:
          Selection.ensureValidFocus(
            ~count=Array.length(allItems) - 1,
            model.selection,
          ),
      },
      Outmsg.Nothing,
    );
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
             switch (suggestResult) {
             | Ok(v) => CompletionResultAvailable({handle, suggestResult: v})
             | Error(errorMsg) => CompletionError({handle, errorMsg})
             }
           },
         client,
       )
     })
  |> Isolinear.Sub.batch;
};

// COMMANDS

module Commands = {
  open Feature_Commands.Schema;

  let acceptSelected =
    define("acceptSelectedSuggestion", Command(AcceptSelected));

  let selectPrevSuggestion =
    define("selectPrevSuggestion", Command(SelectPrevious));

  let selectNextSuggestion =
    define("selectNextSuggestion", Command(SelectNext));
};

// CONTEXTKEYS

module ContextKeys = {
  open WhenExpr.ContextKeys.Schema;

  let suggestWidgetVisible = bool("suggestWidgetVisible", isActive);
};

// KEYBINDINGS

module KeyBindings = {
  open Oni_Input.Keybindings;

  let suggestWidgetVisible = "suggestWidgetVisible" |> WhenExpr.parse;
  let acceptOnEnter =
    "acceptSuggestionOnEnter && suggestWidgetVisible" |> WhenExpr.parse;

  let nextSuggestion = {
    key: "<C-N>",
    command: Commands.selectNextSuggestion.id,
    condition: suggestWidgetVisible,
  };

  let previousSuggestion = {
    key: "<C-P>",
    command: Commands.selectPrevSuggestion.id,
    condition: suggestWidgetVisible,
  };

  let acceptSuggestionEnter = {
    key: "<CR>",
    command: Commands.acceptSelected.id,
    condition: acceptOnEnter,
  };

  let acceptSuggestionTab = {
    key: "<TAB>",
    command: Commands.acceptSelected.id,
    condition: suggestWidgetVisible,
  };

  let acceptSuggestionShiftTab = {
    key: "<S-TAB>",
    command: Commands.acceptSelected.id,
    condition: suggestWidgetVisible,
  };

  let acceptSuggestionShiftEnter = {
    key: "<S-TAB>",
    command: Commands.acceptSelected.id,
    condition: suggestWidgetVisible,
  };
};

module Contributions = {
  let colors = [];

  let configuration = Configuration.[quickSuggestions.spec];

  let commands =
    Commands.[acceptSelected, selectPrevSuggestion, selectNextSuggestion];

  let contextKeys = ContextKeys.[suggestWidgetVisible];

  let keybindings =
    KeyBindings.[
      nextSuggestion,
      previousSuggestion,
      acceptSuggestionEnter,
      acceptSuggestionTab,
      acceptSuggestionShiftTab,
      acceptSuggestionShiftEnter,
    ];
};

module View = {
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
        ~theme,
        ~tokenTheme,
        ~editorFont,
        ~completions: model,
        (),
      ) => {
    /*let hoverEnabled =
      Configuration.getValue(c => c.editorHoverEnabled, state.configuration);*/
    let items = completions |> allItems;

    let colors: Colors.t = {
      suggestWidgetSelectedBackground:
        Feature_Theme.Colors.EditorSuggestWidget.selectedBackground.from(
          theme,
        ),
      suggestWidgetBackground:
        Feature_Theme.Colors.EditorSuggestWidget.background.from(theme),
      suggestWidgetBorder:
        Feature_Theme.Colors.EditorSuggestWidget.border.from(theme),
      editorForeground: Feature_Theme.Colors.Editor.foreground.from(theme),
      normalModeBackground:
        Feature_Theme.Colors.Oni.normalModeBackground.from(theme),
    };

    // TODO
    let focused = completions.selection;

    let maxWidth =
      items
      |> Array.fold_left(
           (maxWidth, this: Filter.result(CompletionItem.t)) => {
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
        let focused: Filter.result(CompletionItem.t) = items[index];
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
              let CompletionItem.{label: text, kind, _} = item;
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
