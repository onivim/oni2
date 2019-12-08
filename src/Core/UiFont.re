type t = {
  fontFile: string,
  fontSize: int,
};

let create = (~fontFile, ~fontSize, ()) => {fontFile, fontSize};