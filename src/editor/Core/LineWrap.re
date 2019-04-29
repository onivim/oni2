/**
 * LineWrap.re
 * 
 * Logic for wrapping lines in Onivim 2
 */


type wrap = {
	/*
	 * The UTF-8 index of the start of a wrapped (virtual) line
	 */
	index: int,

	/*
	 * The length of the virtual line
	 */
	length: int,
};

type t = {
	wraps: array(wrap),
};

let wrapLine: (string, int) => t = (s, width) => {
	
	let len = Zed_utf8.len(s);

	let idx = ref(0);
	let wraps = ref([]);

	while (idx^ < len) {
		let segment = min(width, len - idx);
		let i = idx^;
		let wrap = {
			index: i,
			length: i + segment,
		};
		wraps := [wrap, ...wraps^];
		idx := i + segment;
	};

	wraps
	|> List.rev
	|> List.to_array;
};


