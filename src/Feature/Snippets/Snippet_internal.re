type segment =
| Text(string)
| Placeholder({index: int, contents: list(segment) });

type t = list(segment);

let rec normalize = (segments: list(segment)) => {
	
	let rec loop = (acc, curr: option(string), remaining) => {
		switch ((remaining, curr)) {
		| ([], Some(text)) => [Text(text), ...acc]
		| ([], None) => acc
		| ([Text(newText), ...tail], Some(text)) =>
			loop(acc, Some(text ++ newText), tail)
		| ([Placeholder({index, contents}), ...tail], Some(text)) =>
			loop([Placeholder({index, contents: normalize(contents)}), Text(text), ...acc], None, tail)
		| ([Text(newText), ...tail], None) =>
			loop(acc, Some(newText), tail)
		| ([Placeholder({index, contents}), ...tail], None) =>
			loop([Placeholder({index, contents: normalize(contents)}), ...acc], None, tail)
		}
	}

	loop([], None, segments)
	|> List.rev;
}
