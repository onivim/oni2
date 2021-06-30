module ZedBundled = Oni_Core.ZedBundled;
[@deriving show]
type t =
  | PhysicalKey(PhysicalKey.t)
  | SpecialKey(SpecialKey.t);

let physicalKey = (~key, ~modifiers) =>
  PhysicalKey(PhysicalKey.{key, modifiers});

let specialKey = special => SpecialKey(special);

let toPhysicalKey =
  fun
  | PhysicalKey(key) => Some(key)
  | SpecialKey(_) => None;

let isValidUtf8String = str => {
  ZedBundled.(
    switch (check(str)) {
    | Correct(_) => true
    | Message(_) => false
    }
  );
};

let equals = (keyA, keyB) => {
  switch (keyA, keyB) {
  | (SpecialKey(specialKeyA), SpecialKey(specialKeyB)) =>
    specialKeyA == specialKeyB
  | (PhysicalKey(physicalKeyA), PhysicalKey(physicalKeyB)) =>
    physicalKeyA.key == physicalKeyB.key
    && Modifiers.equals(physicalKeyA.modifiers, physicalKeyB.modifiers)
  | (SpecialKey(_), PhysicalKey(_))
  | (PhysicalKey(_), SpecialKey(_)) => false
  };
};

let isModifier =
  fun
  | PhysicalKey({key: Key.LeftControl, _}) => true
  | PhysicalKey({key: Key.RightControl, _}) => true
  | _ => false;

let ofInternal =
    (
      ~addShiftKeyToCapital,
      (
        key: Matcher_internal.keyPress,
        mods: list(Matcher_internal.modifier),
      ),
    ) => {
  let keyToKeyPress = (~mods=mods, key) => {
    Ok(
      PhysicalKey({
        key,
        modifiers: Matcher_internal.Helpers.internalModsToMods(mods),
      }),
    );
  };
  switch (key) {
  | Matcher_internal.UnmatchedString(str) =>
    if (!isValidUtf8String(str)) {
      [Error("Keybinding is not a valid UTF-8 string")];
    } else {
      ZedBundled.explode(str)
      |> List.map(uchar =>
           if (Uchar.is_char(uchar)) {
             let char = Uchar.to_char(uchar);
             let lowercaseChar = Char.lowercase_ascii(char);
             let isCapitalized = lowercaseChar != char;
             if (isCapitalized && addShiftKeyToCapital) {
               keyToKeyPress(
                 ~mods=[Shift, ...mods],
                 Key.Character(Uchar.of_char(lowercaseChar)),
               );
             } else {
               keyToKeyPress(Key.Character(Uchar.of_char(lowercaseChar)));
             };
           } else {
             keyToKeyPress(Key.Character(uchar));
           }
         );
    }
  | Matcher_internal.Special(special) => [Ok(SpecialKey(special))]
  | Matcher_internal.Physical(key) => [keyToKeyPress(key)]
  };
};

let combineUnmatchedStrings = (keys: list(Matcher_internal.keyMatcher)) => {
  let rec combine = (acc, current, keys) => {
    Matcher_internal.(
      {
        switch (keys) {
        | [hd, ...tail] =>
          switch (hd) {
          | (UnmatchedString(str), mods) =>
            switch (current) {
            // No accumulated string yet - might need to track it to combine later.
            | None => combine(acc, Some((str, mods)), tail)

            // Might be able to accumulate, check if the modifiers match (#2980)
            // ...or if the string is part of a unicode character (#3599)
            | Some((prev, prevMods))
                when prevMods == mods || !isValidUtf8String(prev) =>
              combine(acc, Some((prev ++ str, prevMods)), tail)

            // Modifiers don't match, so append the current to the key sequence,
            // and start a new sequence to track.
            | Some((prev, prevMods)) =>
              combine(
                [(UnmatchedString(prev), prevMods), ...acc],
                Some((str, mods)),
                tail,
              )
            }

          | key =>
            let acc' =
              switch (current) {
              | None => acc
              | Some((str, mods)) => [(UnmatchedString(str), mods), ...acc]
              };
            combine([key, ...acc'], None, tail);
          }
        | [] =>
          switch (current) {
          | None => acc
          | Some((str, mods)) => [(UnmatchedString(str), mods), ...acc]
          }
        };
      }
    );
  };

  combine([], None, keys) |> List.rev;
};

let parse = (~explicitShiftKeyNeeded, str) => {
  let parse = lexbuf =>
    switch (Matcher_parser.keys(Matcher_lexer.token, lexbuf)) {
    | exception Matcher_lexer.Error => Error("Error parsing binding: " ++ str)
    | exception (Matcher_lexer.UnrecognizedModifier(m)) =>
      Error("Unrecognized modifier:" ++ m ++ " in: " ++ str)
    | exception Matcher_parser.Error =>
      Error("Error parsing binding: " ++ str)
    | v => Ok(v)
    };

  let flatMap = (f, r) => Result.bind(r, f);

  let addShiftKeyToCapital = !explicitShiftKeyNeeded;

  let finish = r => {
    r
    |> combineUnmatchedStrings
    |> List.map(ofInternal(~addShiftKeyToCapital))
    |> List.flatten
    |> Base.Result.all;
  };

  str |> Lexing.from_string |> parse |> flatMap(finish);
};

let defaultSuper =
  switch (Revery.Environment.os) {
  | Mac(_) => "Cmd"
  | Windows(_) => "Win"
  | Linux(_) => "Meta"
  | _ => "Super"
  };

let toString = (~super=defaultSuper, ~keyToString=Key.toString, key) => {
  switch (key) {
  | SpecialKey(special) =>
    Printf.sprintf("Special(%s)", SpecialKey.show(special))
  | PhysicalKey({key, modifiers, _}) =>
    let buffer = Buffer.create(16);
    let separator = "+";

    let keyString = keyToString(key);

    let onlyShiftPressed =
      modifiers.shift
      && !modifiers.control
      && !modifiers.super
      && !modifiers.alt;

    let keyString =
      String.length(keyString) == 1 && !onlyShiftPressed
        ? String.lowercase_ascii(keyString) : keyString;

    if (modifiers.super) {
      Buffer.add_string(buffer, super ++ separator);
    };

    if (modifiers.control) {
      Buffer.add_string(buffer, "Ctrl" ++ separator);
    };

    if (modifiers.altGr) {
      Buffer.add_string(buffer, "AltGr" ++ separator);
    } else if (modifiers.alt) {
      Buffer.add_string(buffer, "Alt" ++ separator);
    };

    if ((modifiers.super || modifiers.control || modifiers.alt)
        && modifiers.shift) {
      Buffer.add_string(buffer, "Shift" ++ separator);
    };

    Buffer.add_string(buffer, keyString);

    Buffer.contents(buffer);
  };
};
