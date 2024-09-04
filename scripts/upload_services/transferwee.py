#!/usr/bin/env python3

#
# Copyright (c) 2018-2023 Leonardo Taccari
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

from typing import Any, Dict, List, Optional, Union
import binascii
import functools
import hashlib
import json
import logging
import os.path
import re
import time
import urllib.parse

import requests


WETRANSFER_API_URL = "https://wetransfer.com/api/v4/transfers"
WETRANSFER_DOWNLOAD_URL = WETRANSFER_API_URL + "/{transfer_id}/download"
WETRANSFER_UPLOAD_EMAIL_URL = WETRANSFER_API_URL + "/email"
WETRANSFER_VERIFY_URL = WETRANSFER_API_URL + "/{transfer_id}/verify"
WETRANSFER_UPLOAD_LINK_URL = WETRANSFER_API_URL + "/link"
WETRANSFER_FINALIZE_URL = WETRANSFER_API_URL + "/{transfer_id}/finalize"

WETRANSFER_EXPIRE_IN = 604800
WETRANSFER_USER_AGENT = "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:102.0) Gecko/20100101 Firefox/102.0"


logger = logging.getLogger(__name__)


def download_url(url: str) -> Optional[str]:
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
    logger.debug(f"Getting download URL of {url}")
    # Follow the redirect if we have a short URL
    if url.startswith("https://we.tl/"):
        r = requests.head(
            url,
            allow_redirects=True,
            headers={"User-Agent": WETRANSFER_USER_AGENT},
        )
        logger.debug(f"Short URL {url} redirects to {r.url}")
        url = r.url

    recipient_id = None
    params = urllib.parse.urlparse(url).path.split("/")[2:]

    if len(params) == 2:
        transfer_id, security_hash = params
    elif len(params) == 3:
        transfer_id, recipient_id, security_hash = params
    else:
        return None

    logger.debug(f"Getting direct_link of {url}")
    j = {
        "intent": "entire_transfer",
        "security_hash": security_hash,
    }
    if recipient_id:
        j["recipient_id"] = recipient_id
    s = _prepare_session()
    if not s:
        raise ConnectionError("Could not prepare session")
    r = s.post(WETRANSFER_DOWNLOAD_URL.format(transfer_id=transfer_id), json=j)
    _close_session(s)

    j = r.json()
    return j.get("direct_link")


def _file_unquote(file: str) -> str:
    """Given a URL encoded file unquote it.

    All occurences of `\', `/' and `../' will be ignored to avoid possible
    directory traversals.
    """
    return (
        urllib.parse.unquote(file)
        .replace("../", "")
        .replace("/", "")
        .replace("\\", "")
    )


def download(url: str, file: str = "") -> None:
    """Given a `we.tl/' or `wetransfer.com/downloads/' download it.

    First a direct link is retrieved (via download_url()), the filename can be
    provided via the optional `file' argument. If not provided the filename
    will be extracted to it and it will be fetched and stored on the current
    working directory.
    """
    logger.debug(f"Downloading {url}")
    dl_url = download_url(url)
    if not dl_url:
        logger.error(f"Could not find direct link of {url}")
        return None
    if not file:
        file = _file_unquote(urllib.parse.urlparse(dl_url).path.split("/")[-1])

    logger.debug(f"Fetching {dl_url}")
    r = requests.get(
        dl_url, headers={"User-Agent": WETRANSFER_USER_AGENT}, stream=True
    )
    with open(file, "wb") as f:
        for chunk in r.iter_content(chunk_size=1024):
            f.write(chunk)


def _file_name_and_size(file: str) -> Dict[str, Union[int, str]]:
    """Given a file, prepare the "item_type", "name" and "size" dictionary.

    Return a dictionary with "item_type", "name" and "size" keys.
    """
    filename = os.path.basename(file)
    filesize = os.path.getsize(file)

    return {"item_type": "file", "name": filename, "size": filesize}


def _prepare_session() -> Optional[requests.Session]:
    """Prepare a wetransfer.com session.

    Return a requests session that will always pass the required headers
    and with cookies properly populated that can be used for wetransfer
    requests.
    """
    s = requests.Session()
    s.headers.update(
        {
            "User-Agent": WETRANSFER_USER_AGENT,
            "x-requested-with": "XMLHttpRequest",
        }
    )
    r = s.get("https://wetransfer.com/")
    m = re.search('name="csrf-token" content="([^"]+)"', r.text)
    if m:
        logger.debug(f"Setting x-csrf-token header to {m.group(1)}")
        s.headers.update({"x-csrf-token": m.group(1)})
    else:
        logger.debug(f"Could not find any csrf-token")

    return s


