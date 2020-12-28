/*
 * Copyright (C) 2017-present Arctic Ice Studio <development@arcticicestudio.com>
 * Copyright (C) 2017-present Sven Greb <development@svengreb.de>
 *
 * Project:    Nord Visual Studio Code
 * Repository: https://github.com/arcticicestudio/nord-visual-studio-code
 * License:    MIT
 */

/**
 * @file The husky configuration.
 * @author Arctic Ice Studio <development@arcticicestudio.com>
 * @author Sven Greb <development@svengreb.de>
 * @see https://github.com/typicode/husky
 */

module.exports = {
  hooks: {
    "pre-commit": "lint-staged"
  }
};
