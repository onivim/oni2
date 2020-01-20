module type S = {
  type t('a);

  let empty: t(_);
  let length: t('a) => int;
  let isEmpty: t('a) => bool;
  let push: ('a, t('a)) => t('a);
  let pushFront: ('a, t('a)) => t('a);
  let pop: t('a) => (option('a), t('a));
  let take: (int, t('a)) => (list('a), t('a));
  let toList: t('a) => list('a);
};

type t('a) = {
  front: list('a),
  rear: list('a), // reversed
  length: int,
};

let empty = {front: [], rear: [], length: 0};

let length = queue => queue.length;
let isEmpty = queue => queue.length == 0;

let push = (item, queue) => {
  ...queue,
  rear: [item, ...queue.rear],
  length: queue.length + 1,
};

let pushFront = (item, queue) => {
  ...queue,
  front: [item, ...queue.front],
  length: queue.length + 1,
};

let pop = queue => {
  let queue =
    if (queue.front == []) {
      {...queue, front: List.rev(queue.rear), rear: []};
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

let toList = ({front, rear, _}) => front @ List.rev(rear);
