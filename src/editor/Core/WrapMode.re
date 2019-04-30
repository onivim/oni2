/**
 * WrapMode.re
 */

type t =
| NoWrap
| WrapColumn(int);

let noWrap = NoWrap;

let column = (i) => WrapColumn(i);
