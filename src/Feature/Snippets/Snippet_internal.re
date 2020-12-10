type segment =
| Text(string)
| Placeholder({index: int, contents: list(segment) });

type t = list(segment);
