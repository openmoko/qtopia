;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Copyright (C) 2007-2007 Trolltech ASA. All rights reserved.
;;
;; This file is part of the Windows installer of the Qt Toolkit.
;;
;; This file may be used under the terms of the GNU General Public
;; License version 2.0 as published by the Free Software Foundation
;; and appearing in the file LICENSE.GPL included in the packaging of
;; this file.  Please review the following information to ensure GNU
;; General Public Licensing requirements will be met:
;; http://trolltech.com/products/qt/licenses/licensing/opensource/
;;
;; If you are unsure which license is appropriate for your use, please
;; review the following information:
;; http://trolltech.com/products/qt/licenses/licensing/licensingoverview
;; or contact the sales department at sales@trolltech.com.
;;
;; In addition, as a special exception, Trolltech gives you certain
;; additional rights. These rights are described in the Trolltech GPL
;; Exception version 1.0, which can be found at
;; http://www.trolltech.com/products/qt/gplexception/ and in the file
;; GPL_EXCEPTION.txt in this package.
;;
;; In addition, as a special exception, Trolltech, as the sole copyright
;; holder for Qt Designer, grants users of the Qt/Eclipse Integration
;; plug-in the right for the Qt/Eclipse Integration to link to
;; functionality provided by Qt Designer and its related libraries.
;;
;; Trolltech reserves all rights not expressly granted herein.
;;
;; This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
;; WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
!ifdef MODULE_OPENSOURCE
!macro OPENSOURCE_INITIALIZE
  !define MODULE_OPENSOURCE_PAGE "opensource.ini"
  page custom ModuleOpenSourceShowPage
!macroend
!macro OPENSOURCE_SECTIONS
  Section -ModuleOpenSourceSection
    !ifdef MODULE_OPENSOURCE_ROOT
      SetOutPath "$INSTDIR"
      File "${MODULE_OPENSOURCE_ROOT}\OPENSOURCE-NOTICE.TXT"
    !endif
    CreateShortCut "$SMPROGRAMS\$STARTMENU_STRING\OpenSource Notice.lnk" "$INSTDIR\OPENSOURCE-NOTICE.TXT"
  SectionEnd

  Function ModuleOpenSourceShowPage
    !insertmacro MUI_HEADER_TEXT "Open Source Edition" " "
    !insertmacro MUI_INSTALLOPTIONS_DISPLAY "${MODULE_OPENSOURCE_PAGE}"
    strcpy "$LICENSEE" "Open Source"
    strcpy "$LICENSE_PRODUCT" "OpenSource"
  FunctionEnd
!macroend
!macro OPENSOURCE_DESCRIPTION
!macroend
!macro OPENSOURCE_STARTUP
  !insertmacro MUI_INSTALLOPTIONS_EXTRACT "${MODULE_OPENSOURCE_PAGE}"
!macroend
!macro OPENSOURCE_FINISH
!macroend
!macro OPENSOURCE_UNSTARTUP
!macroend
!macro OPENSOURCE_UNINSTALL
  Section -un.ModuleOpenSourceSection
    Delete "$SMPROGRAMS\$STARTMENU_STRING\OpenSource Notice.lnk"
  SectionEnd
!macroend
!macro OPENSOURCE_UNFINISH
!macroend
!else ;MODULE_OPENSOURCE
!macro OPENSOURCE_INITIALIZE
!macroend
!macro OPENSOURCE_SECTIONS
!macroend
!macro OPENSOURCE_DESCRIPTION
!macroend
!macro OPENSOURCE_STARTUP
!macroend
!macro OPENSOURCE_FINISH
!macroend
!macro OPENSOURCE_UNSTARTUP
!macroend
!macro OPENSOURCE_UNINSTALL
!macroend
!macro OPENSOURCE_UNFINISH
!macroend
!endif ;MODULE_OPENSOURCE

