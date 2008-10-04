TEMPLATE=app
CONFIG+=qt
TARGET=pngscale

# Yes, this doesn't look right but pngscale can also convert .svg files into .png format
QT*=svg

include(pngscale.pro)

