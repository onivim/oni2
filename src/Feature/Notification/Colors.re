open Oni_Core;
open Model;

// COLORS

open ColorTheme.Schema;
include Feature_Theme.Colors;

let infoBackground =
  define("oni.notification.infoBackground", all(hex("#209CEE")));
let infoForeground =
  define("oni.notification.infoForeground", all(hex("#FFF")));
let warningBackground =
  define("oni.notification.warningBackground", all(hex("#FFDD57")));
let warningForeground =
  define("oni.notification.warningForeground", all(hex("#333")));
let errorBackground =
  define("oni.notification.errorBackground", all(hex("#FF3860")));
let errorForeground =
  define("oni.notification.errorForeground", all(hex("#FFF")));

let backgroundFor = (notification: notification) =>
  switch (notification.kind) {
  | Warning => warningBackground
  | Error => errorBackground
  | Info => infoBackground
  };

let foregroundFor = (notification: notification) =>
  switch (notification.kind) {
  | Warning => warningForeground
  | Error => errorForeground
  | Info => infoForeground
  };
