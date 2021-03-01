open Oni_Core;
open Utility;

type internal('item, 'outmsg) = {
  onItemFocused: option('item => 'outmsg),
  onItemSelected: option('item => 'outmsg),
  onCancelled: option(unit => 'outmsg),
  toString: 'item => string,
  items: list('item),
};
type menu('outmsg) =
  | Menu(internal('item, 'outmsg)): menu('outmsg);

let menu:
  (
    ~onItemFocused: 'item => 'outmsg=?,
    ~onItemSelected: 'item => 'outmsg=?,
    ~onCancelled: unit => 'outmsg=?,
    ~toString: 'item => string,
    list('item)
  ) =>
  menu('outmsg) =
  (
    ~onItemFocused=?,
    ~onItemSelected=?,
    ~onCancelled=?,
    ~toString,
    initialItems,
  ) =>
    Menu({
      onItemFocused,
      onItemSelected,
      onCancelled,
      toString,
      items: initialItems,
    });

let mapFunction: ('a => 'b, 'item => 'a, 'item) => 'b =
  (f, orig, item) => {
    f(orig(item));
  };

let map: ('a => 'b, menu('a)) => menu('b) =
  (f, model) => {
    switch (model) {
    | Menu({onItemFocused, onItemSelected, onCancelled, toString, items}) =>
      Menu({
        onItemFocused: onItemFocused |> Option.map(mapFunction(f)),
        onItemSelected: onItemSelected |> Option.map(mapFunction(f)),
        onCancelled: onCancelled |> Option.map(mapFunction(f)),
        toString,
        items,
      })
    };
  };

module Instance = {
  type t('outmsg) =
    | Instance({
        schema: internal('item, 'outmsg),
        text: Component_InputText.model,
        allItems: list('item),
        filteredItems: array(Filter.result('item)),
        focused: option(int),
      })
      : t('outmsg);

  let updateFilteredItems =
    fun
    | Instance({text, schema, allItems, _} as orig) => {
        let format = (item, ~shouldLower) => {
          let str = schema.toString(item);
          shouldLower ? String.lowercase_ascii(str) : str;
        };
        let queryStr = Component_InputText.value(text);
        let query = Zed_utf8.explode(queryStr);
        let filteredItems =
          allItems
          |> List.filter(item =>
               Filter.fuzzyMatches(query, schema.toString(item))
             )
          |> Filter.rank(queryStr, format)
          |> Array.of_list;

        Instance({...orig, filteredItems});
      };

  let text =
    fun
    | Instance({text, _}) => text;

  let paste = (~text) =>
    fun
    | Instance({text: prev, _} as orig) =>
      Instance({...orig, text: Component_InputText.paste(~text, prev)})
      |> updateFilteredItems;

  let key = (~key) =>
    fun
    | Instance({text, _} as orig) =>
      Instance({...orig, text: Component_InputText.handleInput(~key, text)})
      |> updateFilteredItems;

  let input = msg =>
    fun
    | Instance(orig) => {
        let (text', _outmsg) = Component_InputText.update(msg, orig.text);
        Instance({...orig, text: text'}) |> updateFilteredItems;
      };

  let count =
    fun
    | Instance({filteredItems, _}) => Array.length(filteredItems);

  let itemAndHighlights = (~index: int) =>
    fun
    | Instance({filteredItems, schema, _}) => {
        let len = Array.length(filteredItems);

        if (index >= 0 && index < len) {
          let item = filteredItems[index];
          let itemStr = schema.toString(item.item);
          let highlights = item.highlight;

          Some((itemStr, highlights));
        } else {
          None;
        };
      };

  let next =
    fun
    | Instance({filteredItems, focused, _} as orig) => {
        let focused' =
          focused
          |> Utility.IndexEx.nextRollOverOpt(
               ~last=Array.length(filteredItems) - 1,
             );
        Instance({...orig, focused: focused'});
      };

  let focus = index =>
    fun
    | Instance({filteredItems, _} as orig) => {
        let focused' =
          Utility.IntEx.clamp(
            index,
            ~lo=0,
            ~hi=Array.length(filteredItems) - 1,
          );
        Instance({...orig, focused: Some(focused')});
      };

  let previous =
    fun
    | Instance({filteredItems, focused, _} as orig) => {
        let focused' =
          focused
          |> Utility.IndexEx.prevRollOverOpt(
               ~last=Array.length(filteredItems) - 1,
             );
        Instance({...orig, focused: focused'});
      };

  let select: t('a) => option('a) =
    fun
    | Instance({schema, filteredItems, focused, _}) => {
        let len = Array.length(filteredItems);
        focused
        |> OptionEx.flatMap(focusedIndex =>
             if (focusedIndex >= 0 && focusedIndex < len) {
               let item = filteredItems[focusedIndex];
               schema.onItemSelected |> Option.map(f => f(item.item));
             } else {
               None;
             }
           );
      };

  let focused =
    fun
    | Instance({focused, _}) => focused;

  let sub = _model => {
    Isolinear.Sub.none;
  };
};

let instantiate: menu('outmsg) => Instance.t('outmsg) =
  fun
  | Menu(internal) =>
    Instance.Instance({
      schema: internal,
      text:
        Component_InputText.empty
        |> Component_InputText.setPlaceholder(
             ~placeholder="type to search...",
           ),
      allItems: internal.items,
      filteredItems: [||],
      focused: None,
    })
    |> Instance.updateFilteredItems;
