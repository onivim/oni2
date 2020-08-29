module Provider = {
  type t = string => array(string);
  let default = _ => [||];
};
