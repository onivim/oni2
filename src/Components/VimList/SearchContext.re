open Oni_Core;
open Utility;

[@deriving show]
type msg =
  | SearchText(Component_InputText.msg);

type direction =
  | Forward
  | Backward;

type outmsg =
  | Nothing;

type model = {
  direction,
  searchIds: array(string),
  matches: list(int),
  searchText: Component_InputText.model,
  isSearchInputVisible: bool,
};

let queryString = ({searchText, _}) => {
  let text = Component_InputText.value(searchText);

  if (text == "") {
    None;
  } else {
    Some(text);
  };
};

let searchCount = ({matches, _}) => {
  List.length(matches);
};

let isMatch = (~index, {matches, _}) => {
  matches |> List.exists(idx => idx == index);
};

let computeMatches = (~query, items: array(string)) => {
  let match = StringEx.contains(query);
  let len = Array.length(items);

  let rec loop = (matches, idx) =>
    if (idx >= len) {
      matches;
    } else if (match(items[idx])) {
      loop([idx, ...matches], idx + 1);
    } else {
      loop(matches, idx + 1);
    };

  loop([], 0) |> List.rev;
};

let updateMatches = model => {
  let matches =
    model
    |> queryString
    |> Option.map(query => {computeMatches(~query, model.searchIds)})
    |> Option.value(~default=[]);

  {...model, matches};
};

let initial = {
  direction: Forward,
  searchIds: [||],
  matches: [],
  searchText: Component_InputText.create(~placeholder=""),
  isSearchInputVisible: false,
};

let setQuery = (query, model) => {
  {
    ...model,
    searchText: Component_InputText.set(~text=query, model.searchText),
  }
  |> updateMatches;
};

let commit = model => {...model, isSearchInputVisible: false};

let cancel = model => {
  model |> setQuery("") |> commit;
};

let setSearchIds = (searchIds, model) => {
  {...model, searchIds} |> updateMatches;
};

let higherMatch = (~index, matches) => {
  let rec loop = items => {
    switch (items) {
    | [] => None
    | [hd, ...tail] =>
      if (hd > index) {
        Some(hd);
      } else {
        loop(tail);
      }
    };
  };
  loop(matches) |> OptionEx.or_(List.nth_opt(matches, 0));
};

let lowerMatch = (~index, matches) => {
  let rec loop = items => {
    switch (items) {
    | [] => None
    | [hd, ...tail] =>
      if (hd < index) {
        Some(hd);
      } else {
        loop(tail);
      }
    };
  };
  let revMatches = matches |> List.rev;
  loop(revMatches) |> OptionEx.or_(List.nth_opt(revMatches, 0));
};

let nextMatch = (~index, model) => {
  switch (model.direction) {
  | Forward => higherMatch(~index, model.matches)
  | Backward => lowerMatch(~index, model.matches)
  };
};

let currentOrNextMatch = (~index, model) => {
  switch (model.direction) {
  | Forward => higherMatch(~index=index - 1, model.matches)
  | Backward => lowerMatch(~index=index + 1, model.matches)
  };
};

let previousMatch = (~index, model) => {
  switch (model.direction) {
  | Forward => lowerMatch(~index, model.matches)
  | Backward => higherMatch(~index, model.matches)
  };
};

let update = (msg, model) => {
  switch (msg) {
  | SearchText(msg) =>
    let (searchText, _outmsg) =
      Component_InputText.update(msg, model.searchText);
    ({...model, searchText}, Nothing);
  };
};

let isOpen = ({isSearchInputVisible, _}) => isSearchInputVisible;

let show = (~direction, model) =>
  if (model.searchIds |> Array.length > 0) {
    {...model |> setQuery(""), direction, isSearchInputVisible: true};
  } else {
    model;
  };

let close = model => {
  {...model, isSearchInputVisible: false};
};

let keyPress = (~index, key, model) =>
  if (model.isSearchInputVisible) {
    let model' =
      {
        ...model,
        searchText: Component_InputText.handleInput(~key, model.searchText),
      }
      |> updateMatches;

    (model', currentOrNextMatch(~index, model'));
  } else {
    (model, None);
  };

module View = {
  open Revery.UI;

  let make =
      (
        ~isFocused,
        ~font: UiFont.t,
        ~model: model,
        ~theme: ColorTheme.Colors.t,
        ~dispatch: msg => unit,
        (),
      ) => {
    let matchCount = searchCount(model);
    let foregroundColor = Feature_Theme.Colors.Editor.foreground.from(theme);
    let bgColor = Feature_Theme.Colors.Editor.background.from(theme);

    let maybeQueryString = queryString(model);
    if (model.isSearchInputVisible) {
      <View
        style=Style.[
          flexDirection(`Row),
          justifyContent(`Center),
          alignItems(`Center),
          backgroundColor(bgColor),
        ]>
        <View
          style=Style.[
            flexGrow(0),
            paddingLeft(4),
            margin(4),
            flexShrink(0),
          ]>
          <Codicon icon=Codicon.search color=foregroundColor />
        </View>
        <View style=Style.[flexGrow(1), flexShrink(1), margin(8)]>
          <Component_InputText.View
            model={model.searchText}
            theme
            prefix={model.direction == Forward ? "/" : "?"}
            fontFamily={font.family}
            fontSize={font.size}
            isFocused
            dispatch={msg => dispatch(SearchText(msg))}
          />
        </View>
        <View style=Style.[flexGrow(0), margin(4), flexShrink(0)]>
          <Oni_Components.Count uiFont=font theme count=matchCount />
        </View>
      </View>;
    } else {
      maybeQueryString
      |> Option.map(queryString => {
           <View
             style=Style.[
               flexDirection(`Row),
               justifyContent(`Center),
               alignItems(`Center),
               backgroundColor(bgColor),
             ]>
             <View style=Style.[flexGrow(0), margin(4), flexShrink(0)]>
               <Codicon icon=Codicon.search color=foregroundColor />
             </View>
             <View
               style=Style.[
                 flexGrow(1),
                 flexShrink(1),
                 padding(4),
                 margin(8),
               ]>
               <Text
                 style=Style.[color(foregroundColor)]
                 text=queryString
                 fontFamily={font.family}
                 fontSize={font.size}
               />
             </View>
             <View style=Style.[flexGrow(0), margin(4), flexShrink(0)]>
               <Oni_Components.Count uiFont=font theme count=matchCount />
             </View>
           </View>
         })
      |> Option.value(~default=React.empty);
    };
  };
};
