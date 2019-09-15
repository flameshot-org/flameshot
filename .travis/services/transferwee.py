#!/usr/bin/env python3.7

#
# Copyright (c) 2018-2019 Leonardo Taccari
# All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
# TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
# BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#


"""
Download/upload files via wetransfer.com

transferwee is a script/module to download/upload files via wetransfer.com.

It exposes `download' and `upload' subcommands, respectively used to download
files from a `we.tl' or `wetransfer.com/downloads' URLs and upload files that
will be shared via emails or link.
"""


from typing import List
import os.path
import urllib.parse
import zlib

import requests


WETRANSFER_API_URL = 'https://wetransfer.com/api/v4/transfers'
WETRANSFER_DOWNLOAD_URL = WETRANSFER_API_URL + '/{transfer_id}/download'
WETRANSFER_UPLOAD_EMAIL_URL = WETRANSFER_API_URL + '/email'
WETRANSFER_UPLOAD_LINK_URL = WETRANSFER_API_URL + '/link'
WETRANSFER_FILES_URL = WETRANSFER_API_URL + '/{transfer_id}/files'
WETRANSFER_PART_PUT_URL = WETRANSFER_FILES_URL + '/{file_id}/part-put-url'
WETRANSFER_FINALIZE_MPP_URL = WETRANSFER_FILES_URL + '/{file_id}/finalize-mpp'
WETRANSFER_FINALIZE_URL = WETRANSFER_API_URL + '/{transfer_id}/finalize'

WETRANSFER_DEFAULT_CHUNK_SIZE = 5242880


def download_url(url: str) -> str:
    """Given a wetransfer.com download URL download return the downloadable URL.

    The URL should be of the form `https://we.tl/' or
    `https://wetransfer.com/downloads/'. If it is a short URL (i.e. `we.tl')
    the redirect is followed in order to retrieve the corresponding
    `wetransfer.com/downloads/' URL.

    The following type of URLs are supported:
     - `https://we.tl/<short_url_id>`:
        received via link upload, via email to the sender and printed by
        `upload` action
     - `https://wetransfer.com/<transfer_id>/<security_hash>`:
        directly not shared in any ways but the short URLs actually redirect to
        them
     - `https://wetransfer.com/<transfer_id>/<recipient_id>/<security_hash>`:
        received via email by recipients when the files are shared via email
        upload

    Return the download URL (AKA `direct_link') as a str or None if the URL
    could not be parsed.
    """
    # Follow the redirect if we have a short URL
    if url.startswith('https://we.tl/'):
        r = requests.head(url, allow_redirects=True)
        url = r.url

    recipient_id = None
    params = url.replace('https://wetransfer.com/downloads/', '').split('/')

    if len(params) == 2:
        transfer_id, security_hash = params
    elif len(params) == 3:
        transfer_id, recipient_id, security_hash = params
    else:
        return None

    j = {
        "security_hash": security_hash,
    }
    if recipient_id:
        j["recipient_id"] = recipient_id
    r = requests.post(WETRANSFER_DOWNLOAD_URL.format(transfer_id=transfer_id),
                      json=j)

    j = r.json()
    return j.get('direct_link')


def download(url: str) -> None:
    """Given a `we.tl/' or `wetransfer.com/downloads/' download it.

    First a direct link is retrieved (via download_url()), the filename will
    be extracted to it and it will be fetched and stored on the current
    working directory.
    """
    dl_url = download_url(url)
    file = urllib.parse.urlparse(dl_url).path.split('/')[-1]

    r = requests.get(dl_url, stream=True)
    with open(file, 'wb') as f:
        for chunk in r.iter_content(chunk_size=1024):
            f.write(chunk)


def _file_name_and_size(file: str) -> dict:
    """Given a file, prepare the "name" and "size" dictionary.

    Return a dictionary with "name" and "size" keys.
    """
    filename = os.path.basename(file)
    filesize = os.path.getsize(file)

    return {
        "name": filename,
        "size": filesize
    }


def _prepare_email_upload(filenames: List[str], message: str,
                          sender: str, recipients: List[str]) -> str:
    """Given a list of filenames, message a sender and recipients prepare for
    the email upload.

    Return the parsed JSON response.
    """
    j = {
        "files": [_file_name_and_size(f) for f in filenames],
        "from": sender,
        "message": message,
        "recipients": recipients,
        "ui_language": "en",
    }

    r = requests.post(WETRANSFER_UPLOAD_EMAIL_URL, json=j)
    return r.json()


def _prepare_link_upload(filenames: List[str], message: str) -> str:
    """Given a list of filenames and a message prepare for the link upload.

    Return the parsed JSON response.
    """
    j = {
        "files": [_file_name_and_size(f) for f in filenames],
        "message": message,
        "ui_language": "en",
    }

    r = requests.post(WETRANSFER_UPLOAD_LINK_URL, json=j)
    return r.json()