def _close_session(s: requests.Session) -> None:
    """Close a wetransfer.com session.

    Terminate wetransfer.com session.
    """
    s.close()


def _prepare_email_upload(
    filenames: List[str],
    display_name: str,
    message: str,
    sender: str,
    recipients: List[str],
    session: requests.Session,
) -> Dict[Any, Any]:
    """Given a list of filenames, message a sender and recipients prepare for
    the email upload.

    Return the parsed JSON response.
    """
    j = {
        "files": [_file_name_and_size(f) for f in filenames],
        "from": sender,
        "display_name": display_name,
        "message": message,
        "recipients": recipients,
        "ui_language": "en",
    }

    r = session.post(WETRANSFER_UPLOAD_EMAIL_URL, json=j)
    return r.json()


def _verify_email_upload(
    transfer_id: str, session: requests.Session
) -> Dict[Any, Any]:
    """Given a transfer_id, read the code from standard input.

    Return the parsed JSON response.
    """
    code = input("Code:")

    j = {
        "code": code,
        "expire_in": WETRANSFER_EXPIRE_IN,
    }

    r = session.post(
        WETRANSFER_VERIFY_URL.format(transfer_id=transfer_id), json=j
    )
    return r.json()


def _prepare_link_upload(
    filenames: List[str],
    display_name: str,
    message: str,
    session: requests.Session,
) -> Dict[Any, Any]:
    """Given a list of filenames and a message prepare for the link upload.

    Return the parsed JSON response.
    """
    j = {
        "files": [_file_name_and_size(f) for f in filenames],
        "display_name": display_name,
        "message": message,
        "ui_language": "en",
    }

    r = session.post(WETRANSFER_UPLOAD_LINK_URL, json=j)
    return r.json()


def _storm_urls(
    authorization: str,
) -> Dict[str, str]:
    """Given an authorization bearer extract storm URLs.

    Return a dict with the various storm URLs.

    XXX: Here we can basically ask/be redirected anywhere. Should we do some
    XXX: possible sanity check to possibly avoid doing HTTP request to
    XXX: arbitrary URLs?
    """
    # Extract JWT payload and add extra padding to be sure that it can be
    # base64-decoded.
    j = json.loads(binascii.a2b_base64(authorization.split(".")[1] + "=="))
    return {
        "WETRANSFER_STORM_PREFLIGHT": j.get("storm.preflight_batch_url"),
        "WETRANSFER_STORM_BLOCK": j.get("storm.announce_blocks_url"),
        "WETRANSFER_STORM_BATCH": j.get("storm.create_batch_url"),
    }


def _storm_preflight_item(
    file: str,
) -> Dict[str, Union[List[Dict[str, int]], str]]:
    """Given a file, prepare the item block dictionary.

    Return a dictionary with "blocks", "item_type" and "path" keys.
    """
    filename = os.path.basename(file)
    filesize = os.path.getsize(file)

    return {
        "blocks": [{"content_length": filesize}],
        "item_type": "file",
        "path": filename,
    }


def _storm_preflight(
    authorization: str, filenames: List[str]
) -> Dict[Any, Any]:
    """Given an Authorization token and filenames do preflight for upload.

    Return the parsed JSON response.
    """
    j = {
        "items": [_storm_preflight_item(f) for f in filenames],
    }
    requests.options(
        _storm_urls(authorization)["WETRANSFER_STORM_PREFLIGHT"],
        headers={
            "Origin": "https://wetransfer.com",
            "Access-Control-Request-Method": "POST",
            "User-Agent": WETRANSFER_USER_AGENT,
        },
    )
    r = requests.post(
        _storm_urls(authorization)["WETRANSFER_STORM_PREFLIGHT"],
        json=j,
        headers={
            "Authorization": f"Bearer {authorization}",
            "User-Agent": WETRANSFER_USER_AGENT,
        },
    )
    return r.json()


def _md5(file: str) -> str:
    """Given a file, calculate its MD5 checksum.

    Return MD5 digest as str.
    """
    h = hashlib.md5()
    with open(file, "rb") as f:
        for chunk in iter(functools.partial(f.read, 4096), b""):
            h.update(chunk)
    return h.hexdigest()


