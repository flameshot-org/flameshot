# Use default Up1 host and API key if user did not pass
# this variable to qmake

isEmpty(UP1_HOST) {
    UP1_HOST = "https://pastebin.synalabs.hosting"
}

isEmpty(UP1_API_KEY) {
    UP1_API_KEY = "4034a170b4517897238b58ecbe902dee187bf890"
}

DEFINES += UP1_HOST=\\\"$${UP1_HOST}\\\"
DEFINES += UP1_API_KEY=\\\"$${UP1_API_KEY}\\\"
