TEMPLATE = subdirs
SUBDIRS = lib bin plugins tests

tests.depends = lib
bin.depends = lib
plugins.depends = lib
