let lowercase = (uchar: Uchar.t) =>
  if (Uchar.is_char(uchar)) {
    uchar |> Uchar.to_char |> Char.lowercase_ascii |> Uchar.of_char;
  } else {
    uchar;
  };
