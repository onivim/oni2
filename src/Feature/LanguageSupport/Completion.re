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
  | Keyword(CompletionProvider.keywordMsg)
  | Snippet(CompletionProvider.snippetMsg);

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
    | Partial({
        providerModel: [@opaque] 'model,
        meet: CompletionMeet.t,
        cursor: CharacterPosition.t,
        currentItems: list(CompletionItem.t),
        filteredItems: [@opaque] list(CompletionItem.t),
      })
    | Completed({
        providerModel: [@opaque] 'model,
        meet: CompletionMeet.t,
        allItems: list(CompletionItem.t),
        filteredItems: [@opaque] list(CompletionItem.t),
      })
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
          | Partial({meet, _})
          | Completed({meet, _})
          | Accepted({meet}) =>
            let meet' =
              CompletionMeet.shiftMeet(~edits=additionalEdits, meet);
            Accepted({meet: meet'});
          | Failure(_) as failure => failure
          };

        Session({...session, state: state'});
      };

  let cancel = complete([]);

  let start =
    fun
    | Session(session) => Session({...session, state: NotStarted});

  let stop =
    fun
    | Session(session) => Session({...session, state: NotStarted});

  let filter:
    (~query: string, list(CompletionItem.t)) => list(CompletionItem.t) =
    (~query, items) => {
      let explodedQuery = Zed_utf8.explode(query |> String.lowercase_ascii);
      items
      |> List.filter((item: CompletionItem.t) =>
           if (String.length(item.filterText) < String.length(query)) {
             false;
           } else {
             Filter.fuzzyMatches(
               explodedQuery,
               item.filterText |> String.lowercase_ascii,
             );
           }
         );
    };

  let filteredItems =
    fun
    | Session({state, _}) => {
        switch (state) {
        | NotStarted => StringMap.empty
        | Accepted(_) => StringMap.empty
        | Pending(_) => StringMap.empty
        | Failure(_) => StringMap.empty
        | Partial({filteredItems, _})
        | Completed({filteredItems, _}) =>
          filteredItems
          |> List.fold_left(
               (acc, item: CompletionItem.t) => {
                 StringMap.add(item.label, item, acc)
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

             let handleNewItems =
                 (~providerModel, ~meet: CompletionMeet.t, ~cursor) => {
               // TODO: What to do with the error `_outmsg` case?
               let (providerModel', _outmsg) =
                 ProviderImpl.update(internalMsg, providerModel);
               let (completionState, items) =
                 ProviderImpl.items(providerModel');
               switch (completionState, items) {
               | (_, []) => Pending({meet, providerModel: providerModel'})
               | (Incomplete, items) =>
                 Partial({
                   meet,
                   cursor,
                   currentItems: items,
                   filteredItems: items,
                   providerModel: providerModel',
                 })
               | (Complete, items) =>
                 Completed({
                   meet,
                   allItems: items,
                   filteredItems: filter(~query=meet.base, items),
                   providerModel: providerModel',
                 })
               };
             };

             let state' =
               switch (state) {
               | Partial({providerModel, meet, cursor, _}) =>
                 handleNewItems(~providerModel, ~meet, ~cursor)

               | Completed({providerModel, meet, _})
               | Pending({providerModel, meet, _}) =>
                 handleNewItems(~providerModel, ~meet, ~cursor=meet.location)

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
        | Partial({providerModel, cursor, _}) =>
          let (module ProviderImpl) = provider;
          ProviderImpl.sub(
            ~client,
            ~context=
              Exthost.CompletionContext.{
                triggerKind: Exthost.CompletionContext.TriggerForIncompleteCompletions,
                triggerCharacter: None,
              },
            ~buffer=activeBuffer,
            ~position=cursor,
            ~selectedItem,
            providerModel,
          )
          |> Isolinear.Sub.map(msg => Provider(providerMapper(msg)));
        | Pending({providerModel, meet, _})
        | Completed({providerModel, meet, _}) =>
          let (module ProviderImpl) = provider;
          ProviderImpl.sub(
            ~client,
            // TODO: Proper completion context
            ~context=
              Exthost.CompletionContext.{
                triggerKind: Invoke,
                triggerCharacter: None,
              },
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
               | Partial({currentItems, _} as prev) =>
                 Partial({
                   ...prev,
                   meet: newMeet,
                   cursor: position,
                   filteredItems: currentItems,
                 })
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
      (
        ~config,
        ~extensions,
        ~languageConfiguration,
        ~trigger,
        ~buffer,
        ~location,
      ) =>
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
               ~extensions,
               ~languageConfiguration,
               ~base=meet.base,
               ~trigger,
               ~buffer,
               ~location=meet.insertLocation,
             )
             |> Option.map(model => (meet, model))
           })
        |> Option.map(((meet, model)) => {
             let state =
               switch (session.state) {
               | Partial(partial) =>
                 Partial({...partial, meet, cursor: location})
               | _ =>
                 let (_isComplete, items) = ProviderImpl.items(model);
                 switch (items) {
                 | [] => Pending({meet, providerModel: model})
                 | items =>
                   Completed({
                     meet,
                     allItems: items,
                     filteredItems: filter(~query=meet.base, items),
                     providerModel: model,
                   })
                 };
               };

             Session({...session, state});
           })
        |> Option.value(~default=original);
      };

  let reinvoke =
      (
        ~config,
        ~extensions,
        ~languageConfiguration,
        ~trigger,
        ~buffer,
        ~activeCursor,
      ) =>
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
                     ~extensions,
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
                   ~extensions,
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
  allItems: array(CompletionItem.t),
  selection: Selection.t,
  isInsertMode: bool,
  isSnippetMode: bool,
  acceptOnEnter: bool,
  snippetSortOrder: [ | `Bottom | `Hidden | `Inline | `Top],
};

let initial = {
  isInsertMode: false,
  isSnippetMode: false,
  acceptOnEnter: false,
  providers: [
    Session.create(
      ~triggerCharacters=[],
      ~provider=CompletionProvider.keyword,
      ~mapper=msg => Keyword(msg),
      ~revMapper=
        fun
        | Keyword(msg) => Some(msg)
        | _ => None,
    ),
    Session.create(
      ~triggerCharacters=[],
      ~provider=CompletionProvider.snippet,
      ~mapper=msg => Snippet(msg),
      ~revMapper=
        fun
        | Snippet(msg) => Some(msg)
        | _ => None,
    ),
  ],
  allItems: [||],
  selection: Selection.initial,
  snippetSortOrder: `Inline,
};

let configurationChanged = (~config, model) => {
  {
    ...model,
    acceptOnEnter: CompletionConfig.acceptSuggestionOnEnter.get(config),
    snippetSortOrder: CompletionConfig.snippetSuggestions.get(config),
  };
};

let providerCount = ({providers, _}) => List.length(providers) - 1;
let availableCompletionCount = ({allItems, _}) => Array.length(allItems);

let recomputeAllItems =
    (
      ~maybeBuffer,
      ~cursor: CharacterPosition.t,
      ~snippetSortOrder,
      providers: list(Session.t),
    ) => {
  let maybeLine =
    maybeBuffer |> OptionEx.flatMap(Buffer.rawLine(cursor.line));

  providers
  |> List.fold_left(
       (acc, session) => {
         let itemMap = session |> Session.filteredItems;

         // When we have the same key coming from different providers, need to decide between them
         StringMap.merge(
           (
             _key,
             maybeA: option(CompletionItem.t),
             maybeB: option(list(CompletionItem.t)),
           ) =>
             switch (maybeA, maybeB) {
             | (None, None) => None
             | (Some(a), None) => Some([a])
             | (None, Some(_) as b) => b
             | (Some(a), Some(b)) => Some([a, ...b])
             },
           itemMap,
           acc,
         );
       },
       StringMap.empty,
     )
  |> StringMap.map(items => {
       switch (items) {
       // If one or zero items, just keep as-is
       | [] => []
       | [item] => [item]
       // If multiple - remove keywords
       | items =>
         items |> List.filter(item => !(item |> CompletionItem.isKeyword))
       }
     })
  |> StringMap.bindings
  |> List.concat_map(snd)
  |> List.map((item: CompletionItem.t) => {
       maybeLine
       |> Option.map(line => {
            let score = CompletionItemScorer.score(line, item, cursor);
            {...item, score};
          })
       |> Option.value(~default=item)
     })
  |> List.fast_sort((a, b) =>
       CompletionItemSorter.compare(~snippetSortOrder, a, b)
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
  model.isInsertMode
  && !model.isSnippetMode
  && model.allItems
  |> Array.length > 0;
};

let reset = model =>
  if (model.isInsertMode && !model.isSnippetMode) {
    {
      ...model,
      providers: model.providers |> List.map(Session.start),
      allItems: [||],
      selection: None,
    };
  } else {
    {
      ...model,
      providers: model.providers |> List.map(Session.stop),
      allItems: [||],
      selection: None,
    };
  };

let startInsertMode = model => {
  {...model, isInsertMode: true} |> reset;
};

let stopInsertMode = model => {
  {...model, isInsertMode: false} |> reset;
};

let cancel = model => {
  ...model,
  providers: model.providers |> List.map(Session.cancel),
  allItems: [||],
  selection: None,
};

// There are some bugs with completion in snippet mode -
// including the 'tab' key being overloaded. Need to fix
// these and gate with a configuration setting, like:
// `editor.suggest.snippetsPreventQuickSuggestions`
let startSnippet = model => {
  {...model, isSnippetMode: true} |> reset;
};
let stopSnippet = model => {
  {...model, isSnippetMode: false} |> reset;
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

let updateSessions = (~buffer, ~activeCursor, providers, model) => {
  let allItems =
    recomputeAllItems(
      ~cursor=activeCursor,
      ~maybeBuffer=Some(buffer),
      ~snippetSortOrder=model.snippetSortOrder,
      providers,
    );
  let selection =
    Selection.ensureValidFocus(
      ~count=Array.length(allItems),
      model.selection,
    );
  {...model, providers, allItems, selection};
};

let cursorMoved =
    (
      ~languageConfiguration,
      ~buffer,
      ~current: EditorCoreTypes.CharacterPosition.t,
      model,
    ) => {
  // Filter providers, such that the completion meets are still
  // valid in the context of the new cursor position

  let providers' =
    model.providers
    |> List.map(
         Session.refine(~languageConfiguration, ~buffer, ~position=current),
       );

  model |> updateSessions(~activeCursor=current, ~buffer, providers');
};

let invokeCompletion =
    (
      ~config,
      ~extensions,
      ~languageConfiguration,
      ~trigger,
      ~buffer,
      ~activeCursor,
      model,
    ) => {
  let providers' =
    model.providers
    |> List.map(
         Session.reinvoke(
           ~config,
           ~extensions,
           ~languageConfiguration,
           ~trigger,
           ~buffer,
           ~activeCursor,
         ),
       );

  model |> updateSessions(~activeCursor, ~buffer, providers');
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

  model |> updateSessions(~activeCursor, ~buffer, providers');
};

let bufferUpdated =
    (
      ~languageConfiguration,
      ~extensions,
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
         ~extensions,
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
  | Some(focusedIndex) => Some(model.allItems[focusedIndex])
  };
};

let update =
    (
      ~config,
      ~extensions,
      ~languageConfiguration,
      ~maybeBuffer,
      ~activeCursor,
      msg,
      model,
    ) => {
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
             ~extensions,
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
        let result = allItems[focusedIndex];

        let replaceSpan = CompletionItem.replaceSpan(~activeCursor, result);

        let effect =
          Exthost.SuggestItem.InsertTextRules.(
            matches(~rule=InsertAsSnippet, result.insertTextRules)
          )
            ? Outmsg.InsertSnippet({
                replaceSpan,
                snippet: result.insertText,
                additionalEdits: result.additionalTextEdits,
              })
            : Outmsg.ApplyCompletion({
                replaceSpan,
                insertText: result.insertText,
                additionalEdits: result.additionalTextEdits,
              });

        (
          {
            ...model,
            providers:
              List.map(
                provider => {
                  provider |> Session.complete(result.additionalTextEdits)
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
    let providers =
      model.providers |> List.map(provider => Session.update(msg, provider));
    let allItems =
      recomputeAllItems(
        ~cursor=activeCursor,
        ~maybeBuffer,
        ~snippetSortOrder=model.snippetSortOrder,
        providers,
      );
    let selection =
      Selection.ensureValidFocus(
        ~count=Array.length(allItems),
        // If we 'pinned' an item, make sure it's selected
        model.selection,
      );

    let model' = {...model, providers, allItems, selection};

    // If the current selection is different than the one we had before...
    (model', Nothing);
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

  let acceptSuggestionOnEnter =
    bool("acceptSuggestionOnEnter", model => model.acceptOnEnter);
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

  let triggerSuggestControlSpace =
    bind(
      ~key="<C-Space>",
      ~command=Commands.triggerSuggest.id,
      ~condition=triggerSuggestCondition,
    );
  let triggerSuggestControlN =
    bind(
      ~key="<C-N>",
      ~command=Commands.triggerSuggest.id,
      ~condition=triggerSuggestCondition,
    );
  let triggerSuggestControlP =
    bind(
      ~key="<C-P>",
      ~command=Commands.triggerSuggest.id,
      ~condition=triggerSuggestCondition,
    );

  let nextSuggestion =
    bind(
      ~key="<C-N>",
      ~command=Commands.selectNextSuggestion.id,
      ~condition=suggestWidgetVisible,
    );

  let nextSuggestionArrow =
    bind(
      ~key="<DOWN>",
      ~command=Commands.selectNextSuggestion.id,
      ~condition=suggestWidgetVisible,
    );

  let previousSuggestion =
    bind(
      ~key="<C-P>",
      ~command=Commands.selectPrevSuggestion.id,
      ~condition=suggestWidgetVisible,
    );
  let previousSuggestionArrow =
    bind(
      ~key="<UP>",
      ~command=Commands.selectPrevSuggestion.id,
      ~condition=suggestWidgetVisible,
    );

  let acceptSuggestionEnter =
    bind(
      ~key="<CR>",
      ~command=Commands.acceptSelected.id,
      ~condition=acceptOnEnter,
    );

  let acceptSuggestionTab =
    bind(
      ~key="<TAB>",
      ~command=Commands.acceptSelected.id,
      ~condition=suggestWidgetVisible,
    );

  let acceptSuggestionShiftTab =
    bind(
      ~key="<S-TAB>",
      ~command=Commands.acceptSelected.id,
      ~condition=suggestWidgetVisible,
    );

  let acceptSuggestionShiftEnter =
    bind(
      ~key="<S-CR>",
      ~command=Commands.acceptSelected.id,
      ~condition=suggestWidgetVisible,
    );
};

module Contributions = {
  let colors = [];

  let configuration =
    CompletionConfig.[
      quickSuggestions.spec,
      wordBasedSuggestions.spec,
      acceptSuggestionOnEnter.spec,
      snippetSuggestions.spec,
    ];

  let commands =
    Commands.[
      acceptSelected,
      selectPrevSuggestion,
      selectNextSuggestion,
      triggerSuggest,
    ];

  let contextKeys =
    ContextKeys.[acceptSuggestionOnEnter, suggestWidgetVisible];

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
    | Snippet => Codicon.symbolSnippet;

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
        ~buffer,
        ~cursor: CharacterPosition.t,
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

    let maybeLine = buffer |> Buffer.rawLine(cursor.line);

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
           (maxWidth, this: CompletionItem.t) => {
             let textWidth =
               Service_Font.measure(~text=this.label, editorFont);
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
        let focused: CompletionItem.t = items[index];
        switch (focused.detail) {
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
              let item = items[index];
              let CompletionItem.{label: text, kind, _} = item;

              let highlight =
                maybeLine
                |> Option.map(line => {
                     CompletionItemScorer.highlights(line, item, cursor)
                   })
                |> Option.value(~default=[]);

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
