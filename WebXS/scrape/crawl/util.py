#!/usr/bin/env python
import json
import types

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

    # non-configuration-related kwargs.
    # these should not be given to add_argument().
    # instead, they will be setattr()'d.
    non_configs = (
        'on_eval',
    )

    def __init__(self, name, **config_pack):
        """
        name: name of parameter, with any dashes (e.g. --debug).

        config_pack: packed **kwargs that can be unpacked inside an
        ArgumentParser's add_argument() method to specify optional arguments.
        If you want to put additional parameters when add_argument() is called,
        name them directly in this constructor and they will be handled
        properly.
            - some other attributes, not to be used in add_argument() may
              also be specified. If expected, these will be assigned as
              instance attributes.
        """
        self.name = name

        self._fill_from_proto(config_pack)
        self._remove_non_configs(config_pack)
        self.config_pack = config_pack

    def _fill_from_proto(self, pack):
        """
        Fill a config_pack's unspecified values with any defaults
        specified by proto_config.
        """
        for key, value in self.proto_config.iteritems():
            if key not in pack:
                pack[key] = value

        return pack

    def _remove_non_configs(self, pack):
        """
        Remove non-configuration-related entries from a config_pack.
        For such values, assign them as attributes instead.
        """
        value = None
        for entry in self.non_configs:
            if entry in pack:
                value = pack[entry]
                # if it's a function, should end up a bound method
                if isinstance(value, types.FunctionType):
                    value = types.MethodType(value, self)

                setattr(self, entry, value)
                del pack[entry]

        return pack

    def on_eval(self, crawler, value):
        """
        Callback for when a crawler evaluates this parameter.
        This should modify the crawler.

        crawler: crawler being initialized.

        value: value passed by user for this parameter.
        """
        pass


class NonSearchParam(Param):
    """ A non-search-related command-line parameter. """

    def on_eval(self, crawler, value):
        """
        Unless overridden, assign the name of the parameter as an instance
        attribute of the corresponding crawler, with the value given by
        the user.
        """
        setattr(crawler, self.name, value)


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
    

class Status(object):
    """
    Represents either success or failure,
    and an optional message detail.
    Essentially just a wrapper for a boolean and a string.
    """
    
    def __init__(self, success, message=''):
        """
        success: boolean
            - True: success
            - False: failure
        message: string detailing status
        """
        self.success = success
        self.message = message

    def __nonzero__(self):
        """
        Implements truth testing.
        Returns whether or not things are successful.
        """
        return self.success
