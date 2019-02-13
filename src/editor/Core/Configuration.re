/*
 * Configuration.re
 *
 * Configuration settings for the editor
 */

type t = {editorLineNumbers: LineNumber.setting};

let create: unit => t = () => {editorLineNumbers: On};
