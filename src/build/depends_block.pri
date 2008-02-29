# The dependency block that is used for both direct and indirect dependencies
# include is used because a function would have issues with scope

for(dep,QTOPIA_DEPENDS) {
    containstext($$dep,*) {
        QTOPIA_DEPENDS-=$$dep
        files=$$files($$fixpath($$PROJECT_ROOT/$$dep))
        for(file,files) {
            file~=s,$$fixpath($$PROJECT_ROOT/),,q
            pro=$$file/$$tail($$file).pro
            # Make sure that the project exists
            # Don't let a project depend on itself
            exists($$PROJECT_ROOT/$$pro):!equals(s,$$file):QTOPIA_DEPENDS+=$$file
        }
        QTOPIA_DEPENDS+=$$files($$dep)
    }
}

for(it,QTOPIA_DEPENDS) {
    contains(PROCESSED_DEPS,$$it):next()
    PROCESSED_DEPS+=$$it
    echo($$self depends on $$it)
    found=0
    foundempty=0
    # Start looking in the current project root
    ROOTS=$$PROJECT_ROOT
    # Then look in Qtopia
    qtopia|part_of_qtopia:ROOTS+=$$QTOPIA_PROJECT_ROOT
    # Then Qtopia Sync Agent
    qtopiadesktop|part_of_qtopiadesktop:ROOTS+=$$QTOPIADESKTOP_PROJECT_ROOT
    # Then everything else
    ROOTS*=$$PROJECT_ROOTS
    for(root,ROOTS) {
        file=$$root/$$it/$$tail($$it).pro
        echo(Trying $$file)
        exists($$file) {
            profile=$$tail($$file)
            echo(Using $$file)
            found=1
            cmds=$$fromfile($$file,$$DEP_VAR)
            equals(DEP_VAR,DEP_CMDS):isEmpty(cmds) {
                found=0
                foundempty=1
                next()
            }
            for(c,cmds) {
                contains(QMAKE_BEHAVIORS,keep_quotes) {
                    c~=s/^"//
                    c~=s/"$//
                }
                # License info comes through with __SELF__ (because there's no way to check what
                # $$self would be from the other side) so we need to replace __SELF__ with $$profile
                c~=s/__SELF__/$$profile/qg
                echo(dep: $$c)
                eval($$c)
                # Handle any nowarn dependencies
                QTOPIA_DEPENDS-=$$QTOPIA_DEPENDS_NO_WARN
            }
            break()
        }
    }
    equals(found,0):!equals(foundempty,1):warning($$self depends on $$it/$$tail($$it).pro but it doesn't exist in $$ROOTS!)
}