def _prepare_file_upload(transfer_id: str, file: str) -> str:
    """Given a transfer_id and file prepare it for the upload.

    Return the parsed JSON response.
    """
    j = _file_name_and_size(file)
    r = requests.post(WETRANSFER_FILES_URL.format(transfer_id=transfer_id),
                      json=j)
    return r.json()


def _upload_chunks(transfer_id: str, file_id: str, file: str,
                   default_chunk_size: int = WETRANSFER_DEFAULT_CHUNK_SIZE) -> str:
    """Given a transfer_id, file_id and file upload it.

    Return the parsed JSON response.
    """
    f = open(file, 'rb')

    chunk_number = 0
    while True:
        chunk = f.read(default_chunk_size)
        chunk_size = len(chunk)
        if chunk_size == 0:
            break
        chunk_number += 1

        j = {
            "chunk_crc": zlib.crc32(chunk),
            "chunk_number": chunk_number,
            "chunk_size": chunk_size,
            "retries": 0
        }

        r = requests.post(
            WETRANSFER_PART_PUT_URL.format(transfer_id=transfer_id,
                                           file_id=file_id),
            json=j)
        url = r.json().get('url')
        r = requests.options(url,
                             headers={
                                 'Origin': 'https://wetransfer.com',
                                 'Access-Control-Request-Method': 'PUT',
                             })
        r = requests.put(url, data=chunk)

    j = {
        'chunk_count': chunk_number
    }
    r = requests.put(
        WETRANSFER_FINALIZE_MPP_URL.format(transfer_id=transfer_id,
                                           file_id=file_id),
        json=j)

    return r.json()


def _finalize_upload(transfer_id: str) -> str:
    """Given a transfer_id finalize the upload.

    Return the parsed JSON response.
    """
    r = requests.put(WETRANSFER_FINALIZE_URL.format(transfer_id=transfer_id))

    return r.json()


def upload(files: List[str], message: str = '', sender: str = None,
           recipients: List[str] = []) -> str:
    """Given a list of files upload them and return the corresponding URL.

    Also accepts optional parameters:
     - `message': message used as a description of the transfer
     - `sender': email address used to receive an ACK if the upload is
                 successfull. For every download by the recipients an email
                 will be also sent
     - `recipients': list of email addresses of recipients. When the upload
                     succeed every recipients will receive an email with a link

    If both sender and recipient parameters are passed the email upload will be
    used. Otherwise, the link upload will be used.

    Return the short URL of the transfer on success.
    """

    # Check that all files exists
    for f in files:
        if not os.path.exists(f):
            return None

    # Check that there are no duplicates filenames
    # (despite possible different dirname())
    filenames = [os.path.basename(f) for f in files]
    if len(files) != len(set(filenames)):
        return None

    transfer_id = None
    if sender and recipients:
        # email upload
        transfer_id = \
            _prepare_email_upload(filenames, message, sender, recipients)['id']
    else:
        # link upload
        transfer_id = _prepare_link_upload(filenames, message)['id']

    for f in files:
        file_id = _prepare_file_upload(transfer_id, os.path.basename(f))['id']
        _upload_chunks(transfer_id, file_id, f)

    return _finalize_upload(transfer_id)['shortened_url']


if __name__ == '__main__':
    import argparse

    ap = argparse.ArgumentParser(
        prog='transferwee',
        description='Download/upload files via wetransfer.com'
    )
    sp = ap.add_subparsers(dest='action', help='action')

    # download subcommand
    dp = sp.add_parser('download', help='download files')
    dp.add_argument('-g', action='store_true',
                    help='only print the direct link (without downloading it)')
    dp.add_argument('url', nargs='+', type=str, metavar='url',
                    help='URL (we.tl/... or wetransfer.com/downloads/...)')

    # upload subcommand
    up = sp.add_parser('upload', help='upload files')
    up.add_argument('-m', type=str, default='', metavar='message',
                    help='message description for the transfer')
    up.add_argument('-f', type=str, metavar='from', help='sender email')
    up.add_argument('-t', nargs='+', type=str, metavar='to',
                    help='recipient emails')
    up.add_argument('files', nargs='+', type=str, metavar='file',
                    help='files to upload')

    args = ap.parse_args()

    if args.action == 'download':
        if args.g:
            for u in args.url:
                print(download_url(u))
        else:
            for u in args.url:
                download(u)
        exit(0)

    if args.action == 'upload':
        print(upload(args.files, args.m, args.f, args.t))
        exit(0)

    # No action selected, print help message
    ap.print_help()
    exit(1)
