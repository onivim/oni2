type handler('a) = 'a => unit;
type handler2('a, 'b) = ('a, 'b) => unit;
type handler3('a, 'b, 'c) = ('a, 'b, 'c) => unit;
type unsubscribe = unit => unit;

type t('a) = ref(list('a));

let create = () => ref([]);

let dispatch = (v: 'a, handlers: ref(list(handler('a)))) => {
  List.iter(f => f(v), handlers^);
};

let dispatch2 = (a: 'a, b: 'b, handlers: ref(list(handler2('a, 'b)))) => {
  List.iter(f => f(a, b), handlers^);
};

let dispatch3 =
    (a: 'a, b: 'b, c: 'c, handlers: ref(list(handler3('a, 'b, 'c)))) => {
  List.iter(f => f(a, b, c), handlers^);
};

let add = (f: handler('a), handlers: ref(list(handler('a)))) => {
  handlers := [f, ...handlers^];

  () => {
    handlers := List.filter(g => g !== f, handlers^);
  };
};

let add2 = (f: handler2('a, 'b), handlers: ref(list(handler2('a, 'b)))) => {
  handlers := [f, ...handlers^];

  () => {
    handlers := List.filter(g => g !== f, handlers^);
  };
};

let add3 =
    (f: handler3('a, 'b, 'c), handlers: ref(list(handler3('a, 'b, 'c)))) => {
  handlers := [f, ...handlers^];

  () => {
    handlers := List.filter(g => g !== f, handlers^);
  };
};
