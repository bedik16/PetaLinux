SUMMARY = "RPMsg examples: Matrix Multiplication demo"


LICENSE = "BSD"
LIC_FILES_CHKSUM = "file://LICENSE;md5=b30cbe0b980e98bfd9759b1e6ba3d107"

SRC_URI = "\
	file://LICENSE \
	file://Makefile \
        file://httpServer.h \
        file://httpServer.c \
        file://httpServerRemoteFunc.h \
        file://httpServerRemoteFunc.c \
        file://proxy_app.c \
	"

S = "${WORKDIR}"

DEPENDS = "rpmsg-user-module"

FILES_${PN} = "\
	/usr/bin/proxy_app \
"

do_install () {
	install -d ${D}/usr/bin
	install -m 0755 proxy_app ${D}/usr/bin/proxy_app
}
