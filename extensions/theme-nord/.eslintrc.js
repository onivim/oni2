/*
 * Copyright (C) 2017-present Arctic Ice Studio <development@arcticicestudio.com>
 * Copyright (C) 2017-present Sven Greb <development@svengreb.de>
 *
 * Project:    Nord Visual Studio Code
 * Repository: https://github.com/arcticicestudio/nord-visual-studio-code
 * License:    MIT
 */

/**
 * @file The ESLint configuration.
 * @author Arctic Ice Studio <development@arcticicestudio.com>
 * @author Sven Greb <development@svengreb.de>
 * @see https://github.com/babel/eslint-plugin-babel#rules
 * @see https://github.com/tc39/proposal-optional-chaining
 * @see https://eslint.org/docs/user-guide/configuring#specifying-environments
 */

module.exports = {
  extends: "arcticicestudio-base",
  plugins: ["json", "prettier"],
  parser: "babel-eslint",
  env: {
    node: true,
    browser: true
  },
  rules: {
    /* Prioritize format errors found by Prettier. */
    "prettier/prettier": "error"
  }
};
