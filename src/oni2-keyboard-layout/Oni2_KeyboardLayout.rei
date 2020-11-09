type callback = (~language: string, ~layout: string) => unit;

let init: unit => unit;
let subscribe: callback => unit;
