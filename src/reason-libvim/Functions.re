module GetChar = {
  type mode =
    | Wait // getchar()
    | Immediate // getchar(0)
    | Peek; // getchar(1);

  // Todo: Implement mod_mask
  type t = mode => char;
};
