# /snap/local/launchers

Here are the launchers, or wrapper programs to deal with some runtime-fixable
problems for the snapped applications, like setting proper environmental
variables in snap.

In convention launchers are named _something_-launch, for dealing certain
problem with _something_, and usually can be called in a stacked manner to
consolidate their modifications.

```yaml
apps:
  _app_name_:
    command: foo-launch bar-launch _app_command_
```

