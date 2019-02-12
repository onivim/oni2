/*
 * Configuration.re
 *
 * Configuration settings for the editor
 */

type lineNumbers =
| On
| Off
| Relative

type t = {
    editorLineNumbers: lineNumbers,
};

let create: unit => t =
  () => {
    editorLineNumbers: On,  
  };
