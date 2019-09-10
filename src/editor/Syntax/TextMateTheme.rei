/*
 TextMateTheme.rei

 Interface for textmate theme matching
 */

open Revery;
open TextMateScopes;

/*
   [themeSelector] describes a [string] selector,
   along with the [TokenStyle.t] styling associated with that selector.
 */
type themeSelector = (string, TokenStyle.t);

/*
   [t] is a description of a TextMate theme
 */
type t;

/*
   [create] builds a Theme [t] from a list of styles
 */
let create:
  (
    ~defaultBackground: Color.t,
    ~defaultForeground: Color.t,
    list(themeSelector)
  ) =>
  t;

/*
    [of_yojson] instantiates a Theme [t] from JSON
 */
let of_yojson:
  (~defaultBackground: Color.t, ~defaultForeground: Color.t, Yojson.Safe.t) =>
  t;

/*
   [empty] is an empty Theme [t] with no selectors
 */
let empty: t;

/*
    [match] returns the resolved style information,
    given the scopes [string]. The [scopes] should include
    the full ancestor list, separated by spaces, for example:
    "text.html.basic source.php string.quoted.double.php"

    Returns styling information based on the selecotrs.
 */
let match: (t, string) => ResolvedStyle.t;

/*
   [show] returns a string representation of the Theme [t]
 */
let show: t => string;
