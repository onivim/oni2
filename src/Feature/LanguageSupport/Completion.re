open EditorCoreTypes;
open Oni_Core;
open Utility;
open Exthost;

[@deriving show]
type command =
  | AcceptSelected
  | SelectPrevious
  | SelectNext
  | TriggerSuggest;

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
    })
  | CompletionDetailsAvailable({
      handle: int,
      suggestItem: Exthost.SuggestItem.t,
    })
  | CompletionDetailsError({
      handle: int,
      errorMsg: string,
    });

[@deriving show]
type provider = {
  handle: int,
  selector: DocumentSelector.t,
  triggerCharacters: [@opaque] list(Uchar.t),
  supportsResolveDetails: bool,
  extensionId: string,
};

module Internal = {
  let stringsToUchars: list(string) => list(Uchar.t) =
    strings => {
      strings
      |> List.filter_map(str =>
           if (String.length(str) != 1) {
             None;
           } else {
             Some(Uchar.of_char(str.[0]));
           }
         );
    };
};

// CONFIGURATION

module QuickSuggestionsSetting = {
  [@deriving show]
  type t = {
    comments: bool,
    strings: bool,
    other: bool,
  };

  let initial = {comments: false, strings: false, other: true};

  let enabledFor = (~syntaxScope: SyntaxScope.t, {comments, strings, other}) => {
    let isCommentAndAllowed = syntaxScope.isComment && comments;

    let isStringAndAllowed = syntaxScope.isString && strings;

    let isOtherAndAllowed =
      !syntaxScope.isComment && !syntaxScope.isString && other;

    isCommentAndAllowed || isStringAndAllowed || isOtherAndAllowed;
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
  [@deriving show]
  type t = {
    chainedCacheId: option(Exthost.ChainedCacheId.t),
    handle: int,
    label: string,
    kind: Exthost.CompletionKind.t,
    detail: option(string),
    documentation: option(Exthost.MarkdownString.t),
    insertText: string,
    insertTextRules: Exthost.SuggestItem.InsertTextRules.t,
    sortText: string,
    suggestRange: option(Exthost.SuggestItem.SuggestRange.t),
    commitCharacters: list(string),
    additionalTextEdits: list(Exthost.Edit.SingleEditOperation.t),
    command: option(Exthost.Command.t),
  };

  let create = (~handle, item: Exthost.SuggestItem.t) => {
    chainedCacheId: item.chainedCacheId,
    handle,
    label: item.label,
    kind: item.kind,
    detail: item.detail,
    documentation: item.documentation,
    insertText: item |> Exthost.SuggestItem.insertText,
    insertTextRules: item.insertTextRules,
    sortText: item |> Exthost.SuggestItem.sortText,
    suggestRange: item.suggestRange,
    commitCharacters: item.commitCharacters,
    additionalTextEdits: item.additionalTextEdits,
    command: item.command,
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
    trigger: Exthost.CompletionContext.t,
    buffer: [@opaque] Oni_Core.Buffer.t,
    base: string,
    location: EditorCoreTypes.Location.t,
    supportsResolve: bool,
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
           if (String.length(filterText) < String.length(query)) {
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

  let updateItem = (~item: Exthost.SuggestItem.t, {state, _} as model) => {
    switch (state) {
    | Waiting
    | Failure(_)
    | Accepted => model
    | Completed({allItems, _}) =>
      let allItems =
        allItems
        |> List.map((previousItem: Exthost.SuggestItem.t) =>
             if (previousItem.chainedCacheId == item.chainedCacheId) {
               item;
             } else {
               previousItem;
             }
           );

      let filteredItems = filter(~query=model.base, allItems);
      {...model, state: Completed({filteredItems, allItems})};
    };
  };

  let isActive = ({state, _}) => {
    switch (state) {
    | Accepted => false
    | Waiting => true
    | Failure(_) => false
    | Completed(_) => true
    };
  };

  let create = (~trigger, ~buffer, ~base, ~location, ~supportsResolve) => {
    trigger,
    state: Waiting,
    buffer,
    base,
    location,
    supportsResolve,
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

  let update = (~createNew, ~buffer, ~base, ~location, previous) =>
    // If different buffer or location... start over!
    if (Oni_Core.Buffer.getId(buffer)
        != Oni_Core.Buffer.getId(previous.buffer)
        || location != previous.location) {
      if (createNew) {
        Some(
          create(
            ~trigger=previous.trigger,
            ~buffer,
            ~base,
            ~location,
            ~supportsResolve=previous.supportsResolve,
          ),
        );
      } else {
        None;
      };
    } else {
      // Refine results
      Some(refine(~base, previous));
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
    let sortValue = String.compare(a.item.sortText, b.item.sortText);
    if (sortValue == 0) {
      String.compare(a.item.label, b.item.label);
    } else {
      sortValue;
    };
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

let focused = ({allItems, selection, _}) => {
  selection
  |> OptionEx.flatMap(idx =>
       if (idx < Array.length(allItems) && idx >= 0) {
         Some(allItems[idx]);
       } else {
         None;
       }
     );
};

let isActive = (model: model) => {
  model.allItems |> Array.length > 0;
};

let stopInsertMode = model => {
  ...model,
  handleToSession: IntMap.empty,
  allItems: [||],
  selection: None,
};

let cursorMoved =
    (~previous as _, ~current: EditorCoreTypes.Location.t, model) => {
  // Filter providers, such that the completion meets are still
  // valid in the context of the new cursor position

  let isCompletionMeetStillValid = (meetLocation: EditorCoreTypes.Location.t) => {
    // If the line changed, the meet is no longer valid
    meetLocation.line == current.line
    // If the cursor moved before the meet, on the same line,
    // it is no longer valid. The after case is a bit trickier -
    // the cursor moves first, before we get the buffer update,
    // so the meet may not be valid until the buffer update comes through.
    // We'll allow cursor moves after the meet, for now.
    && Index.toZeroBased(meetLocation.column)
    <= Index.toZeroBased(current.column);
  };

  let handleToSession =
    IntMap.filter(
      (_key, session: Session.t) => {
        isCompletionMeetStillValid(Session.(session.location))
      },
      model.handleToSession,
    );

  let allItems = recomputeAllItems(handleToSession);

  let count = Array.length(allItems);

  {
    ...model,
    handleToSession,
    allItems,
    selection: Selection.ensureValidFocus(~count, model.selection),
  };
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
      triggerCharacters: Internal.stringsToUchars(triggerCharacters),
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

let invokeCompletion = (~buffer, ~activeCursor, model) => {
  let candidateProviders =
    model.providers
    |> List.filter(prov =>
         Exthost.DocumentSelector.matchesBuffer(~buffer, prov.selector)
       );

  let location = activeCursor;

  let handleToSession =
    List.fold_left(
      (acc: IntMap.t(Session.t), curr: provider) => {
        acc
        |> IntMap.add(
             curr.handle,
             Session.create(
               ~trigger=
                 Exthost.CompletionContext.{
                   triggerKind: Invoke,
                   triggerCharacter: None,
                 },
               ~buffer,
               ~base="",
               ~location,
               ~supportsResolve=curr.supportsResolveDetails,
             ),
           )
      },
      IntMap.empty,
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

let startCompletion =
    (~startNewSession, ~trigger, ~buffer, ~activeCursor, model) => {
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
            ~triggerCharacters=curr.triggerCharacters,
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
                   startNewSession
                     ? Some(
                         Session.create(
                           ~buffer,
                           ~trigger,
                           ~base,
                           ~location,
                           ~supportsResolve=curr.supportsResolveDetails,
                         ),
                       )
                     : None;
                 }
               | Some(previous) => {
                   Session.update(
                     ~createNew=startNewSession,
                     ~buffer,
                     ~base,
                     ~location,
                     previous,
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

let bufferUpdated =
    (~buffer, ~config, ~activeCursor, ~syntaxScope, ~triggerKey, model) => {
  let quickSuggestionsSetting = Configuration.quickSuggestions.get(config);

  let anySessionsActive =
    model.handleToSession
    |> IntMap.exists((_key, session) => session |> Session.isActive);

  let trigger =
    Exthost.CompletionContext.{
      triggerKind: TriggerCharacter,
      triggerCharacter: triggerKey,
    };

  if (!
        QuickSuggestionsSetting.enabledFor(
          ~syntaxScope,
          quickSuggestionsSetting,
        )) {
    // If we already had started a session (ie, manually triggered) -
    // make sure to continue, even if suggestions are off
    if (anySessionsActive) {
      startCompletion(
        ~trigger,
        ~startNewSession=false,
        ~buffer,
        ~activeCursor,
        model,
      );
    } else {
      model;
    };
  } else {
    startCompletion(
      ~trigger,
      ~startNewSession=true,
      ~buffer,
      ~activeCursor,
      model,
    );
  };
};

let update = (~maybeBuffer, ~activeCursor, msg, model) => {
  let default = (model, Outmsg.Nothing);
  switch (msg) {
  | Command(TriggerSuggest) =>
    maybeBuffer
    |> Option.map(buffer => {
         (invokeCompletion(~buffer, ~activeCursor, model), Outmsg.Nothing)
       })
    |> Option.value(~default)

  | Command(AcceptSelected) =>
    let allItems = allItems(model);

    if (allItems == [||]) {
      default;
    } else {
      switch (model.selection) {
      | None => default
      | Some(focusedIndex) =>
        let result: Filter.result(CompletionItem.t) = allItems[focusedIndex];

        let handle = result.item.handle;

        getMeetLocation(~handle, model)
        |> Option.map((location: EditorCoreTypes.Location.t) => {
             let meetColumn =
               Exthost.SuggestItem.(
                 switch (result.item.suggestRange) {
                 | Some(SuggestRange.Single({startColumn, _})) =>
                   startColumn |> Index.fromOneBased
                 | Some(SuggestRange.Combo({insert, _})) =>
                   Exthost.OneBasedRange.(
                     insert.startColumn |> Index.fromOneBased
                   )
                 | None => location.column
                 }
               );

             let effect =
               Exthost.SuggestItem.InsertTextRules.(
                 matches(~rule=InsertAsSnippet, result.item.insertTextRules)
               )
                 ? Outmsg.InsertSnippet({
                     meetColumn,
                     snippet: result.item.insertText,
                   })
                 : Outmsg.ApplyCompletion({
                     meetColumn,
                     insertText: result.item.insertText,
                   });

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
               effect,
             );
           })
        |> Option.value(~default);
      };
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

  | CompletionDetailsAvailable({handle, suggestItem}) =>
    let handleToSession =
      model.handleToSession
      |> IntMap.update(
           handle,
           Option.map(prev => Session.updateItem(~item=suggestItem, prev)),
         );

    let allItems = recomputeAllItems(handleToSession);
    ({...model, handleToSession, allItems}, Outmsg.Nothing);

  | CompletionDetailsError(_) => (model, Outmsg.Nothing)
  };
};

let sub = (~client, model) => {
  // Subs for each pending handle..
  let handleSubs =
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
               | Ok(v) =>
                 CompletionResultAvailable({handle, suggestResult: v})
               | Error(errorMsg) => CompletionError({handle, errorMsg})
               }
             },
           client,
         )
       });

  // And subs for the current item, if the handle supports resolution
  let resolveSub =
    model
    |> focused
    |> OptionEx.flatMap((focusedItem: Filter.result(CompletionItem.t)) => {
         let handle = focusedItem.item.handle;

         model.handleToSession
         |> IntMap.find_opt(handle)
         |> OptionEx.flatMap(
              ({supportsResolve, buffer, location, _}: Session.t) =>
              if (supportsResolve) {
                focusedItem.item.chainedCacheId
                |> Option.map(chainedCacheId => {
                     Service_Exthost.Sub.completionItem(
                       ~handle,
                       ~chainedCacheId,
                       ~buffer,
                       ~position=location,
                       ~toMsg=
                         fun
                         | Ok(suggestItem) =>
                           CompletionDetailsAvailable({handle, suggestItem})
                         | Error(errorMsg) =>
                           CompletionDetailsError({handle, errorMsg}),
                       client,
                     )
                   });
              } else {
                None;
              }
            );
       })
    |> Option.value(~default=Isolinear.Sub.none);

  [resolveSub, ...handleSubs] |> Isolinear.Sub.batch;
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

  let triggerSuggest =
    define("editor.action.triggerSuggest", Command(TriggerSuggest));
};

// CONTEXTKEYS

module ContextKeys = {
  open WhenExpr.ContextKeys.Schema;

  let suggestWidgetVisible = bool("suggestWidgetVisible", isActive);
};

// KEYBINDINGS

module KeyBindings = {
  open Oni_Input.Keybindings;

  let suggestWidgetVisible =
    "editorTextFocus && suggestWidgetVisible" |> WhenExpr.parse;
  let acceptOnEnter =
    "acceptSuggestionOnEnter && suggestWidgetVisible && editorTextFocus"
    |> WhenExpr.parse;

  let triggerSuggestCondition =
    "editorTextFocus && insertMode && !suggestWidgetVisible" |> WhenExpr.parse;

  let triggerSuggestControlSpace = {
    key: "<C-Space>",
    command: Commands.triggerSuggest.id,
    condition: triggerSuggestCondition,
  };
  let triggerSuggestControlN = {
    key: "<C-N>",
    command: Commands.triggerSuggest.id,
    condition: triggerSuggestCondition,
  };
  let triggerSuggestControlP = {
    key: "<C-N>",
    command: Commands.triggerSuggest.id,
    condition: triggerSuggestCondition,
  };

  let nextSuggestion = {
    key: "<C-N>",
    command: Commands.selectNextSuggestion.id,
    condition: suggestWidgetVisible,
  };

  let nextSuggestionArrow = {
    key: "<DOWN>",
    command: Commands.selectNextSuggestion.id,
    condition: suggestWidgetVisible,
  };

  let previousSuggestion = {
    key: "<C-P>",
    command: Commands.selectPrevSuggestion.id,
    condition: suggestWidgetVisible,
  };
  let previousSuggestionArrow = {
    key: "<UP>",
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
    Commands.[
      acceptSelected,
      selectPrevSuggestion,
      selectNextSuggestion,
      triggerSuggest,
    ];

  let contextKeys = ContextKeys.[suggestWidgetVisible];

  let keybindings =
    KeyBindings.[
      triggerSuggestControlN,
      triggerSuggestControlP,
      triggerSuggestControlSpace,
      nextSuggestion,
      nextSuggestionArrow,
      previousSuggestion,
      previousSuggestionArrow,
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

  let kindToColor = (colors: Oni_Core.ColorTheme.Colors.t) =>
    Feature_Theme.Colors.SymbolIcon.(
      {
        fun
        | Method => methodForeground.from(colors)
        | Function => functionForeground.from(colors)
        | Constructor => constructorForeground.from(colors)
        | Field => fieldForeground.from(colors)
        | Variable => variableForeground.from(colors)
        | Class => classForeground.from(colors)
        | Struct => structForeground.from(colors)
        | Interface => interfaceForeground.from(colors)
        | Module => moduleForeground.from(colors)
        | Property => propertyForeground.from(colors)
        | Event => eventForeground.from(colors)
        | Operator => operatorForeground.from(colors)
        | Unit => unitForeground.from(colors)
        | Value
        | Constant => constantForeground.from(colors)
        | Enum => enumeratorForeground.from(colors)
        | EnumMember => enumeratorMemberForeground.from(colors)
        | Keyword => keywordForeground.from(colors)
        | Text => textForeground.from(colors)
        | Color => colorForeground.from(colors)
        | File => fileForeground.from(colors)
        | Reference => referenceForeground.from(colors)
        | Customcolor => colorForeground.from(colors)
        | Folder => folderForeground.from(colors)
        | TypeParameter => typeParameterForeground.from(colors)
        | User => nullForeground.from(colors)
        | Issue => nullForeground.from(colors)
        | Snippet => snippetForeground.from(colors);
      }
    );

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
        ~theme: Oni_Core.ColorTheme.Colors.t,
        ~colors: Colors.t,
        ~editorFont: Service_Font.font,
        (),
      ) => {
    let icon = kind |> kindToIcon;

    let iconColor = kind |> kindToColor(theme);

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
                theme
                colors
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
