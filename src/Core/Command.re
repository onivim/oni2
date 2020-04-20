open Kernel;

module Log = (val Log.withNamespace("Oni2.Core.Command"));

[@deriving show]
type t('msg) = {
  id: string,
  title: option(string),
  category: option(string),
  icon: option([@opaque] IconTheme.IconDefinition.t),
  isEnabledWhen: WhenExpr.t,
  msg: [ | `Arg0('msg) | `Arg1(Json.t => 'msg)],
};

let map = (f, command) => {
  ...command,
  msg:
    switch (command.msg) {
    | `Arg0(msg) => `Arg0(f(msg))
    | `Arg1(msgf) => `Arg1(arg => f(msgf(arg)))
    },
};

module Lookup = {
  type command('msg) = t('msg);
  type t('msg) = KeyedStringMap.t(command('msg));

  let fromList = commands =>
    commands
    |> List.to_seq
    |> Seq.map(command => (KeyedStringMap.key(command.id), command))
    |> KeyedStringMap.of_seq;

  let get = (key, lookup) =>
    KeyedStringMap.find_opt(KeyedStringMap.key(key), lookup);

  let add = (key, command, lookup) =>
    KeyedStringMap.add(KeyedStringMap.key(key), command, lookup);

  let union = (xs, ys) =>
    KeyedStringMap.union(
      (key, _x, y) => {
        Log.warnf(m =>
          m("Encountered duplicate default: %s", KeyedStringMap.keyName(key))
        );
        Some(y);
      },
      xs,
      ys,
    );

  let unionMany = lookups =>
    List.fold_left(union, KeyedStringMap.empty, lookups);

  let map = (f, lookup) => KeyedStringMap.map(map(f), lookup);

  let toList = lookup =>
    KeyedStringMap.to_seq(lookup)
    |> Seq.map(((_key, definition)) => definition)
    |> List.of_seq;
};
