// All feature and their dependencies
//
// This list is generated from $QTDIR/src/tools/qfeatures.txt
//
// Keyboard accelerators and shortcuts
//#define QT_NO_ACCEL

// Asynchronous image I/O
//#define QT_NO_ASYNC_IMAGE_IO

// Asynchronous I/O
//#define QT_NO_ASYNC_IO

// Buttons
//#define QT_NO_BUTTON

// Named colors
//#define QT_NO_COLORNAMES

// Cursors
//#define QT_NO_CURSOR

// QDataStream
//#define QT_NO_DATASTREAM

// Dialogs
//#define QT_NO_DIALOG

// Special widget effects (fading, scrolling)
//#define QT_NO_EFFECTS

// Framed widgets
//#define QT_NO_FRAME

// Freetype font engine
//#define QT_NO_FREETYPE

// JPEG image I/O
//#define QT_NO_IMAGEIO_JPEG

// MNG image I/O
//#define QT_NO_IMAGEIO_MNG

// PNG image I/O
//#define QT_NO_IMAGEIO_PNG

// PPM image I/O
//#define QT_NO_IMAGEIO_PPM

// XBM image I/O
//#define QT_NO_IMAGEIO_XBM

// Smooth QImage scaling
//#define QT_NO_IMAGE_SMOOTHSCALE

// TrueColor QImage
//#define QT_NO_IMAGE_TRUECOLOR

// Automatic widget layout
//#define QT_NO_LAYOUT

// Networking
//#define QT_NO_NETWORK

// Palettes
//#define QT_NO_PALETTE

// External process invocation.
//#define QT_NO_PROCESS

// Alpha-blended cursor
//#define QT_NO_QWS_ALPHA_CURSOR

// 1-bit monochrome
//#define QT_NO_QWS_DEPTH_1

// 15 or 16-bit color
//#define QT_NO_QWS_DEPTH_16

// 24-bit color
//#define QT_NO_QWS_DEPTH_24

// 32-bit color
//#define QT_NO_QWS_DEPTH_32

// 4-bit greyscale
//#define QT_NO_QWS_DEPTH_4

// 8-bit color
//#define QT_NO_QWS_DEPTH_8

// 8-bit grayscale
//#define QT_NO_QWS_DEPTH_8GRAYSCALE

// Favour code size over graphics speed
//#define QT_NO_QWS_GFX_SPEED

// Console keyboard
//#define QT_NO_QWS_KEYBOARD

// Mach64 acceleration
//#define QT_NO_QWS_MACH64

// Window Manager
//#define QT_NO_QWS_MANAGER

// Matrox MGA acceleration
//#define QT_NO_QWS_MATROX

// Qt/Embedded window system properties.
//#define QT_NO_QWS_PROPERTIES

// Saving of fonts
//#define QT_NO_QWS_SAVEFONTS

// SVGA
//#define QT_NO_QWS_SVGALIB

// Transformed frame buffer
//#define QT_NO_QWS_TRANSFORMED

// Virtual frame buffer
//#define QT_NO_QWS_VFB

// 4-bit VGA
//#define QT_NO_QWS_VGA_16

// Voodoo3 acceleration
//#define QT_NO_QWS_VOODOO3

// Range-control widgets
//#define QT_NO_RANGECONTROL

// Semi-modal dialogs
//#define QT_NO_SEMIMODAL

// Playing sounds
//#define QT_NO_SOUND

// QStringList
//#define QT_NO_STRINGLIST

// Character set conversions
//#define QT_NO_TEXTCODEC

// QTextStream
//#define QT_NO_TEXTSTREAM

// Unicode property tables
//#define QT_NO_UNICODETABLES

// Input validators
//#define QT_NO_VALIDATOR

// QWMatrix
//#define QT_NO_WMATRIX

// Non-Unicode text conversions
#if !defined(QT_NO_CODECS) && (defined(QT_NO_TEXTCODEC))
#define QT_NO_CODECS
#endif

