module StringUtil = Utility.StringUtil;

// Type defining a simplified 'syntax scope' of a token -
// is it a comment, string, both, or neither?
// This is important for features like auto-closing pairs -
// we may not care to close certain characters in contexts like strings or comments.
type t = 
| None
| String
| Comment
| StringAndComment;

let ofScopes = (scopes: list(string)) => {
	let anyStrings = scopes
	|> List.exists((scope) => StringUtil.startsWith(~prefix="string", scope));

	let anyComments = scopes
	|> List.exists((scope) => StringUtil.startsWith(~prefix="comments", scope));

	if (anyStrings && anyComments) {
		StringAndComment
	} else if(anyStrings) {
		String
	} else if (anyComments) {
		Comment
	} else {
		StringAndComment
	}
};

let ofScope = scope => {
	let isString = StringUtil.startsWith(~prefix="string", scope);
	let isComment = StringUtil.startsWith(~prefix="comments", scope);

	if (anyStrings && anyComments) {
		StringAndComment
	} else if(anyStrings) {
		String
	} else if (anyComments) {
		Comment
	} else {
		StringAndComment
	}
}
