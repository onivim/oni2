/*
 * BufferRenderer.re
 *
 * BufferRenderer tracks state related to custom buffer renderers.
 * Custom renderers, as an alternative to the textual renderer,
 * would be useful in several situations:
 * - A custom 'welcome screen'
 * - Show images visually (or other binary formats - cmi/cmts woudl be cool!)
 * - Terminal
 * - Potentially diff views?
 */

[@deriving show({with_path: false})]
type t =
  | Editor
  | Welcome
  | Version
  | Terminal(Feature_Terminal.rendererState);

[@deriving show({with_path: false})]
type action =
  | RendererAvailable(int, t);
