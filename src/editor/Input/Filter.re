let filter = key => {
  !String.equal(key, "<S-SHIFT>")
  && !String.equal(key, "<A-SHIFT>")
  && !String.equal(key, "<D-SHIFT>")
  && !String.equal(key, "<D->")
  && !String.equal(key, "<D-S->")
  && !String.equal(key, "<C->")
  && !String.equal(key, "<A-C->")
  && !String.equal(key, "<SHIFT>")
  && !String.equal(key, "<S-C->");
};
