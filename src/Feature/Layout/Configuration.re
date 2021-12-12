open Oni_Core;
open Config.Schema;

module Codec: {
  let showLayoutTabs: Config.Schema.codec([ | `always | `smart | `off]);
  let layoutTabPosition: Config.Schema.codec([ | `top | `bottom]);
} = {
  let showLayoutTabs =
    custom(
      ~decode=
        Json.Decode.(
          string
          |> map(
               fun
               | "always" => `always
               | "smart" => `smart
               | "off" => `off
               | _ => `smart,
             )
        ),
      ~encode=
        Json.Encode.(
          fun
          | `always => string("always")
          | `smart => string("smart")
          | `off => string("off")
        ),
    );

  let layoutTabPosition =
    custom(
      ~decode=
        Json.Decode.(
          string
          |> map(
               fun
               | "top" => `top
               | "bottom" => `bottom
               | _ => `bottom,
             )
        ),
      ~encode=
        Json.Encode.(
          fun
          | `top => string("top")
          | `bottom => string("bottom")
        ),
    );
};

module VimSettings = {
  open Config.Schema;

  let singleTabMode =
    vim(
      "__singletabmode", // We don't have a vim setting for this, today.
      ~compatibilityDefault=true,
      _setting =>
      false
    );

  let layoutTabPosition =
    vim(
      "__layouttabposition", // No vim setting for this, either...
      ~compatibilityDefault=`top,
      _setting =>
      `top
    );
};

let showLayoutTabs =
  setting("oni.layout.showLayoutTabs", Codec.showLayoutTabs, ~default=`smart);

let layoutTabPosition =
  setting(
    ~vim=VimSettings.layoutTabPosition,
    "oni.layout.layoutTabPosition",
    Codec.layoutTabPosition,
    ~default=`bottom,
  );

let singleTabMode =
  setting(
    ~vim=VimSettings.singleTabMode,
    "oni.layout.singleTabMode",
    bool,
    ~default=false,
  );
