CONFIG+=depotProject

# A theme is almost the same as a stub but it now has i18n
keyword.theme.commands+=\
    "CONFIG*=stub no_target no_dest"\
    "CONFIG-=qt"\
    "CONFIG-=qtopia"\
    "CONFIG-=qtopiadesktop"
PROJECT_KEYWORDS+=theme

