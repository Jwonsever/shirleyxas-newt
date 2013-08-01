#!/usr/bin/env python
import json

class JsonList(list):
    """ A list that can easily convert itself to JSON. """

    @property
    def json(self):
        """ Return a JSON representation of this list. """
        return json.dumps(self)


class Param(object):
    """ A command-line parameter. """

    # prototype config_pack. Defines desired default values.
    # If an attribute is defined here but not in the given
    # config_pack, the incoming one will be filled with the
    # default attribute.
    proto_config = {
        'default': ''
    }

    def __init__(self, name, **config_pack):
        """
        name: name of parameter, with any dashes (e.g. --debug).

        config_pack: packed **kwargs that can be unpacked inside an
        ArgumentParser's add_argument() method to specify optional arguments.
        If you want to put additional parameters when add_argument() is called,
        name them directly in this constructor and they will be handled
        properly.
        """
        self.name = name
        self.config_pack = self.fill_from_proto(config_pack)

    def fill_from_proto(self, pack):
        """
        Fill a config_pack's unspecified values with any defaults
        specified by proto_config.
        """
        for key, value in self.proto_config.iteritems():
            if key not in pack:
                pack[key] = value

        return pack


class SearchParam(Param):
    """ A search-related command-line parameter. """

    def __init__(self, name, input_name, **config_pack):
        """
        name: name of parameter, with any dashes (e.g. --debug).

        input_name: name attribute of <input> element corresponding
        to this parameter.

        config_pack: packed **kwargs that can be unpacked inside an
        ArgumentParser's add_argument() method to specify optional arguments.
        If you want to put additional parameters when add_argument() is called,
        name them directly in this constructor and they will be handled
        properly.
        """
        super(SearchParam, self).__init__(name, **config_pack)
        self.input_name = input_name


class ParamList(list):
    """ A list of Params. """

    def __init__(self, *elems):
        """
        NOTE:
        For some reason, the list() constructor appears to complain when not
        fed an iterable. Perhaps it does not do unpacking properly?
        For this reason, this constructor packs its arguments into an
        iterable and feeds this iterable to the list constructor.
        """
        list.__init__(self, elems)

    def hasParam(self, name):
        """
        Whether or not this list has a Param with the given name attribute.
        """
        #print len(self)
        for param in self:
            #print param.name, name
            if self.names_match(param.name, name):
                return True

        return False

    def getParam(self, name):
        """
        Get the Param with the given name attribute.
        """
        for param in self:
            #print param.name
            if self.names_match(param.name, name):
                return param

        # not found
        raise KeyError('Param with name "{0}" not found'.format(name))

    def names_match(self, name1, name2):
        """
        Param name attributes sometimes have leading dashes.
        any leading dashes on both the target names
        will be ignored when comparing.
        """
        name1 = self.strip_dashes(name1)
        name2 = self.strip_dashes(name2)
        return name1 == name2

    def strip_dashes(self, name):
        """
        Remove leading dashes from a string.
        """
        return name.lstrip('-')
