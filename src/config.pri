CONFIG += c++11
CONFIG(harmonydebug) {
    DEFINES += HARMONY_DEBUG
    QMAKE_CXXFLAGS += -fprofile-arcs -ftest-coverage
    QMAKE_LFLAGS_DEBUG += -lgcov -coverage
}

