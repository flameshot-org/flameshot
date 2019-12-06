# Use default Up1 host and API key if user did not pass
# this variable to qmake

isEmpty(UP1_HOST) {
    UP1_HOST = "https://share.riseup.net"
}

isEmpty(UP1_API_KEY) {
    UP1_API_KEY = "59Mnk5nY6eCn4bi9GvfOXhMH54E7Bh6EMJXtyJfs"
}

DEFINES += UP1_HOST=\\\"$${UP1_HOST}\\\"
DEFINES += UP1_API_KEY=\\\"$${UP1_API_KEY}\\\"
