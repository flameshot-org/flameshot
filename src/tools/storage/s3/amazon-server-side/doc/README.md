# Flameshot Amazon S3 Storage

Flameshot S3 storage is using presigned multipart upload, the idea is explained here:

* [Upload files to AWS S3 using pre-signed POST data and a Lambda function](Upload files to AWS S3 using pre-signed POST data and a Lambda function)
* [Uploading Objects to S3 Using One-Time Presigned URLs](https://medium.com/@laardee/uploading-objects-to-s3-using-one-time-presigned-urls-4374943f0801)

Supported operations:
- upload
- delete

## Upload

### Step 1 - get one-time presigned url

C++ example:
```c++
QUrl creds(m_s3Settings.credsUrl());
QNetworkRequest requestCreds(creds);
if (m_s3Settings.xApiKey().length() > 0) {
    requestCreds.setRawHeader(
      QByteArray("X-API-Key"),
      QByteArray(m_s3Settings.xApiKey().toLocal8Bit()));
}
m_networkAMGetCreds->get(requestCreds);
```

Shell example:
```shell script
curl --location --request GET "https://api.img.examples.flameshot.org/v2/image/" \
--header "X-API-Key: aws-secret-key"  
```

Response example:
```json
{
  "formData": {
    "url": "https://screenshots-example-flameshot.org.s3-accelerate.amazonaws.com",
    "fields": {
      "acl": "private",
      "Content-Type": "image/png",
      "Key": "some-key.png",
      "tagging": "<Tagging><TagSet><Tag><Key>deleteToken</Key><Value>some-value</Value></Tag></TagSet></Tagging>",
      "bucket": "screenshots-example-flameshot.org",
      "X-Amz-Algorithm": "AWS4-HMAC-SHA256",
      "X-Amz-Credential": "some-data/eu-central-1/s3/aws4_request",
      "X-Amz-Date": "20200929T100000Z",
      "X-Amz-Security-Token": "some-security-token",
      "Policy": "some-policy",
      "X-Amz-Signature": "some-signature"
    }
  },
  "resultURL": "https://img.examples.flameshot.org/some-key.png",
  "deleteToken": "some-delete-token-x"
}
```

* `resultURL` - url to s3 bucket to upload image directly to Amazon
* `deleteToken` - contains token which will be required for implementing `DELETE` operation.

### Step 2 - do direct multi-part upload to the s3 bucket
Upload to S3 bucket

Send `POST` request to Amazon S3, based on the results from the `Step 1`.

C++ example:
```c++
void ImgS3Uploader::uploadToS3(QJsonDocument& response)
{
    // set paramets from "fields"
    QHttpMultiPart* multiPart =
      new QHttpMultiPart(QHttpMultiPart::FormDataType);

    // read JSON response
    QJsonObject json = response.object();
    QString resultUrl = json["resultURL"].toString();
    QJsonObject formData = json["formData"].toObject();
    QString url = formData["url"].toString();
    m_deleteToken = json["deleteToken"].toString();

    QJsonObject fields = formData["fields"].toObject();
    foreach (auto key, fields.keys()) {
        QString field = fields[key].toString();
        QHttpPart part;
        part.setHeader(QNetworkRequest::ContentDispositionHeader,
                       QVariant("form-data; name=\"" + key + "\""));
        part.setBody(field.toLatin1());
        multiPart->append(part);
    }

    QHttpPart imagePart;
    imagePart.setHeader(QNetworkRequest::ContentTypeHeader,
                        QVariant("image/png"));
    imagePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                        QVariant("form-data; name=\"file\""));

    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    buffer.open(QIODevice::WriteOnly);
    pixmap().save(&buffer, "PNG");

    imagePart.setBody(byteArray);
    multiPart->append(imagePart);

    setImageUrl(QUrl(resultUrl));

    QUrl qUrl(url);
    QNetworkRequest request(qUrl);

    // upload
    m_networkAMUpload->post(request, multiPart);
}
```


# Delete

Shell example:
```shell script
curl --location --request DELETE 'https://api.img.rnd.namecheap.net/v2/image/sH5M3zETezZgA95mmKCMfq.png' \
--header 'X-API-Key: aws-secret-key' \
--header 'Authorization: Bearer some-delete-token-x' 
```
Where `some-delete-token-x` is a value from the request from the `step 1`

C++ example:
```c++
QNetworkRequest request;
m_storageImageName = fileName;
m_deleteToken = deleteToken;
request.setUrl(m_s3Settings.credsUrl().toUtf8() + fileName);
request.setRawHeader("X-API-Key", m_s3Settings.xApiKey().toLatin1());
request.setRawHeader("Authorization", "Bearer " + deleteToken.toLatin1());
m_networkAMRemove->deleteResource(request);
```
