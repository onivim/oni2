type t('a) = {
  front: list('a),
  rear: Queue.t(list('a)),
  length: int,
};

let empty = {front: [], rear: Queue.empty, length: 0};

let length = queue => queue.length;
let isEmpty = queue => queue.length == 0;

let push = (item, queue) => {
  ...queue,
  rear: Queue.push([item], queue.rear),
  length: queue.length + 1,
};

let pushReversedChunk = (chunk, queue) =>
  if (chunk == []) {
    queue;
  } else {
    {
      ...queue,
      rear: Queue.push(chunk, queue.rear),
      length: queue.length + List.length(chunk),
    };
  };

let pushChunk = chunk => pushReversedChunk(List.rev(chunk));

let pushFront = (item, queue) => {
  ...queue,
  front: [item, ...queue.front],
  length: queue.length + 1,
};

let pop = queue => {
  let queue =
    if (queue.front == []) {
      switch (Queue.pop(queue.rear)) {
      | (Some(chunk), rear) => {...queue, front: chunk, rear}
      | (None, rear) => {...queue, front: [], rear}
      };
    } else {
      queue;
    };

  switch (queue.front) {
  | [item, ...tail] => (
      Some(item),
      {...queue, front: tail, length: queue.length - 1},
    )
  | [] => (None, queue)
  };
};

let rec take = (count, queue) =>
  if (count == 0) {
    ([], queue);
  } else {
    switch (pop(queue)) {
    | (Some(item), queue) =>
      let (items, queue) = take(count - 1, queue);
      ([item, ...items], queue);

    | (None, queue) => ([], queue)
    };
  };

let toList = ({front, rear, _}) =>
  front @ (Queue.toList(rear) |> ListEx.safeConcat);