def _storm_prepare_item(file: str) -> Dict[str, Union[int, str]]:
    """Given a file, prepare the block for blocks dictionary.

    Return a dictionary with "content_length" and "content_md5_hex" keys.
    """
    filesize = os.path.getsize(file)

    return {"content_length": filesize, "content_md5_hex": _md5(file)}


def _storm_prepare(authorization: str, filenames: List[str]) -> Dict[Any, Any]:
    """Given an Authorization token and filenames prepare for block uploads.

    Return the parsed JSON response.
    """
    j = {
        "blocks": [_storm_prepare_item(f) for f in filenames],
    }
    requests.options(
        _storm_urls(authorization)["WETRANSFER_STORM_BLOCK"],
        headers={
            "Origin": "https://wetransfer.com",
            "Access-Control-Request-Method": "POST",
            "User-Agent": WETRANSFER_USER_AGENT,
        },
    )
    r = requests.post(
        _storm_urls(authorization)["WETRANSFER_STORM_BLOCK"],
        json=j,
        headers={
            "Authorization": f"Bearer {authorization}",
            "Origin": "https://wetransfer.com",
            "User-Agent": WETRANSFER_USER_AGENT,
        },
    )
    return r.json()


def _storm_finalize_item(
    file: str, block_id: str
) -> Dict[str, Union[List[str], str]]:
    """Given a file and block_id prepare the item block dictionary.

    Return a dictionary with "block_ids", "item_type" and "path" keys.

    XXX: Is it possible to actually have more than one block?
    XXX: If yes this - and probably other parts of the code involved with
    XXX: blocks - needs to be instructed to handle them instead of
    XXX: assuming that one file is associated with one block.
    """
    filename = os.path.basename(file)

    return {
        "block_ids": [
            block_id,
        ],
        "item_type": "file",
        "path": filename,
    }


def _storm_finalize(
    authorization: str, filenames: List[str], block_ids: List[str]
) -> Dict[Any, Any]:
    """Given an Authorization token, filenames and block ids finalize upload.

    Return the parsed JSON response.
    """
    j = {
        "items": [
            _storm_finalize_item(f, bid)
            for f, bid in zip(filenames, block_ids)
        ],
    }
    requests.options(
        _storm_urls(authorization)["WETRANSFER_STORM_BATCH"],
        headers={
            "Origin": "https://wetransfer.com",
            "Access-Control-Request-Method": "POST",
            "User-Agent": WETRANSFER_USER_AGENT,
        },
    )

    for i in range(0, 5):
        r = requests.post(
            _storm_urls(authorization)["WETRANSFER_STORM_BATCH"],
            json=j,
            headers={
                "Authorization": f"Bearer {authorization}",
                "Origin": "https://wetransfer.com",
                "User-Agent": WETRANSFER_USER_AGENT,
            },
        )
        if r.status_code == 200:
            break
        else:
            # HTTP request can have 425 HTTP status code and fails with
            # error_code 'BLOCKS_STILL_EXPECTED'. Retry in that and any
            # non-200 cases.
            logger.debug(
                f"Request against "
                + f"{_storm_urls(authorization)['WETRANSFER_STORM_BATCH']} "
                + f"returned {r.status_code}, retrying in {2 ** i} seconds"
            )
            time.sleep(2**i)

    return r.json()


def _storm_upload(url: str, file: str) -> None:
    """Given an url and file upload it.

    Does not return anything.
    """
    requests.options(
        url,
        headers={
            "Origin": "https://wetransfer.com",
            "Access-Control-Request-Method": "PUT",
            "User-Agent": WETRANSFER_USER_AGENT,
        },
    )
    with open(file, "rb") as f:
        requests.put(
            url,
            data=f,
            headers={
                "Origin": "https://wetransfer.com",
                "Content-MD5": binascii.b2a_base64(
                    binascii.unhexlify(_md5(file)), newline=False
                ),
                "X-Uploader": "storm",
                "User-Agent": WETRANSFER_USER_AGENT,
            },
        )


def _finalize_upload(
    transfer_id: str, session: requests.Session
) -> Dict[Any, Any]:
    """Given a transfer_id finalize the upload.

    Return the parsed JSON response.
    """
    j = {
        "wants_storm": True,
    }
    r = session.put(
        WETRANSFER_FINALIZE_URL.format(transfer_id=transfer_id), json=j
    )

    return r.json()


