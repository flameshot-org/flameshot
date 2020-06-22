# Use default ImgS3 client_id if user did not pass
# this variable to qmake
isEmpty(IMG_S3_CLIENT_ID) {
    IMG_S3_CLIENT_ID = "313baf0c7b4d3f0"
}

DEFINES += IMG_S3_CLIENT_ID=\\\"$${IMG_S3_CLIENT_ID}\\\"
