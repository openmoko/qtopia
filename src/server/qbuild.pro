TEMPLATE=app
CONFIG+=qtopia
TARGET=qpe

# Don't build our sub-projects
CONFIG-=subdirs

SOURCEPATH+=main phone
include(main/main.pri)

# This is documented in src/build/doc/src/deviceprofiles.qdoc
!isEmpty(device) {
    # If server.pri exists, pull it in
    exists($$DEVICE_CONFIG_PATH/server/server.pri) {
        SOURCEPATH+=$$DEVICE_CONFIG_PATH/server
        include($$DEVICE_CONFIG_PATH/server/server.pri)
    } else {
        pull_in_all=1
    }
}
<script>
if ( project.isProperty("device") && project.property("pull_in_all").strValue() == "1" ) {
    var forms = project.files("/devices/"+project.property("device").strValue()+"/server/*.ui");
    var headers = project.files("/devices/"+project.property("device").strValue()+"/server/*.h");
    var sources = project.files("/devices/"+project.property("device").strValue()+"/server/*.cpp");
    if ( headers.length || sources.length || forms.length ) {
        project.info("Pulling in server/*.ui, server/*.h and server/*.cpp is deprecated. You should provide a server.pri file instead.");
        for(var ii in forms)
            project.property("FORMS").append(forms[ii].path());
        for(var ii in headers)
            project.property("HEADERS").append(headers[ii].path());
        for(var ii in sources)
            project.property("SOURCES").append(sources[ii].path());
        project.property("INCLUDEPATH").append("/devices/"+project.property("device").strValue()+"/server");
    }
}
</script>

# Pull in server projects
CONFIG+=qbuild_server_deps
<script>
var projects = project.property("PROJECTS").value();
for ( var ii in projects ) {
    var proj = projects[ii];
    if ( proj.match(/^server\/./) ) {
        //project.message(proj);
        var path = "/src/"+proj;
        proj = path+"/qbuild.pro";
        project.property("VPATH").append(path);
        qbuild.invoke("include", proj);
    }
}
var depends = project.property("DEPENDS");
var list = depends.value();
depends.setValue("");
var bad = new Array;
for ( var ii in list ) {
    if ( list[ii].match(/^\/src\/server\/./) ) {
        var proj = list[ii].replace(/^\/src\//, "").replace(/::.*/, "");
        if ( !project.property("PROJECTS").contains(proj) )
            bad.push(proj);
    } else {
        depends.append(list[ii]);
    }
}
if ( bad.length )
    qbuild.invoke("disable_project", "Required server projects are not enabled ("+bad.join(", ")+").");
</script>

NON_PUBLIC_HEADERS+=$$HEADERS
PUBLIC_HEADERS_OVERRIDE+=$$HEADERS
HEADERS=
HEADERS_NAME=server
CONFIG+=headers

STRING_LANGUAGE=en_US
AVAILABLE_LANGUAGES=$$QTOPIA_AVAILABLE_LANGUAGES
LANGUAGES=$$QTOPIA_LANGUAGES
TS_DIR=$$path(main,project)


nct_lupdate [
    TYPE=RULE
    commands="#(ve)[ ""$VERBOSE_SHELL"" = 1 ] && set -x
        QTOPIA_DEPOT_PATH="""$$path(/,project)"""
        QPEDIR="""$$path(/,generated)"""
        TS_DIR="""$$TS_DIR"""
        AVAILABLE_LANGUAGES="""$$AVAILABLE_LANGUAGES"""
        STRING_LANGUAGE="""$$STRING_LANGUAGE"""
        cd $TS_DIR
        find $QTOPIA_DEPOT_PATH\
            \( \( -path ""$QTOPIA_DEPOT_PATH/apps/*"" -o\
                  -path ""$QTOPIA_DEPOT_PATH/plugins/*"" -o\
                  -path ""$QTOPIA_DEPOT_PATH/i18n/*"" \) -a\
               \( -name "".directory"" -o -name ""*.desktop"" \) \) -o\
            \( \( -path ""$QTOPIA_DEPOT_PATH/etc/*"" -o\
                  -path ""$QTOPIA_DEPOT_PATH/ui/*"" \) -a\
               \( -name ""*.conf"" -o -name ""*.scheme"" -o -name ""*.desktop"" \) \) -o\
            \( -path ""$QTOPIA_DEPOT_PATH/etc/qds/*"" \) -o\
            \( \( -path ""$QTOPIA_DEPOT_PATH/pics/*"" \) -a\
               \( -name ""config"" \) \) -o\
            \( -path ""$QTOPIA_DEPOT_PATH/services/*"" \) |\
        $QPEDIR/src/build/bin/nct_lupdate\
            -nowarn\
            -depot\
            ""$QTOPIA_DEPOT_PATH""\
            ""$AVAILABLE_LANGUAGES""\
            ""$STRING_LANGUAGE""\
            -
    "
]
i18n_depend_on_qt(nct_lupdate)

lupdate.TYPE=RULE
lupdate.prerequisiteActions+=nct_lupdate

# Install .directory files
for(l,LANGUAGES) {
    file=$$path(/i18n/$$l/.directory,existing)
    isEmpty(file):warning("Cannot locate /i18n/"$$l"/.directory")
    directory.commands+=\
        "#(e)$$MKSPEC.MKDIR $$QTOPIA_IMAGE/i18n/"$$l\
        "$$MKSPEC.INSTALL_FILE "$$file" $$QTOPIA_IMAGE/i18n/"$$l
}
directory.hint=image

# Install dictionaries
qdawggen=$$path(QtopiaSdk:/bin/qdawggen,generated)
!equals(QTOPIA_HOST_ENDIAN,$$QTOPIA_TARGET_ENDIAN):qdawggen+=-e
commands=$$COMMAND_HEADER
for(lang,LANGUAGES) {
    # `words' must exist... any other files are done too
    exists(/etc/dict/$$lang/words) {
        dicts.commands+=\
            "#(e)$$MKSPEC.MKDIR $$QTOPIA_IMAGE/etc/dict/"$$lang\
            "#(e)find $$path(/etc/dict/"$$lang",project) -maxdepth 1 -type f | xargs -r -t $$qdawggen $$QTOPIA_IMAGE/etc/dict/"$$lang
    }
}
dicts.depends=/src/tools/qdawggen/target
dicts.hint=image

enable_singleexec {
    DEFINES+=SINGLE_EXEC
    <script>
    var projects = project.property("PROJECTS").value();
    for ( var ii in projects ) {
        var proj = projects[ii];
        if ( proj != "server" && !proj.match(/^server\/./) ) {
            var path = "/src/"+proj;
            proj = project.sproject(path).project();
            if (proj.config("enable_singleexec") && proj.config("singleexec")) {
                project.property("DEPENDS").unite(path);
            }
        }
    }
    </script>

    patchqt [
        commands=\
            "$$path(/src/build/bin/patchqt,generated) $$QTOPIA_IMAGE/bin/qpe $$QTOPIA_PREFIX"
        depends=install_target
        hint=image
    ]
}