// QCop IPC
#if !defined(QT_NO_COP) && (defined(QT_NO_DATASTREAM))
#define QT_NO_COP
#endif

// QSnoop
#if !defined(QT_NO_SNOOP) && (defined(QT_NO_COP))
#define QT_NO_SNOOP
#endif


// QDir
#if !defined(QT_NO_DIR) && (defined(QT_NO_STRINGLIST))
#define QT_NO_DIR
#endif

// Drawing utility functions
#if !defined(QT_NO_DRAWUTIL) && (defined(QT_NO_PALETTE))
#define QT_NO_DRAWUTIL
#endif

// QFontDatabase
#if !defined(QT_NO_FONTDATABASE) && (defined(QT_NO_STRINGLIST))
#define QT_NO_FONTDATABASE
#endif

// BMP image I/O
#if !defined(QT_NO_IMAGEIO_BMP) && (defined(QT_NO_DATASTREAM))
#define QT_NO_IMAGEIO_BMP
#endif

// XPM image I/O
#if !defined(QT_NO_IMAGEIO_XPM) && (defined(QT_NO_TEXTSTREAM))
#define QT_NO_IMAGEIO_XPM
#endif

// 16-bit QImage
#if !defined(QT_NO_IMAGE_16_BIT) && (defined(QT_NO_IMAGE_TRUECOLOR))
#define QT_NO_IMAGE_16_BIT
#endif

// Image file text strings
#if !defined(QT_NO_IMAGE_TEXT) && (defined(QT_NO_STRINGLIST))
#define QT_NO_IMAGE_TEXT
#endif

// Main-windows
#if !defined(QT_NO_MAINWINDOW) && (defined(QT_NO_LAYOUT))
#define QT_NO_MAINWINDOW
#endif

// QPicture
#if !defined(QT_NO_PICTURE) && (defined(QT_NO_DATASTREAM))
#define QT_NO_PICTURE
#endif

// Pixmap transformations
#if !defined(QT_NO_PIXMAP_TRANSFORMATION) && (defined(QT_NO_WMATRIX))
#define QT_NO_PIXMAP_TRANSFORMATION
#endif

// Visible cursor
#if !defined(QT_NO_QWS_CURSOR) && (defined(QT_NO_CURSOR))
#define QT_NO_QWS_CURSOR
#endif

// In-place Input Methods
#if !defined(QT_NO_QWS_IM) && (defined(QT_NO_DATASTREAM))
#define QT_NO_QWS_IM
#endif

// Multi-process architecture
#if !defined(QT_NO_QWS_MULTIPROCESS) && (defined(QT_NO_NETWORK))
#define QT_NO_QWS_MULTIPROCESS
#endif

// Remote frame buffer (VNC)
#if !defined(QT_NO_QWS_VNC) && (defined(QT_NO_NETWORK))
#define QT_NO_QWS_VNC
#endif

// Session management
#if !defined(QT_NO_SESSIONMANAGER) && (defined(QT_NO_STRINGLIST))
#define QT_NO_SESSIONMANAGER
#endif

// QSizeGrip
#if !defined(QT_NO_SIZEGRIP) && (defined(QT_NO_PALETTE))
#define QT_NO_SIZEGRIP
#endif

// Translations via QObject::tr()
#if !defined(QT_NO_TRANSLATION) && (defined(QT_NO_DATASTREAM))
#define QT_NO_TRANSLATION
#endif

// Widget stacks
#if !defined(QT_NO_WIDGETSTACK) && (defined(QT_NO_FRAME))
#define QT_NO_WIDGETSTACK
#endif

// BDF font files
#if !defined(QT_NO_BDF) && (defined(QT_NO_TEXTSTREAM) || defined(QT_NO_STRINGLIST))
#define QT_NO_BDF
#endif

