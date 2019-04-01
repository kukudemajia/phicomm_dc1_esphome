## Python Version

This package provides a basic python version checking utility.  
It will check for a range of python versions and either report
an error or exit depending on the parameters provided.

## Example

```
$ python
Python 3.5.1+ (default, Mar 30 2016, 22:46:26)
[GCC 5.3.1 20160330] on linux
Type "help", "copyright", "credits" or "license" for more information.
>>> import python_version
>>>
>>> try:
...     python_version.check(min=(3, 0, 0), max=(4, 0, 0))
... except Exception as e:
...     print(repr(e))
... else:
...     print("All good!")
...
All good!
>>> try:
...     python_version.check(min=(3, 6, 0), max=(4, 0, 0))
... except Exception as e:
...     print(repr(e))
... else:
...     print("All good!")
...
VersionConflict('requires python version >= 3.6.0 but the running python version is 3.5.1+',)
>>> try:
...     python_version.check(min=(2, 7, 0), max=(2, 7, 999))
... except Exception as e:
...     print(repr(e))
... else:
...     print("All good!")
...
VersionConflict('requires python version <= 2.7.999 but the running python version is 3.5.1+',)
>>> try:
...     python_version.check(min=(2, 7, 0), max=(2, 7, 999), exit_on_error=True)
... except Exception as e:
...     print(repr(e))
... else:
...     print("All good!")
...
requires python version <= 2.7.999 but the running python version is 3.5.1+
$
```


