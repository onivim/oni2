open Revery;
open Revery.UI.Components;

let colorToCSS = color => {
  let (r, g, b, _) = Color.toRgba(color);
  Printf.sprintf(
    "#%02X%02X%02X",
    int_of_float(r *. 255.),
    int_of_float(g *. 255.),
    int_of_float(b *. 255.),
  );
};

let folder: SVGIcon.t =
  (~size=24, ~strokeWidth=2, ~color=Colors.white, ()) =>
    <SVG
      src={
            `Str(
              Printf.sprintf(
                {|<svg
                  xmlns="http://www.w3.org/2000/svg"
                  width="%d"
                  height="%d"
                  viewBox="0 0 %d %d"
                  fill="none"
                  stroke="%s"
                  stroke-width="%d"
                  stroke-linecap="round"
                  stroke-linejoin="round"
                  class="feather feather-folder"
                >
                  <path d="M22 19a2 2 0 0 1-2 2H4a2 2 0 0 1-2-2V5a2 2 0 0 1 2-2h5l2 3h9a2 2 0 0 1 2 2z" />
                </svg>
            |},
                size,
                size,
                size,
                size,
                colorToCSS(color),
                strokeWidth,
              ),
            )
          }
    />;

let codeSandbox: SVGIcon.t =
  (~size=24, ~strokeWidth=2, ~color=Colors.white, ()) =>
    <SVG
      src={
            `Str(
              Printf.sprintf(
                {|<svg
                  xmlns="http://www.w3.org/2000/svg"
                  width="%d"
                  height="%d"
                  viewBox="0 0 %d %d"
                  fill="none"
                  stroke="%s"
                  stroke-width="%d"
                  stroke-linecap="round"
                  stroke-linejoin="round"
                  class="feather feather-search"
                >
                  <path d="M21 16V8a2 2 0 0 0-1-1.73l-7-4a2 2 0 0 0-2 0l-7 4A2 2 0 0 0 3 8v8a2 2 0 0 0 1 1.73l7 4a2 2 0 0 0 2 0l7-4A2 2 0 0 0 21 16z"/>
                  <polyline points="7.5 4.21 12 6.81 16.5 4.21"/>
                  <polyline points="7.5 19.79 7.5 14.6 3 12"/>
                  <polyline points="21 12 16.5 14.6 16.5 19.79"/>
                  <polyline points="3.27 6.96 12 12.01 20.73 6.96"/>
                  <line x1="12" y1="22.08" x2="12" y2="12"/>
                </svg>
            |},
                size,
                size,
                size,
                size,
                colorToCSS(color),
                strokeWidth,
              ),
            )
          }
    />;
let search: SVGIcon.t =
  (~size=24, ~strokeWidth=2, ~color=Colors.white, ()) =>
    <SVG
      src={
            `Str(
              Printf.sprintf(
                {|<svg
                  xmlns="http://www.w3.org/2000/svg"
                  width="%d"
                  height="%d"
                  viewBox="0 0 %d %d"
                  fill="none"
                  stroke="%s"
                  stroke-width="%d"
                  stroke-linecap="round"
                  stroke-linejoin="round"
                  class="feather feather-codesandbox"
                >
                  <circle cx="11" cy="11" r="8"/>
                  <line x1="21" y1="21" x2="16.65" y2="16.65"/>
                </svg>
            |},
                size,
                size,
                size,
                size,
                colorToCSS(color),
                strokeWidth,
              ),
            )
          }
    />;

let gitPullRequest: SVGIcon.t =
  (~size=24, ~strokeWidth=2, ~color=Colors.white, ()) =>
    <SVG
      src={
            `Str(
              Printf.sprintf(
                {|<svg
                  xmlns="http://www.w3.org/2000/svg"
                  width="%d"
                  height="%d"
                  viewBox="0 0 %d %d"
                  fill="none"
                  stroke="%s"
                  stroke-width="%d"
                  stroke-linecap="round"
                  stroke-linejoin="round"
                  class="feather feather-git-pull-request"
                >
                  <circle cx="18" cy="18" r="3"/>
                  <circle cx="6" cy="6" r="3"/>
                  <path d="M13 6h3a2 2 0 0 1 2 2v7"/>
                  <line x1="6" y1="9" x2="6" y2="21"/>
                </svg>
            |},
                size,
                size,
                size,
                size,
                colorToCSS(color),
                strokeWidth,
              ),
            )
          }
    />;
