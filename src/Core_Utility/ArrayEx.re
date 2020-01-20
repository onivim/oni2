exception Found(int);

let findIndex = (predicate, array) =>
  try(
    {
      for (i in 0 to Array.length(array) - 1) {
        if (predicate(array[i])) {
          raise(Found(i));
        };
      };
      None;
    }
  ) {
  | Found(i) => Some(i)
  };
