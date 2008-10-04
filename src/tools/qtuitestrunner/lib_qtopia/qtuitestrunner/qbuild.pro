TEMPLATE=lib
CONFIG+=qt
TARGET=qtuitestrunner

MODULE_NAME=qtuitestrunner
QT+=network testlib

MODULES*=qtopiabase::headers
MODULES*=qtopia::headers

SOURCEPATH=..
SOURCEPATH+=../../lib_qt
SOURCEPATH+=/src/libraries/qtuitest
SOURCEPATH+=/src/libraries/qtopia

include(../lib_qtopia.pro)

<script>
// Set up the TESTS_SHARED_DIR define to allow finding the shared scripts directory
var srcdir = qbuild.invoke("path", "/", "existing");
project.property("TESTS_SHARED_DIR").setValue(srcdir + "/tests/shared");
</script>
DEFINES*=TESTS_SHARED_DIR=$$define_string($$TESTS_SHARED_DIR)


