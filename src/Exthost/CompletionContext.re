
type triggerKind = 
| Invoke
| TriggerCharacter
| TriggerForIncompleteCompletions;

type t = {
	triggerKind: triggerKind,
	triggerCharacter: option(string)
};

let kindToInt = fun 
| Invoke => 0
| TriggerCharacter => 1
| TriggerForIncompleteCompletions => 2;

let kindToJson = kind => `Int(kindToInt(kind));

let to_yojson = ({triggerKind, triggerCharacter}) => {
	let kindJson = kindToJson(triggerKind);
	let characterJson = switch(triggerCharacter) {
	| None => `Null
	| Some(v) => `String(v);
	};
	`Assoc([
		("triggerKind", kindJson),
		("triggerCharacter", characterJson)
	])
};
