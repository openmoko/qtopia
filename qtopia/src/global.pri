# This file is -appended- to every .pro by $QPEDIR/src/config.pri

!contains( PROCESSED_PRI, global.pri ) {
    PROCESSED_PRI += global.pri

    include($${QTOPIA_DEPOT_PATH}/src/projects.pri)
    ALL_PROJECTS=$${LIBRARY_PROJECTS} $${APP_PROJECTS} $${PLUGIN_PROJECTS} $${SERVER} $${THEME_PROJECTS}

    !contains(TEMPLATE,subdirs) {
	qtopiaapp:CONFIG+=qtopia
	qtopia:CONFIG+=qtopialib qtopiainc

        # If the QTOPIA_COMPAT_VER is not specified, do what we used to do
        isEmpty(QTOPIA_COMPAT_VER) {
            QTOPIA_COMPAT_VER=2
        } else {
            QTOPIA_PHONE {
                message=QTOPIA_COMPAT_VER has no meaning in Qtopia Phone Edition.
                message($$message)
                QTOPIA_COMPAT_VER=2
            }
        }
        !equals(QTOPIA_COMPAT_VER,1.5):!equals(QTOPIA_COMPAT_VER,1.7):!equals(QTOPIA_COMPAT_VER,2) {
            message=The QTOPIA_COMPAT_VER you specified ($$QTOPIA_COMPAT_VER) is not recognised. Please specify 1.5, 1.7 or 2
            error($$message)
        }

	# Basic libraries
	qtopiadesktop {
	    CONFIG += desktop
	    !contains(TARGET,qdbase) {
		LIBS          = -lqd-qdbase $$LIBS
		win32:DEFINES += QDBASE_DLL
	    }
	    qtopialib:LIBS+=-lqd-qpe
	    pimlib {
		win32:DEFINES+=QTOPIAPIM_DLL
		LIBS+=-lqd-qpepim
	    }
	} else {
            pimlib:equals(QTOPIA_COMPAT_VER,2):LIBS+=-lqpepim1
	    pimlib:LIBS+=-lqpepim
	    qtopialib {
		QTOPIA_PHONE:LIBS+=-lqtopiaphone
		equals(QTOPIA_COMPAT_VER,2):LIBS+=-lqtopia2
		equals(QTOPIA_COMPAT_VER,2)|equals(QTOPIA_COMPAT_VER,1.7):LIBS+=-lqtopia
		LIBS+=-lqpe
	    }
	    qssaudio:DEFINES+=QSSAUDIO
            QTOPIA_VOIP:DEFINES+=QTOPIA_VOIP
	}
	# Hardcoded settings
	CONFIG+=link_prl create_prl depend_prl
	qtest:INCLUDEPATH+=$$(QTESTDIR)/qtestlib
	INCLUDEPATH+=$$(QPEDIR)/include
	DEPENDPATH+=$$(QPEDIR)/include
	LIBS+=-L$$(QPEDIR)/lib
	!qte_depot:!isEmpty(QTEDIR):LIBS+=-L$${QTEDIR}/lib

        buildSingleexec:!qtopia {
            # libqte depends on jpeg so it must appear before it but qmake doesn't want
            # us to do this for some reason. Just add it in ourselves. It means that -lqte
            # will appear twice in the link line but that's ok.
            qt {
                qte=qte
                thread:qte=qte-mt
                LIBS+=-l$$qte
            }
            LIBS+=-ljpeg -lpthread -ldl
            # -lopenobex when obex is compiled in!
        }

	# ID of this project
	TMPQPEDIR=$$(QPEDIR)
	QTOPIA_ID=$$(PWD)
	win32 {
	    TMPQPEDIR~=s,\\,/,g
	    QTOPIA_ID=$${PWD}
	}
	SRCDIR=$${QTOPIA_ID}
        QTOPIA_PKG_ID=$$QTOPIA_ID
	depotProject {
	    QTOPIA_ID~=s,$${TMPQPEDIR}/src/,,q
	    QTOPIA_ID~=s,$${TMPQPEDIR}src/,,q
	    QTOPIA_PKG_ID=$$QTOPIA_ID
	    equals(QTOPIA_ID, $$SRCDIR) {
		# Not a Qtopia project, check if it's a theme
		THEME_QTOPIA_ID=$$QTOPIA_ID
		THEME_QTOPIA_ID~=s,$${TMPQPEDIR}/etc/themes/,,q
		THEME_QTOPIA_ID~=s,$${TMPQPEDIR}etc/themes/,,q
		contains(THEME_PROJECTS, $$THEME_QTOPIA_ID) {
		    # It is a theme
		    QTOPIA_ID=$$THEME_QTOPIA_ID
		    QTOPIA_PKG_ID=../etc/themes/$${QTOPIA_ID}
		}
	    }
	    SRCDIR=$${QTOPIA_DEPOT_PATH}/src/$${QTOPIA_PKG_ID}
	    !desktop {
		# Qtopia projects must be included in ALL_PROJECTS
		requires(contains(ALL_PROJECTS,$${QTOPIA_ID}))
		# Qtopia projects all have the same version
		isEmpty(VERSION):VERSION=$$QPE_VERSION
	    }
	    # Qtopia Desktop doesn't do versions
	    qtopiadesktop:VERSION=

	    # Check the list of apps to quicklaunch
	    !buildStaticLib:buildQuicklaunch {
		QUICKLAUNCH_LIST = $${APP_PROJECTS}

		# Feel free to add more masks to maintain clarity of the regex
		# Use '.' instead of '/' or qmake will complain
		QUICKLAUNCH_MASK = .*(games|settings).*
		QUICKLAUNCH_LIST ~= s/$${QUICKLAUNCH_MASK}//g

		!contains(QUICKLAUNCH_LIST,$${QTOPIA_ID}):CONFIG-=buildQuicklaunch
	    }

        }

        depotProject:IPK_ROOT=$$(QPEDIR)
        else:IPK_ROOT=$$QTOPIA_ID
        IPK_PATH=$$IPK_ROOT/ipkg
        RPM_PATH=$$IPK_ROOT/rpm

	# Configuration specific settings
	buildSingleexec {
	    DEFINES+=SINGLE_EXEC
	    buildQuicklaunch:DEFINES+=QTOPIA_INTERNAL_INITAPP
	}

	!qtopiadesktop {
	    QTOPIA_PHONE:DEFINES+=QTOPIA_PHONE
	    QTOPIA_DATA_LINKING:DEFINES+=QTOPIA_DATA_LINKING
	}

	# Hacks
	isEmpty(TARGET):error(Please set a target in $$(PWD))
	OLD_TARGET=$${TARGET}
	QMAKE_LIBDIR_QT=
	# Guess some needed values
	isEmpty(QTOPIA_PROJECT_TYPE) {
	    QTOPIA_SPLIT_ID=$$split(QTOPIA_ID,/)
	    QTOPIA_PROJECT_TYPE=$$member(QTOPIA_SPLIT_ID,-2)
	}

	# Ipk groups
	include ($${QTOPIA_DEPOT_PATH}/src/ipk_groups.pri)

	# Default values for ipkgs
        isEmpty(PACKAGE_NAME):PACKAGE_NAME=qpe-$${OLD_TARGET}
        isEmpty(PACKAGE_DESCRIPTION):PACKAGE_DESCRIPTION=No description written for this package
        isEmpty(PACKAGE_VERSION):PACKAGE_VERSION=$$VERSION
        isEmpty(PACKAGE_MAINTAINER) {
            depotProject:qtopia_depot:PACKAGE_MAINTAINER=Trolltech (www.trolltech.com)
            else:PACKAGE_MAINTAINER=Unspecified
        }
        isEmpty(PACKAGE_LICENSE) {
            free_package:PACKAGE_LICENSE=GPL
            else:PACKAGE_LICENSE=Unspecified
        }
	isEmpty(IPK_GROUP_PROJECTS):IPK_GROUP_PROJECTS=$${QTOPIA_PKG_ID} # Default to one package
	# End ipk groups

	qtopiadesktop {
	    CONFIG+=nocompat
	    DEFINES+=QTOPIA_DESKTOP
	    # This enables some legacy code for Qtopia Desktop apps that use the PIM library
	    contains(LIBS,-lqd-qpepim):DEFINES+=PALMTOPCENTER

	    plugin {
		TEMPLATE=lib
		win32:DEFINES+=QTOPIADESKTOP_DLL QTOPIA_DLL
		LIBS+=-lqd-common -lqd-qpe
		win32:target.path =/plugins
		!win32:target.path=/qtopiadesktop/plugins
		INSTALLS+=target
	    }

	    INCLUDEPATH=$$(QPEDIR)/src/qtopiadesktop/include $$(QPEDIR)/src/qtopiadesktop/include/common $${INCLUDEPATH}
	    DEPENDPATH=$$(QPEDIR)/src/qtopiadesktop/include $$(QPEDIR)/src/qtopiadesktop/include/common $${INCLUDEPATH}
	}
	desktop {
	    requires(!isEmpty(DQTDIR))
	    CONFIG+=thread
	    INCLUDEPATH=$${DQTDIR}/include $${INCLUDEPATH}
	    DEPENDPATH=$${DQTDIR}/include $${INCLUDEPATH}

	    win32:QMAKE_MOC=$${DQTDIR}\bin\moc.exe
	    else:QMAKE_MOC=$${DQTDIR}/bin/moc
	    win32:QMAKE_UIC=$${DQTDIR}\bin\uic.exe
	    else:QMAKE_UIC=$${DQTDIR}/bin/uic
	    QMAKE_INCDIR_QT=$${DQTDIR}/include
	    QMAKE_LIBDIR_QT=$${DQTDIR}/lib
	} else {
	    CONFIG+=no_install_prl
	    DEFINES+=$${QCONFIGDEFINE}

	    win32:QMAKE_MOC=$${QTEDIR}\bin\moc.exe
	    else:QMAKE_MOC=$${QTEDIR}/bin/moc
	    win32:QMAKE_UIC=$${QTEDIR}\bin\uic.exe
	    else:QMAKE_UIC=$${QTEDIR}/bin/uic
	    QMAKE_INCDIR_QT=$${QTEDIR}/include
	}

	qtopiaplugin {
	    TEMPLATE=lib
	    PACKAGE_DEPENDS*=qpe-libqtopia2
	    buildSingleexec {
		CONFIG+=staticlib
		TARGET=plugin_$${QTOPIA_PROJECT_TYPE}_$${TARGET}
		DESTDIR=$$(QPEDIR)/lib
		DEFINES+=QTOPIA_PLUGIN_TYPE=\"$${QTOPIA_PROJECT_TYPE}\"
		DEFINES+=QTOPIA_PLUGIN_NAME=\"$${OLD_TARGET}\"
		S_S=$${QTOPIA_DEPOT_PATH}/bin/seserver_gen -f libs -d $${QTOPIA_DEPOT_PATH} -s $${QTOPIA_ID} -t $${TARGET}
		contains(ALL_PROJECTS,$${QTOPIA_ID}) {
		    system( $${S_S} -i )
		} else {
		    system( $${S_S} )
		}
	    } else {
		!qtopiadesktop {
		    target.path=/plugins/$${QTOPIA_PROJECT_TYPE}
		}
		INSTALLS+=target
	    }
	}

	qtopiaapp {
	    PACKAGE_DEPENDS*=qpe-libqtopia2
	    # This is a hack to allow Qtopia apps' .pri files to be used (datebook, addressbook, todolist)
	    qtopiadesktop {
		SOURCES-=main.cpp
	    } else {
		buildQuicklaunch {
		    TEMPLATE=lib
		    target.path=/plugins/application
		    !static:INSTALLS+=target
		    CONFIG+=plugin
		    DEFINES+=QTOPIA_APP_INTERFACE
		    instSymlink.path=/bin
		    instSymlink.commands=$${COMMAND_HEADER}\
                        rm -f "$(INSTALL_ROOT)$${instSymlink.path}/$${TARGET}";\
                        ln -s quicklauncher "$(INSTALL_ROOT)$${instSymlink.path}/$${TARGET}"
		    INSTALLS+=instSymlink
		} else {
		    exists($$(QPEDIR)/plugins/application/lib$${TARGET}.so):\
			system(rm $$(QPEDIR)/plugins/application/lib$${TARGET}.so)

		    buildSingleexec {
			SOURCES-=main.cpp
			TEMPLATE=lib
			CONFIG+=staticlib
			instSymlink.path=/bin
			instSymlink.commands=$${COMMAND_HEADER}ln -sf qpe "$(INSTALL_ROOT)$${instSymlink.path}/$${TARGET}"
			INSTALLS+=instSymlink

			TARGET=app_$${TARGET}
			DESTDIR=$$(QPEDIR)/lib
			# server LIBS+=
			S_S=$${QTOPIA_DEPOT_PATH}/bin/seserver_gen -f libs -d $${QTOPIA_DEPOT_PATH} -s $${QTOPIA_ID} -t $${TARGET}
			contains(ALL_PROJECTS,$${QTOPIA_ID}) {
			    system( $${S_S} -i )
			} else {
			    system( $${S_S} )
			}
			# server INCLUDEPATH+=
			S_S=$${QTOPIA_DEPOT_PATH}/bin/seserver_gen -f includes -u $${UI_HEADERS_DIR} -s $${QTOPIA_ID}
			contains(ALL_PROJECTS,$${QTOPIA_ID}) {
			    system( $${S_S} -i )
			} else {
			    system( $${S_S} )
			}
			S_S=$${QTOPIA_DEPOT_PATH}/bin/seserver_gen -f apps -d $${QTOPIA_DEPOT_PATH} -s $${QTOPIA_ID}
			contains(ALL_PROJECTS,$${QTOPIA_ID}) {
			    system( $${S_S} -i )
			} else {
			    system( $${S_S} )
			}
		    } else {
			TEMPLATE=app
			target.path=/bin
			INSTALLS+=target
		    }
		}
	    }
	}

	# Libs and plugins built for qtopiadesktop should have qd- prepended to them
	qtopiadesktop {
	    !equals(TARGET, qtopiadesktop):!equals(TEMPLATE, app):TARGET=qd-$${TARGET}
	    OLD_TARGET=$${TARGET}
	}

	contains(TEMPLATE,lib) {
	    win32 {
		CONFIG+=dll
		DEFINES+=EXPORT_$${TARGET}=__decl(dllexport)
	    }
	    buildStaticlib {
		CONFIG+=staticlib
		S_S=$${QTOPIA_DEPOT_PATH}/bin/seserver_gen -f libs -d $$QTOPIA_DEPOT_PATH -s $${QTOPIA_ID} -t $${TARGET}
		contains(ALL_PROJECTS,$${QTOPIA_ID}) {
		    system( $${S_S} -i )
		} else {
		    system( $${S_S} )
		}
	    }
	    !staticlib:isEmpty(target.path) {
		win32:target.path=/
		!win32:target.path=/lib
		INSTALLS+=target
	    }
	}

	# Update/release translations
        qtopiadesktop {
	    !contains(TEMPLATE,lib) {
	        TRTARGET=$${OLD_TARGET}
            } else {
	        TRTARGET=lib$${OLD_TARGET}
	    }
        } else {
	    !contains(TEMPLATE,lib)|contains(CONFIG,qtopiaapp) {
	        TRTARGET=$${OLD_TARGET}
            } else {
	        TRTARGET=lib$${OLD_TARGET}
	    }
        }
	DEFINES+=QTOPIA_TARGET=\"$$TARGET\"
	DEFINES+=QTOPIA_TRTARGET=\"$$TRTARGET\"
	OPTQTOPIA=$(INSTALL_ROOT)
	!win32:qtopiadesktop:OPTQTOPIA=$$OPTQTOPIA/qtopiadesktop
	LINSTALL_TEMPLATE=$${COMMAND_HEADER}\
	    $${QTOPIA_DEPOT_PATH}/bin/linstall "TRTARGET" "$${TRANSLATIONS}" "PDATARGETS" "$${DQTDIR}"\
		"$$OPTQTOPIA" "$${SRCDIR}"
	linstall.commands=$${LINSTALL_TEMPLATE}
	linstall.commands~=s,TRTARGET,$${TRTARGET},gq
	linstall.commands~=s,PDATARGETS,$${PDATARGETS},gq
	win32:linstall.commands~=s,/,\\,g
	linstall.CONFIG=no_path
	isEmpty(DQTDIR):linstall.commands=$${COMMAND_HEADER}echo "Can't install translations because Qt 3 is not available"
	!isEmpty(TRANSLATIONS):INSTALLS+=linstall
	!isEmpty(lupdate.command_override) {
	    # some pro files need to define lupdate.commands
	    # differently.  Such as src/qt/qt.pro
	    # if it is already defined, don't override.
	    lupdate.commands=$${COMMAND_HEADER}\
		cd $${SRCDIR};\
		$${lupdate.command_override}
	} else {
	    lupdate.commands=$${COMMAND_HEADER}\
		cd $${SRCDIR};\
                TRANSFILES=;\
                TRANSLATABLES="$(TRANSLATABLES)"; for transfile in \$$TRANSLATABLES; do\
                    [ -f \$$transfile ] && TRANSFILES="\$$TRANSFILES \$$transfile";\
                done;\
		TRANSLATIONS="$$TRANSLATIONS"; for lang in \$$TRANSLATIONS; do\
		    $${DQTDIR}/bin/lupdate \$$TRANSFILES -ts $${TRTARGET}-\$$lang.ts;\
		done
	}

	# Special translation install
	!win32:!isEmpty(NON_CODE_TRANSLATABLES):!isEmpty(NON_CODE_TRTARGETS) {
	    nct_lupdate.commands=$${COMMAND_HEADER}\
		cd $${SRCDIR};\
                TRANSFILES=;\
                NON_CODE_TRANSLATABLES="$${NON_CODE_TRANSLATABLES}"; for transfile in \$${NON_CODE_TRANSLATABLES}; do\
                    [ -f \$$transfile ] && TRANSFILES="\$$TRANSFILES \$$transfile";\
                done;\
		$${QTOPIA_DEPOT_PATH}/bin/nct_lupdate "$${TRANSLATIONS}" \$$TRANSFILES
	    QMAKE_EXTRA_UNIX_TARGETS+=nct_lupdate
	    lupdate.depends+=nct_lupdate
	    contains(INSTALLS, linstall) {
		nct_linstall.commands=$${LINSTALL_TEMPLATE}
		nct_linstall.commands~=s,TRTARGET,$$NON_CODE_TRTARGETS,gq
		nct_linstall.commands~=s,PDATARGETS,,gq
		nct_linstall.CONFIG=no_path
		isEmpty(DQTDIR):nct_linstall.commands=$${COMMAND_HEADER}echo "Can't install translations because Qt 3 is not available"
		INSTALLS+=nct_linstall
	    }
	}

	# Packaging
	qtopiadesktop {
	    packages.commands=$${COMMAND_HEADER}echo "Not building packages for QtopiaDesktop"
	} else {
	    # Ipkg
	    DEB_VERSION=2.0
	    WORKING_PATH=$$IPK_ROOT/.packaging-working-dir/ipk/$${PACKAGE_NAME}
	    CONTROL_PATH=$${WORKING_PATH}/control
	    CONTROL_FILE=$${CONTROL_PATH}/control
	    DATA_PATH=$${WORKING_PATH}/data
	    QTOPIA_PATH=opt/Qtopia

	    # Generates dependancy data for this project and installs files to DATA_PATH
	    unix_mkipk_deps.depends=install
	    unix_mkipk_deps.commands=$${COMMAND_HEADER}\
		mkdir -p $${DATA_PATH};\
		mkdir -p $${CONTROL_PATH};\
		echo "$${PACKAGE_DEPENDS}" | sed 's/\( [^(]\)/,\1/g' >> $${CONTROL_PATH}/DEPS

	    TAR_OPTIONS=--remove-files -cz --owner=0 --group=0
	    unix_mkipk.commands=$${COMMAND_HEADER}\
		if [ -e $${DATA_PATH} ]; then\
		    echo "ERROR: $${DATA_PATH} exists!";\
		    exit 1;\
		fi;\
		if [ -e $${IPK_PATH}/$${PACKAGE_NAME}_$${PACKAGE_VERSION}_$${QTOPIA_ARCH}.ipk ]; then\
		    echo "Package $${PACKAGE_NAME}_$${PACKAGE_VERSION}_$${QTOPIA_ARCH}.ipk already exists!";\
		    exit 0;\
		fi;\
		IPK_GROUP_PROJECTS="$$IPK_GROUP_PROJECTS"; for i in \$${IPK_GROUP_PROJECTS};do
            depotProject:unix_mkipk.commands+=\
		    make -C $$(QPEDIR)/src/\$$i -f Makefile.target
            else:unix_mkipk.commands+=\
		    make -C \$$i -f Makefile
            unix_mkipk.commands+=\
                        unix_mkipk_deps INSTALL_ROOT=$${DATA_PATH}/$${QTOPIA_PATH} || exit \$$?;\
		done;\
		# This is now a function since it gets called many times
		make_ipk_file()\
		{\
		    cd $${DATA_PATH};\
		    if [ ! -z "\$$1" ]; then\
			lang="\$$1";\
			suffix="_i18n_\$$lang";\
			deps="$${PACKAGE_NAME}";\
			# Global translations
			if [ "`echo $${PACKAGE_NAME} | cut -c -4`" = "qpe-" -a "$${PACKAGE_NAME}" != "qpe-taskbar" ]; then\
			    deps="\$$deps qpe-taskbar_i18n_\$$lang";\
			fi;\
			path="$${QTOPIA_PATH}/i18n/\$$lang";\
			if [ ! -d "$${DATA_PATH}/\$$path" ]; then\
			    return;\
			fi;\
		    else\
			suffix=;\
			deps=`cat $${CONTROL_PATH}/DEPS`; \
			deps=`echo \$$deps | sort -u`; \
			path="$${QTOPIA_PATH}";\
		    fi;\
		    echo "Package: $${PACKAGE_NAME}\$$suffix" > $${CONTROL_FILE};\
		    echo "Maintainer: $$PACKAGE_MAINTAINER" >> $${CONTROL_FILE};\
		    echo "License: $$PACKAGE_LICENSE" >> $${CONTROL_FILE};\
		    echo "Version: $${PACKAGE_VERSION}" >> $${CONTROL_FILE};\
		    echo "Arch: $${QTOPIA_ARCH}" >> $${CONTROL_FILE};\
		    echo "Architecture: $${QTOPIA_ARCHITECTURE}" >> $${CONTROL_FILE};\
                    echo "Description: $$PACKAGE_DESCRIPTION" >> $$CONTROL_FILE;\
		    echo -n "Depends: " >> $${CONTROL_FILE};\
		    echo \$$deps >> $${CONTROL_FILE};\
		    echo -n "Files:" >> $${CONTROL_FILE};\
		    find \$$path -type f -o -type l -printf "/%P " >> $${CONTROL_FILE};\
		    if [ ! -x "`which mkfs.jffs2`" ]; then\
			echo -e "\nInstalled-Size: \$$(du -h -s \$$path | awk '{print \$$1}')" >> $${CONTROL_FILE};\
		    else\
			echo -e "\nInstalled-Size: \$$(mkfs.jffs2 -r \$$path | wc -c)" >> $${CONTROL_FILE};\
		    fi;\
		    echo >> $${CONTROL_FILE};\
		    for i in preinst prerm postinst postrm;do \
			export FILE_LOC=$${SRCDIR}/$${PACKAGE_NAME}.\$$i;\
			if [ -e \$$FILE_LOC ]; then install -c \$$FILE_LOC $${WORKING_PATH}/\$$i; fi;\
		    done;\
		    cd $${WORKING_PATH};\
		    echo "$${DEB_VERSION}" > debian-binary;\
		    cd $${CONTROL_PATH};\
		    tar $${TAR_OPTIONS} -f $${WORKING_PATH}/control.tar.gz ./control;\
		    cd $${DATA_PATH};\
		    tar $${TAR_OPTIONS} -f $${WORKING_PATH}/data.tar.gz \$$path;\
		    if [ ! -e $${IPK_PATH} ];then mkdir -p $${IPK_PATH};fi;\
		    cd $${WORKING_PATH};\
		    tar $${TAR_OPTIONS} -f $${IPK_PATH}/$${PACKAGE_NAME}_$${PACKAGE_VERSION}_$${QTOPIA_ARCH}\$$suffix.ipk \
			./debian-binary ./control.tar.gz ./data.tar.gz \
			`for i in preinst prerm postinst postrm;do \
			    if [ -e \$$i ]; then echo \$$i; fi;\
			done`;\
		};\
		# Create packages for the languages that depend on the package being created now
		LANGUAGES="$$LANGUAGES"; for lang in \$${LANGUAGES}; do\
		    make_ipk_file \$$lang;\
		done;\
		# remove i18n stuff so it doesn't get in the regular package
		[ -d $${DATA_PATH}/$${QTOPIA_PATH}/i18n ] && rm -r $${DATA_PATH}/$${QTOPIA_PATH}/i18n;\
		make_ipk_file;\
		rm -rf $${WORKING_PATH}

	    QMAKE_EXTRA_UNIX_TARGETS+=unix_mkipk unix_mkipk_deps
	    packages.depends=unix_mkipk
	}

	nocompat:DEFINES+=QPE_NO_COMPAT

	# Provide a dummy install_target rule if there is none already
	!contains(INSTALLS, target) {
	    install_target.commands=@true
	    QMAKE_EXTRA_UNIX_TARGETS+=install_target
	}

        !win32 {
            # remove_target removes the TARGET file
            contains(TEMPLATE,app):tg=$(TARGET)
            else:tg=$(DESTDIR)$(TARGET)
            remove_target.commands=-$(DEL_FILE) $$tg
            QMAKE_EXTRA_UNIX_TARGETS+=remove_target

            # relink removes the target then runs make (to link it again)
            relink.commands=$$COMMAND_HEADER\
                make
            relink.depends=remove_target
        }

	###
	### Place any "normal" stuff above this section.
	###
	### This place is reserved for behaviour-altering hacks.
	###

	# These variables are used by the "special" installers
	win32:COMMAND_SEPARATOR=&
	!win32:COMMAND_SEPARATOR=;

	# Pictures need to be handled specially because they have icons which must be generated
	!isEmpty(PICS_INSTALLS) {
	    auto_pics.commands=$$PICS_INSTALLS
	    auto_pics.commands~=s,\b(.*)\b,\
		$${QTOPIA_DEPOT_PATH}/bin/installpic "$${LANGUAGES}" $${QTOPIA_ICON_SIZES}\
		    $(INSTALL_ROOT)$${\1.path} $${\1.files} $${COMMAND_SEPARATOR}\
		,g
	    auto_pics.commands=$${COMMAND_HEADER}$${auto_pics.commands}
	    auto_pics.CONFIG=no_path
	    win32 {
		auto_pics.commands~=s,/,\,g
		auto_pics.commands+=echo.
	    }
	    INSTALLS+=auto_pics
	}

	# Help files need to be handled specially because there may be multiple languages present
	!isEmpty(HELP_INSTALLS) {
	    auto_help.commands=$$HELP_INSTALLS
	    auto_help.commands~=s,\b(.*)\b,\
		$${QTOPIA_DEPOT_PATH}/bin/installhelp "$${TRANSLATIONS}" \
		    $${\1.source} $(INSTALL_ROOT)/help "$${\1.files}" $${COMMAND_SEPARATOR}\
		,g
	    auto_help.commands=$${COMMAND_HEADER}$${auto_help.commands}
	    auto_help.CONFIG=no_path
	    win32 {
		auto_help.commands~=s,/,\,g
		auto_help.commands+=echo.
	    }
	    INSTALLS+=auto_help
	}

    } else {

	GENERIC_SUBDIRS_COMMAND=$${COMMAND_HEADER}\
	    SUBDIRS="$$SUBDIRS"; for i in \$${SUBDIRS}; do\
		if [ -e \$$i/Makefile ]; then\
		    (\
			cd \$$i;\
			$(MAKE) $@ || exit \$$?;\
		    )\
		fi;\
	    done;\
	    COMPONENT_MKFILE=Makefile.`echo \$$i | sed -e 's,\.pro$$,,'`;\
	    if [ -e "\$$COMPONENT_MKFILE" ]; then\
		$(MAKE) -f \$$COMPONENT_MKFILE $@ || exit \$$?;\
	    fi

	lupdate.commands=$$GENERIC_SUBDIRS_COMMAND
	packages.commands=$$GENERIC_SUBDIRS_COMMAND
	sdk.commands=$$GENERIC_SUBDIRS_COMMAND
	devsdk.commands=$$GENERIC_SUBDIRS_COMMAND
	relink.commands=$$GENERIC_SUBDIRS_COMMAND

	# install_target for subdirs .pro files
	!contains(INSTALLS, target) {
	    install_target.commands=$$GENERIC_SUBDIRS_COMMAND
	    QMAKE_EXTRA_UNIX_TARGETS+=install_target
	}

    }

    sdk.depends+=install
 
 #    the devsdk package is a 'small' package that _only_ contains source files, so disable the dependency to 'sdk'
 #    devsdk.depends+=sdk

    QMAKE_EXTRA_UNIX_TARGETS+=packages sdk devsdk relink
    !isEmpty(DQTDIR):QMAKE_EXTRA_UNIX_TARGETS+=lupdate
}