// Big5 Codec
#if !defined(QT_NO_BIG5CODEC) && (defined(QT_NO_CODECS))
#define QT_NO_BIG5CODEC
#endif

// Character map loaded codec
#if !defined(QT_NO_CODEC_CHARMAP) && (defined(QT_NO_CODECS))
#define QT_NO_CODEC_CHARMAP
#endif

// EUCKR Codec
#if !defined(QT_NO_EUCKRCODEC) && (defined(QT_NO_CODECS))
#define QT_NO_EUCKRCODEC
#endif

// GBK Codec
#if !defined(QT_NO_GBKCODEC) && (defined(QT_NO_CODECS))
#define QT_NO_GBKCODEC
#endif

// Grid layout widgets
#if !defined(QT_NO_GRID) && (defined(QT_NO_FRAME) || defined(QT_NO_LAYOUT))
#define QT_NO_GRID
#endif

// Horizonal box layout widgets
#if !defined(QT_NO_HBOX) && (defined(QT_NO_FRAME) || defined(QT_NO_LAYOUT))
#define QT_NO_HBOX
#endif

// QIconSet
#if !defined(QT_NO_ICONSET) && (defined(QT_NO_IMAGE_SMOOTHSCALE) || defined(QT_NO_PALETTE))
#define QT_NO_ICONSET
#endif

// JP Unicode tables
#if !defined(QT_NO_JPUNICODE) && (defined(QT_NO_CODECS))
#define QT_NO_JPUNICODE
#endif

// QLCDNumber
#if !defined(QT_NO_LCDNUMBER) && (defined(QT_NO_FRAME) || defined(QT_NO_PALETTE))
#define QT_NO_LCDNUMBER
#endif

// MIME
#if !defined(QT_NO_MIME) && (defined(QT_NO_DIR))
#define QT_NO_MIME
#endif

// Animated images
#if !defined(QT_NO_MOVIE) && (defined(QT_NO_ASYNC_IO) || defined(QT_NO_ASYNC_IMAGE_IO))
#define QT_NO_MOVIE
#endif

// Printing
#if !defined(QT_NO_PRINTER) && (defined(QT_NO_TEXTSTREAM) || defined(QT_NO_STRINGLIST))
#define QT_NO_PRINTER
#endif

// Full Screen pointer based Input Methods
#if !defined(QT_NO_QWS_FSIM) && (defined(QT_NO_QWS_IM))
#define QT_NO_QWS_FSIM
#endif

// Arabic and Hebrew Codecs
#if !defined(QT_NO_RTLCODEC) && (defined(QT_NO_CODECS))
#define QT_NO_RTLCODEC
#endif

// Some simple 8-bit codecs
#if !defined(QT_NO_SIMPLE_CODECS) && (defined(QT_NO_CODECS))
#define QT_NO_SIMPLE_CODECS
#endif

// QStyle
#if !defined(QT_NO_STYLE) && (defined(QT_NO_DRAWUTIL))
#define QT_NO_STYLE
#endif

// Scaling and rotation
#if !defined(QT_NO_TRANSFORMATIONS) && (defined(QT_NO_PIXMAP_TRANSFORMATION))
#define QT_NO_TRANSFORMATIONS
#endif

// TSCII Codec
#if !defined(QT_NO_TSCIICODEC) && (defined(QT_NO_CODECS))
#define QT_NO_TSCIICODEC
#endif

// QVariant
#if !defined(QT_NO_VARIANT) && (defined(QT_NO_STRINGLIST) || defined(QT_NO_DATASTREAM))
#define QT_NO_VARIANT
#endif

// DNS
#if !defined(QT_NO_DNS) && (defined(QT_NO_NETWORK) || defined(QT_NO_STRINGLIST) || defined(QT_NO_TEXTSTREAM))
#define QT_NO_DNS
#endif

// EUCJP Codec
#if !defined(QT_NO_EUCJPCODEC) && (defined(QT_NO_JPUNICODE))
#define QT_NO_EUCJPCODEC
#endif

