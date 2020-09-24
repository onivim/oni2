open Oni_Core;
open Utility;

  [@deriving show]
  type msg =
  | SearchText(Component_InputText.msg);

  type query =
  | Forward(string)
  | Backward(string);

  type outmsg =
  | Nothing;
  
  type t = {
    maybeQuery: option(query),
    searchIds: array(string),
    matches: list(int),
    searchText: Component_InputText.model,
    isSearchTextInputVisible: bool,
  };

  let queryString = ({maybeQuery, _}) => {
    maybeQuery
    |> Option.map(fun
    | Forward(query)
    | Backward(query) => query);
  }

  let searchCount = ({matches, _}) => {
    List.length(matches)
  };

  let isMatch = (~index, ({matches, _})) => {
    matches
    |> List.exists(idx => idx == index)
  };

  let computeMatches = (~query, items: array(string)) => {
    let match = StringEx.contains(query );
    let len = Array.length(items);

    let rec loop = (matches, idx) => {
      if (idx >= len) {
        matches
      } else {
        if (match(items[idx])) {
          loop([idx, ...matches], idx + 1)
        } else {
          loop(matches, idx + 1)
        }
      }
    };

    loop([], 0) |> List.rev;
  };

  let updateMatches = (model) => {
    let matches = model
    |> queryString
    |> Option.map(query => {
      computeMatches(~query, model.searchIds)
    })
    |> Option.value(~default=[]);
    
    {
      ...model,
      matches
    };
  }

  let initial = {
    maybeQuery: None,
    searchIds: [||],
    matches: [],
    searchText: Component_InputText.create(~placeholder=""),
    isSearchTextInputVisible: false,
  };

  let setQuery = (query, model) => {
    {
      ...model,
      maybeQuery: Some(query)
    } |> updateMatches;
  };

  let setSearchIds = (searchIds, model) => {
    {
      ...model,
      searchIds,
    } |> updateMatches;
  }

  let higherMatch = (~index, matches) => {
    let rec loop = (items) => {
      switch (items) {
      | [] => None
      | [hd, ...tail] => if (hd > index) {
        Some(hd)
      } else {
        loop(tail);
      }
      }
    };
    loop(matches)
    |> OptionEx.or_(List.nth_opt(matches, 0))
  };
  
  let lowerMatch = (~index, matches) => {
    let rec loop = (items) => {
      switch (items) {
      | [] => None
      | [hd, ...tail] => if (hd < index) {
        Some(hd)
      } else {
        loop(tail);
      }
      }
    };
    let revMatches = matches |> List.rev;
    loop(revMatches)
    |> OptionEx.or_(List.nth_opt(revMatches, 0))
  };

  let nextMatch = (~index, model) => {
    switch (model.maybeQuery) {
    | None => None
    | Some(Forward(_)) => higherMatch(~index, model.matches)
    | Some(Backward(_)) => lowerMatch(~index, model.matches)
    }
  };
  
  let previousMatch = (~index, model) => {
    switch (model.maybeQuery) {
    | None => None
    | Some(Forward(_)) => lowerMatch(~index, model.matches)
    | Some(Backward(_)) => higherMatch(~index, model.matches)
    }
  };

  let update = (msg, model) => {
    switch (msg) {
    | SearchText(msg) => 
      let (searchText, _outmsg) = Component_InputText.update(
        msg,
        model.searchText
      );
      ({...model, searchText}, Nothing)
      
    }
  };
