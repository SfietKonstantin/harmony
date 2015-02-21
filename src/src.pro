TEMPLATE = subdirs
SUBDIRS = lib bin plugins tests

bin.depends = lib
plugins.depends = lib
tests.depends = lib plugins
