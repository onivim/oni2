type operation = Native.operation =
| NoPending
| Delete
| Yank
| Change
| LeftShift
| RightShift
| Filter
| SwitchCase
| Indent
| Format
| Colon
| MakeUpperCase
| MakeLowerCase
| Join
| JoinNS
| Rot13
| Replace
| Insert
| Append
| Fold
| FoldOpen
| FoldOpenRecursive
| FoldClose
| FoldCloseRecursive
| FoldDelete
| FoldDeleteRecursive
| Format2
| Function
| NumberAdd
| NumberSubtract
| Comment;

let operationToString = fun
| NoPending => ""
| Delete => "d"
| Yank => "y"
| Change => "c"
| LeftShift => "<"
| RightShift => ">"
| Filter => "!"
| SwitchCase => "g~"
| Indent => "="
| Format => "gq"
| Colon => ":"
| MakeUpperCase => "gU"
| MakeLowerCase => "gu"
| Join => "J"
| JoinNS => "gJ"
| Rot13 => "g?"
| Replace => "r"
| Insert => "I"
| Append => "A"
| Fold => "zf"
| FoldOpen => "zo"
| FoldOpenRecursive => "zO"
| FoldClose => "zc"
| FoldCloseRecursive => "zC"
| FoldDelete => "zd"
| FoldDeleteRecursive => "zD"
| Format2 => "gw"
| Function => "g@"
| NumberAdd => "+"
| NumberSubtract => "-"
| Comment => "gc";

type pending = {
    operation,
    register: int,
    count: int,
};

let get = () => {
    None;
}
