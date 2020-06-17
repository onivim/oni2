let isSpace =
  fun
  | ' '
  | '\012'
  | '\n'
  | '\r'
  | '\t' => true
  | _ => false;

/** [contains(query, str)] returns true if [str] contains the substring [query], false otherwise. */
let contains = (query, str) => {
  let re = Str.regexp_string(query);
  try({
    let _: int = Str.search_forward(re, str, 0);
    true;
  }) {
  | Not_found => false
  };
};

let explode = str =>
  str |> String.to_seq |> List.of_seq |> List.map(c => String.make(1, c));

exception NoMatchException;

/**
  [forAll(~start, ~stop, ~f, str)] validates a predicate [f] for each character, from [start] (inclusive) to [stop] (exclusive)

  Returns [true] if predicate [f(c)] returns [true] from all characters from [start] to [stop], [false] otherwise.
*/
let forAll = (~start=0, ~stop=?, ~f, str) => {
  let stop = stop |> Option.value(~default=String.length(str));

  let rec loop = i =>
    if (i >= stop) {
      true;
    } else if (f(str.[i])) {
      loop(i + 1);
    } else {
      false;
    };

  loop(start);
};

let startsWith = (~prefix, str) => {
  let prefixLength = String.length(prefix);
  let strLength = String.length(str);

  if (prefixLength > strLength) {
    false;
  } else {
    let rec match = i =>
      if (i == prefixLength) {
        true;
      } else {
        prefix.[i] == str.[i] && match(i + 1);
      };
    match(0);
  };
};

let endsWith = (~postfix, str) => {
  let postfixLength = String.length(postfix);
  let strLength = String.length(str);

  if (postfixLength > strLength) {
    false;
  } else {
    let rec match = i =>
      if (i == postfixLength) {
        true;
      } else {
        postfix.[postfixLength - i - 1] == str.[strLength - i - 1]
        && match(i + 1);
      };
    match(0);
  };
};

let trimLeft = str => {
  let length = String.length(str);

  let rec aux = i =>
    if (i >= length) {
      "";
    } else if (isSpace(str.[i])) {
      aux(i + 1);
    } else if (i == 0) {
      str;
    } else {
      String.sub(str, i, length - i);
    };

  aux(0);
};

let trimRight = str => {
  let length = String.length(str);

  let rec aux = j =>
    if (j <= 0) {
      "";
    } else if (isSpace(str.[j])) {
      aux(j - 1);
    } else if (j == length - 1) {
      str;
    } else {
      String.sub(str, 0, j + 1);
    };

  aux(length - 1);
};

let indentation = str => {
  let rec loop = i =>
    if (i >= String.length(str)) {
      i;
    } else if (isSpace(str.[i])) {
      loop(i + 1);
    } else {
      i;
    };

  loop(0);
};

let extractSnippet = (~maxLength, ~charStart, ~charEnd, text) => {
  let originalLength = String.length(text);

  let indentation = {
    let rec aux = i =>
      if (i >= originalLength) {
        originalLength;
      } else if (isSpace(text.[i])) {
        aux(i + 1);
      } else {
        i;
      };

    aux(0);
  };

  let remainingLength = originalLength - indentation;
  let matchLength = charEnd - charStart;

  if (remainingLength > maxLength) {
    // too long

    let (offset, length) =
      if (matchLength >= maxLength) {
        (
          // match is lonegr than allowed
          charStart,
          maxLength,
        );
      } else if (charEnd - indentation > maxLength) {
        (
          // match ends out of bounds
          charEnd - maxLength,
          maxLength,
        );
      } else {
        (
          // match is within bounds
          indentation,
          maxLength,
        );
      };

    if (offset > indentation) {
      // We're cutting non-indentation from the start, so add ellipsis

      let ellipsis = "...";
      let ellipsisLength = String.length(ellipsis);

      // adjust for ellipsis
      let (offset, length) = {
        let availableEnd = length - (charEnd - offset);

        if (ellipsisLength > length) {
          (
            // ellipsis won't even fit... not much to do then I guess
            offset,
            length,
          );
        } else if (ellipsisLength <= availableEnd) {
          (
            // fits at the end, so take it from there
            offset,
            length - ellipsisLength,
          );
        } else {
          // won't fit at the end
          let remainder = ellipsisLength - availableEnd;

          if (remainder < charStart - offset) {
            (
              // remainder will fit at start
              offset + remainder,
              length - ellipsisLength,
            );
          } else {
            (
              // won't fit anywhere, so just chop it off the end
              offset,
              length - ellipsisLength,
            );
          };
        };
      };

      (
        ellipsis ++ String.sub(text, offset, length),
        ellipsisLength + charStart - offset,
        ellipsisLength + min(length, charEnd - offset),
      );
    } else {
      (
        // We're only cutting indentation from the start
        String.sub(text, offset, length),
        charStart - offset,
        min(length, charEnd - offset),
      );
    };
  } else if (indentation > 0) {
    // not too long, but there's indentation

    // do not remove indentation included in match
    let offset = indentation > charStart ? charStart : indentation;
    let length = min(maxLength, originalLength - offset);

    (String.sub(text, offset, length), charStart - offset, charEnd - offset);
  } else {
    (
      // not too long, no indentation
      text,
      charStart,
      charEnd,
    );
  };
};

let removeWindowsNewLines = s =>
  List.init(String.length(s), String.get(s))
  |> List.filter(c => c != '\r')
  |> List.map(c => String.make(1, c))
  |> String.concat("");
let splitNewLines = s => s |> String.split_on_char('\n') |> Array.of_list;

let removeTrailingNewLine = s => {
  let len = String.length(s);
  if (len > 0 && s.[len - 1] == '\n') {
    String.sub(s, 0, len - 1);
  } else {
    s;
  };
};