// JIS Codec
#if !defined(QT_NO_JISCODEC) && (defined(QT_NO_JPUNICODE))
#define QT_NO_JISCODEC
#endif

// Single-line edits
#if !defined(QT_NO_LINEEDIT) && (defined(QT_NO_STYLE))
#define QT_NO_LINEEDIT
#endif

// Menu-oriented widgets
#if !defined(QT_NO_MENUDATA) && (defined(QT_NO_ICONSET))
#define QT_NO_MENUDATA
#endif

// Network file access
#if !defined(QT_NO_NETWORKPROTOCOL) && (defined(QT_NO_NETWORK) || defined(QT_NO_DIR))
#define QT_NO_NETWORKPROTOCOL
#endif

// The "BeOS" style
#if !defined(QT_NO_QWS_BEOS_WM_STYLE) && (defined(QT_NO_QWS_MANAGER) || defined(QT_NO_IMAGEIO_XPM))
#define QT_NO_QWS_BEOS_WM_STYLE
#endif

// The "Hydro" style
#if !defined(QT_NO_QWS_HYDRO_WM_STYLE) && (defined(QT_NO_QWS_MANAGER) || defined(QT_NO_IMAGEIO_XPM))
#define QT_NO_QWS_HYDRO_WM_STYLE
#endif

// The "KDE2" style
#if !defined(QT_NO_QWS_KDE2_WM_STYLE) && (defined(QT_NO_QWS_MANAGER) || defined(QT_NO_IMAGEIO_XPM))
#define QT_NO_QWS_KDE2_WM_STYLE
#endif

// The "KDE" style
#if !defined(QT_NO_QWS_KDE_WM_STYLE) && (defined(QT_NO_QWS_MANAGER) || defined(QT_NO_IMAGEIO_XPM))
#define QT_NO_QWS_KDE_WM_STYLE
#endif

// The "Windows" style
#if !defined(QT_NO_QWS_WINDOWS_WM_STYLE) && (defined(QT_NO_QWS_MANAGER) || defined(QT_NO_IMAGEIO_XPM))
#define QT_NO_QWS_WINDOWS_WM_STYLE
#endif

// SJIS Codec
#if !defined(QT_NO_SJISCODEC) && (defined(QT_NO_JPUNICODE))
#define QT_NO_SJISCODEC
#endif

// Playing sounds
#if !defined(QT_NO_SOUNDSERVER) && (defined(QT_NO_DIR) || defined(QT_NO_TEXTCODEC))
#define QT_NO_SOUNDSERVER
#endif

// Status bars
#if !defined(QT_NO_STATUSBAR) && (defined(QT_NO_LAYOUT) || defined(QT_NO_DRAWUTIL))
#define QT_NO_STATUSBAR
#endif

// Windows style
#if !defined(QT_NO_STYLE_WINDOWS) && (defined(QT_NO_STYLE))
#define QT_NO_STYLE_WINDOWS
#endif

// Vertical box layout widgets
#if !defined(QT_NO_VBOX) && (defined(QT_NO_HBOX))
#define QT_NO_VBOX
#endif

// XML
#if !defined(QT_NO_XML) && (defined(QT_NO_STRINGLIST) || defined(QT_NO_TEXTSTREAM) || defined(QT_NO_TEXTCODEC))
#define QT_NO_XML
#endif

// Check-boxes
#if !defined(QT_NO_CHECKBOX) && (defined(QT_NO_BUTTON) || defined(QT_NO_STYLE))
#define QT_NO_CHECKBOX
#endif

// Cut and paste
#if !defined(QT_NO_CLIPBOARD) && (defined(QT_NO_QWS_PROPERTIES) || defined(QT_NO_MIME))
#define QT_NO_CLIPBOARD
#endif

