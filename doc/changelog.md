# Change log

## 1.0.0

First release.

## 1.1.0

- API changes in order to get more control over assigned input numbers, other than sequential numbering. See documentation for the `inputs` namespace, methods with suffix "Ext". More than one button matrix is allowed this way.
- Fixed wrong behaviour of ALPS RKJX series of rotary encoders (and potentially others).
- Minor documentation improvements and fixes.

## 1.1.1

- Fixed bug in `inputs::start()` that was casuing a wrong call to `abort()` in some circumstances.