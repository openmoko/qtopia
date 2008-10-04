requires(contains(arch,arm))

TEMPLATE=app
CONFIG+=embedded
TARGET=logread

include(logread.pro)
