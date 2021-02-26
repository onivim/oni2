/*
 * ConfigurationValues.re
 *
 * Configuration settings for the editor
 */
[@deriving show({with_path: false})]
type vimUseSystemClipboard = {
  yank: bool,
  delete: bool,
  paste: bool,
};

type t = {
  filesExclude: list(string),
  vimUseSystemClipboard,
  // Experimental feature flags
  // These are 'use-at-your-own-risk' features
  // Turn on tree-sitter for supported filetypes:
  // - JSON
};

let default = {
  filesExclude: ["_esy", "node_modules", ".git"],
  vimUseSystemClipboard: {
    yank: true,
    delete: false,
    paste: false,
  },
};
