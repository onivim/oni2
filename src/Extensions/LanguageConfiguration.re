module Comments = {
	type t = {
		lineComment: string,
		blockComment: (string, string),
	};
};

module Brackets = {
	type pair = (string, string);

	type t = list(pair);
}

module AutoClosingPairs = {
	type autoClosingPair = {
		openPair: string,
		closePair: string,
		//TODO: notIn
	};

	type t = list(autoClosingPair);
};

type t = {
	comments: Comments.t,
	brackets: Brackets.t,
	autoClosingPairs: AutoClosingPairs.t,
	autoCloseBefore: string,

	// TODO:
	// surroundingPairs
	// folding
	// wordPattern
	// indentationRules
}

