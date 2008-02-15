Section "Greenphone USB driver"
    SetOverwrite on
    SetOutPath "$INSTDIR\drivers\greenphone"
    File "$${QPEDIR}\src\qtopiadesktop\dist\extras\greenphone_driver\usbser.inf"
    File "$${QPEDIR}\src\qtopiadesktop\dist\extras\greenphone_driver\Usbser.sys"
    File "$${QPEDIR}\src\qtopiadesktop\dist\extras\greenphone_driver\DPInst.exe"
    File "$${QPEDIR}\src\qtopiadesktop\dist\extras\greenphone_driver\dpinst.xml"
    ExecWait '"$INSTDIR\drivers\greenphone\DPInst.exe"'
SectionEnd

Section un.GreenphoneUSBdriver
    ExecWait '"$INSTDIR\drivers\greenphone\DPInst.exe" /Q /U "$INSTDIR\drivers\greenphone\usbser.inf"'
    Delete "$INSTDIR\drivers\greenphone\usbser.inf"
    Delete "$INSTDIR\drivers\greenphone\Usbser.sys"
    Delete "$INSTDIR\drivers\greenphone\DPInst.exe"
    Delete "$INSTDIR\drivers\greenphone\dpinst.xml"
    RMDir "$INSTDIR\drivers\greenphone"
    RMDir "$INSTDIR\drivers"
SectionEnd