// Dials
#if !defined(QT_NO_DIAL) && (defined(QT_NO_RANGECONTROL) || defined(QT_NO_STYLE))
#define QT_NO_DIAL
#endif

// Drag and drop
#if !defined(QT_NO_DRAGANDDROP) && (defined(QT_NO_QWS_PROPERTIES) || defined(QT_NO_MIME))
#define QT_NO_DRAGANDDROP
#endif

// QLabel
#if !defined(QT_NO_LABEL) && (defined(QT_NO_FRAME) || defined(QT_NO_STYLE))
#define QT_NO_LABEL
#endif

// Progress bars
#if !defined(QT_NO_PROGRESSBAR) && (defined(QT_NO_FRAME) || defined(QT_NO_STYLE))
#define QT_NO_PROGRESSBAR
#endif

// Radio-buttons
#if !defined(QT_NO_RADIOBUTTON) && (defined(QT_NO_BUTTON) || defined(QT_NO_STYLE))
#define QT_NO_RADIOBUTTON
#endif

// Scroll bars
#if !defined(QT_NO_SCROLLBAR) && (defined(QT_NO_RANGECONTROL) || defined(QT_NO_STYLE))
#define QT_NO_SCROLLBAR
#endif

// Sliders
#if !defined(QT_NO_SLIDER) && (defined(QT_NO_RANGECONTROL) || defined(QT_NO_STYLE))
#define QT_NO_SLIDER
#endif

// Compact Windows style
#if !defined(QT_NO_STYLE_COMPACT) && (defined(QT_NO_STYLE_WINDOWS))
#define QT_NO_STYLE_COMPACT
#endif

// Group boxes
#if !defined(QT_NO_GROUPBOX) && (defined(QT_NO_FRAME) || defined(QT_NO_STYLE) || defined(QT_NO_LAYOUT))
#define QT_NO_GROUPBOX
#endif

// Cut and paste non-text
#if !defined(QT_NO_MIMECLIPBOARD) && (defined(QT_NO_CLIPBOARD))
#define QT_NO_MIMECLIPBOARD
#endif

// Splitters
#if !defined(QT_NO_SPLITTER) && (defined(QT_NO_FRAME) || defined(QT_NO_STYLE) || defined(QT_NO_LAYOUT))
#define QT_NO_SPLITTER
#endif

// Tool tips
#if !defined(QT_NO_TOOLTIP) && (defined(QT_NO_LABEL))
#define QT_NO_TOOLTIP
#endif

// QHeader
#if !defined(QT_NO_HEADER) && (defined(QT_NO_ICONSET) || defined(QT_NO_STYLE))
#define QT_NO_HEADER
#endif

// Horizontal group boxes
#if !defined(QT_NO_HGROUPBOX) && (defined(QT_NO_GROUPBOX))
#define QT_NO_HGROUPBOX
#endif

// Properties
#if !defined(QT_NO_PROPERTIES) && (defined(QT_NO_ICONSET) || defined(QT_NO_VARIANT))
#define QT_NO_PROPERTIES
#endif

// Scrollable view widgets
#if !defined(QT_NO_SCROLLVIEW) && (defined(QT_NO_SCROLLBAR) || defined(QT_NO_FRAME))
#define QT_NO_SCROLLVIEW
#endif

// Tab-bars
#if !defined(QT_NO_TABBAR) && (defined(QT_NO_ICONSET) || defined(QT_NO_STYLE))
#define QT_NO_TABBAR
#endif

// Table-like widgets
#if !defined(QT_NO_TABLEVIEW) && (defined(QT_NO_SCROLLBAR) || defined(QT_NO_FRAME))
#define QT_NO_TABLEVIEW
#endif

// Button groups
#if !defined(QT_NO_BUTTONGROUP) && (defined(QT_NO_GROUPBOX) || defined(QT_NO_BUTTON))
#define QT_NO_BUTTONGROUP
#endif

// QCanvas
#if !defined(QT_NO_CANVAS) && (defined(QT_NO_SCROLLVIEW))
#define QT_NO_CANVAS
#endif

