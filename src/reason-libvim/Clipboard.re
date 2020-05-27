let _provider: ref(option(Types.clipboardProvider)) = ref(None);

let setProvider = (provider: Types.clipboardProvider) => {
  _provider := Some(provider);
};
