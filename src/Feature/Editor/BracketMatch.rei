
type direction =
| Forwards
| Backwards;

type t = {
	line: int,
	byteIndex: int,
};

let matchingPair: (
	~buffer: EditorBuffer.t, 
	~line: int, 
	~byteIndex: int, 
	~direction: direction,
	~current: Uchar.t,
	~destination: Uchar.t) => option(t);
