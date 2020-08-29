module Provider = {
    type t = unit => array(string);
    let default = () => [||];
};
