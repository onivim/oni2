module Position {
    type t = 
    | ZeroBasedPosition(int)
    | OneBasedPosition(int);

    let toZeroBasedIndex = (pos: t) => switch(pos) {
    | ZeroBasedPosition(n) => n
    | OneBasedPosition(n) => n - 1
    };
}
