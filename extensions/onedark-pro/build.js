const fs = require('fs')
const editorConfig = require('./src/editor.json')
const { classic, bold, vivid } = require('./src/syntax')

fs.writeFileSync(
  './themes/OneDark-Pro.json',
  JSON.stringify(
    {
      ...editorConfig,
      ...classic
    },
    '',
    2
  )
)

fs.writeFileSync(
  './themes/OneDark-Pro-bold.json',
  JSON.stringify(
    {
      ...editorConfig,
      ...bold
    },
    '',
    2
  )
)

fs.writeFileSync(
  './themes/OneDark-Pro-vivid.json',
  JSON.stringify(
    {
      ...editorConfig,
      ...vivid
    },
    '',
    2
  )
)
