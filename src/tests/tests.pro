TEMPLATE = subdirs

SUBDIRS += \
    harmonyrunner \
    lib \
    unit

unit.depends = lib