// Document Object Model
#if !defined(QT_NO_DOM) && (defined(QT_NO_XML) || defined(QT_NO_MIME))
#define QT_NO_DOM
#endif

// QIconView
#if !defined(QT_NO_ICONVIEW) && (defined(QT_NO_SCROLLVIEW))
#define QT_NO_ICONVIEW
#endif

// SQL classes
#if !defined(QT_NO_SQL) && (defined(QT_NO_PROPERTIES))
#define QT_NO_SQL
#endif

// Tool-buttons
#if !defined(QT_NO_TOOLBUTTON) && (defined(QT_NO_BUTTON) || defined(QT_NO_ICONSET) || defined(QT_NO_STYLE))
#define QT_NO_TOOLBUTTON
#endif

// Vertical group boxes
#if !defined(QT_NO_VGROUPBOX) && (defined(QT_NO_HGROUPBOX))
#define QT_NO_VGROUPBOX
#endif

// Horizontal button groups
#if !defined(QT_NO_HBUTTONGROUP) && (defined(QT_NO_BUTTONGROUP))
#define QT_NO_HBUTTONGROUP
#endif

// QListBox
#if !defined(QT_NO_LISTBOX) && (defined(QT_NO_SCROLLVIEW) || defined(QT_NO_STRINGLIST))
#define QT_NO_LISTBOX
#endif

// Menu bars
#if !defined(QT_NO_MENUBAR) && (defined(QT_NO_MENUDATA) || defined(QT_NO_STYLE) || defined(QT_NO_FRAME))
#define QT_NO_MENUBAR
#endif

// FTP file access
#if !defined(QT_NO_NETWORKPROTOCOL_FTP) && (defined(QT_NO_DNS) || defined(QT_NO_NETWORKPROTOCOL))
#define QT_NO_NETWORKPROTOCOL_FTP
#endif

// HTTP file access
#if !defined(QT_NO_NETWORKPROTOCOL_HTTP) && (defined(QT_NO_DNS) || defined(QT_NO_NETWORKPROTOCOL))
#define QT_NO_NETWORKPROTOCOL_HTTP
#endif

// Popup-menus
#if !defined(QT_NO_POPUPMENU) && (defined(QT_NO_MENUDATA) || defined(QT_NO_STYLE) || defined(QT_NO_FRAME))
#define QT_NO_POPUPMENU
#endif

// RichText (HTML) display
#if !defined(QT_NO_RICHTEXT) && (defined(QT_NO_MIME) || defined(QT_NO_TEXTSTREAM) || defined(QT_NO_DRAWUTIL) || defined(QT_NO_IMAGE_SMOOTHSCALE) || defined(QT_NO_LAYOUT))
#define QT_NO_RICHTEXT
#endif

// Tab widgets
#if !defined(QT_NO_TABWIDGET) && (defined(QT_NO_TABBAR) || defined(QT_NO_WIDGETSTACK))
#define QT_NO_TABWIDGET
#endif

// Vertical button groups
#if !defined(QT_NO_VBUTTONGROUP) && (defined(QT_NO_HBUTTONGROUP))
#define QT_NO_VBUTTONGROUP
#endif

// Push-buttons
#if !defined(QT_NO_PUSHBUTTON) && (defined(QT_NO_BUTTON) || defined(QT_NO_POPUPMENU))
#define QT_NO_PUSHBUTTON
#endif

// QListView
#if !defined(QT_NO_LISTVIEW) && (defined(QT_NO_HEADER) || defined(QT_NO_SCROLLVIEW))
#define QT_NO_LISTVIEW
#endif

// "What's this" help
#if !defined(QT_NO_WHATSTHIS) && (defined(QT_NO_TOOLTIP) || defined(QT_NO_TOOLBUTTON))
#define QT_NO_WHATSTHIS
#endif

