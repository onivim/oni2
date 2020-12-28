/*
 * Copyright (C) 2017-present Arctic Ice Studio <development@arcticicestudio.com>
 * Copyright (C) 2017-present Sven Greb <development@svengreb.de>
 *
 * Project:    Nord Visual Studio Code
 * Repository: https://github.com/arcticicestudio/nord-visual-studio-code
 * License:    MIT
 */

/**
 * @file The lint-staged configuration.
 * @author Arctic Ice Studio <development@arcticicestudio.com>
 * @author Sven Greb <development@svengreb.de>
 * @see https://github.com/okonet/lint-staged#configuration
 */

module.exports = {
  "*.{js,json,md,yml}": "prettier --list-different",
  "*.{js,json}": "eslint --ext .js,.json",
  "*.md": "remark --no-stdout"
};
