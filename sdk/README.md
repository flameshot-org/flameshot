# Flameshot action-plugin SDK

The SDK builds and checks external action plugins without requiring a Flameshot
development checkout. It needs Python 3.10 or newer.

```sh
flameshot-plugin create org.example.my-tool --language python
flameshot-plugin test org.example.my-tool --image screenshot.png
flameshot-plugin pack org.example.my-tool
flameshot plugins install org.example.my-tool.flameshot-plugin
```

Templates are included for Python, POSIX shell, Rust, and C++. Rust and C++ use
a small executable wrapper so their projects can be validated before the native
binary has been built. The schemas in `schemas/` describe manifest API 1 and
structured result protocol 1.

Plugins receive PNG data on standard input. The recommended `json` output mode
returns exactly one UTF-8 JSON object on standard output. Diagnostics belong on
standard error.