// Multi-line edits
#if !defined(QT_NO_MULTILINEEDIT) && (defined(QT_NO_TABLEVIEW) || defined(QT_NO_POPUPMENU))
#define QT_NO_MULTILINEEDIT
#endif

// QTextView
#if !defined(QT_NO_TEXTVIEW) && (defined(QT_NO_RICHTEXT) || defined(QT_NO_SCROLLVIEW))
#define QT_NO_TEXTVIEW
#endif

// QAction
#if !defined(QT_NO_ACTION) && (defined(QT_NO_TOOLBUTTON) || defined(QT_NO_POPUPMENU))
#define QT_NO_ACTION
#endif

// QMessageBox
#if !defined(QT_NO_MESSAGEBOX) && (defined(QT_NO_DIALOG) || defined(QT_NO_PUSHBUTTON) || defined(QT_NO_LABEL))
#define QT_NO_MESSAGEBOX
#endif

// Spin boxes
#if !defined(QT_NO_SPINBOX) && (defined(QT_NO_RANGECONTROL) || defined(QT_NO_PUSHBUTTON) || defined(QT_NO_VALIDATOR) || defined(QT_NO_LINEEDIT))
#define QT_NO_SPINBOX
#endif

// QTextBrowser
#if !defined(QT_NO_TEXTBROWSER) && (defined(QT_NO_TEXTVIEW))
#define QT_NO_TEXTBROWSER
#endif

// QTable
#if !defined(QT_NO_TABLE) && (defined(QT_NO_HEADER) || defined(QT_NO_SCROLLVIEW) || defined(QT_NO_LINEEDIT))
#define QT_NO_TABLE
#endif

// Toolbars
#if !defined(QT_NO_TOOLBAR) && (defined(QT_NO_POPUPMENU) || defined(QT_NO_TOOLBUTTON) || defined(QT_NO_MAINWINDOW))
#define QT_NO_TOOLBAR
#endif

// Platinum style
#if !defined(QT_NO_STYLE_PLATINUM) && (defined(QT_NO_STYLE_WINDOWS) || defined(QT_NO_PUSHBUTTON) || defined(QT_NO_SCROLLBAR))
#define QT_NO_STYLE_PLATINUM
#endif

// QWizard
#if !defined(QT_NO_WIZARD) && (defined(QT_NO_DIALOG) || defined(QT_NO_PUSHBUTTON) || defined(QT_NO_LAYOUT) || defined(QT_NO_LABEL) || defined(QT_NO_ACCEL) || defined(QT_NO_WIDGETSTACK))
#define QT_NO_WIZARD
#endif

// QComboBox
#if !defined(QT_NO_COMBOBOX) && (defined(QT_NO_LISTBOX) || defined(QT_NO_POPUPMENU) || defined(QT_NO_LINEEDIT))
#define QT_NO_COMBOBOX
#endif

// QProgressDialog
#if !defined(QT_NO_PROGRESSDIALOG) && (defined(QT_NO_SEMIMODAL) || defined(QT_NO_PUSHBUTTON) || defined(QT_NO_LABEL) || defined(QT_NO_ACCEL) || defined(QT_NO_PROGRESSBAR))
#define QT_NO_PROGRESSDIALOG
#endif

// QTabDialog
#if !defined(QT_NO_TABDIALOG) && (defined(QT_NO_DIALOG) || defined(QT_NO_TABWIDGET) || defined(QT_NO_PUSHBUTTON) || defined(QT_NO_LAYOUT))
#define QT_NO_TABDIALOG
#endif

// Motif style
#if !defined(QT_NO_STYLE_MOTIF) && (defined(QT_NO_TRANSFORMATIONS) || defined(QT_NO_TABBAR) || defined(QT_NO_PUSHBUTTON) || defined(QT_NO_SCROLLBAR))
#define QT_NO_STYLE_MOTIF
#endif

