let guard = f =>
  try(Ok(f())) {
  | exn => Error(exn)
  };
