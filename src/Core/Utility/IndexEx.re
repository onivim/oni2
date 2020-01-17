let prevRollOver = (~first=0, ~last, current) =>
  if (first >= last) {
    first;
  } else if (current <= first) {
    last;
  } else {
    current - 1;
  };

let prevRollOverOpt = (~first=0, ~last) =>
  fun
  | Some(index) => Some(prevRollOver(index, ~first, ~last))
  | None => Some(last);

let nextRollOver = (~first=0, ~last, current) =>
  if (first >= last) {
    first;
  } else if (current >= last) {
    first;
  } else {
    current + 1;
  };

let nextRollOverOpt = (~first=0, ~last) =>
  fun
  | Some(index) => Some(nextRollOver(index, ~first, ~last))
  | None => Some(first);
