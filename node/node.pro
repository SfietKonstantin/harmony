TEMPLATE = aux

OTHER_FILES += package.json \
    public/css/styles.css \
    public/index.html \
    public/main.js \
    public/main-config.coffee \
    public/modules/home/views/home.html \
    public/modules/login/login.coffee \
    public/modules/login/views/login.html \
    public/modules/settings/settings.coffee \
    public/modules/settings/settings-debug.coffee \
    public/modules/settings/views/settings.html \
    public/modules/settings/views/settings-general.html \
    public/modules/settings/views/settings-debug.html \
    public/modules/test/test.coffee \
    public/modules/test/views/test.html

system($$PWD/build.sh)

WEBAPP_INSTALL_PREFIX = /usr/share/harbour-harmony/node
WEBAPP_FILES = node_modules \
    *.js \
    lib/*.js \
    public/*.js \
    public/index.html \
    public/css/* \
    public/lib/* \
    public/modules/login/*.js \
    public/modules/login/views/*.html \
    public/modules/settings/*.js \
    public/modules/settings/views/*.html \
    public/pages/*.html

for (webappFile, WEBAPP_FILES) {
    path = $${WEBAPP_INSTALL_PREFIX}/$${dirname(webappFile)}
    eval(webappFile_$${path}.files += $${webappFile})
    eval(webappFile_$${path}.path += $${path})
    eval(INSTALLS += webappFile_$${path})
}
