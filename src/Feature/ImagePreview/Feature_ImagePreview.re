open Revery.UI;

module Styles = {
  open Style;
  let container = [
    flexGrow(1),
    flexDirection(`Column),
    justifyContent(`Center),
    alignItems(`Center),
  ];
};

module View = {
  let make = (~filePath: string, ()) => {
    <View style=Styles.container> <Image src={`File(filePath)} /> </View>;
  };
};
