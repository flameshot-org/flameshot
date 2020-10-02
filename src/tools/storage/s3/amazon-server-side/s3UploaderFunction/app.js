'use strict';

const uuid = require('short-uuid');
const AWS = require('aws-sdk');
AWS.config.update({ region: process.env.AWS_REGION || 'us-east-1' });
const s3 = new AWS.S3();
const cloudfront = new AWS.CloudFront();

const allowedImageType = 'png';

// Main Lambda entry point
exports.handler = async (event) => {
  console.log('Event: ', event);

  let result;
  if (event.resource === '/v2/image/{fileName}' && event.httpMethod === 'DELETE') {
    let token = (event.headers.Authorization && event.headers.Authorization.split(' ')[1]) || '';
    result = await deleteObject(event.pathParameters.fileName, token);
    if (result === false) {
      return {
        statusCode: 400,
        isBase64Encoded: false,
        headers: {
          'Access-Control-Allow-Origin': '*'
        },
        body: 'Bad request'
      };
    }
  } else {
    result = await getUploadDetails(event.resource === '/v2/image' ? 2 : 1);
  }

  console.log('Result: ', result);

  return {
    statusCode: 200,
    isBase64Encoded: false,
    headers: {
      'Access-Control-Allow-Origin': '*'
    },
    body: JSON.stringify(result)
  };
};

const getUploadDetails = async function(version) {
  const actionId = uuid.generate();
  const deleteToken = uuid.generate();

  const s3Params = {
    Bucket: process.env.UploadBucket,
    Fields: {
      Key:  `${actionId}.${allowedImageType}`,
    },
    Conditions: [
      [ 'eq', '$acl', 'private' ],
      [ 'eq', '$Content-Type', `image/${allowedImageType}` ],
      [ 'content-length-range', 0, 10485760 ], //allows a file size from 0B to 10 MiB
    ],
    Expires: 60,
  };

  if (version === 2) {
    s3Params.Fields.tagging = getDeleteTags(deleteToken);
  }

  console.log('getUploadURL: ', s3Params);

  const signedForm = await createPresignedPost(s3Params);
  signedForm.url = `https://${process.env.UploadBucket}.s3-accelerate.amazonaws.com`;
  signedForm.fields = Object.assign({
    acl: 'private',
    'Content-Type': `image/${allowedImageType}`,
  }, signedForm.fields);

  const uploadDetails = {
    formData: signedForm,
    resultURL: `${process.env.BasePath}${s3Params.Fields.Key}`
  };

  if (version === 2) {
    uploadDetails.deleteToken = deleteToken;
  }

  return uploadDetails;
};

const deleteObject = async (key, token) => {
  const s3ObjectParams = {Bucket: process.env.UploadBucket, Key: key};
  const {TagSet: tags} = await s3.getObjectTagging(s3ObjectParams).promise();

  const storedTokenInfo = tags.find(v => v.Key === 'deleteToken');
  if (storedTokenInfo === undefined) {
    console.log(`Unable to find storedTokenInfo for requested key "${key}"`);
    return false;
  }

  if (storedTokenInfo.Value !== token) {
    console.log(`storedTokenInfo != passed token: ${storedTokenInfo.Value} != ${token}`);
    return false;
  }

  await s3.deleteObject(s3ObjectParams).promise();
  await cloudfront.createInvalidation({
    DistributionId: process.env.DistributionId,
    InvalidationBatch: {
      CallerReference: uuid.generate(),
      Paths: {
        Quantity: 1,
        Items: [`/${key}`]
      }
    }
  }).promise();

  return {status: 'ok'}
};

const getDeleteTags = (token) => `<Tagging><TagSet><Tag><Key>deleteToken</Key><Value>${token}</Value></Tag></TagSet></Tagging>`;

const createPresignedPost = params =>
    new Promise((resolve, reject) =>
        s3.createPresignedPost( params, (err, data) => err ? reject(err) : resolve(data) )
    );