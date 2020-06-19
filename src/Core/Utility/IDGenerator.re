module Make = (()) => {
  let current' = ref(0);

  let get = () => {
    let i = current'^;
    current' := i + 1;
    i;
  };
};
