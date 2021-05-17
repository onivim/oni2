type t('a) = {
  capacity: int,
  backingArray: array('a),
  mutable startPosition: int,
  mutable nextInsertPosition: int,
};

let make = (~capacity, default) => {
  capacity,
  backingArray: Array.make(capacity, default),
  startPosition: 0,
  nextInsertPosition: 0,
};

let push = (item, ringBuffer) => {
  let {backingArray, nextInsertPosition, startPosition, capacity} = ringBuffer;
  let idx = nextInsertPosition mod capacity;

  let startPosition =
    nextInsertPosition >= capacity ? startPosition + 1 : startPosition;

  backingArray[idx] = item;

  ringBuffer.nextInsertPosition = nextInsertPosition + 1;
  ringBuffer.startPosition = startPosition;
};

let size = ringBuffer => {
  let {nextInsertPosition, capacity, _} = ringBuffer;
  nextInsertPosition >= capacity ? capacity : nextInsertPosition;
};

let getAt = (index, ringBuffer) => {
  let {backingArray, startPosition, capacity, _} = ringBuffer;
  let idx = abs((index - startPosition) mod capacity);
  backingArray[idx];
};

let capacity = ({capacity, _}) => capacity;
