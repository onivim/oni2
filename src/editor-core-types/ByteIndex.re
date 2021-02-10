[@deriving show({with_path: false})]
type t = int;

let zero = 0;

let ofInt = Fun.id;
let toInt = Fun.id;

let (+) = (a, b) => a + b;
let (<) = (a, b) => a < b;
let (>) = (a, b) => a > b;
let (<=) = (a, b) => a <= b;
let (>=) = (a, b) => a >= b;

let next = (str, idx) => Zed_utf8.next(str, idx);

let%test_module "next" =
  (module
   {
     let%test "ascii" = {
       next("abc", zero) |> toInt == 1;
     };
     let%test "utf-8" = {
       next("κόσμε", zero) |> toInt == 2;
     };
   });
