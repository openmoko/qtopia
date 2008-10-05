!qbuild_server_deps{
TEMPLATE=lib
CONFIG+=qtopia staticlib
TARGET=qpe_ui_components_firstuse


DEP.headers.TYPE=DEPENDS PERSISTED
DEP.headers.EVAL=\
    "INCLUDEPATH*="$$paths(.,project)
}
DEPENDS*=\
    /src/server/core_server::persisted\
    /src/settings/language::exists\
    /src/settings/systemtime::exists\

include(firstuse.pro)

enable_singleexec {
    <script>
        var rule = project.rule();
        rule.prerequisiteActions = "/src/settings/language/ui_languagesettingsbase.h";
        rule.prerequisiteActions.append("#(oh)ensure_uicdir");
        //rule.inputFiles = qbuild.invoke("path", "/src/settings/language/.uic/ui_languagesettingsbase.h", "generated");
        rule.outputFiles = qbuild.invoke("path", "/src/server/.uic/ui_languagesettingsbase.h", "generated");
        rule.commands = "cp $$path(/src/settings/language/.uic/ui_languagesettingsbase.h,generated) $$[OUTPUT.0.ABS]";
        project.rule("target_pre").prerequisiteActions.append(rule.name);
    </script>
}

