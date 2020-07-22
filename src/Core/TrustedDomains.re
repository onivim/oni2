open Utility;

module Internal = {
  let trustedDomains = [
    "https://open-vsx.org/",
    "https://www.open-vsx.org/",
    "https://github.com/",
    "https://raw.githubusercontent.com/",
    "https://microsoft.com/",
    "https://go.microsoft.com/",
    "https://v2.onivim.io/",
    "https://onivim.github.io/",
  ];
};

let isUrlAllowed = (url: string) => {
  Internal.trustedDomains
  |> List.exists(prefix => {StringEx.startsWith(~prefix, url)});
};

let isAllowed = (uri: Uri.t) => {
  uri |> Uri.toString |> isUrlAllowed;
};