// CDE style
#if !defined(QT_NO_STYLE_CDE) && (defined(QT_NO_STYLE_MOTIF))
#define QT_NO_STYLE_CDE
#endif

// Interlace-friendly style
#if !defined(QT_NO_STYLE_INTERLACE) && (defined(QT_NO_STYLE_MOTIF))
#define QT_NO_STYLE_INTERLACE
#endif

// Motif-plus style
#if !defined(QT_NO_STYLE_MOTIFPLUS) && (defined(QT_NO_STYLE_MOTIF))
#define QT_NO_STYLE_MOTIFPLUS
#endif

// SGI style
#if !defined(QT_NO_STYLE_SGI) && (defined(QT_NO_STYLE_MOTIF))
#define QT_NO_STYLE_SGI
#endif

// QIconView in-place renaming
#if !defined(QT_NO_ICONVIEW_RENAME) && (defined(QT_NO_ICONVIEW) || defined(QT_NO_MULTILINEEDIT) || defined(QT_NO_VBOX))
#define QT_NO_ICONVIEW_RENAME
#endif

// QColorDialog
#if !defined(QT_NO_COLORDIALOG) && (defined(QT_NO_LAYOUT) || defined(QT_NO_LABEL) || defined(QT_NO_PUSHBUTTON) || defined(QT_NO_DIALOG) || defined(QT_NO_TABLEVIEW) || defined(QT_NO_VALIDATOR) || defined(QT_NO_LINEEDIT))
#define QT_NO_COLORDIALOG
#endif

// QWorkSpace
#if !defined(QT_NO_WORKSPACE) && (defined(QT_NO_POPUPMENU) || defined(QT_NO_MENUBAR) || defined(QT_NO_TOOLBUTTON) || defined(QT_NO_LABEL) || defined(QT_NO_ACCEL) || defined(QT_NO_VBOX))
#define QT_NO_WORKSPACE
#endif

// QInputDialog
#if !defined(QT_NO_INPUTDIALOG) && (defined(QT_NO_DIALOG) || defined(QT_NO_COMBOBOX) || defined(QT_NO_LABEL) || defined(QT_NO_LAYOUT) || defined(QT_NO_SPINBOX) || defined(QT_NO_WIDGETSTACK))
#define QT_NO_INPUTDIALOG
#endif

// QFontDialog
#if !defined(QT_NO_FONTDIALOG) && (defined(QT_NO_DIALOG) || defined(QT_NO_FONTDATABASE) || defined(QT_NO_COMBOBOX) || defined(QT_NO_PUSHBUTTON) || defined(QT_NO_LABEL) || defined(QT_NO_CHECKBOX) || defined(QT_NO_VGROUPBOX))
#define QT_NO_FONTDIALOG
#endif

// QPrintDialog
#if !defined(QT_NO_PRINTDIALOG) && (defined(QT_NO_DIALOG) || defined(QT_NO_LISTVIEW) || defined(QT_NO_PRINTER) || defined(QT_NO_COMBOBOX) || defined(QT_NO_DIR) || defined(QT_NO_LABEL) || defined(QT_NO_SPINBOX) || defined(QT_NO_RADIOBUTTON) || defined(QT_NO_BUTTONGROUP))
#define QT_NO_PRINTDIALOG
#endif

// QFileDialog
#if !defined(QT_NO_FILEDIALOG) && (defined(QT_NO_LISTVIEW) || defined(QT_NO_NETWORKPROTOCOL) || defined(QT_NO_COMBOBOX) || defined(QT_NO_MESSAGEBOX) || defined(QT_NO_SEMIMODAL) || defined(QT_NO_TOOLBUTTON) || defined(QT_NO_BUTTONGROUP) || defined(QT_NO_VBOX) || defined(QT_NO_PROGRESSBAR) || defined(QT_NO_SPLITTER) || defined(QT_NO_WIDGETSTACK))
#define QT_NO_FILEDIALOG
#endif

