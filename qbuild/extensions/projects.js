/****************************************************************************
**
** This file is part of the Qt Extended Opensource Package.
**
** Copyright (C) 2008 Trolltech ASA.
**
** Contact: Qt Extended Information (info@qtextended.org)
**
** This file may be used under the terms of the GNU General Public License
** version 2.0 as published by the Free Software Foundation and appearing
** in the file LICENSE.GPL included in the packaging of this file.
**
** Please review the following information to ensure GNU General Public
** Licensing requirements will be met:
**     http://www.fsf.org/licensing/licenses/info/GPLv2.html.
**
**
****************************************************************************/

/*!

\extension projects

This extension handles the PROJECTS variable.

*/

function projects_init()
{
    // Populate PROJECTS
    if ( project.name.match(/projects\.pri/) ) {
        var pi = project.pathIterator().cd("/src");
        // We do this in reverse, loading the source tree before device profile overrides
        var paths = pi.files("projects.pri").reverse();
        for ( var ii in paths ) {
            qbuild.invoke("include", paths[ii].filesystemPath());
        }
    } else {
        var root = project.sproject("/extensions/projects.pri").fileModeProject();
        var projects = project.property("PROJECTS");
        var themes = project.property("THEMES");
        if ( root.isProperty("PROJECTS") )
            projects.setValue(root.property("PROJECTS").value());
        if ( root.isProperty("THEMES") )
            themes.setValue(root.property("THEMES").value());
    }
}

function projects_include_modules()
{
    var modules = project.property("QTOPIA_MODULES").value();
    for ( var ii in modules ) {
        var module = modules[ii];
### module
        include(module_<0>.pri)
###
    }
}

function projects_expected()
{
    if ( project.absName.match(/^\/src\//) ) {
        var check = project.absName.replace(/^\/src\//, "").replace(/\/$/, "")
        if ( project.property("PROJECTS").contains(check) )
            return true;
    }
    if ( project.absName.match(/^\/examples\//) ) {
        var check = ".."+project.absName;
        if ( project.property("PROJECTS").contains(check) )
            return true;
    }
    if ( project.absName.match(/^\/etc\/themes\//) ) {
        var check = project.absName.replace(/^\/etc\/themes\//, "").replace(/\/$/, "")
        if ( project.property("THEMES").contains(check) )
            return true;
    }
    return false;
}

