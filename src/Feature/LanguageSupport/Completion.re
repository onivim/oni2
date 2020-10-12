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
type providerMsg =
  | Exthost(CompletionProvider.exthostMsg)
  | Keyword(CompletionProvider.keywordMsg);

[@deriving show]
type msg =
  | Command(command)
  | Provider(providerMsg);
//  | CompletionResultAvailable({
//      handle: int,
//      suggestResult: Exthost.SuggestResult.t,
//    })
//  | CompletionError({
//      handle: int,
//      errorMsg: string,
//    })
//  | CompletionDetailsAvailable({
//      handle: int,
//      suggestItem: Exthost.SuggestItem.t,
//    })
//  | CompletionDetailsError({
//      handle: int,
//      errorMsg: string,
//    });

type exthostMsg;

[@deriving show]
type provider = {
  handle: int,
  selector: DocumentSelector.t,
  triggerCharacters: [@opaque] list(Uchar.t),
  supportsResolveDetails: bool,
  extensionId: string,
  implementation:
    [@opaque]
    CompletionProvider.provider(
      CompletionProvider.exthostModel,
      CompletionProvider.exthostMsg,
    ),
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

module Session = {
  [@deriving show]
  type state =
    | Waiting
    | Completed({
        base: string,
        location: CharacterPosition.t,
        allItems: list(CompletionItem.t),
        filteredItems: [@opaque] list(Filter.result(CompletionItem.t)),
      })
    // TODO
    //| Incomplete(list(Exthost.SuggestItem.t))
    | Failure(string)
    | Accepted;

  [@deriving show]
  type session('model, 'msg) = {
    state,
    base: string,
    trigger: [@opaque] Exthost.CompletionContext.t,
    triggerCharacters: [@opaque] list(Uchar.t),
    buffer: [@opaque] Oni_Core.Buffer.t,
    location: EditorCoreTypes.CharacterPosition.t,
    provider: [@opaque] CompletionProvider.provider('model, 'msg),
    providerModel: option('model),
    providerMapper: 'msg => providerMsg,
    revMapper: providerMsg => option('msg),
    supportsResolve: bool,
  };

  type t =
    | Session(session('model, 'msg)): t;

  let location =
    fun
    | Session({state, _}) =>switch (state) {
    | Completed({location, _}) => Some(location)
    | _ => None
    };

  let handle = fun
  | Session({provider, _}) => {
    let (module ProviderImpl) = provider;
    ProviderImpl.handle
  };

//  let base =
//    fun
//    | Session({base, _}) => base;

  let complete =
    fun
    | Session(session) => Session({...session, state: Accepted});

  let filter:
    (~query: string, list(CompletionItem.t)) =>
    list(Filter.result(CompletionItem.t)) =
    (~query, items) => {
      let toString = (item: CompletionItem.t, ~shouldLower) =>
        shouldLower ? String.lowercase_ascii(item.label) : item.label;

      let explodedQuery = Zed_utf8.explode(query);

      items
      |> List.filter((item: CompletionItem.t) =>
           if (String.length(item.filterText) < String.length(query)) {
             false;
           } else {
             Filter.fuzzyMatches(explodedQuery, item.filterText);
           }
         )
      |> Filter.rank(query, toString);
    };

  let filteredItems =
    fun
    | Session({state, _}) => {
        switch (state) {
        | Accepted => []
        | Waiting => []
        | Failure(_) => []
        | Completed({filteredItems, location, _}) => filteredItems
        |> List.map(item => (location, item));
        };
      };

  let update = msg =>
    fun
    | Session(
        {provider, providerModel, revMapper, base, state, _} as session,
      ) => {
        revMapper(msg)
        |> Option.map(internalMsg => {
             let (module ProviderImpl) = provider;
             let isFuzzyMatching = base != "";
             let providerModel' =
               providerModel
               |> Option.map(
                    ProviderImpl.update(~isFuzzyMatching, internalMsg),
                  );

             let state' =
               providerModel'
               |> Option.map(model => {
                    switch (model |> ProviderImpl.items) {
                    | Error(msg) => Failure(msg)
                    | Ok([]) => state
                    | Ok(items) =>
                      let filteredItems = filter(~query=base, items);
                      Completed({base, location, filteredItems, allItems: items});
                    }
                  })
               |> Option.value(~default=state);

             Session({
               ...session,
               state: state',
               providerModel: providerModel',
             });
           })
        |> Option.value(~default=Session(session));
      };

  let sub = (~selectedItem, ~client) =>
    fun
    | Session({providerModel, provider, providerMapper, location, buffer, _}) => {
        let (module ProviderImpl) = provider;
        providerModel
        |> Option.map(model => {
             ProviderImpl.sub(
               ~client,
               ~buffer,
               ~position=location,
               ~selectedItem,
               model,
             )
             |> Isolinear.Sub.map(msg => Provider(providerMapper(msg)))
           })
        |> Option.value(~default=Isolinear.Sub.none);
      };

  let isActive =
    fun
    | Session({state, providerModel, _}) => {
        Option.is_some(providerModel) && switch (state) {
        | Accepted => false
        | Waiting => true
        | Failure(_) => false
        | Completed(_) => true
        };
      };

  let create =
      (
        ~provider: CompletionProvider.provider('model, 'msg),
        ~mapper: 'msg => providerMsg,
        ~revMapper: providerMsg => option('msg),
        ~triggerCharacters,
//        ~trigger,
//        ~buffer,
//        ~base,
//        ~location,
//        ~supportsResolve,
      ) => {
      Session({
//        trigger,
        triggerCharacters,
        state: Waiting,
//        buffer,
//        base,
//        location,
//        supportsResolve,
        provider,
        providerModel: None,
        providerMapper: mapper,
        revMapper,
      });
    };

//    switch (candidate) {
//    | Session({provider, _} as session) =>
//      let (module ProviderImpl) = provider;
//      let maybeModel =
//        ProviderImpl.create(~base, ~trigger, ~buffer, ~location);
//      maybeModel
//      |> Option.map(model => {
//           let items = ProviderImpl.items(model);
//           let state =
//             switch (items) {
//             | Ok([]) => Waiting
//             | Ok(items) =>
//               Completed({
//                 allItems: items,
//                 filteredItems: filter(~query=base, items),
//               })
//             | Error(msg) => Failure(msg)
//             };
//
//           Session({...session, state, providerModel: Some(model)});
//         })
//       |> Option.value(~default=candidate);
//    };
//  };

  let refine = (~base) =>
    fun
    | Session(current) => {
        let state' =
          switch (current.state) {
          | Accepted => Accepted
          | Waiting => Waiting
          | Failure(_) as failure => failure
          | Completed({allItems, _} as prev) =>
            Completed({
              ...prev,
              filteredItems: filter(~query=base, allItems),
            })
          };

        Session({...current, base, state: state'});
      };

     let refine2 = (~buffer, ~position) => {
        
        let maybeMeet =
          CompletionMeet.fromBufferPosition(
            ~triggerCharacters=previous.triggerCharacters,
            ~position=activeCursor,
            buffer,
          );
     };

    let tryInvoke = (~base, ~trigger, ~buffer, ~location) => fun
    | Session({provider, _} as session) as original => {
      let (module ProviderImpl) = provider;
      let maybeModel =
        ProviderImpl.create(~base, ~trigger, ~buffer, ~location);
      maybeModel
      |> Option.map(model => {
           let items = ProviderImpl.items(model);
           let state =
             switch (items) {
             | Ok([]) => Waiting
             | Ok(items) =>
               Completed({
                 base,
                 location,
                 allItems: items,
                 filteredItems: filter(~query=base, items),
               })
             | Error(msg) => Failure(msg)
             };

           Session({...session, state, providerModel: Some(model)});
         })
       |> Option.value(~default=original);
        };

  let reinvoke = (~buffer, ~activeCursor) =>
    fun
    | Session(previous) as model => {
        let maybeMeet =
          CompletionMeet.fromBufferPosition(
            ~triggerCharacters=previous.triggerCharacters,
            ~position=activeCursor,
            buffer,
          );

        maybeMeet
        |> OptionEx.flatMap(({base, location, _}: CompletionMeet.t)
             // If different buffer or location... start over!
             =>
                switch (previous.state) {
                | Completed({location, buffer, _}) => {
                   if (Oni_Core.Buffer.getId(buffer)
                       != Oni_Core.Buffer.getId(previous.buffer)
                       || location != previous.location) {
                        // try to complete
                        tryInvoke(~base, ~location, model);
                    } else {
                        // The base and location are the same
                        // Just refine existing query
                        refine(~base, model);
                    }
                }
                | _incomplete => tryInvoke(model)
                })
        |> Option.value(~default=model)
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
  sessions: list(Session.t),
  providers: list(provider),
  allItems: array((CharacterPosition.t, Filter.result(CompletionItem.t))),
  selection: Selection.t,
};

let initial = {
  sessions: [
    Session.create(
      ~triggerCharacters=[],
      // Remove from command
      ~provider=CompletionProvider.keyword,
      ~mapper=msg => Keyword(msg),
      ~revMapper=
        fun
        | Keyword(msg) => Some(msg)
        | _ => None,
    )
  ],
  providers: [],
  allItems: [||],
  selection: Selection.initial,
};

let providerCount = ({providers, _}) => List.length(providers);
let availableCompletionCount = ({allItems, _}) => Array.length(allItems);

let recomputeAllItems = (sessions: list(Session.t)) => {
  sessions
  |> List.map(session => {
       session
       |> Session.filteredItems
     })
  |> List.flatten
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
  model.allItems |> Array.length > 0;
};

let stopInsertMode = model => {
  ...model,
  sessions: [],
  allItems: [||],
  selection: None,
};

let cursorMoved =
    (~previous as _, ~current: EditorCoreTypes.CharacterPosition.t, model) => {
  // Filter providers, such that the completion meets are still
  // valid in the context of the new cursor position

  let isCompletionMeetStillValid =
      (meetLocation: EditorCoreTypes.CharacterPosition.t) => {
    // If the line changed, the meet is no longer valid
    meetLocation.line == current.line
    // If the cursor moved before the meet, on the same line,
    // it is no longer valid. The after case is a bit trickier -
    // the cursor moves first, before we get the buffer update,
    // so the meet may not be valid until the buffer update comes through.
    // We'll allow cursor moves after the meet, for now.
    && CharacterIndex.toInt(meetLocation.character)
    <= CharacterIndex.toInt(current.character);
  };

  let sessions =
    List.filter(
      (session: Session.t) => {
        isCompletionMeetStillValid(Session.location(session))
      },
      model.sessions,
    );

  let allItems = recomputeAllItems(sessions);

  let count = Array.length(allItems);

  {
    ...model,
    sessions,
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
      implementation: CompletionProvider.exthost(~handle, ~selector),
    },
    ...model.providers,
   ],
   sessions: [
         Session.create(
           ~triggerCharacters=triggerCharacters,
           ~provider=CompletionProvider.exthost(~selector, ~handle),
           ~mapper=msg => Exthost(msg),
           ~revMapper=
             fun
             | Exthost(msg) => Some(msg)
             | _ => None,
         ),
         ...model.sessions,
   ],
};

let unregister = (~handle, model) => {
  ...model,
  providers: List.filter(prov => prov.handle != handle, model.providers),
  sessions: List.filter(session => Session.handle(session) != Some(handle), model.sessions),
};

let updateSessions = (sessions, model) => {
  let allItems = recomputeAllItems(sessions);
  let selection =
    Selection.ensureValidFocus(
      ~count=Array.length(allItems),
      model.selection,
    );
    {...model, sessions, allItems, selection};
}

let invokeCompletion = (~buffer, ~activeCursor, model) => {
  //  let candidateProviders =
  //    model.providers
  //    |> List.filter(prov =>
  //         Exthost.DocumentSelector.matchesBuffer(~buffer, prov.selector)
  //       );

  let location = activeCursor;

  let sessions =
    model.providers
    |> List.map((curr: provider) => {
         Session.create(
//           ~trigger=
//             Exthost.CompletionContext.{
//               triggerKind: Invoke,
//               triggerCharacter: None,
//             },
           ~triggerCharacters=curr.triggerCharacters,
//           ~buffer,
//           ~base="",
//           ~location,
//           ~supportsResolve=curr.supportsResolveDetails,
           ~provider=CompletionProvider.exthost(~selector=curr.selector, ~handle=curr.handle),
           ~mapper=msg => Exthost(msg),
           ~revMapper=
             fun
             | Exthost(msg) => Some(msg)
             | _ => None,
         )
       });

  let _keywordSession =
    Session.create(
//      ~trigger=
//        Exthost.CompletionContext.{
//          triggerKind: Invoke,
//          triggerCharacter: None,
//        },
      ~triggerCharacters=[],
//      ~buffer,
//      ~base="",
//      ~location,
      // Remove from command
//      ~supportsResolve=false,
      ~provider=CompletionProvider.keyword,
      ~mapper=msg => Keyword(msg),
      ~revMapper=
        fun
        | Keyword(msg) => Some(msg)
        | _ => None,
    );
//    |> Option.map(v => [v])
//    |> Option.value(~default=[]);

  let allItems = recomputeAllItems(sessions);
  let selection =
    Selection.ensureValidFocus(
      ~count=Array.length(allItems),
      model.selection,
    );

  {...model, sessions, allItems, selection};
};

let startCompletion =
    (~startNewSession, ~trigger, ~buffer, ~activeCursor, model) => {
  //invokeCompletion(~buffer, ~activeCursor, model);
  //model;

    model.sessions
    |> List.map(previous => {
         Session.reinvoke(
           ~buffer,
           ~activeCursor,
           previous,
         )
       })
    |> updateSessions;
  // TODO: Revisit why this is needed?
  //  let candidateProviders =
  //    model.providers
  //    |> List.filter(prov =>
  //         Exthost.DocumentSelector.matchesBuffer(~buffer, prov.selector)
  //       );
  //
  //  let handleToSession =
  //    List.fold_left(
  //      (acc: IntMap.t(Session.t), curr: provider) => {
  //        let maybeMeet =
  //          CompletionMeet.fromBufferPosition(
  //            ~triggerCharacters=curr.triggerCharacters,
  //            ~position=activeCursor,
  //            buffer,
  //          );
  //
  //        switch (maybeMeet) {
  //        | None => acc |> IntMap.remove(curr.handle)
  //        | Some({base, location, _}) =>
  //          acc
  //          |> IntMap.update(
  //               curr.handle,
  //               fun
  //               | None => {
  //                   startNewSession
  //                     ?
  //                         Session.create(
  //                           ~buffer,
  //                           ~trigger,
  //                           ~base,
  //                           ~location,
  //                           ~supportsResolve=curr.supportsResolveDetails,
  //                           ~provider=curr.implementation,
  //                           ~mapper=msg =>
  //                           ExtensionProviderMsg(msg)
  //                         )
  //                         : None
  //                 }
  //               | Some(previous) => {
  //                   Session.update(
  //                     ~createNew=startNewSession,
  //                     ~buffer,
  //                     ~base,
  //                     ~location,
  //                     previous,
  //                   );
  //                 },
  //             )
  //        };
  //      },
  //      model.handleToSession,
  //      candidateProviders,
  //    );
  //
  let allItems = recomputeAllItems(sessions);
  let selection =
    Selection.ensureValidFocus(
      ~count=Array.length(allItems),
      model.selection,
    );
  //
  {...model, sessions, allItems, selection};
};

let refine = (~buffer, ~position, model) => {
    model.sessions
    List.map(Session.refine(~buffer, ~position))
    |> updateSessions;
}

let bufferUpdated =
    (~buffer, ~config, ~activeCursor, ~syntaxScope, ~triggerKey, model) => {
  let quickSuggestionsSetting = Configuration.quickSuggestions.get(config);

  let anySessionsActive = model.sessions |> List.exists(Session.isActive);

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
      refine(
        ~trigger,
        ~buffer,
        ~activeCursor,
        model,
      );
    } else {
      model;
    };
  } else {
    invokeCompletion(
      ~buffer,
      ~activeCursor,
      model,
    );
  };
};

let update = (~maybeBuffer, ~activeCursor, msg, model) => {
  prerr_endline("MSG: " ++ show_msg(msg));
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
              })
            : Outmsg.ApplyCompletion({
                meetColumn,
                insertText: result.item.insertText,
              });

        (
          {
            ...model,
            sessions:
              List.map(
                session => {session |> Session.complete},
                model.sessions,
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
    let sessions =
      model.sessions |> List.map(session => Session.update(msg, session));
    let allItems = recomputeAllItems(sessions);
    let selection =
      Selection.ensureValidFocus(
        ~count=Array.length(allItems),
        model.selection,
      );
    ({...model, sessions, allItems, selection}, Nothing);
  //  | CompletionResultAvailable({handle, suggestResult}) =>
  //    let handleToSession =
  //      IntMap.update(
  //        handle,
  //        Option.map(prev =>
  //          Session.receivedItems(
  //            Exthost.SuggestResult.(suggestResult.completions),
  //            prev,
  //          )
  //        ),
  //        model.handleToSession,
  //      );
  //    let allItems = recomputeAllItems(handleToSession);
  //    (
  //      {
  //        ...model,
  //        handleToSession,
  //        allItems,
  //        selection:
  //          Selection.ensureValidFocus(
  //            ~count=Array.length(allItems) - 1,
  //            model.selection,
  //          ),
  //      },
  //      Outmsg.Nothing,
  //    );
  //  | CompletionError({handle, errorMsg}) => (
  //      {
  //        ...model,
  //        handleToSession:
  //          IntMap.update(
  //            handle,
  //            Option.map(prev => Session.error(~errorMsg, prev)),
  //            model.handleToSession,
  //          ),
  //      },
  //      Outmsg.Nothing,
  //    )
  //
  //  | CompletionDetailsAvailable({handle, suggestItem}) =>
  //    let handleToSession =
  //      model.handleToSession
  //      |> IntMap.update(
  //           handle,
  //           Option.map(prev => Session.updateItem(~item=suggestItem, prev)),
  //         );
  //
  //    let allItems = recomputeAllItems(handleToSession);
  //    ({...model, handleToSession, allItems}, Outmsg.Nothing);
  //
  //  | CompletionDetailsError(_) => (model, Outmsg.Nothing)
  };
};

let sub = (~client, model) => {
  // Subs for each pending handle..
  prerr_endline(" -- sub");
  model.sessions
  |> List.map((meet: Session.t) => {
       Session.sub(~client, ~selectedItem=None, meet);
     })
  |> Isolinear.Sub.batch

                        //               triggerCharacter: None,
                        //             },
                        //           ~handle,
                        //           ~buffer=meet.buffer,
                        //           ~position=meet.location,
                        //           ~toMsg=
                        //             suggestResult => {
                        //               switch (suggestResult) {
                        //               | Ok(v) =>
                        //                 CompletionResultAvailable({handle, suggestResult: v})
                        //               | Error(errorMsg) => CompletionError({handle, errorMsg})
                        //               }
                        //             },
                        //           client,
                        //         )
                        //       });
                        //
                        //  // And subs for the current item, if the handle supports resolution
                        //  let resolveSub =
                        //    model
                        //    |> focused
                        //    |> OptionEx.flatMap((focusedItem: Filter.result(CompletionItem.t)) => {
                        //         let handle = focusedItem.item.handle;
                        //
                        //         model.handleToSession
                        //         |> IntMap.find_opt(handle)
                        //         |> OptionEx.flatMap(({supportsResolve, _}: Session.t) =>
                        //              if (supportsResolve) {
                        //                focusedItem.item.chainedCacheId
                        //                |> Option.map(chainedCacheId => {
                        //                     Service_Exthost.Sub.completionItem(
                        //                       ~handle,
                        //                       ~chainedCacheId,
                        //                       ~toMsg=
                        //                         fun
                        //                         | Ok(suggestItem) =>
                        //                           CompletionDetailsAvailable({handle, suggestItem})
                        //                         | Error(errorMsg) =>
                        //                           CompletionDetailsError({handle, errorMsg}),
                        //                       client,
                        //                     )
                        //                   });
                        //              } else {
                        //                None;
                        //              }
                        ; //             Exthost.CompletionContext.{
                        //            );
 //           ~context=
  //       })
  //           // TODO: proper trigger kind
  //    |> Option.value(~default=Isolinear.Sub.none);
  //         Service_Exthost.Sub.completionItems(
  //
  //  [resolveSub, ...handleSubs] |> Isolinear.Sub.batch;
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
              let Filter.{highlight, item} = items[index] |> snd;
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
