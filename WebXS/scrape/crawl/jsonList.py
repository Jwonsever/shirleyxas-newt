#!/usr/bin/env python
import json

class JsonList(list):
    """ A list that can easily convert itself to JSON. """

    @property
    def json(self):
        """ Return a JSON representation of this list. """
        return json.dumps(self)