def upload(
    files: List[str],
    display_name: str = "",
    message: str = "",
    sender: Optional[str] = None,
    recipients: Optional[List[str]] = [],
) -> str:
    """Given a list of files upload them and return the corresponding URL.

    Also accepts optional parameters:
     - `display_name': name used as a title of the transfer
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
    logger.debug(f"Checking that all files exists")
    for f in files:
        if not os.path.exists(f):
            raise FileNotFoundError(f)

    # Check that there are no duplicates filenames
    # (despite possible different dirname())
    logger.debug(f"Checking for no duplicate filenames")
    filenames = [os.path.basename(f) for f in files]
    if len(files) != len(set(filenames)):
        raise FileExistsError("Duplicate filenames")

    logger.debug(f"Preparing to upload")
    transfer = None
    s = _prepare_session()
    if not s:
        raise ConnectionError("Could not prepare session")
    if sender and recipients:
        # email upload
        transfer = _prepare_email_upload(
            files, display_name, message, sender, recipients, s
        )
        transfer = _verify_email_upload(transfer["id"], s)
    else:
        # link upload
        transfer = _prepare_link_upload(files, display_name, message, s)

    logger.debug(
        "From storm_upload_token WETRANSFER_STORM_PREFLIGHT URL is: "
        + _storm_urls(transfer["storm_upload_token"])[
            "WETRANSFER_STORM_PREFLIGHT"
        ],
    )
    logger.debug(
        "From storm_upload_token WETRANSFER_STORM_BLOCK URL is: "
        + _storm_urls(transfer["storm_upload_token"])[
            "WETRANSFER_STORM_BLOCK"
        ],
    )
    logger.debug(
        "From storm_upload_token WETRANSFER_STORM_BLOCK URL is: "
        + _storm_urls(transfer["storm_upload_token"])[
            "WETRANSFER_STORM_BATCH"
        ],
    )
    logger.debug(f"Get transfer id {transfer['id']}")
    logger.debug(f"Doing preflight storm")
    _storm_preflight(transfer["storm_upload_token"], files)
    logger.debug(f"Preparing storm block upload")
    blocks = _storm_prepare(transfer["storm_upload_token"], files)
    for f, b in zip(files, blocks["data"]["blocks"]):
        logger.debug(f"Uploading file {f}")
        _storm_upload(b["presigned_put_url"], f)
    logger.debug(f"Finalizing storm batch upload")
    _storm_finalize(
        transfer["storm_upload_token"],
        files,
        [b["block_id"] for b in blocks["data"]["blocks"]],
    )
    logger.debug(f"Finalizing upload with transfer id {transfer['id']}")
    shortened_url = _finalize_upload(transfer["id"], s)["shortened_url"]
    _close_session(s)
    return shortened_url


if __name__ == "__main__":
    from sys import exit
    import argparse

    log = logging.getLogger(__name__)
    log.setLevel(logging.INFO)
    log.addHandler(logging.StreamHandler())

    ap = argparse.ArgumentParser(
        prog="transferwee",
        description="Download/upload files via wetransfer.com",
    )
    sp = ap.add_subparsers(dest="action", help="action", required=True)

    # download subcommand
    dp = sp.add_parser("download", help="download files")
    dp.add_argument(
        "-g",
        action="store_true",
        help="only print the direct link (without downloading it)",
    )
    dp.add_argument(
        "-o",
        type=str,
        default="",
        metavar="file",
        help="output file to be used",
    )
    dp.add_argument(
        "-v", action="store_true", help="get verbose/debug logging"
    )
    dp.add_argument(
        "url",
        nargs="+",
        type=str,
        metavar="url",
        help="URL (we.tl/... or wetransfer.com/downloads/...)",
    )

    # upload subcommand
    up = sp.add_parser("upload", help="upload files")
    up.add_argument(
        "-n",
        type=str,
        default="",
        metavar="display_name",
        help="title for the transfer",
    )
    up.add_argument(
        "-m",
        type=str,
        default="",
        metavar="message",
        help="message description for the transfer",
    )
    up.add_argument("-f", type=str, metavar="from", help="sender email")
    up.add_argument(
        "-t", nargs="+", type=str, metavar="to", help="recipient emails"
    )
    up.add_argument(
        "-v", action="store_true", help="get verbose/debug logging"
    )
    up.add_argument(
        "files", nargs="+", type=str, metavar="file", help="files to upload"
    )

    args = ap.parse_args()

    if args.v:
        log.setLevel(logging.DEBUG)

    if args.action == "download":
        if args.g:
            for u in args.url:
                print(download_url(u))
        else:
            for u in args.url:
                download(u, args.o)
        exit(0)

    if args.action == "upload":
        print(upload(args.files, args.n, args.m, args.f, args.t))
        exit(0)
