const requestLight = require("@onivim/request-light")
const url = process.argv[2]

requestLight
    .xhr({
        type: "get",
        url: url,
    })
    .then((response) => {
        if (response.status == 200) {
            process.stdout.write(response.responseText)
        }
    })
