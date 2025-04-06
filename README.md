![Tests](https://img.shields.io/github/actions/workflow/status/percevalw/pysimstring/tests.yml?branch=master&label=tests&style=flat-square)
[![PyPI](https://img.shields.io/pypi/v/pysimstring?color=blue&style=flat-square)](https://pypi.org/project/pysimstring/)
[![License](https://img.shields.io/github/license/percevalw/pysimstring?color=x&style=flat-square)](https://github.com/percevalw/pysimstring/blob/master/LICENSE)

# SimString Python Package

Orginal version by [chokkan](https://github.com/chokkan/simstring) and [QuickUMLS](https://github.com/Georgetown-IR-Lab/simstring).

This version removes the [libiconv](https://www.gnu.org/software/libiconv/) dependency
which required a conda installation before installing the simstring package on Windows.

## How to use

Install it:

```bash
pip install pysimstring
```

and use it:

```python
import os
import tempfile
from pathlib import Path
from typing import Union

import pysimstring.simstring as simstring


class SimstringWriter:
    def __init__(self, path: Union[str, Path]):
        """
        A context class to write a simstring database

        Parameters
        ----------
        path: Union[str, Path]
            Path to database
        """
        os.makedirs(path, exist_ok=True)
        self.path = path

    def __enter__(self):
        path = os.path.join(self.path, "terms.simstring")
        self.db = simstring.writer(path, 3, False, True)
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.db.close()

    def insert(self, term):
        self.db.insert(term)


def test_simstring():
    terms = ["paracetamol", "doliprane"]
    path = tempfile.mkdtemp()

    with SimstringWriter(path) as ss_db:
        for term in terms:
            ss_db.insert("##" + term + "##")

    ss_reader = simstring.reader(os.path.join(path, "terms.simstring"))
    ss_reader.measure = getattr(simstring, "jaccard")
    ss_reader.threshold = 0.5

    assert ss_reader.retrieve("##paracetomol##") == ("##paracetamol##",)
    assert ss_reader.retrieve("##doliprano##") == ("##doliprane##",)
```
