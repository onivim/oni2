/*
 * Configuration.re
 *
 * Configuration settings for the editor
 */

type tablineMode =
  | Buffers
  | Tabs
  | Hybrid;

type t = {
  editorLineNumbers: LineNumber.setting,
  editorMinimapEnabled: bool,
  tablineMode,
};

let create: unit => t =
  () => {
    editorLineNumbers: On,
    editorMinimapEnabled: true,
    tablineMode: Buffers,
  };
