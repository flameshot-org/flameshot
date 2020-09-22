- Feature Name: upload_plugin
- Start Date: 2020-09-21
- RFC PR: TBD 

# Summary
[summary]: #summary

This feature will allow users to select different options for uploading annotated images. It will also allow users to create their own out of tree plugins. 


# Motivation
[motivation]: #motivation

There have been many requests over the years to support different image hosting sites. There have also been requests to enable encrypted uploads, support proxies, and delete uploaded images. Flameshot should not need to keep track of every different image hosting API and constantly need to support them.

By moving to an uploader plugin architecture, we can support a couple popular image hosting websites by keeping some plugins "in source tree". Then users can follow our guide / API to create and manage their own plugins. 


# Guide-level explanation
[guide-level-explanation]: #guide-level-explanation

At a high level the user will be able to select which plugin shall be activated when the upload button is pressed. This will be set in the existing configuration tool. The program will search a predefined directory to enumerate the plugins when the configuration tool is opened. 

When a user wants to create a new plugin, they will refer to our sample plugins and API. 


# Reference-level explanation
[reference-level-explanation]: #reference-level-explanation

At a detailed design level I propose embedding a python interpreter into flameshot, and writing plugins in python. 

The interface shall pass the annotated QtPixmap by copy to the python plugin as a buffer of ints and a buffer length. The python function shall return a bool representing success or failure. This minimal C API will avoid ABI issues in the future and decrease the complexity and interactions between the plugin and main process.

The cpp caller shall make this python call from a lamda expression in a secondary thread to not block the main GUI thread. On returning, the cpp lamda shall emit a DBUS notification and then that thread shall end. If the thread lasts longer than 30seconds, a timeout shall activate and a failure notification shall be shown. 

I recommend any configuration for a plugin **not** be managed through the flameshot API or config file. Each plugin is responsible for loading its own configuration. 


# Drawbacks
[drawbacks]: #drawbacks

Implementing this feature with python plugins requires that users must have python3 installed to do any uploads. 

Having python plugins in a c++ application may not be intuitive. 


# Rationale and alternatives
[rationale-and-alternatives]: #rationale-and-alternatives

An alternative design could be pure c++ plugins. These could be compiled into .so and .dll files and dynamically loaded at runtime. There are a few reasons I think embedding python is a better solution.
1. Most image uploading platforms support a python API out of the box
    * [Imgur](https://pypi.org/project/imgur-uploader/)
    * [Dropbox](https://www.dropbox.com/developers)
    * [S3](https://boto3.amazonaws.com/v1/documentation/api/latest/guide/s3-uploading-files.html)
    * [Owncloud / Nextcloud](https://github.com/owncloud/pyocclient)

1. This gives users the power to **easily** modify and control the upload process. For example, if they want to adjust a proxy setting, no need to learn c++ and CMake, just edit a python file.

1. The average user will know python better than c++

1. Flameshot does not need to upstream a script for every platform. We can have very clear minimal examples (imgur / nextcloud) and let the community contribute others. 

1. The python script will effectively be "sandboxed". If user A loads a faulty plugin written by user B, the worst that will happen is the upload will fail. If they wrote the plugin in c++ they could forget to delete memory, corrupt pointer, etc.

1. C++ does not have a stable ABI. Plugins compiled on compiler X version 1, may not work if flameshot is compiled on compiler Y version 2. 

1. The users writing scripts can *test their software* outside of flameshot using normal python tools. 


# Prior art
[prior-art]: #prior-art

This is a good starting point for [embedding Python](https://docs.python.org/3/extending/embedding.html)

# Unresolved questions
[unresolved-questions]: #unresolved-questions
Here are some things that need to be worked through before writing this feature:

1. What is the exact API we want to agree on.

1. How do we version the API so an "old" plugin works on an API change. 

1. How exactly do we want to manage the upload thread

1. What are all the UI elements that need to change. 


# Future possibilities
[future-possibilities]: #future-possibilities

This could potentially be a framework in the future for users to write custom "draw plugins" for example we could pass the image to a python script that adds a watermark and returns the pixmap back to c++. 