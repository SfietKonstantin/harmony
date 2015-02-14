TEMPLATE = subdirs

SUBDIRS += \
    harmonyrunner \
    lib \
    tst_identificationservice \
    tst_nodemanager \
    tst_certificatemanager \
    tst_harmonynodebridge \
    tst_harmonyextension

tst_harmonyextension.depends = lib
