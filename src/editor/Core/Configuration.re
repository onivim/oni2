/*
 * Configuration.re
 *
 * Configuration settings for the editor
 */

type t = {
  editorLineNumbers: LineNumber.setting,
  editorMinimapEnabled: bool,
};

let create: unit => t =
  () => {editorLineNumbers: On, editorMinimapEnabled: true};
