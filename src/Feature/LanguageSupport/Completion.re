open EditorCoreTypes;
open Oni_Core;
open Utility;

[@deriving show]
type command =
  | AcceptSelected
  | SelectPrevious
  | SelectNext
  | TriggerSuggest;

[@deriving show]
type providerMsg =
  | Exthost(CompletionProvider.exthostMsg)
  | Keyword(CompletionProvider.keywordMsg);

[@deriving show]
type msg =
  | Command(command)
  | Provider(providerMsg);

type exthostMsg;

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

module Session = {
  [@deriving show]
  type state('model) =
    | NotStarted
    | Pending({
        providerModel: [@opaque] 'model,
        meet: CompletionMeet.t,
      })
    | Completed({
        providerModel: [@opaque] 'model,
        meet: CompletionMeet.t,
        allItems: list(CompletionItem.t),
        filteredItems: [@opaque] list(Filter.result(CompletionItem.t)),
      })
    // TODO
    //| Incomplete(list(Exthost.SuggestItem.t))
    | Failure(string)
    | Accepted({meet: CompletionMeet.t});

  [@deriving show]
  type session('model, 'msg) = {
    state: state('model),
    triggerCharacters: [@opaque] list(Uchar.t),
    provider: [@opaque] CompletionProvider.provider('model, 'msg),
    providerMapper: 'msg => providerMsg,
    revMapper: providerMsg => option('msg),
  };

  type t =
    | Session(session('model, 'msg)): t;

  let location =
    fun
    | Session({state, _}) =>
      switch (state) {
      | Pending({meet, _}) => Some(CompletionMeet.(meet.location))
      | Completed({meet, _}) => Some(CompletionMeet.(meet.location))
      | _ => None
      };

  let handle =
    fun
    | Session({provider, _}) => {
        let (module ProviderImpl) = provider;
        ProviderImpl.handle;
      };

  let complete = additionalEdits =>
    fun
    | Session({state, _} as session) => {
        let state' =
          switch (state) {
          | NotStarted => NotStarted
          | Pending({meet, _})
          | Completed({meet, _})
          | Accepted({meet}) =>
            let meet' =
              CompletionMeet.shiftMeet(~edits=additionalEdits, meet);
            Accepted({meet: meet'});
          | Failure(_) as failure => failure
          };

        Session({...session, state: state'});
      };

  let start =
    fun
    | Session(session) => Session({...session, state: NotStarted});

  let stop =
    fun
    | Session(session) => Session({...session, state: NotStarted});

  let cursorMoved = (~position: CharacterPosition.t) =>
    fun
    | Session({state, _} as prevState) => {
        let isCompletionMeetStillValid = (meet: CompletionMeet.t) => {
          // If the line changed, the meet is no longer valid
          meet.location.line == position.line
          // If the cursor moved before the meet, on the same line,
          // it is no longer valid. The after case is a bit trickier -
          // the cursor moves first, before we get the buffer update,
          // so the meet may not be valid until the buffer update comes through.
          // We'll allow cursor moves after the meet, for now.
          && CharacterIndex.toInt(meet.location.character)
          <= CharacterIndex.toInt(position.character);
        };
        let state' =
          switch (state) {
          | Pending({meet, _}) as prev when isCompletionMeetStillValid(meet) => prev
          | Pending(_) => NotStarted
          | Completed({meet, _}) as prev
              when isCompletionMeetStillValid(meet) => prev
          | Completed(_) => NotStarted
          | Accepted({meet, _}) as prev when isCompletionMeetStillValid(meet) => prev
          | Accepted(_) => NotStarted
          | prev => prev
          };

        Session({...prevState, state: state'});
      };

  let filter:
    (~query: string, list(CompletionItem.t)) =>
    list(Filter.result(CompletionItem.t)) =
    (~query, items) => {
      let toString = (item: CompletionItem.t, ~shouldLower) =>
        shouldLower ? String.lowercase_ascii(item.label) : item.label;

      let explodedQuery = Zed_utf8.explode(query);

      let items' =
        items
        |> List.filter((item: CompletionItem.t) =>
             if (String.length(item.filterText) < String.length(query)) {
               false;
             } else {
               Filter.fuzzyMatches(explodedQuery, item.filterText);
             }
           )
        |> Filter.rank(query, toString);

      // Tag as fuzzy matched
      if (explodedQuery != []) {
        items'
        |> List.map((filterItem: Filter.result(CompletionItem.t)) =>
             {
               ...filterItem,
               item: {
                 ...filterItem.item,
                 isFuzzyMatching: true,
               },
             }
           );
      } else {
        items';
      };
    };

  let filteredItems =
    fun
    | Session({state, _}) => {
        switch (state) {
        | NotStarted => StringMap.empty
        | Accepted(_) => StringMap.empty
        | Pending(_) => StringMap.empty
        | Failure(_) => StringMap.empty
        | Completed({filteredItems, meet, _}) =>
          filteredItems
          |> List.fold_left(
               (acc, item: Filter.result(CompletionItem.t)) => {
                 StringMap.add(item.item.label, (meet.location, item), acc)
               },
               StringMap.empty,
             )
        };
      };

  let update = msg =>
    fun
    | Session({provider, revMapper, state, _} as session) => {
        revMapper(msg)
        |> Option.map(internalMsg => {
             let (module ProviderImpl) = provider;

             let state' =
               switch (state) {
               | Completed({providerModel, meet, _})
               | Pending({providerModel, meet, _}) =>
                 open CompletionMeet;
                 let providerModel' =
                   ProviderImpl.update(
                     ~isFuzzyMatching=meet.base != "",
                     internalMsg,
                     providerModel,
                   );
                 switch (ProviderImpl.items(providerModel')) {
                 | Ok([]) => Pending({meet, providerModel: providerModel'})
                 | Ok(items) =>
                   Completed({
                     meet,
                     allItems: items,
                     filteredItems: filter(~query=meet.base, items),
                     providerModel: providerModel',
                   })
                 | Error(msg) => Failure(msg)
                 };
               | _ => state
               };

             Session({...session, state: state'});
           })
        |> Option.value(~default=Session(session));
      };

  let sub = (~activeBuffer, ~selectedItem, ~client) =>
    fun
    | Session({state, provider, providerMapper, _}) => {
        switch (state) {
        | Pending({providerModel, meet, _})
        | Completed({providerModel, meet, _}) =>
          let (module ProviderImpl) = provider;
          ProviderImpl.sub(
            ~client,
            ~buffer=activeBuffer,
            ~position=CompletionMeet.(meet.location),
            ~selectedItem,
            providerModel,
          )
          |> Isolinear.Sub.map(msg => Provider(providerMapper(msg)));
        | _ => Isolinear.Sub.none
        };
      };

  let create =
      (
        ~provider: CompletionProvider.provider('model, 'msg),
        ~mapper: 'msg => providerMsg,
        ~revMapper: providerMsg => option('msg),
        ~triggerCharacters,
      ) => {
    Session({
      triggerCharacters,
      state: NotStarted,
      provider,
      providerMapper: mapper,
      revMapper,
    });
  };

  let refine = (~languageConfiguration, ~buffer, ~position) =>
    fun
    | Session(current) => {
        let maybeMeet =
          CompletionMeet.fromBufferPosition(
            ~languageConfiguration,
            ~triggerCharacters=current.triggerCharacters,
            ~position,
            buffer,
          );

        let state' =
          maybeMeet
          |> Option.map(newMeet => {
               switch (current.state) {
               | NotStarted => NotStarted
               | Accepted(_) as accepted => accepted
               | Pending(_) as pending => pending
               | Failure(_) as failure => failure
               | Completed({allItems, meet, _} as prev)
                   when CompletionMeet.matches(meet, newMeet) =>
                 Completed({
                   ...prev,
                   meet: newMeet,
                   filteredItems:
                     filter(~query=CompletionMeet.(newMeet.base), allItems),
                 })
               // The meet changed on us - reset
               | Completed(_) => NotStarted
               }
             })
          |> Option.value(~default=NotStarted);

        Session({...current, state: state'});
      };

  let tryInvoke =
      (~config, ~languageConfiguration, ~trigger, ~buffer, ~location) =>
    fun
    | Session({provider, _} as session) as original => {
        let (module ProviderImpl) = provider;
        let maybeMeet =
          CompletionMeet.fromBufferPosition(
            ~languageConfiguration,
            ~triggerCharacters=session.triggerCharacters,
            ~position=location,
            buffer,
          );
        maybeMeet
        |> OptionEx.flatMap((meet: CompletionMeet.t) => {
             ProviderImpl.create(
               ~config,
               ~languageConfiguration,
               ~base=meet.base,
               ~trigger,
               ~buffer,
               ~location=meet.location,
             )
             |> Option.map(model => (meet, model))
           })
        |> Option.map(((meet, model)) => {
             let items = ProviderImpl.items(model);
             let state =
               switch (items) {
               | Ok([]) => Pending({meet, providerModel: model})
               | Ok(items) =>
                 Completed({
                   meet,
                   allItems: items,
                   filteredItems: filter(~query=meet.base, items),
                   providerModel: model,
                 })
               | Error(msg) => Failure(msg)
               };

             Session({...session, state});
           })
        |> Option.value(~default=original);
      };

  let reinvoke =
      (~config, ~languageConfiguration, ~trigger, ~buffer, ~activeCursor) =>
    fun
    | Session(previous) as model => {
        let maybeMeet =
          CompletionMeet.fromBufferPosition(
            ~languageConfiguration,
            ~triggerCharacters=previous.triggerCharacters,
            ~position=activeCursor,
            buffer,
          );

        maybeMeet
        |> Option.map(({location, _}: CompletionMeet.t)
             // If different buffer or location... start over!
             =>
               switch (previous.state) {
               | Accepted({meet})
               | Completed({meet, _}) =>
                 if (Oni_Core.Buffer.getId(buffer)
                     != CompletionMeet.(meet.bufferId)
                     || location != CompletionMeet.(meet.location)) {
                   // try to complete
                   tryInvoke(
                     ~config,
                     ~languageConfiguration,
                     ~trigger,
                     ~buffer,
                     ~location=activeCursor,
                     model,
                   );
                 } else {
                   // The base and location are the same
                   // Just refine existing query
                   refine(
                     ~languageConfiguration,
                     ~buffer,
                     ~position=activeCursor,
                     model,
                   );
                 }
               | _incomplete =>
                 tryInvoke(
                   ~config,
                   ~languageConfiguration,
                   ~trigger,
                   ~buffer,
                   ~location=activeCursor,
                   model,
                 )
               }
             )
        |> Option.value(~default=model |> stop);
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
  providers: list(Session.t),
  allItems: array((CharacterPosition.t, Filter.result(CompletionItem.t))),
  selection: Selection.t,
  isInsertMode: bool,
};

let initial = {
  isInsertMode: false,
  providers: [
    Session.create(
      ~triggerCharacters=[],
      // Remove from command
      ~provider=CompletionProvider.keyword,
      ~mapper=msg => Keyword(msg),
      ~revMapper=
        fun
        | Keyword(msg) => Some(msg)
        | _ => None,
    ),
  ],
  allItems: [||],
  selection: Selection.initial,
};

let providerCount = ({providers, _}) => List.length(providers) - 1;
let availableCompletionCount = ({allItems, _}) => Array.length(allItems);

let recomputeAllItems = (providers: list(Session.t)) => {
  providers
  |> List.fold_left(
       (acc, session) => {
         let itemMap = session |> Session.filteredItems;
         // When we have the same key coming from different providers, need to decide between them
         StringMap.union(
           (
             _key,
             (locA, itemA: Filter.result(CompletionItem.t)),
             (locB, itemB: Filter.result(CompletionItem.t)),
           ) =>
             if (CompletionItem.prefer(itemA.item, itemB.item) < 0) {
               Some((locA, itemA));
             } else {
               Some((locB, itemB));
             },
           acc,
           itemMap,
         );
       },
       StringMap.empty,
     )
  |> StringMap.bindings
  |> List.map(snd)
  |> List.fast_sort(((_loc, a), (_loc, b)) =>
       CompletionItemSorter.compare(a, b)
     )
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
  model.isInsertMode && model.allItems |> Array.length > 0;
};

let startInsertMode = model => {
  {
    isInsertMode: true,
    providers: model.providers |> List.map(Session.start),
    allItems: [||],
    selection: None,
  };
};

let stopInsertMode = model => {
  isInsertMode: false,
  providers: model.providers |> List.map(Session.stop),
  allItems: [||],
  selection: None,
};

let register =
    (
      ~handle,
      ~selector,
      ~triggerCharacters,
      ~supportsResolveDetails,
      ~extensionId as _,
      model,
    ) => {
  let triggerCharacters = Internal.stringsToUchars(triggerCharacters);

  {
    ...model,
    providers: [
      Session.create(
        ~triggerCharacters,
        ~provider=
          CompletionProvider.exthost(
            ~selector,
            ~handle,
            ~supportsResolve=supportsResolveDetails,
          ),
        ~mapper=msg => Exthost(msg),
        ~revMapper=
          fun
          | Exthost(msg) => Some(msg)
          | _ => None,
      ),
      ...model.providers,
    ],
  };
};

let unregister = (~handle, model) => {
  ...model,
  providers:
    List.filter(
      session => Session.handle(session, ()) != Some(handle),
      model.providers,
    ),
};

let updateSessions = (providers, model) => {
  let allItems = recomputeAllItems(providers);
  let selection =
    Selection.ensureValidFocus(
      ~count=Array.length(allItems),
      model.selection,
    );
  {...model, providers, allItems, selection};
};

let cursorMoved =
    (~previous as _, ~current: EditorCoreTypes.CharacterPosition.t, model) => {
  // Filter providers, such that the completion meets are still
  // valid in the context of the new cursor position

  let providers' =
    model.providers |> List.map(Session.cursorMoved(~position=current));

  model |> updateSessions(providers');
};

let invokeCompletion =
    (~config, ~languageConfiguration, ~trigger, ~buffer, ~activeCursor, model) => {
  let providers' =
    model.providers
    |> List.map(
         Session.reinvoke(
           ~config,
           ~languageConfiguration,
           ~trigger,
           ~buffer,
           ~activeCursor,
         ),
       );

  model |> updateSessions(providers');
};

let refine = (~languageConfiguration, ~buffer, ~activeCursor, model) => {
  let providers' =
    model.providers
    |> List.map(
         Session.refine(
           ~languageConfiguration,
           ~buffer,
           ~position=activeCursor,
         ),
       );

  model |> updateSessions(providers');
};

let bufferUpdated =
    (
      ~languageConfiguration,
      ~buffer,
      ~config,
      ~activeCursor,
      ~syntaxScope,
      ~triggerKey,
      model,
    ) => {
  let quickSuggestionsSetting = CompletionConfig.quickSuggestions.get(config);

  let trigger =
    Exthost.CompletionContext.{
      triggerKind: TriggerCharacter,
      triggerCharacter: triggerKey,
    };

  if (!
        CompletionConfig.QuickSuggestionsSetting.enabledFor(
          ~syntaxScope,
          quickSuggestionsSetting,
        )) {
    // If we already had started a session (ie, manually triggered) -
    // make sure to continue, even if suggestions are off
    model |> refine(~languageConfiguration, ~buffer, ~activeCursor);
  } else {
    model
    |> invokeCompletion(
         ~config,
         ~languageConfiguration,
         ~trigger,
         ~buffer,
         ~activeCursor,
       );
  };
};

let selected = model => {
  switch (model.selection) {
  | None => None
  | Some(focusedIndex) =>
    let (_: CharacterPosition.t, result: Filter.result(CompletionItem.t)) = model.
                                                                    allItems[focusedIndex];
    Some(result.item);
  };
};

let tryToMaintainSelected = (~previousIndex, ~previousLabel, model) => {
  let len = Array.length(model.allItems);
  let idxToReplace = min(Array.length(model.allItems) - 1, previousIndex);

  let getItemAtIndex = idx => {
    let (_position, filterResult: Filter.result(CompletionItem.t)) = model.
                                                                    allItems[idx];
    filterResult.item;
  };

  // Sanity check - make sure there is a valid position.
  // Completions list might be empty...
  if (idxToReplace >= len) {
    model;
  } else if
    // Nothing to do - still in a good spot!
    (getItemAtIndex(idxToReplace).label == previousLabel) {
    model;
  } else {
    let idx = ref(-1);
    let foundCurrent = ref(None);
    while (idx^ < len && foundCurrent^ == None) {
      incr(idx);
      if (getItemAtIndex(idx^).label == previousLabel) {
        foundCurrent := Some(idx^);
      };
    };

    switch (foundCurrent^) {
    | Some(idx) when idx == idxToReplace => model
    | Some(idx) =>
      // Swap idx and idx to replace, to maintain selected
      let a = model.allItems[idxToReplace];
      let b = model.allItems[idx];
      model.allItems[idx] = a;
      model.allItems[idxToReplace] = b;
      model;
    | None => model
    };
  };
};

let update =
    (~config, ~languageConfiguration, ~maybeBuffer, ~activeCursor, msg, model) => {
  let default = (model, Outmsg.Nothing);
  switch (msg) {
  | Command(TriggerSuggest) =>
    maybeBuffer
    |> Option.map(buffer => {
         let trigger =
           Exthost.CompletionContext.{
             triggerKind: Invoke,
             triggerCharacter: None,
           };
         (
           invokeCompletion(
             ~config,
             ~languageConfiguration,
             ~trigger,
             ~buffer,
             ~activeCursor,
             model,
           ),
           Outmsg.Nothing,
         );
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
        let (
          location: CharacterPosition.t,
          result: Filter.result(CompletionItem.t),
        ) = allItems[focusedIndex];

        let meetColumn =
          Exthost.SuggestItem.(
            switch (result.item.suggestRange) {
            | Some(SuggestRange.Single({startColumn, _})) =>
              startColumn - 1 |> CharacterIndex.ofInt
            | Some(SuggestRange.Combo({insert, _})) =>
              Exthost.OneBasedRange.(
                insert.startColumn - 1 |> CharacterIndex.ofInt
              )
            | None => location.character
            }
          );

        let effect =
          Exthost.SuggestItem.InsertTextRules.(
            matches(~rule=InsertAsSnippet, result.item.insertTextRules)
          )
            ? Outmsg.InsertSnippet({
                meetColumn,
                snippet: result.item.insertText,
                additionalEdits: result.item.additionalTextEdits,
              })
            : Outmsg.ApplyCompletion({
                meetColumn,
                insertText: result.item.insertText,
                additionalEdits: result.item.additionalTextEdits,
              });

        (
          {
            ...model,
            providers:
              List.map(
                provider => {
                  provider
                  |> Session.complete(result.item.additionalTextEdits)
                },
                model.providers,
              ),
            allItems: [||],
            selection: None,
          },
          effect,
        );
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

  | Provider(msg) =>
    // Before updating any items, check to see if there is already
    // a selected item. We don't want to surprise the user
    // and change the selected underneath them when populating!
    let maybeCurrentSelection =
      switch (model.selection) {
      | None => None
      | Some(idx) => selected(model) |> Option.map(item => (idx, item))
      };

    let providers =
      model.providers |> List.map(provider => Session.update(msg, provider));
    let allItems = recomputeAllItems(providers);
    let selection =
      Selection.ensureValidFocus(
        ~count=Array.length(allItems),
        // If we 'pinned' an item, make sure it's selected
        model.selection,
      );

    let model' = {...model, providers, allItems, selection};

    let model'' =
      switch (maybeCurrentSelection) {
      | Some((previousIndex, previousItem)) =>
        model'
        |> tryToMaintainSelected(
             ~previousIndex,
             ~previousLabel=previousItem.label,
           )
      | None => model'
      };

    // If the current selection is different than the one we had before...
    (model'', Nothing);
  };
};

let sub = (~activeBuffer, ~client, model) =>
  // Subs for each pending handle..
  if (model.isInsertMode) {
    let selectedItem = selected(model);
    model.providers
    |> List.map((meet: Session.t) => {
         Session.sub(~activeBuffer, ~client, ~selectedItem, meet)
       })
    |> Isolinear.Sub.batch;
  } else {
    Isolinear.Sub.none;
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
  open Feature_Input.Schema;

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

  let configuration =
    CompletionConfig.[quickSuggestions.spec, wordBasedSuggestions.spec];

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
           (maxWidth, (_loc, this: Filter.result(CompletionItem.t))) => {
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
        let focused: Filter.result(CompletionItem.t) = items[index] |> snd;
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
              let Filter.{highlight, item, _} = items[index] |> snd;
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
