type triggerKind =
  | Invoke
  | TriggerCharacter
  | TriggerForIncompleteCompletions;

type t = {
  triggerKind,
  triggerCharacter: option(string),
};

let kindToInt =
  fun
  | Invoke => 0
  | TriggerCharacter => 1
  | TriggerForIncompleteCompletions => 2;

let kindToJson = kind => `Int(kindToInt(kind));

let to_yojson = ({triggerKind, triggerCharacter}) => {
  let kindJson = kindToJson(triggerKind);
  switch (triggerCharacter) {
  | None =>
    // If no trigger character - don't send at all.
    `Assoc([("triggerKind", kindJson)])
  | Some(v) =>
    `Assoc([("triggerKind", kindJson), ("triggerCharacter", `String(v))])
  };
};
